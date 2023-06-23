#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

#define STARTED 1
#define NOT_STARTED 0
#define INCREASE 2
#define DECREASE 0
// Default caps
#define CAPTURE_RATE_DEFAULT 48000
#define CAPTURE_CHANNELS_DEFAULT 2
#define CAPTURE_FORMAT_DEFAULT SND_PCM_FORMAT_S16
#define PLAYBACK_RATE_DEFAULT 48000
#define PLAYBACK_CHANNELS_DEFAULT 2
#define PLAYBACK_FORMAT_DEFAULT SND_PCM_FORMAT_S16

/* Playback variables */
static char *device; /* playback device */
static snd_pcm_format_t format = PLAYBACK_FORMAT_DEFAULT; /* sample format */
static unsigned int rate = PLAYBACK_RATE_DEFAULT; /* stream rate */
static unsigned int channels = PLAYBACK_CHANNELS_DEFAULT; /* count of channels */
static unsigned int buffer_time = 500000; /* ring buffer length in us */
static unsigned int period_time = 20000; /* period time in us */
static int verbose = 0; /* verbose flag */
static int resample = 1; /* enable alsa-lib resampling */
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
static snd_output_t *output = NULL;
static unsigned char* writeOutPtr = NULL;

/* Capture variables */
static char *device_cap; /* default capture device */
static snd_pcm_format_t format_cap = CAPTURE_FORMAT_DEFAULT; /* sample format for capture */
static unsigned int rate_cap = CAPTURE_RATE_DEFAULT; /* stream rate for capture */
static unsigned int channels_cap = CAPTURE_CHANNELS_DEFAULT; /* count of channels for capture*/
static unsigned int buffers_cap_no = 64; /* number of capture buffers */
static unsigned int buffer_cap_size;
static unsigned char* buffers_cap;
static unsigned char* readInPtr = NULL;
static unsigned long spaceLeftInBuffs = 0;
static int sampleResCap = 0;
static snd_pcm_sframes_t period_size_cap;

static unsigned long numFramesReady = 0;
pthread_cond_t eventVariableFilledBuffers = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex_num_frames_ready = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_protect_event_variable = PTHREAD_MUTEX_INITIALIZER;

static unsigned int is_started = NOT_STARTED;

static int xrun_recovery(snd_pcm_t *handle, int err);

void modify_num_frames_ready(int cmd, int val)
{
   // lock the mutex
   pthread_mutex_lock (&mutex_num_frames_ready);

   /* Modify how many frames are ready to go out;
    * do not allow to make it < 0 or bigger than the number of frames we can house in the buffers;
    */
   switch (cmd) {
      case INCREASE:
         numFramesReady = numFramesReady + val;
         if (numFramesReady > (buffers_cap_no * period_size_cap))
            numFramesReady = buffers_cap_no * period_size_cap;
         break;
      case DECREASE:
         if (val > numFramesReady)
            numFramesReady = 0;
         else
            numFramesReady = numFramesReady - val;
         break;
      default:
         //do nothing
         break;
   }

   // unlock the mutex
   pthread_mutex_unlock (&mutex_num_frames_ready);
}

// pthread function to read in from the driver to the application
void *read_in(void *_handle)
{
   snd_pcm_t* handle_cap = (snd_pcm_t*) _handle;
   int j, error, rc;

   readInPtr = buffers_cap;
   // space left in the buffers in frames
   spaceLeftInBuffs = buffers_cap_no * period_size_cap;

   //Read loop
   while(1)
   {
      // Sleep for 30ms so that we do not read data too often. If we read too often, we get a lot of -EAGAINs since data is not ready.
      // Note that sleep time should depend on DMA and capture rate: ((rate_in_bytes_per_sec * x) / 1000) = period_size_in_bytes
      // ... we should wait for at least x msec not to get -EAGAINs
      usleep(30000);
      // Read whatever the driver has got ready;
      // but no more than we can house in the buffers
      error = snd_pcm_readi (handle_cap, readInPtr, spaceLeftInBuffs);
      if (error == -EAGAIN)
         continue;
      else if (error < 0)
      {
         printf("readi error: (%d)\n", error);
         exit(1);
      }
      else
      {
         /* After successful readi(), increase the pointer and decrease how much space is left in the buffers */
         readInPtr = readInPtr + (error * sampleResCap * channels_cap);    // frames_read * bytes_per_sample * channels;
         spaceLeftInBuffs = spaceLeftInBuffs - error;

         /* Cyclic buffer: reset the pointer */
         if(spaceLeftInBuffs == 0)
         {
            readInPtr = buffers_cap;
            spaceLeftInBuffs = buffers_cap_no * period_size_cap;
         }

         /* Increment the number of frames ready to be written out to sound card; */
         modify_num_frames_ready(INCREASE, error);

         /*
          * We have filled some buffers; send out the news for whoever is interested;
          * This covers two cases:
          * (1) Initially we wait until the buffer is filled with at least (3 * period_size) of data before we start playback,
          * (2) or we want to wake up the thread which is waiting for data
          */
         // start playback when we have at least (3 * period_size) of data in the buffer
         // note that period size for playback and period size for capture are now set to be equal
         // it simplifies calculations for various clauses (e.g. the one beneath) and things become clearer in general
         //if((numFramesReady >= ((buffers_cap_no * period_size_cap)/2)) || (is_started == STARTED))
         if((numFramesReady >= (period_size_cap * 3)) || (is_started == STARTED))
         {
            is_started = STARTED;
            rc = pthread_cond_signal(&eventVariableFilledBuffers);
            if (rc != 0)
            {
               printf("read_in: signalling event of filled buffers failed: (%d)\n", rc);
               exit(EXIT_FAILURE);
            }
         }
      }
   }
}

// pthread function to write out to soundcard
void *write_out(void *_handle)
{
   snd_pcm_t* handle = (snd_pcm_t*) _handle;
   int cptr, err;

   writeOutPtr = buffers_cap;

   while(1)
   {
      /* Wait for the event to be signalled that buffers have been filled;
       * This covers two cases:
       * (1) Initially, when we start writing out, we want to wait for the buffers to get filled;
       * (2) Similarly, if it happens that we write_out faster than read_in, we want to wait;
       */
      pthread_cond_wait(&eventVariableFilledBuffers, &mutex_protect_event_variable);

      while(numFramesReady >= period_size)
      {
         cptr = period_size;
         while (cptr > 0)
         {
            err = snd_pcm_writei(handle, writeOutPtr, cptr);
            if (err == -EAGAIN)
               continue;
            if (err < 0)
            {
               printf("write_out error: %s\n", snd_strerror(err));
               if (xrun_recovery(handle, err) < 0)
               {
                  printf("write_out error: %s\n", snd_strerror(err));
                  exit(EXIT_FAILURE);
               }
               break; // skip one period
            }
            cptr -= err;

            // We have successfully written_out some frames;
            // increment write_out pointer
            //writeOutPtr = writeOutPtr + (err * sizeof(buffers_cap[0]) * sampleResCap * channels_cap);
            writeOutPtr = writeOutPtr + (err * sampleResCap * channels_cap);
            // frames_written_out * bytes_per_sample * channels;
            // Decrease the variable which shows how many frames are ready to be written out to sound card;
            modify_num_frames_ready(DECREASE, err);

            /* Cyclic buffer */
            if(writeOutPtr >= (buffers_cap + buffers_cap_no * buffer_cap_size))
            {
               writeOutPtr = buffers_cap;
            }
         }
      }
   }
}

static int set_hwparams_playback(snd_pcm_t *handle,
	snd_pcm_hw_params_t *params,
	snd_pcm_access_t access)
{
	unsigned int rrate, val;
	snd_pcm_uframes_t size;
	snd_pcm_uframes_t frames;
	int err, dir;
	/* Fill params with a full configuration space for the PCM */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	/* set hardware resampling */
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if (err < 0) {
		printf("Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}
   err = snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) &val);
   if (err < 0)
   {
      printf("Unable to get access type for playback: %s\n", snd_strerror(err));
      return err;
   }
   printf("access type: %s\n", snd_pcm_access_name((snd_pcm_access_t)val));
	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0) {
		printf("Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}
   err = snd_pcm_hw_params_get_format(params, (snd_pcm_format_t *)&val);
   if (err < 0)
   {
      printf("Unable to get format for playback: %s\n", snd_strerror(err));
      return err;
   }
	printf("format: %s\n", snd_pcm_format_name((snd_pcm_format_t)val));
	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0)
	{
		printf("Unable to set channels for playback: %s\n", snd_strerror(err));
		return err;
	}
   err = snd_pcm_hw_params_get_channels(params, &val);
   if (err < 0)
   {
      printf("Unable to get channels for playback: %s\n", snd_strerror(err));
      return err;
   }
   printf("number of channels: %d\n", val);
	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0)
	{
		printf("Unable to set rate for playback: %s\n", snd_strerror(err));
		return err;
	}
	if (rrate != rate)
	{
	   printf("Rate %d Hz is not supported by your hardware ==> Using %d instead.\n",
	         rate, rrate);
	}
	else
	{
	   err = snd_pcm_hw_params_get_rate(params, &val, &dir);
	   if (err < 0)
	   {
	      printf("Unable to get rate for playback: %s\n", snd_strerror(err));
	      return err;
	   }
	   printf("rate: %d samples/sec\n", val);
	}
	/* set the buffer time */
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
	if (err < 0) {
		printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
		return err;
	}
	/* set the period time */
   err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
   if (err < 0) {
      printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
      return err;
   }
   /* write the parameters to device */
   err = snd_pcm_hw_params(handle, params);
   if (err < 0) {
      printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
      return err;
   }
   err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
   if (err < 0) {
      printf("Unable to get period size for playback: %s\n", snd_strerror(err));
      return err;
   }
   period_size = size;
   printf("period size: %d frames\n", (int)size);
	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if (err < 0) {
		printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
		return err;
	}
	buffer_size = size;
   printf("buffer size: %d frames\n", (int)buffer_size);
   printf("hw params for playback have been set to driver!\n");

	return 0;
}

static int set_hwparams_capture(snd_pcm_t *handle_cap,
   snd_pcm_hw_params_t *hwparams_cap,
   snd_pcm_access_t access)
{
   int dir_cap;
   snd_pcm_uframes_t size;
   int error, dir;
   unsigned int rate, exact_rate_cap;
   unsigned int val;
   //static unsigned int channels;

   /* Fill hwparams_cap with default values from configuration space */
   snd_pcm_hw_params_any(handle_cap, hwparams_cap);
   /* Set Access Mode for capture */
   error = snd_pcm_hw_params_set_access(handle_cap, hwparams_cap, access);
   if (error < 0)
   {
      printf("Unable to set access mode for capture: (%s)\n", snd_strerror(error));
      return error;
   }
   error = snd_pcm_hw_params_get_access(hwparams_cap, (snd_pcm_access_t *) &val);
   if (error < 0)
   {
      printf("Unable to get access type for capture: %s\n", snd_strerror(error));
      return error;
   }
   printf("access type: %s\n", snd_pcm_access_name((snd_pcm_access_t)val));
   /* Set Format for capture */
   error = snd_pcm_hw_params_set_format(handle_cap, hwparams_cap, format_cap);
   if (error < 0)
   {
      printf("Format %s is not supported by your hardware ==> Using %s instead.\n",
            snd_pcm_format_name(format_cap), snd_pcm_format_name(CAPTURE_FORMAT_DEFAULT));
      // Try to set again, with default value
      format_cap = CAPTURE_FORMAT_DEFAULT;
      error = snd_pcm_hw_params_set_format(handle_cap, hwparams_cap, format_cap);
      if (error < 0)
      {
         printf("Unable to set format for capture: (%s)\n", snd_strerror(error));
         return error;
      }
   }
   else
   {
      error = snd_pcm_hw_params_get_format(hwparams_cap, (snd_pcm_format_t *)&val);
      if (error < 0)
      {
         printf("Unable to get format for capture: %s\n", snd_strerror(error));
         return error;
      }
      printf("format: %s\n", snd_pcm_format_name((snd_pcm_format_t)val));
   }
   /* Set number of channels for capture */
   error = snd_pcm_hw_params_set_channels(handle_cap, hwparams_cap, channels_cap);
   if (error < 0)
   {
      printf("Number of channels %d is not supported by your hardware ==> Using %d instead.\n",
            channels_cap, CAPTURE_CHANNELS_DEFAULT);
      // Try to set again, with default value
      channels_cap = CAPTURE_CHANNELS_DEFAULT;
      error = snd_pcm_hw_params_set_channels(handle_cap, hwparams_cap, channels_cap);
      if (error < 0)
      {
         printf("Unable to set channels for capture: (%s)\n", snd_strerror(error));
         return error;
      }
   }
   else
   {
      error = snd_pcm_hw_params_get_channels(hwparams_cap, &val);
      if (error < 0)
      {
         printf("Unable to get channels for capture: %s\n", snd_strerror(error));
         return error;
      }
      printf("number of channels: %d\n", val);
   }
   /* Set stream Rate for capture */
   exact_rate_cap = rate_cap;
   error = snd_pcm_hw_params_set_rate_near(handle_cap, hwparams_cap, &exact_rate_cap, 0);
   if (error < 0)
   {
      printf("Unable to set rate for capture: (%s)\n", snd_strerror(error));
      return error;
   }
   if (rate_cap != exact_rate_cap)
   {
      printf("Rate %d Hz is not supported by your hardware ==> Using %d instead.\n",
            rate_cap, exact_rate_cap);
   }
   else
   {
      error = snd_pcm_hw_params_get_rate(hwparams_cap, &val, &dir);
      if (error < 0)
      {
         printf("Unable to get rate for capture: %s\n", snd_strerror(error));
         return error;
      }
      printf("rate: %d samples/sec\n", val);
   }
   /* Write the hardware parameters to the driver */
   error = snd_pcm_hw_params(handle_cap, hwparams_cap);
   if (error < 0)
   {
      printf("Unable to set hw params for capture: (%s) (%d)\n", snd_strerror(error), error);
      return error;
   }
   /* Get period size */
   error = snd_pcm_hw_params_get_period_size(hwparams_cap, &size, 0);
   if (error < 0)
   {
      printf("Unable to get period size for capture: %s\n", snd_strerror(error));
      return error;
   }
   period_size_cap = size;
   printf("period_size: %d frames\n", (int)period_size_cap);
   error = snd_pcm_hw_params_get_buffer_size(hwparams_cap, &size);
   if (error < 0)
   {
      printf("Unable to get buffer size for capture: %s\n", snd_strerror(error));
      return error;
   }
   printf("buffer size: %d frames\n", (int)size);
   printf("hw params for capture have been set to driver!\n\n");
   return 0;
}

/*
 * Underrun and suspend recovery
 */
static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (verbose)
	 printf("stream recovery\n");
	if (err == -EPIPE) { /* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
		 printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
		 sleep(1); /* wait until the suspend flag is released */
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
			 printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

static void list_alsa_devices(snd_pcm_stream_t stream)
{
   int card, err, dev;
   snd_ctl_t *handle;
   snd_ctl_card_info_t *info;
   snd_pcm_info_t *pcminfo;
   snd_ctl_card_info_alloca(&info);
   snd_pcm_info_alloca(&pcminfo);

   card = -1;
   printf("**** List of %s Hardware Devices ****\n", snd_pcm_stream_name(stream));
   while(snd_card_next(&card) >= 0 && card >= 0)
   {
      char name[32];

      sprintf(name, "hw:%d", card);
      if ((err = snd_ctl_open(&handle, name, 0)) < 0)
      {
         printf("list_alsa_devices: Error control open; card (%i): %s\n", card, snd_strerror(err));
         continue;
      }

      if ((err = snd_ctl_card_info(handle, info)) < 0)
      {
         printf("list_alsa_devices: Error control hardware info; card (%i): %s\n", card, snd_strerror(err));
         snd_ctl_close(handle);
         continue;
      }

      dev = -1;
      while (1)
      {
         unsigned int count;
         if (snd_ctl_pcm_next_device(handle, &dev) < 0)
            printf("list_alsa_devices: Error calling snd_ctl_pcm_next_device()\n");
         if (dev < 0)
            break;

         snd_pcm_info_set_device(pcminfo, dev);
         snd_pcm_info_set_stream(pcminfo, stream);

         if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
         {
            if (err != -ENOENT)
               printf("list_alsa_devices: Error getting info on PCM device; card (%i): %s\n", card, snd_strerror(err));
            continue;
         }

         // for capture, only list our devices
         if((stream == SND_PCM_STREAM_CAPTURE) && ((strstr(snd_ctl_card_info_get_longname(info), "DGC dada")) == 0))
            break;

         printf("card %i: %s, device %i: %s [%s]; Use \"%s,%i\"\n",
                card, snd_ctl_card_info_get_name(info),
                dev, snd_pcm_info_get_id(pcminfo), snd_pcm_info_get_name(pcminfo), name, dev);
      }
      snd_ctl_close(handle);
   }

   return;
}

static void acquire_default_device(char *name, snd_pcm_stream_t stream)
{
   int card, dev, err;
   snd_ctl_t *handle;
   snd_ctl_card_info_t *info;
   snd_pcm_info_t *pcminfo;
   snd_ctl_card_info_alloca(&info);
   snd_pcm_info_alloca(&pcminfo);
   char buf[8];

   card = -1;
   while(snd_card_next(&card) >= 0 && card >= 0)
   {
      sprintf(name, "hw:%d", card);
      if ((err = snd_ctl_open(&handle, name, 0)) < 0)
      {
         printf("acquire_default_device: Error control open; card (%i): %s\n", card, snd_strerror(err));
         return;
      }
      if ((err = snd_ctl_card_info(handle, info)) < 0)
      {
         printf("acquire_default_device: Error control hardware info; card (%i): %s\n", card, snd_strerror(err));
         snd_ctl_close(handle);
         return;
      }

      dev = -1;
      if (snd_ctl_pcm_next_device(handle, &dev) < 0 || dev < 0)
      {
         printf("acquire_default_device: Error calling snd_ctl_pcm_next_device() or no devices\n");
         snd_ctl_close(handle);
         return;
      }

      snd_pcm_info_set_device(pcminfo, dev);
      snd_pcm_info_set_stream(pcminfo, stream);

      if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
      {
         //printf("acquire_default_device: Error getting info on PCM device; card (%i): %s\n", card, snd_strerror(err));
         snd_ctl_close(handle);
         // break;
         continue;
      }

      // Add device number to device description string (we should already have card number at this point)
      sprintf(buf, ",%d", dev);
      strcat(name, buf);

      snd_ctl_close(handle);

      // Any found device is enough for playback
      if(stream == SND_PCM_STREAM_PLAYBACK)
         break;
      // ...but for capture we need our device
      if((stream == SND_PCM_STREAM_CAPTURE) && ((strstr(snd_ctl_card_info_get_longname(info), "DGC dada")) != 0))
         break;
   }
return;
}

static void help(void)
{
   int k;
   printf(
      "Usage: pcm [OPTION]... [FILE]...\n"
      "-h help\n"
      "-l display the list of capture and playback devices\n"
      "-D specify playback device in format: \"hw:<card_number>,<device_number>\"\n"
      "-d specify capture device in format: \"hw:<card_number>,<device_number>\"\n"
      "-r stream rate for capture in Hz\n"
      "-c count of channels in stream for capture\n"
      "-b ring buffer size in us for playback\n"
      "-p period size in us for playback\n"
      "-o sample format for capture\n"
      "-L display the list of ALSA formats\n"
      "-v show detailed PCM setup for capture and playback\n"
      "\n");
}

int main(int argc, char *argv[])
{
	struct option long_option[] =
 {
	 {"help", 0, NULL, 'h'},
	 {"list", 1, NULL, 'l'},
	 {"device", 1, NULL, 'D'},
	 {"device_cap", 1, NULL, 'd'},
	 {"rate", 1, NULL, 'r'},
	 {"channels", 1, NULL, 'c'},
	 {"buffer", 1, NULL, 'b'},
	 {"period", 1, NULL, 'p'},
	 {"format", 1, NULL, 'o'},
	 {"List", 1, NULL, 'L'},
	 {"verbose", 1, NULL, 'v'},
	 {NULL, 0, NULL, 0},
 };
	/* playback variables */
   int k, err;
	snd_pcm_t *handle;
   snd_pcm_hw_params_t *hwparams;
   snd_pcm_hw_params_alloca(&hwparams);
   // write-out thread
   pthread_t thread_out;
   int ret_thread_out;

	/* capture variables */
	snd_pcm_t *handle_cap;
	int count;
   // hardware parameters struct for capture
   snd_pcm_hw_params_t *hwparams_cap;
   snd_pcm_hw_params_alloca(&hwparams_cap);
   // read-in thread
   pthread_t thread_in;
   int ret_thread_in;

   /* help and other variables */
   int morehelp;
   char **devices;
   int devicesNum;
   int i;
   // to hold all the options entered from command line
   char options_entered[11] = {0};
   char temp[2] = {0};

   morehelp = 0;

	while (1)
	{
		int c;
		if ((c = getopt_long(argc, argv, "hlD:d:r:c:b:p:o:Lv", long_option, NULL)) < 0)
		   break;
      // Collect all the entered options
      temp[0] = c;
      strcat(options_entered, temp);

		switch (c) {
			case 'h':
				morehelp++;
				break;
			case 'l':
			   // display the list of VISION ALSA capture devices and ALSA playback devices
			   list_alsa_devices(SND_PCM_STREAM_CAPTURE);
			   list_alsa_devices(SND_PCM_STREAM_PLAYBACK);
			   return 0;
			case 'D':
				device = strdup(optarg);
				break;
         case 'd':
            device_cap = strdup(optarg);
            break;
			case 'r':
			   // Unsupported rates are handled by snd_pcm_hw_params_set_rate_near()
			   rate_cap = atoi(optarg);
				break;
			case 'c':
			   channels_cap = atoi(optarg);
				break;
			case 'b':
				buffer_time = atoi(optarg);
				buffer_time = buffer_time < 1000 ? 1000 : buffer_time;
				buffer_time = buffer_time > 1000000 ? 1000000 : buffer_time;
				break;
			case 'p':
				period_time = atoi(optarg);
				period_time = period_time < 1000 ? 1000 : period_time;
				period_time = period_time > 1000000 ? 1000000 : period_time;
				break;
			case 'o':
				for (format_cap = 0; format_cap < SND_PCM_FORMAT_LAST; format_cap++)
				{
					const char *format_name = snd_pcm_format_name(format_cap);
					if (format_name)
					   if (!strcasecmp(format_name, optarg))
					      break;
				}
				if (format_cap == SND_PCM_FORMAT_LAST)
				{
				   printf("*** Format for capture is not an ALSA format => using default; (Use -L to display the list of ALSA formats)\n");
				   format_cap = CAPTURE_FORMAT_DEFAULT;
				}
				if (!snd_pcm_format_linear(format_cap) &&
					!(format_cap == SND_PCM_FORMAT_FLOAT_LE || format_cap == SND_PCM_FORMAT_FLOAT_BE))
				{
					printf("Invalid (non-linear/float) format %s\n", optarg);
					return 1;
				}
				break;
         case 'L':
            // display the list of ALSA formats
            printf("List of ALSA formats: ");
            for (i = 0; i < SND_PCM_FORMAT_LAST; i++)
               printf("%s ", snd_pcm_format_name(i));
            printf("\n");
            return 0;
			case 'v':
				verbose = 1;
				break;
		}
	}
	if (morehelp) {
		help();
		return 0;
	}
	err = snd_output_stdio_attach(&output, stdout, 0);
	if (err < 0) {
		printf("Output failed: %s\n", snd_strerror(err));
		return 0;
	}

   // program started with no device arguments: get default capture & playback devices
   if((argc == 1) || ((strchr(options_entered, 'D') == 0) && (strchr(options_entered, 'd') == 0)))
   {
      // Allocate memory for capture and playback devices names
      device = calloc(32, sizeof(char));
      device_cap = calloc(32, sizeof(char));
      printf("\n*** No capture and playback devices specified: USING DEFAULT CAPTURE AND PLAYBACK DEVICES!\n");
      acquire_default_device(device_cap, SND_PCM_STREAM_CAPTURE);
      acquire_default_device(device, SND_PCM_STREAM_PLAYBACK);
   }
	// In case only playback device was specified
	if ((strchr(options_entered, 'D') != 0) && (strchr(options_entered, 'd') == 0))
	{
	   printf("*** No capture device specified: USING DEFAULT CAPTURE DEVICE!\n");
	   device_cap = calloc(32, sizeof(char));
      acquire_default_device(device_cap, SND_PCM_STREAM_CAPTURE);
	}
   // In case only capture device was specified
   if ((strchr(options_entered, 'd') != 0) && (strchr(options_entered, 'D') == 0))
   {
      printf("*** No playback device specified: USING DEFAULT PLAYBACK DEVICE!\n");
      device = calloc(32, sizeof(char));
      acquire_default_device(device, SND_PCM_STREAM_PLAYBACK);
   }

	/* Playback: PCM open and parameters initialisation */
	printf("\nPLAYBACK:\n");
	// Open the PCM interface for playback in blocking mode
	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		return 0;
	}
   printf("Playback device is %s\n", device);
	if ((err = set_hwparams_playback(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("Setting of hwparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Capture: PCM open and parameters initialisation */
	printf("\nCAPTURE:\n");
   // Open the PCM interface for capture
	// in non-blocking mode
	if ((err = snd_pcm_open(&handle_cap, device_cap, SND_PCM_STREAM_CAPTURE, 1)) < 0)
   {
      printf("Capture open error: %s\n", snd_strerror(err));
      return 0;
   }
   printf("Capture device is %s\n", device_cap);
   // Set capture parameters
   if ((err = set_hwparams_capture(handle_cap, hwparams_cap, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
   {
      printf("Setting of hwparams_cap for capture failed: %s\n", snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   // print out detailed setup of capture and playback card
   if (verbose > 0)
   {
    printf("PLAYBACK SETUP VERBOSELY:\n");
    snd_pcm_dump(handle, output);
    printf("\nCAPTURE SETUP VERBOSELY:\n");
    snd_pcm_dump(handle_cap, output);
   }

   // Notify the user in case rates are not equal for capture and playback
   if (rate_cap != rate)
      printf("*** Note that rate for capture and rate for playback are not equal => Playback sound artifacts will occur!\n");

   /* Allocate the buffer for audio data */
   // Get the sample resolution in bits
   if ((count = snd_pcm_hw_params_get_sbits(hwparams_cap)) < 0)
   {
      printf("Getting sample resolution for capture failed: %s\n", snd_strerror(count));
      exit(EXIT_FAILURE);
   }
   // to bytes
   sampleResCap = count / 8;
   buffer_cap_size = period_size_cap * sampleResCap * channels_cap;    // period_size_in_frames * bytes_per_sample * channels
   buffers_cap = (unsigned char *) calloc(buffers_cap_no, buffer_cap_size);
   if (buffers_cap == NULL)
   {
      printf("Not enough memory\n");
      exit(EXIT_FAILURE);
   }
   // Create capture thread
   ret_thread_in = pthread_create(&thread_in, NULL, read_in, (void*) handle_cap);
   if(ret_thread_in)
   {
       printf("Error - pthread_create() for thread_in; return code: %d\n", ret_thread_in);
       exit(EXIT_FAILURE);
   }

   // Create playback thread
   ret_thread_out = pthread_create(&thread_out, NULL, write_out, (void*) handle);
   if(ret_thread_out)
   {
       printf("Error - pthread_create() for thread_out; return code: %d\n",ret_thread_out);
       exit(EXIT_FAILURE);
   }

   // Suspend the execution of main() until these threads terminate
   pthread_join(thread_in, NULL);
   pthread_join(thread_out, NULL);

   // Close playback
   snd_pcm_close(handle);
	// Close capture
	snd_pcm_drain(handle_cap);
	snd_pcm_close(handle_cap);
	free(buffers_cap);
	// Free memory for device names
	// Note that it is valid to free even if we specified device in command line (no calloc as done for default) because in such case strdup allocated memory
	free(device);
	free(device_cap);

	return 0;
}
