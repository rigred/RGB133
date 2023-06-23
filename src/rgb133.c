/*
 * rgb133.c
 *
 * Copyright (c) 2009 Datapath Limited All rights reserved.
 *
 * All information contained herein is proprietary and
 * confidential to Datapath Limited and is licensed under
 * the terms of the Datapath Limited Software License.
 * Please read the LICENCE file for full license terms
 * and conditions.
 *
 * http://www.datapath.co.uk/
 * support@datapath.co.uk
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>

/* PCI includes */
#include <linux/pci.h>
#include <linux/io.h>

/* Char dev headers */
#include <linux/cdev.h>

/* Timer headers */
#include <linux/timer.h>

/* Linux Driver Headers */
#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133api.h"
#include "rgb133kernel.h"
#include "rgb133capapi.h"
#include "rgb133deviceapi.h"

#include "rgb133dma.h"
#include "rgb133v4l2.h"
#include "rgb133ioctl.h"
#include "rgb133irq.h"
#include "rgb133pci.h"
#include "rgb133registry.h"
#include "rgb133mapping.h"
#include "rgb133timer.h"

#include "rgb133debug.h"

#include "rgb133control.h"

#include "rgb133vdif.h"

#ifdef RGB133_USER_MODE
#include "rgb133usersg.h"
#endif /* RGB133_USER_DMA */

#include <media/v4l2-dev.h>

#ifdef RGB133_CONFIG_HAVE_NEW_V4L2
// DMJ - v4l2-dev.h included in rgb133ioctl without config check.
#include <media/v4l2-device.h>
#endif

#include "devicetypes.h"

// For initialising Multiline Messages buffers
#define MAX_LINE_LENGTH 512

/** Debug Config **/
extern unsigned long old_debug_level;
unsigned long rgb133_debug = 1;                             /* The debug level */
unsigned long rgb133_debug_mask = RGB133_LINUX_DBG_NONE;    /* The debug mask */

/** FPS Output */
int rgb133_show_fps = RGB133_NO_SHOW_FPS;                   /* FPS Output */

/** Default coloured buffers */
int rgb133_coloured_buffers = RGB133_NO_COLOURED_BUFFERS;                   /* To test if DMA's are working */

/* static variables - module parameters */
static int rgb133_device_index = 0;                         /* The number of Vision / VisionLC devices in the system */
static int rgb133_devices_count = 0;

/** Device Config **/
static int rgb133_captures = RGB133_MAX_CAP_PER_CHANNEL;    /* The number of devices per system */

static int rgb133_video_nr = -1;                            /* /dev/video$rgb133_video_nr - -1 is auto-detect */

/* Expose all inputs in the system as V4L2 device nodes */
int rgb133_expose_inputs = RGB133_NO_EXPOSE_INPUTS;         /* Expose device channels as inputs */

/** Default Interlacing Type **/
unsigned long rgb133_interlace = V4L2_FIELD_NONE; // Was INTERLACED

/** Behaviour on non-v4l2 pixel formats **/
unsigned long rgb133_non_vl42_pix_fmts = RGB133_ENABLE_NON_V4L2_PIX_FMTS;

/** Default RGB byte order **/
unsigned long rgb133_flip_rgb = RGB133_RGB_BYTEORDER;

/** Default Colour Domain */
unsigned long rgb133_colour_domain = RGB133_COLOUR_DOMAIN_AUTO;

/** Limit the video timings **/
unsigned long rgb133_limit_vid_timings = RGB133_VID_TIMINGS_DEFAULT;

/** Parameters to make Skype work **/
int rgb133_dumb_buffer_width = RGB133_DUMB_BUFFER_WIDTH_DEFAULT;
int rgb133_dumb_buffer_height = RGB133_DUMB_BUFFER_HEIGHT_DEFAULT;
int rgb133_dumb_buffer_rate = RGB133_DUMB_BUFFER_RATE_DEFAULT;

/* No signal text control, messages, colours and counter */
int rgb133_nosig = RGB133_DISPLAY_NO_SIGNAL_MSG;                /* What to do on no signal - 0 is display message */

extern char vdif_string_nosignal[64];
extern char vdif_string_outofrange[64];
extern char vdif_string_unrec[64];

extern int rgb133_msg_colour;

int rgb133_nosig_counter = RGB133_SHOW_COUNTER;

/* Default behaviour for analog GTF modes */
unsigned long rgb133_offset_gtf = RGB133_NO_GTF_OFFSET;

/* Timestamping Data */
unsigned long rgb133_timestamp_info = RGB133_TIMESTAMP_INFO_SILENT;

/* Frame Debug */
unsigned long rgb133_frame_debug = RGB133_NO_FRAME_DEBUG;

/* Default detection mode */
unsigned long rgb133_detect_mode = RGB133_DETECT_MODE_DEFAULT;

/* Scaling */
unsigned long rgb133_scaling = RGB133_SCALING_DEFAULT;
unsigned long rgb133_scaling_aspect_ratio = RGB133_SCALING_ASPECT_RATIO_DEFAULT;

/* HDCP */
unsigned long rgb133_hdcp = RGB133_HDCP_OFF;

/* Overscan Cropping */
unsigned long rgb133_overscan_cropping = RGB133_OVERSCAN_CROPPING_ON;

/* Diagnostics Mode */
unsigned long rgb133_diagnostics_mode = RGB133_NO_DIAGNOSTICS_MODE;                   /* Enable / disable diagnostics mode */

/* Full Range YUV */
unsigned long rgb133_full_range = RGB133_FULL_RANGE_YUV;

/* Input Ganging */
unsigned long rgb133_ganging_type = RGB133_GANGING_TYPE_DISABLED;

/* Audio Source */
unsigned long rgb133_audio_source = RGB133_AUDIO_EMBEDDED;

/* Audio Digital Mute */
unsigned long rgb133_audio_mute_digital = RGB133_AUDIO_MUTE_OFF;

/* Audio Analog Mute */
unsigned long rgb133_audio_mute_analog = RGB133_AUDIO_MUTE_BALANCED;

/* Audio Volume */
int rgb133_audio_volume_unbalanced = RGB133_AUDIO_VOLUME_DEFAULT;
int rgb133_audio_volume_balanced = RGB133_AUDIO_VOLUME_DEFAULT;

/** Driver Devices **/
static struct rgb133_dev rgb133_devices[RGB133_MAX_DEVICES];

/* v4l2 interface defiitions */
#ifdef RGB133_CONFIG_HAVE_NEW_V4L2
static int rgb133_open(struct file*);
static int rgb133_close(struct file*);
#else
static int rgb133_open(struct inode*, struct file*);
static int rgb133_close(struct inode*, struct file*);
#endif /* RGB133_CONFIG_HAVE_NEW_V4L2 */
ssize_t rgb133_read(struct file*, char __user*, size_t, loff_t*);
ssize_t rgb133_write(struct file*, const char __user*, size_t, loff_t*);
static unsigned int rgb133_poll(struct file*, struct poll_table_struct*);
static int rgb133_mmap(struct file*, struct vm_area_struct*);

const char* PrintDetectMode(unsigned long mode);

void GetSizeOfIOCTLStructs(long * pInStruct, long * pOutStruct);

#ifdef RGB133_CONFIG_HAVE_V4L2_FOPS
const struct v4l2_file_operations rgb133_fops = {
#else
const struct file_operations rgb133_fops = {
#endif
   .owner          = THIS_MODULE,
   .open           = rgb133_open,
   .release        = rgb133_close,
   .read           = rgb133_read,
   .write          = rgb133_write,
   .mmap           = rgb133_mmap,
   .poll           = rgb133_poll,
#ifdef RGB133_CONFIG_HAVE_UNLOCKED_IOCTL_IN_V4L2_FOPS
   .unlocked_ioctl = video_ioctl2,
#else
   .ioctl          = video_ioctl2, /* Use the v4l ioctl ifc (we'll call back
                                * into our own functions via this ifc)
                                */
#endif
#ifndef RGB133_CONFIG_HAVE_V4L2_FOPS
   .llseek     = no_llseek,
#endif
};

#ifdef RGB133_USER_MODE
static spinlock_t user_mode_lock;
static int user_mode_lock_init = 0;
static int mmap_dev_num = 0;
#endif /* RGB133_USER_MODE */

/* v4l2 interface file operation defiitions */
#ifdef RGB133_CONFIG_HAVE_NEW_V4L2
int rgb133_open(struct file* file)
#else
int rgb133_open(struct inode* inode, struct file* file)
#endif
{
   struct video_device* vfd = video_devdata(file);
   struct rgb133_dev* dev = video_get_drvdata(vfd);
   struct rgb133_handle* h = 0;
   PIRP pIrp = NULL;

   enum v4l2_buf_type type = 0;

   int ret = 0;

   int i = 0;
   int j = 0;
   int k = 0;

   if (old_debug_level != rgb133_debug)
   {
      old_debug_level = rgb133_debug;
      CaptureSetDebugLevel(h, rgb133_debug);
   }

   KernelMutexLock(dev->pLock);

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: START - dev[%d](0x%p)\n", dev->index, dev));
   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

#ifndef RGB133_CONFIG_HAVE_V4L2_FOPS
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_open: minor(%d), type(%s)\n",
         dev->index, v4l2_type_names[V4L2_BUF_TYPE_VIDEO_CAPTURE]));
#endif /* !RGB133_CONFIG_HAVE_V4L2_FOPS */

   if(file->private_data != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: file(0x%p)->priv_data(0x%p) for dev[%d](0x%p) in non-NULL\n",
            file, file->private_data, dev->index, dev));
      KernelMutexUnlock(dev->pLock);
      return -EFAULT;
   }

   /* Allocate and initialise handle data */
   h = KernelVzalloc(sizeof(*h));
   if(NULL == h)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: failed to get memory for private handle\n"));
      KernelMutexUnlock(dev->pLock);
      return -ENOMEM;
   }
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: alloc %d bytes to h(0x%p)\n",
         (int)sizeof(*h), h));
   h->dev = dev;
   h->minor = vfd->minor;
   h->channel = -1;
   h->capture = -1;
   h->memorytype = 0;
   h->bSysFSOpen = FALSE;
   h->bClosing = FALSE;
   h->pIrp = NULL;

   h->sCapture.ScalingMode = rgb133_scaling;
   h->sCapture.ScalingAR = rgb133_scaling_aspect_ratio;

   /* Initialise Streaming flags and AutoCap Event */
   rgb133_set_streaming(h, RGB133_STRM_NOT_STARTED);

   /* Lookup the default channel number */
   {
      int loop = 0;
      for(loop=0; loop<RGB133_MAX_CHANNEL_PER_CARD; loop++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: dev[%d](0x%p)->pDevMap[%d].minor(%d) h->minor(%d)\n",
               dev->index, dev, loop,
               dev->pDevMap[loop].minor, h->minor));
         if(dev->pDevMap[loop].minor == h->minor)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_open: matched minor(%d) @ (%d)\n",
                  h->minor, loop));
            break;
         }
      }

      if(loop == RGB133_MAX_CHANNEL_PER_CARD)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: failed to match h->minor(%d)\n",
               h->minor));
         KernelMutexUnlock(dev->pLock);
         return -ENODEV;
      }

      h->channel = dev->pDevMap[loop].channel;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: using h->channel(%d)\n",
            h->channel));
   }

   /* Init locks - Must do this as soon as we've got a handle, before anything that calls it can be run. */
   h->pSem = KernelVzalloc(sizeof(struct semaphore));
   if(h->pSem == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: [%d][%d] failed to allocate %d bytes to h->pSem free h(0x%p)\n",
            h->dev->index, h->channel, sizeof(struct semaphore), h));
      KernelVfree(h);
      KernelMutexUnlock(dev->pLock);
      return -ENOMEM;
   }
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: allocated %d bytes to h->pSem(0x%p)\n",
         sizeof(struct semaphore), h->pSem));

   h->pSpinlock = KernelVzalloc(sizeof(spinlock_t));
   if(h->pSpinlock == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: [%d][%d] failed to allocate %d bytes to h->pSpinlock free h->pSem(0x%p) h(0x%p)\n",
            h->dev->index, h->channel, sizeof(spinlock_t), h->pSem, h));
      KernelVfree(h->pSem);
      KernelVfree(h);
      KernelMutexUnlock(dev->pLock);
      return -ENOMEM;
   }
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: allocated %d bytes to h->pSem(0x%p)\n",
         sizeof(spinlock_t), h->pSpinlock));

   sema_init(h->pSem, 1);
   spin_lock_init(h->pSpinlock);

   h->EnterCriticalSection = &rgb133_enter_critical_section;
   h->ExitCriticalSection = &rgb133_exit_critical_section;

   h->AcquireSpinLock = rgb133_acquire_spinlock;
   h->ReleaseSpinLock = rgb133_release_spinlock;

   /* Only open the capture if we're not the control device,
    * if we've got a valid pDE we're a capture device
    */
   if(dev->pDE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: Video Mode Open for dev[%d](0x%p) - channel[%d]\n",
            dev->index, dev, h->channel));
      {
         int j;

#ifdef RGB133_USER_MODE
         /* Check for User Mode init */
         if(!dev->userInit.init)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_open: User Mode is not initialised\n"));
            if(h->pIrp)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h->pIrp(0x%p)\n", h->pIrp));
               KernelVfree(h->pIrp);
            }
            if(h)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h(0x%p)\n", h));
               KernelVfree(h);
            }
            KernelMutexUnlock(dev->pLock);
            return -EDEADLK;
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_open: User Mode is initialised\n"));
         }

         if(dev->dirty[0] != RGB133_CAPTURE_NOT_IN_USE)
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_open: User Mode driver already open for channel[%d]\n",
                  channel));
            if(h->pIrp)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h->pIrp(0x%p)\n", h->pIrp));
               KernelVfree(h->pIrp);
            }
            if(h)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h(0x%p)\n", h));
               KernelVfree(h);
            }
            KernelMutexUnlock(dev->pLock);
            return -EINVAL;
         }
#endif /* RGB133_USER_MODE */

         /* Set up Irp for capture open*/
         pIrp = AllocAndSetupIrp(h);
         if(NULL == pIrp)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: failed to allocate IRP\n"));
            KernelMutexUnlock(dev->pLock);
            return -ENOMEM;
         }
         h->pIrp = pIrp;

         /* Open the capture */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: APIOpenCapture h(0x%p)->dev(0x%p).channel[%d].capture(%d) pDE(0x%p)\n",
               h, dev, dev->index, h->channel, h->capture, dev->pDE));
         if((ret = APIOpenCapture(h, pIrp)) < 0)
         {
            if(ret == -EBADRQC)
            {
               RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_open: Diagnostics-mode open: return as normal\n"));
               file->private_data = h;
               KernelMutexUnlock(dev->pLock);
               return 0;
            }
            if(ret == -EIO)
            {
               RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_open: Sysfs open, return as normal\n"));
               file->private_data = h;
               KernelMutexUnlock(dev->pLock);
               return 0;
            }
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: APIOpenCapture failed(%d)\n", ret));
            KernelMutexUnlock(dev->pLock);
            return ret;
         }

         /* Enable the capture */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: enable h(0x%p)->dev[%d].channel[%d].capture(%d) pDE(0x%p)\n",
               h, h->dev->index, h->channel, h->capture, dev->pDE));
         if((ret = APIEnableCapture(h)) < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_open: APIEnableCapture failed(%d)\n", ret));
            KernelMutexUnlock(dev->pLock);
            return ret;
         }

         /* Get the number of channels on the device */
         if(dev->channels <= 0)
         {
            if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
            {
               dev->channels = DeviceGetChannels(dev, &rgb133_devices[RGB133_CONTROL_DEVICE_NUM]);
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: got %d channels for dev[%d].pDE(0x%p)\n",
                     dev->channels, dev->index, dev->pDE));
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: force 1 channel for dev[%d].pDE(0x%p) - Expose Inputs\n",
                     dev->index, dev->pDE));
               dev->channels = 1;
            }
         }
      }

      if(file->f_flags & O_NONBLOCK)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: non-blocking IO for dev[%d].channel[%d].capture[%d] not supported\n",
               dev->index, h->channel, h->capture));
      }

      /* Initialise no signal stuff */
      h->no_sig_jiffies = 0;
      h->day = 0;
      h->hr  = 0;
      h->m   = 0;
      h->s   = 0;
      h->ms  = 0;

      /* Initialise the No Signal buffer ptrs */
      h->pNoSignalBufIn = NULL;
      h->pNoSignalBufOut = NULL;

      /* Initialise rate stuff */
      h->rate.timeout          = 0;
      h->rate.tv_now.tv_sec    = 0;
      h->rate.tv_now.tv_usec   = 0;
      h->rate.tv_later.tv_sec  = 0;
      h->rate.tv_later.tv_usec = 0;

      /* Initialise fps stuff */
      h->frame_count = 0;

      /* Initialise frame info */
      h->frame.seq = -1;
      h->frame.ts.tv_sec = -1;
      h->frame.ts.tv_usec = -1;

      /* For each pMessagesIn/pMessagesOut, allocate memory */
      for(i=0; i<(sizeof(h->pMessagesIn)/sizeof(char*)); i++)
      {
         h->pMessagesIn[i] = KernelVzalloc(MAX_LINE_LENGTH);

         if(h->pMessagesIn[i] == NULL)
         {
            for(j=0; j<i; j++)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h->pMessagesIn[%d](0x%p)\n", j, h->pMessagesIn[j]));
               KernelVfree(h->pMessagesIn[j]);
               h->pMessagesIn[j] = NULL;
            }
            return -ENOMEM;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: alloc %d bytes to h->pMessagesIn[%d](0x%p)\n",
               MAX_LINE_LENGTH, i, h->pMessagesIn[i]));
      }

      for(i=0; i<(sizeof(h->pMessagesOut)/sizeof(char*)); i++)
      {
         h->pMessagesOut[i] = KernelVzalloc(MAX_LINE_LENGTH);

         if(h->pMessagesOut[i] == NULL)
         {
            for(j=0; j<i; j++)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h->pMessagesOut[%d](0x%p)\n", j, h->pMessagesOut[j]));
               KernelVfree(h->pMessagesOut[j]);
               h->pMessagesOut[j] = NULL;
            }

            // If allocation fails at this point, also deallocate pMessagesIn[]
            for(k=0; k<(sizeof(h->pMessagesIn)/sizeof(char*)); k++)
               {
                  if(h->pMessagesIn[k] != NULL)
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: free h->pMessagesIn[%d](0x%p)\n", k, h->pMessagesIn[k]));
                     KernelVfree(h->pMessagesIn[k]);
                     h->pMessagesIn[k] = NULL;
                  }
               }

            return -ENOMEM;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_open: alloc %d bytes to h->pMessagesOut[%d](0x%p)\n",
               MAX_LINE_LENGTH, i, h->pMessagesOut[i]));
      }

      /*  Now we've got the VideoParams, set the control defaults
       *  if we're the first user in.
       */
      if(!h->dev->init[h->channel])
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: Setup colour domain(%lu), initial control values for channel[%d].capture[%d]\n",
               rgb133_colour_domain, h->channel, h->capture));
         CaptureSetColourDomain(h, NULL, h->channel, CaptureMapColourDomainToWin(rgb133_colour_domain));
         CaptureSetControls(h);

         /* Flag channel on device as initialised */
         h->dev->init[h->channel] = 1;
      }

      if(dev->bSetEquilisation)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: Setup equilisation for channel[%d]\n",
               h->channel));
         CaptureSetEquilisation(h);
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: Control Mode Open for dev[%d](0x%p) - channel[%d]\n",
            dev->index, dev, h->channel));

#ifdef RGB133_USER_MODE
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_open: Ctrl - users(%d)\n", dev->devUsers[h->channel]));
      if(dev->devUsers[h->channel] == 0)
      {
         /* If we're first in and the lock's not init'd
          * init event stuff */
         if(!user_mode_lock_init)
         {
            spin_lock_init(&user_mode_lock);
            user_mode_lock_init = 1;
         }
         mmap_dev_num = 0;
      }
#else
      if(dev->ctrlUsers == 0)
      {
         int i = 0, j = 0;

         while(rgb133_devices[i].pDE && i < RGB133_MAX_DEVICES)
         {
            for(j = 0; j < DeviceGetChannels(&rgb133_devices[i], &rgb133_devices[RGB133_CONTROL_DEVICE_NUM]); j++ )
            {
               DeviceClearSignalNotificationEvent(rgb133_devices[i].pDE, j);
            }
            i++;
         }
      }
#endif /* RGB133_USER_MODE */

      dev->ctrlUsers++;
   }

   /* Open is ok, assign our driver handle as private data */
   file->private_data = h;

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_open: END - h(0x%p) [%d][%d][%d] - pRGBCapture[%d][%d](0x%p)\n",
      h, dev->index, h->channel, h->capture,
      CaptureGetChannelNumber(CaptureGetCapturePtr(h)),
      CaptureGetCaptureNumber(CaptureGetCapturePtr(h)),
      CaptureGetCapturePtr(h)));
   KernelMutexUnlock(dev->pLock);
   return 0;
}

#ifdef RGB133_CONFIG_HAVE_NEW_V4L2
int rgb133_close(struct file* file)
#else
int rgb133_close(struct inode* inode, struct file* file)
#endif /* RGB133_CONFIG_HAVE_NEW_V4L2 */
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   PRGBCAPTUREAPI pRGBCapture = 0;

   int ret = 0;

   int i = 0;

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: START - h(0x%p) [%d][%d][%d] users(%d) - pRGBCapture[%d][%d](0x%p)\n",
         h, dev->index, h->channel, h->capture, DeviceGetCurrentUsers(dev),
         CaptureGetChannelNumber(CaptureGetCapturePtr(h)),
         CaptureGetCaptureNumber(CaptureGetCapturePtr(h)),
         CaptureGetCapturePtr(h)));

   KernelMutexLock(dev->pLock);

   if(dev->pDE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: Close video device on dev[%d].channel(%d).capture(%d).users(%d)\n",
            dev->index, h->channel, h->capture, DeviceGetCurrentUsers(dev)));

      /*  Flag that this handle is closing, notifies the auto-capture
       *  mechanism that it is not safe to instruct a new DMA.  Must
       *  be done before disabling capture.  Disabling capture may wait
       *  for a DMA to complete and while doing so allow in another
       *  DMA command to be fired off.
       */
      h->bClosing = TRUE;

      /* If necessary stop streaming, this should have been called via the VIDIOC_STREAMOFF ioctl
       * but it's better to be safe than sorry */
      h->EnterCriticalSection(h);
      if(rgb133_is_streaming(h))
      {
         rgb133_set_streaming(h, RGB133_STRM_NOT_STARTED);
      }
      h->ExitCriticalSection(h);

      if (!h->bSysFSOpen && CaptureIsEnabled(h))
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: Disable dev[%d].channel[%d].capture[%d]\n",
               dev->index, h->channel, h->capture));
         APIDisableCapture(h);
      }

      /* Drop into Windows driver close */
      if((ret = APICloseCapture(h)) < 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_close: APICloseCapture failed\n"));
         FreeIrp(h->pIrp);
         KernelMutexUnlock(dev->pLock);
         return ret;
      }
      FreeIrp(h->pIrp);

      if (!h->bSysFSOpen)
      {
         h->EnterCriticalSection(h);

         if(rgb133_is_mapped(h))
         {
            if(h->buffers)
            {
               int i;
               for (i=0;i<h->numbuffers;i++)
               {
                  if(h->buffers[i] != NULL)
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: free mmapped buffers in h(0x%p)->buffers[%d](0x%p)(0x%p)\n",
                           h, i, &h->buffers[i], h->buffers[i]));
                     __rgb133_buffer_mmap_dma_free(&h->buffers[i]);
                  }
               }
               rgb133_set_mapped(h, FALSE);
            }
         }

         h->ExitCriticalSection(h);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: Uninit handle q\n"));
         rgb133_q_uninit(h);
      }
      else
      {
         h->bSysFSOpen = FALSE;
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_close: Close Control channel(%d) (no pDE)\n", h->channel));
      dev->ctrlUsers--;
#ifdef RGB133_USER_MODE
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_close: control users(%d)\n", dev->ctrlUsers));
      if(dev->ctrlUsers == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_close: Set userInit.init(0)\n"));
         h->dev->userInit.init = 0;
      }
#endif /* RGB133_USER_MODE */

      if(dev->ctrlUsers == 0)
      {
         while(dev->events)
         {
            sVWSignalEvent *pEvent = dev->events;
            dev->events = pEvent->next;

            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free pEvent(0x%p)\n", pEvent));
            KernelVfree(pEvent);
         }
         dev->events = 0;
      }
   }

   /* For each pMessagesIn/pMessagesOut, free memory */
   for(i=0; i<(sizeof(h->pMessagesIn)/sizeof(char*)); i++)
   {
      if(h->pMessagesIn[i] != NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->pMessagesIn[%d](0x%p)\n", i, h->pMessagesIn[i]));
         KernelVfree(h->pMessagesIn[i]);
         h->pMessagesIn[i] = NULL;
      }
   }

   for(i=0; i<(sizeof(h->pMessagesOut)/sizeof(char*)); i++)
   {
      if(h->pMessagesOut[i] != NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->pMessagesOut[%d](0x%p)\n", i, h->pMessagesOut[i]));
         KernelVfree(h->pMessagesOut[i]);
         h->pMessagesOut[i] = NULL;
      }
   }

   /* Free up the No Signal buffer ptrs */
   if(h->pNoSignalBufIn)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free pNoSignalBufIn(0x%p)\n", h->pNoSignalBufIn));
      KernelVfree(h->pNoSignalBufIn);
      h->pNoSignalBufIn = NULL;
   }
   if(h->pNoSignalBufOut)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free pNoSignalBufOut(0x%p)\n", h->pNoSignalBufOut));
      KernelVfree(h->pNoSignalBufOut);
      h->pNoSignalBufOut = NULL;
   }

   {
      int channel = h->channel;
      int capture = h->capture;
      if(h)
      {
         /* Free IO request structures */
         if (h->ioctlsin != NULL)
         {
            for (i = 0; i < h->numbuffers; i++)
            {
               if (h->ioctlsin[i] != NULL)
               {
                  RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->ioctlsin[%d](0x%p)\n", i, h->ioctlsin[i]));
                  KernelVfree(h->ioctlsin[i]);
                  h->ioctlsin[i] = NULL;
               }
            }

            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->ioctlsin(0x%p) main structure\n", h->ioctlsin));
            KernelVfree(h->ioctlsin);
            h->ioctlsin = NULL;
         }

         if (h->buffers)
         {
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->buffers(0x%p)\n", h->buffers));
            KernelVfree(h->buffers);
            h->buffers = 0;
            h->numbuffers = 0;
         }

         if(h->pSem)
         {
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->pSem(0x%p)\n", h->pSem));
            KernelVfree(h->pSem);
         }
         if(h->pSpinlock)
         {
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h->pSpinlock(0x%p)\n", h->pSpinlock));
            KernelVfree(h->pSpinlock);
         }

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_close: free h(0x%p)\n", h));
         KernelVfree(h);
      }

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_close: END - [%d][%d][%d]\n",
            dev->index, channel, capture));
   }

   file->private_data = 0;
   KernelMutexUnlock(dev->pLock);

   return ret;
}

void CopyDeviceParms(struct _sVWAllDeviceParms* to, struct _sVWAllDeviceParms* from)
{
   if(to && from)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "CopyDeviceParms: %lu bytes From(0x%p) to (0x%p)\n",
            (unsigned long)sizeof(struct _sVWAllDeviceParms), from, to));
      memcpy(to, from, sizeof(struct _sVWAllDeviceParms));
   }
}

void CopyClientParms(struct _sVWClientParms* to, struct _sVWClientParms* from)
{
   if(to && from)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "CopyClientParms: %lu bytes From(0x%p) to (0x%p)\n",
            (unsigned long)sizeof(struct _sVWClientParms), from, to));
      memcpy(to, from, sizeof(struct _sVWClientParms));
   }
}

void CopyInfo(struct _sVWDeviceInfo* to, struct _sVWDeviceInfo* from)
{
   if(to && from)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "CopyInfo: %lu bytes From(0x%p) to (0x%p)\n",
            (unsigned long)sizeof(struct _sVWDeviceInfo), from, to));
      memcpy(to, from, sizeof(struct _sVWDeviceInfo));
   }
}

void ReadControlMatchDevice(struct rgb133_dev** ppDev, int* pOffset, int local_device)
{
   int i = 0;
   int matched = 0;
   int offset = 0;
   struct rgb133_dev* dev = 0;

   for(i=0; i<rgb133_device_index; i++)
   {
      int j;
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ReadControlMatchDevice: use dev[%d](0x%p)\n",
            i, &rgb133_devices[i]));
      dev = &rgb133_devices[i];
      for(j=0; j<RGB133_MAX_CHANNEL_PER_CARD; j++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ReadControlMatchDevice: check local_device(%d) with index(%d) - offset(%d)",
               local_device, dev->pDevMap[offset].index, offset));
         if(local_device == dev->pDevMap[offset].index)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "ReadControlMatchDevice: matched local_device(%d) with index(%d) @ offset(%d)\n",
                  local_device, dev->pDevMap[offset].index, offset));
            matched = 1;
            break;
         }

         offset++;
      }

      if(matched)
         break;

      offset = 0;
   }

   if(!matched)
   {
      *ppDev = 0;
      *pOffset = -1;
   }
   else
   {
      *ppDev = dev;
      *pOffset = offset;
   }
}

int rgb133_read_control(struct file* file, char __user* data, size_t count)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = 0;
   struct rgb133_dev* ctrl = 0;
   struct _sVWSystemInfo* VWSystemInfo = 0;
   struct _sVWDeviceInfo* VWDeviceInfo = 0;
   struct _sVWInputInfo* VWInputInfo = 0;
   struct _sVWDevice* VWDevice = 0;
   struct _sVWReadReg* VWReadReg = 0;
   struct _sVWReadFlash* VWReadFlash = 0;
   struct _sVWFlashInfo* VWFlashInfo = 0;
   struct _sVWBoardType* VWBoardType = 0;
   struct _sVWDebugLevel* VWDebugLevel = 0;
   struct _sVWSignalEvent* VWSignalEvent = 0;
   struct _sVWMagic VWMagic;
   int i;
   int devices = rgb133_device_index;
   int channels = 0;
   int inputs = 0;
   int offset = -1;
   int read_magic = -1;

   char version[16];

   int rc = -EINVAL;

#ifdef RGB133_USER_MODE
   struct _sAppEvent* appEv = 0;
   struct _sIrqEvent* irqEv = 0;
   unsigned long flags;
#endif /* RGB133_USER_MODE */

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_read_control: START - data(0x%p) count(%d)\n",
         data, count));

   /* Get the magic number */
   rc = copy_from_user((char*)&VWMagic, data, sizeof(VWMagic));
   read_magic = VWMagic.magic;

#ifdef RGB133_USER_MODE
   /* Is the user mode service up and running */
   if(!h->dev->userInit.init)
   {
      if(read_magic != VW_MAGIC_DEBUG_LEVEL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: User Mode service is not running\n"));
         return -EDEADLK;
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_DEBUG_LEVEL allowed!\n"));
      }
   }
#endif /* RGB133_USER_MODE */

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: Lookup magic(%d)\n", read_magic));
   switch(read_magic)
   {
      case VW_MAGIC_SYSTEM_NUM_DEVICES:
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_SYSTEM_NUM_DEVICES: Reading %lu bytes of Num System Device Info for %d devices\n",
               (unsigned long)sizeof(sVWSystemInfo), devices));
         VWSystemInfo = KernelVzalloc(sizeof(sVWSystemInfo));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_SYSTEM_NUM_DEVICES: alloc %d bytes to VWSystemInfo(0x%p)\n",
               (int)sizeof(sVWSystemInfo), VWSystemInfo));
         if(VWSystemInfo == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_SYSTEM_NUM_DEVICES: failed to alloc %d bytes to VWSystemInfo\n",
                  (int)sizeof(sVWSystemInfo)));
            return -ENOMEM;
         }

         VWSystemInfo->devices = devices;

         rc = KernelCopyToUser(data, (char*)VWSystemInfo, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_SYSTEM_NUM_DEVICES: free VWSystemInfo(0x%p)\n", VWSystemInfo));
         KernelVfree(VWSystemInfo);
         break;
      }
      case VW_MAGIC_SYSTEM_INFO:
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: Reading %lu bytes of System Device Info for %d devices\n",
               (unsigned long)sizeof(sVWSystemInfo), devices));
         VWSystemInfo = KernelVzalloc(sizeof(sVWSystemInfo));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: alloc %d bytes to VWSystemInfo(0x%p)\n",
               (int)sizeof(sVWSystemInfo), VWSystemInfo));
         if(VWSystemInfo == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: failed to alloc %d bytes to VWSystemInfo\n",
                  (int)sizeof(sVWSystemInfo)));
            return -ENOMEM;
         }

         if(rgb133_expose_inputs == RGB133_EXPOSE_INPUTS)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: adjust device from(%d) to(%d)\n",
                  devices, rgb133_devices_count));
            devices = rgb133_devices_count;
         }

         for(i=0; i<devices; i++)
         {
            if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
            {
               dev = &rgb133_devices[i];
               channels = DeviceGetChannels(dev, &rgb133_devices[RGB133_CONTROL_DEVICE_NUM]);
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: lookup exposed device[%d]\n",
                     i));
               ReadControlMatchDevice(&dev, &offset, i);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: matched exposed device[%d] on dev[%d] @ offset(%d)\n",
                     i, dev->index, offset));
               channels = 1;
            }

            if(dev)
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: Check channels on dev[%d](0x%p), ctrl_dev[%d](0x%p)\n",
                     dev->index, dev, RGB133_CONTROL_DEVICE_NUM, &rgb133_devices[RGB133_CONTROL_DEVICE_NUM]));
               VWSystemInfo->map[i].min = inputs;
               VWSystemInfo->map[i].max = inputs+(channels-1);
               VWSystemInfo->inputs += channels;
               inputs += channels;
            }
         }

         VWSystemInfo->devices = devices;

         {
            sprintf(version, "%u.%u.%u",
                  (RGB133_VERSION >> 16) & 0xff,
                  (RGB133_VERSION >> 8 ) & 0xff,
                   RGB133_VERSION        & 0xff);
            strcpy(VWSystemInfo->version, version);
         }

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: Devices(%d), Inputs(%d)\n", devices, inputs));

         rc = KernelCopyToUser(data, (char*)VWSystemInfo, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_SYSTEM_INFO: free VWSystemInfo(0x%p)\n", VWSystemInfo));
         KernelVfree(VWSystemInfo);
         break;
      }
      case VW_MAGIC_DEVICE_INFO:
      {
         VWDeviceInfo = KernelVzalloc(sizeof(sVWDeviceInfo));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: alloc %d bytes to VWDeviceInfo(0x%p)\n",
               (int)sizeof(sVWDeviceInfo), VWDeviceInfo));
         if(VWDeviceInfo == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: failed to alloc %d bytes to VWDeviceInfo\n",
                  (int)sizeof(sVWDeviceInfo)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWDeviceInfo, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: Reading Device Info for device[%d]\n",
               VWDeviceInfo->device));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: lookup device[%d] - dev[%d](0x%p)\n",
                  VWDeviceInfo->device, VWDeviceInfo->device, &rgb133_devices[VWDeviceInfo->device]));
            dev = &rgb133_devices[VWDeviceInfo->device];
            memcpy(&VWDeviceInfo->name, dev->core.name, min(sizeof(VWDeviceInfo->name), sizeof(dev->core.name)));
            memcpy(&VWDeviceInfo->node, dev->core.node, min(sizeof(VWDeviceInfo->node), sizeof(dev->core.node)));
            VWDeviceInfo->channels = DeviceGetChannels(dev, &rgb133_devices[RGB133_CONTROL_DEVICE_NUM]);
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: match device[%d] - &dev(0x%p) &offset(0x%p)\n",
                  VWDeviceInfo->device, &dev, &offset));
            ReadControlMatchDevice(&dev, &offset, VWDeviceInfo->device);
            if(dev)
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: matched exposed device[%d] on dev[%d] @ offset(%d)\n",
                     VWDeviceInfo->device, dev->index, offset));
               memcpy(&VWDeviceInfo->name, dev->pDevMap[offset].name, min(sizeof(VWDeviceInfo->name), sizeof(dev->pDevMap[offset].name)));
               memcpy(&VWDeviceInfo->node, dev->pDevMap[offset].node, min(sizeof(VWDeviceInfo->node), sizeof(dev->pDevMap[offset].node)));
               VWDeviceInfo->channels = 1;
            }
         }

         if(dev)
         {
            DeviceGetVHDLVersion(dev, VWDeviceInfo);
            DeviceGetLinkID(dev, VWDeviceInfo);
         }
         rc = KernelCopyToUser(data, (char*)VWDeviceInfo, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_DEVICE_INFO: free VWDeviceInfo(0x%p)\n", VWDeviceInfo));
         KernelVfree(VWDeviceInfo);
         break;
      }
      case VW_MAGIC_INPUT_INFO:
      {
         VWInputInfo = KernelVzalloc(sizeof(sVWInputInfo));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_INPUT_INFO: alloc %d bytes to VWInputInfo(0x%p)\n",
               (int)sizeof(sVWInputInfo), VWInputInfo));
         if(VWInputInfo == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_INPUT_INFO: failed to alloc %d bytes to VWInputInfo\n",
                  (int)sizeof(sVWInputInfo)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWInputInfo, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_INPUT_INFO: Reading %lu bytes of Input Info for device: %d\n",
               (unsigned long)sizeof(sVWInputInfo), VWInputInfo->device));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWInputInfo->device];
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWInputInfo->device);
         }

         // Get the client info
         if(dev)
         {
            VWInputInfo->clients = 0;
            for(i=0; i<RGB133_MAX_CHANNEL_PER_CARD; i++)
               VWInputInfo->clients += DeviceGetUsersOnChannel(dev, i);
         }
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_INPUT_INFO: Device[%d] has %d open clients\n",
               VWInputInfo->device, VWInputInfo->clients));
         rc = KernelCopyToUser(data, (char*)VWInputInfo, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_INPUT_INFO: free VWInputInfo(0x%p)\n", VWInputInfo));
         KernelVfree(VWInputInfo);
         break;
      }
      case VW_MAGIC_DEVICE:
      {
         VWDevice = KernelVzalloc(sizeof(sVWDevice));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_DEVICE: alloc %d bytes to VWDevice(0x%p)\n",
               (int)sizeof(sVWDevice), VWDevice));
         if(VWDevice == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_DEVICE: failed to alloc %d bytes to VWDevice\n",
                  (int)sizeof(sVWDevice)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWDevice, data, count);

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWDevice->device];
            offset = VWDevice->input;
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWDevice->device);
         }

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_DEVICE: Read data for dev[%d], Channel[%d]\n",
               dev->index, offset));

         // Fetch all the data for the device
         CaptureGetDeviceParameters(dev,
               &VWDevice->inputs[VWDevice->input].curDeviceParms,
               &VWDevice->inputs[VWDevice->input].minDeviceParms,
               &VWDevice->inputs[VWDevice->input].maxDeviceParms,
               &VWDevice->inputs[VWDevice->input].defDeviceParms,
               &VWDevice->inputs[VWDevice->input].detDeviceParms,
               offset, 0);

         rc = KernelCopyToUser(data, (char*)VWDevice, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_DEVICE: free VWDevice(0x%p)\n", VWDevice));
         KernelVfree(VWDevice);
         break;
      }
      case VW_MAGIC_READ_REG:
      {
         VWReadReg = KernelVzalloc(sizeof(sVWReadReg));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_READ_REG: alloc %d bytes to VWReadReg(0x%p)\n",
               (int)sizeof(sVWReadReg), VWReadReg));
         if(VWReadReg == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_READ_REG: failed to alloc %d bytes to VWReadReg\n",
                  (int)sizeof(sVWReadReg)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWReadReg, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_READ_REG: Read reg for device[%d], offset(%d)\n",
               VWReadReg->device, VWReadReg->offset));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWReadReg->device];
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWReadReg->device);
         }

         if(dev)
         {
            CaptureAddressPeek(dev, VWReadReg->offset, RGB133_DIAGS_PCI, &VWReadReg->value);
         }

         rc = KernelCopyToUser(data, (char*)VWReadReg, (int)count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_READ_REG: free VWReadReg(0x%p)\n", VWReadReg));
         KernelVfree(VWReadReg);
         break;
      }
      case VW_MAGIC_READ_FLASH:
      {
         VWReadFlash = KernelVzalloc(count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_READ_FLASH: alloc %d bytes to VWReadFlash(0x%p)\n",
               (int)count, VWReadFlash));
         if(VWReadFlash == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_READ_FLASH: failed to alloc %d bytes to VWReadFlash\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user(VWReadFlash, data, sizeof(sVWReadFlash));

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_READ_FLASH: Read flash for device[%d] into data(0x%p) for size(%u)\n",
               VWReadFlash->device, &VWReadFlash->data[0], VWReadFlash->dataSize));

         memset(&VWReadFlash->data[0], 0xae, VWReadFlash->dataSize);

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWReadFlash->device];
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWReadFlash->device);
         }

         if(dev)
         {
            if(CaptureReadFlashImage(dev, &VWReadFlash->data[0], &VWReadFlash->dataSize))
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_READ_FLASH: reading flash failed\n"));
               VWReadFlash->status = 1;
            }

#if 0
            {
               int i = 0;
               for(i=0; i<32; i++)
               {
                  if(!(i%16))
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_read_control: VW_MAGIC_READ_FLASH: imageData(%p): 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
                           &VWReadFlash->data[0],
                           VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++],
                           VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++],
                           VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++],
                           VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i++], VWReadFlash->data[i]));
                  }
               }
            }
#endif

            if(VWReadFlash->dataSize > count)
            {
               RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_read_control: VW_MAGIC_READ_FLASH: Truncating flash image from %u to %d bytes\n",
                     VWReadFlash->dataSize, (int)count));
            }
         }

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: VW_MAGIC_READ_FLASH: Copying %d bytes back\n", (int)count));
         rc = KernelCopyToUser(data, (char*)VWReadFlash, (int)count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_READ_FLASH: free VWReadFlash(0x%p)\n", VWReadFlash));
         KernelVfree(VWReadFlash);
         break;
      }
      case VW_MAGIC_FLASH_INFO:
      {
         VWFlashInfo = KernelVzalloc(sizeof(sVWFlashInfo));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_FLASH_INFO: alloc %d bytes to VWFlashInfo(0x%p)\n",
               (int)sizeof(sVWFlashInfo), VWFlashInfo));
         if(VWFlashInfo == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_FLASH_INFO: failed to alloc %d bytes to VWFlashInfo\n",
                  (int)sizeof(sVWFlashInfo)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWFlashInfo, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_FLASH_INFO: Read flash info for device[%d]\n",
               VWFlashInfo->device));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWFlashInfo->device];
            offset = 0;
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWFlashInfo->device);
         }

         if(dev)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_FLASH_INFO: DeviceGetFlashable(dev[%d], channel[%d], ...)\n",
                  dev->index, offset));
            DeviceIsFlashable(dev, offset, &VWFlashInfo->flashable);
         }

         rc = KernelCopyToUser(data, (char*)VWFlashInfo, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_FLASH_INFO: free VWFlashInfo(0x%p)\n", VWFlashInfo));
         KernelVfree(VWFlashInfo);
         break;
      }
      case VW_MAGIC_BOARD_TYPE:
      {
         VWBoardType = KernelVzalloc(sizeof(sVWBoardType));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_BOARD_TYPE: alloc %d bytes to VWBoardType(0x%p)\n",
               (int)sizeof(sVWBoardType), VWBoardType));
         if(VWBoardType == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: VW_MAGIC_BOARD_TYPE: failed to alloc %d bytes to VWBoardType\n",
                  (int)sizeof(sVWBoardType)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWBoardType, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: VW_MAGIC_BOARD_TYPE: Read board type for raw device[%d]\n",
               VWBoardType->device));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWBoardType->device];
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWBoardType->device);
         }

         if(dev)
         {
            CaptureGetBoardType(dev, &VWBoardType->type);
         }

         rc = KernelCopyToUser(data, (char*)VWBoardType, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: VW_MAGIC_BOARD_TYPE: free VWBoardType(0x%p)\n", VWBoardType));
         KernelVfree(VWBoardType);
         break;
      }
      case VW_MAGIC_DEBUG_LEVEL:
      {
         VWDebugLevel = KernelVzalloc(sizeof(sVWDebugLevel));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: alloc %d bytes to VWDebugLevel(0x%p)\n",
               (int)sizeof(sVWDebugLevel), VWDebugLevel));
         if(VWDebugLevel == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: failed to alloc %d bytes to VWDebugLevel\n",
                  (int)sizeof(sVWDebugLevel)));
            return -ENOMEM;
         }
         rc = copy_from_user(VWDebugLevel, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: Read debug level(%d)\n",
               rgb133_debug));
         VWDebugLevel->level = rgb133_debug_mask;

         rc = KernelCopyToUser(data, (char*)VWDebugLevel, count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: free VWDebugLevel(0x%p)\n", VWDebugLevel));
         KernelVfree(VWDebugLevel);
         break;
      }
      case VW_MAGIC_SIGNAL_EVENT:
      {
         //prevent crashes (h->dev->events NULL) for multiple clients VWindow on mode changes
         if(h->dev->events)
         {
            VWSignalEvent = KernelVzalloc(sizeof(sVWSignalEvent));
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: alloc %d bytes to VWSignalEvent(0x%p)\n",
                  (int)sizeof(sVWSignalEvent), VWSignalEvent));
            if(VWSignalEvent == 0)
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: failed to alloc %d bytes to VWSignalEvent\n",
                     (int)sizeof(sVWSignalEvent)));
               return -ENOMEM;
            }
            rc = copy_from_user(VWSignalEvent, data, count);

            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: Read first signal event\n"));

            ctrl = h->dev;

            KernelMutexLock(ctrl->pLock);
            VWSignalEvent->device = ctrl->events->device;
            VWSignalEvent->channel = ctrl->events->channel;
            dev = &rgb133_devices[VWSignalEvent->device];

            DeviceClearSignalNotificationEvent(dev->pDE, VWSignalEvent->channel);

            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control: check list(0x%p)->next(0x%p)\n",
                  ctrl->events, ctrl->events->next));
            if(ctrl->events->next)
            {
               sVWSignalEvent* pEvent = ctrl->events;
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control: assign new front of list(0x%p)\n",
                     ctrl->events->next));
               ctrl->events = ctrl->events->next;
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: free pEvent(0x%p)\n", pEvent));
               KernelVfree(pEvent);
            }
            else
            {
               sVWSignalEvent* pEvent = ctrl->events;
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control: clear front of list(0x%p)\n",
                     &ctrl->events));
               ctrl->events = 0;
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: free pEvent(0x%p)\n", pEvent));
               KernelVfree(pEvent);
            }

            KernelMutexUnlock(ctrl->pLock);

            rc = KernelCopyToUser(data, (char*)VWSignalEvent, count);
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: free VWSignalEvent(0x%p)\n", VWSignalEvent));
            KernelVfree(VWSignalEvent);
         }
         break;
      }
#ifdef RGB133_USER_MODE
      case VW_MAGIC_APP_EVENT:
      {
         //spin_lock_irqsave(&user_mode_lock, flags);
         spin_lock(&user_mode_lock);
         appEv = KernelVzalloc(sizeof(sAppEvent));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: APP - alloc %d bytes to appEv(0x%p)\n",
               (int)sizeof(sAppEvent), appEv));
         if(appEv == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: APP - failed to alloc %d bytes to appEv\n",
                  (int)sizeof(sVWBoardType)));
            return -ENOMEM;
         }
         rc = copy_from_user(appEv, data, sizeof(sAppEvent));

         dev = &rgb133_devices[appEv->id];
         //spin_unlock_irqrestore(&user_mode_lock, flags);
         spin_unlock(&user_mode_lock);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: APP - Read Event for device(%d)(0x%p)\n",
               appEv->id, dev));

         /* Get the data */
         //spin_lock_irqsave(&dev->appEvQ.lock, flags);

         if(dev->appEvQ.event == 0)
         {
            /* Define a q */
            DEFINE_WAIT(wait);

            /* Wait */
            prepare_to_wait(&dev->appEvQ.q, &wait, TASK_INTERRUPTIBLE);

            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: APP - Call schedule() wait on q(0x%p) for device(%d)\n",
                  &dev->appEvQ.q, appEv->id));
            //spin_unlock_irqrestore(&dev->appEvQ.lock, flags);
            if(dev->appEvQ.event == 0)
               schedule();
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control(%d): APP - Nearly missed an event(0x%p) for device[%d]\n",
                     appEv->id, dev->appEvQ.event, dev->index))
            }

            /* We got woken up */
            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control: APP - Call finish_wait(&dev->ev.q(0x%p), &wait(0x%p)) for device(%d)\n",
               &dev->appEvQ.q, &wait, appEv->id));
            finish_wait(&dev->appEvQ.q, &wait);
         }
         else
         {
            //spin_unlock_irqrestore(&dev->appEvQ.lock, flags);
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: APP - q(0x%p) already signalled for device(%d)(0x%p)\n",
                  &dev->appEvQ.q, appEv->id, dev));
         }

         /* Check and set the data */
         //spin_lock_irqsave(&dev->appEvQ.lock, flags);

         /* Have we been signalled to exit? */
         if(dev->appEvQ.exit)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: APP - device(%d)(0x%p) signalled to exit...\n",
                  appEv->id, dev));
            appEv->exit = 1;
            appEv->event = 0x0;
         }
         else
         {
            if(dev->appEvQ.event)
            {
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control(%d): APP - device(0x%p) assign command(0x%lx) to event(0x%p)\n",
                     appEv->id, dev, dev->appEvQ.event->command, dev->appEvQ.event));
               appEv->event     = dev->appEvQ.event->command;
               appEv->id        = dev->core.nr;
               appEv->exit      = 0;
               appEv->channel   = dev->appEvQ.event->channel;
               appEv->capture   = dev->appEvQ.event->capture;
               appEv->users     = dev->appEvQ.event->users;
               appEv->force     = dev->appEvQ.event->force;

               appEv->mdl_vaddr = dev->appEvQ.event->mdl_vaddr;
               appEv->mdl_size  = dev->appEvQ.event->mdl_size;

               appEv->offset    = dev->appEvQ.event->offset;

               appEv->wait_type = dev->appEvQ.event->wait_type;

               appEv->debug_mask = dev->appEvQ.event->debug_mask;

               CopyDeviceParms(&appEv->deviceParms, &dev->deviceParms);
               CopyClientParms(&appEv->clientParms, &dev->clientParms);

               if(dev->appEvQ.event->pData && dev->appEvQ.event->dataSize)
               {
                  RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control(%d): copy %lu bytes to user(0x%p) from pData(0x%p)\n",
                        appEv->id, dev->appEvQ.event->dataSize, appEv->pData, dev->appEvQ.event->pData));
                  rc = KernelCopyToUser(appEv->pData, (void*)dev->appEvQ.event->pData, dev->appEvQ.event->dataSize);
                  dev->appEvQ.event->pData = 0;
                  dev->appEvQ.event->dataSize = 0;
               }
               else if(dev->appEvQ.event->dataSize)
               {
                  appEv->dataSize = dev->appEvQ.event->dataSize;
               }

               {
                  struct rgb133_app_event* pLAppEv = dev->appEvQ.event;
                  dev->appEvQ.event = pLAppEv->next;
                  if(pLAppEv)
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control(%d): Pop pAppEv(0x%p) from front of list(0x%p), next appEv(0x%p)\n",
                           appEv->id, pLAppEv, &dev->appEvQ, dev->appEvQ.event));
                     memset(pLAppEv, 0, sizeof(*pLAppEv));
                  }
               }
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control(%d): APP - Bad event(0x%p) for device[%d]\n",
                     appEv->id, dev->appEvQ.event, dev->index));
            }
         }

         //spin_unlock_irqrestore(&dev->appEvQ.lock, flags);

         /* Send the code back to the user */
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_control: APP - Send data(0x%lx) and exit(0x%x) into user space for device(%d)\n",
               appEv->event, appEv->exit, appEv->id));
         rc = KernelCopyToUser(data, (char*)appEv, sizeof(*appEv));

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: APP - free appEv(0x%p)\n", appEv));
         KernelVfree(appEv);
         break;
      }
      case VW_MAGIC_IRQ_EVENT:
      {
         irqEv = KernelVzalloc(sizeof(sIrqEvent));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: IRQ - alloc %d bytes to irqEv(0x%p)\n",
               (int)sizeof(sIrqEvent), irqEv));
         if(irqEv == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_control: IRQ - failed to alloc %d bytes to irqEv\n",
                  (int)sizeof(sVWBoardType)));
            return -ENOMEM;
         }
         rc = copy_from_user(irqEv, data, sizeof(sIrqEvent));

         spin_lock_irqsave(&user_mode_lock, flags);
         dev = &rgb133_devices[irqEv->id];
         spin_unlock_irqrestore(&user_mode_lock, flags);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: IRQ - Read Event for device(%d)\n",
               irqEv->id));

         /* Get the data */
         //spin_lock_irqsave(&dev->irqEvQ.lock, flags);

         if(dev->irqEvQ.event == 0)
         {
            /* Define a q */
            DEFINE_WAIT(wait);

            /* Wait */
            prepare_to_wait(&dev->irqEvQ.q, &wait, TASK_INTERRUPTIBLE);

            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: IRQ - Call schedule() wait on q(0x%p) for device(%d)\n",
                  &dev->irqEvQ.q, irqEv->id));
            //spin_unlock_irqrestore(&dev->irqEvQ.lock, flags);
            if(dev->irqEvQ.event == 0)
               schedule();
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_read_control(%d): IRQ - Nearly missed an event(0x%p) for device[%d]\n",
                     irqEv->id, dev->irqEvQ.event, dev->index))
            }

            /* We got woken up */
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: IRQ - Call finish_wait(&dev->ev.q(0x%p), &wait(0x%p)) for device(%d)\n",
               &dev->irqEvQ.q, &wait, irqEv->id));
            finish_wait(&dev->irqEvQ.q, &wait);
         }
         else
         {
            //spin_unlock_irqrestore(&dev->irqEvQ.lock, flags);
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control: IRQ - q(0x%p) already signalled for device(%d)(0x%p)\n",
                  &dev->irqEvQ.q, irqEv->id, dev));
         }

         /* Check and set the data */
         //spin_lock_irqsave(&dev->irqEvQ.lock, flags);

         /* Have we been signalled to exit? */
         if(dev->irqEvQ.exit)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control:  IRQ - device(%d) signalled to exit...\n",
                  irqEv->id));
            irqEv->exit = 1;
            irqEv->event = 0x0;
         }
         else
         {
            if(dev->irqEvQ.event)
            {
               irqEv->event = dev->irqEvQ.event->flags;
               irqEv->id = dev->core.nr;
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control:  IRQ - device(%d) signal event(0x%lx) to device(%d)...\n",
                     irqEv->id, irqEv->event, irqEv->id));

               /* Auto-clear the event and flags */
               /* TODO: multiple interrupts */
               dev->irqEvQ.event->flags = 0x0;

               {
                  struct rgb133_irq_event* pLIrqEv = dev->irqEvQ.event;
                  dev->irqEvQ.event = pLIrqEv->next;
                  if(pLIrqEv)
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read_control(%d): deactivate pIrqEv(0x%p)\n",
                           irqEv->id, pLIrqEv));
                     pLIrqEv->active = 0;
                  }
               }
            }
         }

         spin_unlock_irqrestore(&dev->irqEvQ.lock, dev->irqEvQ.flags);

         /* Send the code back to the user */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read_control: IRQ - Send data(0x%lx) and exit(0x%x) into user space for device(0x%x)\n",
               irqEv->event, irqEv->exit, irqEv->id));
         rc = KernelCopyToUser(data, (char*)irqEv, sizeof(*irqEv));

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_control: IRQ - free irqEv(0x%p)\n", irqEv));
         KernelVfree(irqEv);
         break;
      }
#endif /* RGB133_USER_MODE */
      default:
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_read_control: Invalid Read(%d)(%d)\n", read_magic, (int)count));
         msleep(100);
         return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_read_control: END - count(%d)\n", (int)count));
   return count;
}

int rgb133_s_parm(struct file* file, void* priv, struct v4l2_streamparm* strm);

ssize_t rgb133_read(struct file* file, char __user* data, size_t count, loff_t *ppos)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   int rc = -EINVAL;

   if(count == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read: [%d][%d][%d] - bad request for 0 bytes into buffer(0x%p)!\n",
            h->dev->index, h->channel, h->capture, data));
      return count;
   }

   if(DeviceIsControl(dev->control))
   {
      return rgb133_read_control(file, data, count);
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_read: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_read: START [%d][%d][%d] - request for %d bytes into buffer(0x%p)\n",
         h->dev->index, h->channel, h->capture, (int)count, data));

   if(rgb133_is_mapped(h)) /* Some apps using mmap io "read" after setting new output,
                           ** we can't allow this as the buffer isn't setup correctly  */
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read: dev[%d].channel[%d].capture[%d] is mmapped\n",
            dev->index, h->channel, h->capture));
      return count;  /* buffer size */
   }
   else
   {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
      if(!access_ok(data, count))
#else
      if(!access_ok(VERIFY_WRITE, data, count))
#endif
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read: bad user data pointer (0x%p) for dev[%d].channel[%d].capture[%d]\n",
                  data, dev->index, h->channel, h->capture));
         return count;
      }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
      else if(!access_ok(data, count))
#else
      else if(!access_ok(VERIFY_READ, data, count))
#endif
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read: bad user data pointer (0x%p) for dev[%d].channel[%d].capture[%d]\n",
                  data, dev->index, h->channel, h->capture));
         return count;
      }
   }

   if(!h->pNoSignalBufIn)
   {
      h->pNoSignalBufIn = KernelVzalloc(count);
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read: [%d][%d][%d] alloc %d bytes to pNoSignalBufIn(0x%p)\n",
            dev->index, h->channel, h->capture, (int)count, h->pNoSignalBufIn));

      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read: [%d][%d][%d] init read io buffer with RED\n",
            dev->index, h->channel, h->capture));
      CaptureInitBuffer(h, 0, h->pNoSignalBufIn, count, h->sCapture.pixelformat, RGB133_RED, RGB133_CAP_NO_COPY_USER);
   }

   /* Init the output buffer */
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_read: [%d][%d][%d] copy %d bytes from pNoSignalBufIn(0x%p) to user data(0x%p)\n",
         dev->index, h->channel, h->capture, (int)count, h->pNoSignalBufIn, data));
   KernelCopyToUser(data, h->pNoSignalBufIn, count);

   /* Actual buffer IO */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_read: [%d][%d][%d] get %d bytes for video_buffer, user buffer(0x%p)\n",
         dev->index, h->channel, h->capture, (int)count, data));

   h->EnterCriticalSection(h);
   count = APIRequestAndWaitForData(h, data, count);
   h->ExitCriticalSection(h);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_read: [%d][%d][%d] return count(%d) bytes\n",
         dev->index, h->channel, h->capture, (int)count));

   if(rgb133_show_fps == RGB133_SHOW_FPS)
   {
      h->fps = ReportFPS(h);
   }

   return count;
}

ssize_t rgb133_write(struct file* file, const char __user* data, size_t count, loff_t *ppos)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;

   struct _sVWMagic VWMagic;
   struct _sVWDevice* VWDevice = 0;
   struct _sVWWriteFlash* VWWriteFlash = 0;
   struct _sVWDebugLevel* VWDebugLevel = 0;
   int write_magic = -1;
   int offset = -1;
   int rc = -EINVAL;

#ifdef RGB133_USER_MODE
   PRGB133_USER_DMA          pUser_dma = 0;
   struct _sVWUserInit       userInit;
   struct _sVWDeviceInfo     deviceInfo;
   struct _sVWAllDeviceParms* deviceParms;
   struct _sVWClientParms*    clientParms;
   struct _sVWDeviceInitInfo deviceInitInfo;
   struct _sControl*         pCtrl;
   struct _sImageData*       pImageData;
   struct rgb133_unified_buffer*   q = 0;

   int i;
#endif /* RGB133_USER_MODE */

   if(!DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_write: Device is not control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_write: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: check size(%d)\n", (int)count));

   /* Get the magic number */
   rc = copy_from_user((char*)&VWMagic, data, sizeof(VWMagic));
   write_magic = VWMagic.magic;

#ifdef RGB133_USER_MODE
   /* Is the user mode service up and running */
   if(!h->dev->userInit.init &&
      write_magic != VW_MAGIC_INIT_USER)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: User Mode service is not running\n"));
      return -EDEADLK;
   }
#endif /* RGB133_USER_MODE */

   switch(write_magic)
   {
#ifdef RGB133_USER_MODE
      case VW_MAGIC_CONTROL:
         pCtrl = KernelVzalloc(count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to pCtrl(0x%p)\n",
               (int)count, pCtrl));
         if(pCtrl == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to pCtrl\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user((char*)pCtrl, data, count);

         //spin_lock_irqsave(&rgb133_devices[pCtrl->device].ctrlEv.lock, flags);
         //spin_lock_irqsave(&user_mode_lock, flags);
         spin_lock(&user_mode_lock);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Returning status(%d) of command(0x%x) for device(%d)\n",
               pCtrl->status, pCtrl->device, pCtrl->command));
         rgb133_devices[pCtrl->device].ctrlEv.signalled              = 1;
         rgb133_devices[pCtrl->device].ctrl.status                   = pCtrl->status;
         rgb133_devices[pCtrl->device].ctrl.init                     = pCtrl->init;
         rgb133_devices[pCtrl->device].ctrl.flags                    = pCtrl->flags;
         rgb133_devices[pCtrl->device].ctrl.value                    = pCtrl->value;
         rgb133_devices[pCtrl->device].ctrl.type                     = pCtrl->type;

         //spin_unlock_irqrestore(&rgb133_devices[pCtrl->device].ctrlEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
               &rgb133_devices[pCtrl->device].ctrlEv.q, pCtrl->device));
         wake_up(&rgb133_devices[pCtrl->device].ctrlEv.q);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free pCtrl(0x%p)\n", pCtrl));
         KernelVfree(pCtrl);
         pCtrl = 0;
         //spin_unlock_irqrestore(&user_mode_lock, flags);
         spin_unlock(&user_mode_lock);
         break;
      case VW_MAGIC_USER_DMA:
         pUser_dma = KernelVzalloc(sizeof(RGB133_USER_DMA));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to pUser_dma(0x%p)\n",
               (int)sizeof(RGB133_USER_DMA), pUser_dma));
         if(pUser_dma == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to pUser_dma\n",
                  (int)count));
            return -ENOMEM;
         }

         rc = copy_from_user((char*)pUser_dma, data, count);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Handle USER DMA for device(%d) on channel[%d].capture[%d]\n",
               pUser_dma->index, pUser_dma->channel, pUser_dma->capture));
         {
            RGB133_KERNEL_DMA* kernel_dma;
            struct pci_dev* pdev = 0;
            dev = &rgb133_devices[pUser_dma->index];
            pdev = dev->core.pci;
            q = CaptureGetQPtr(dev, pUser_dma->channel, pUser_dma->capture);
            if(rgb133_buffer_q_reading(q))
            {
               kernel_dma = rgb133_read_buffer_get_kernel_dma(q);
            }
            else
            {
               kernel_dma = rgb133_stream_buffer_get_kernel_dma(q);
            }

            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: Called - dev[%d](0x%p), pdev(0x%p)\n",
                  pUser_dma->index, dev, pdev));

            /* Map SG List */
            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: Map SG - pdev(0x%p) - SGList(0x%p) - page_count(%d)\n",
               pdev, kernel_dma->pSGList, kernel_dma->page_count));
            kernel_dma->SG_length = pci_map_sg(pdev, kernel_dma->pSGList, kernel_dma->page_count, PCI_DMA_FROMDEVICE);
            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: Map SG - Length(%d)\n",
                  kernel_dma->SG_length));

            if(kernel_dma->SG_length > 0)
            {
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: Fill SG Array - user_dma(0x%p) - data(0x%p) - length(%d)\n",
                     kernel_dma, pUser_dma->mdl_vaddr, kernel_dma->SG_length));
               rgb133_kernel_dma_fill_sg_array(kernel_dma, (unsigned long)pUser_dma->mdl_vaddr, 0, -1);
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: Failed to Fill SG Array - user_dma(0x%p) - data(0x%p)\n",
                     kernel_dma, pUser_dma->mdl_vaddr));
            }

            /* Sync the DMA */
            rgb133_kernel_dma_sync_for_device(pdev, kernel_dma->pSGList, kernel_dma->page_count);

            pUser_dma->page_count = kernel_dma->page_count;
            pUser_dma->SG_handle = kernel_dma->SG_handle;
            pUser_dma->SG_length = kernel_dma->SG_length;

            for(i=0; i<kernel_dma->page_count; i++)
            {
               pUser_dma->SGArray[i].src = kernel_dma->pSGArray[i].src;
               pUser_dma->SGArray[i].dst = kernel_dma->pSGArray[i].dst;
               pUser_dma->SGArray[i].size = kernel_dma->pSGArray[i].size;
            }

            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Copy %d bytes from 0x%p back to the user(0x%p)\n",
                  (int)count, pUser_dma, data));
            rc = KernelCopyToUser((void *)data, (void *)pUser_dma, count);
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free pUser_dma(0x%p)\n", pUser_dma));
            KernelVfree(pUser_dma);
         }
         break;
      case VW_MAGIC_DEVICE_PARMS:
      {
         deviceParms = KernelVzalloc(sizeof(*deviceParms));
         rc = copy_from_user((char*)deviceParms, data, count);
         //spin_lock_irqsave(&rgb133_devices[deviceParms.device].parmsEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Got device params for device(%d)\n", deviceParms->device));
         rgb133_devices[deviceParms->device].parmsEv.signalled = 1;

         CopyDeviceParms(&rgb133_devices[deviceParms->device].deviceParms, deviceParms);

         //spin_unlock_irqrestore(&rgb133_devices[deviceParms.device].parmsEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
               &rgb133_devices[deviceParms->device].parmsEv.q, deviceParms->device));
         wake_up(&rgb133_devices[deviceParms->device].parmsEv.q);
         KernelVfree(deviceParms);
         break;
      }
      case VW_MAGIC_CLIENT_PARMS:
      {
         clientParms = KernelVzalloc(sizeof(*clientParms));
         rc = copy_from_user((char*)clientParms, data, count);
         //spin_lock_irqsave(&rgb133_devices[clientParms->device].parmsEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Got client params for device(%d)\n", clientParms->device));
         rgb133_devices[clientParms->device].parmsEv.signalled = 1;

         CopyClientParms(&rgb133_devices[clientParms->device].clientParms, clientParms);

         //spin_unlock_irqrestore(&rgb133_devices[clientParms->device].parmsEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
               &rgb133_devices[clientParms->device].parmsEv.q, clientParms->device));
         wake_up(&rgb133_devices[clientParms->device].parmsEv.q);
         KernelVfree(clientParms);
         break;
      }
      case VW_MAGIC_DEVICE_INFO:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Write %d bytes of user device info(0x%p) into deviceInfo(0x%p)\n",
               (int)sizeof(sVWDeviceInfo), data, &deviceInfo));
         rc = copy_from_user((char*)&deviceInfo, data, count);

         dev = &rgb133_devices[deviceInfo.device];
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: Using dev(0x%p)\n", dev));

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: Set device(%d)->channels(%d)\n",
               deviceInfo.device, deviceInfo.channels));
         dev->channels = deviceInfo.channels;
         break;
      case VW_MAGIC_DEVICE_INFO_INT:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Write %d bytes of user device info(0x%p) into deviceInfo(0x%p)\n",
               (int)sizeof(sVWDeviceInfo), data, &deviceInfo));
         rc = copy_from_user((char*)&deviceInfo, data, count);
         //spin_lock_irqsave(&rgb133_devices[deviceInfo.device].infoEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Got device info int for device(%d)\n", deviceInfo.device));

         dev = &rgb133_devices[deviceInfo.device];
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: Using dev(0x%p)\n", dev));

         CopyInfo(&dev->deviceInfo, &deviceInfo);

         //spin_unlock_irqrestore(&rgb133_devices[deviceInfo.device].infoEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
               &rgb133_devices[deviceInfo.device].infoEv.q, deviceInfo.device));
         wake_up(&rgb133_devices[deviceInfo.device].infoEv.q);
         break;
      case VW_MAGIC_DEVICE_INIT_INFO:
         rc = copy_from_user((char*)&deviceInitInfo, data, count);
         //spin_lock_irqsave(&user_mode_lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: Set device(%d)->user_mode_init(%d)\n",
               deviceInitInfo.device, deviceInitInfo.init));
         rgb133_devices[deviceInitInfo.device].userInit.init = deviceInitInfo.init;
         //spin_unlock_irqrestore(&user_mode_lock, flags);
         break;
      case VW_MAGIC_INIT_USER:
         rc = copy_from_user((char*)&userInit, data, count);
         //spin_lock_irqsave(&user_mode_lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: Set control device init(%d)\n",
               userInit.init));
         rgb133_devices[RGB133_CONTROL_DEVICE_NUM].userInit.init = userInit.init;
         //spin_unlock_irqrestore(&user_mode_lock, flags);
         break;
      case VW_MAGIC_IMAGE_DATA:
      {
         pImageData = KernelVzalloc(count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to pImageData(0x%p)\n",
               (int)count, pImageData));
         if(pImageData == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to pImageData\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user((char*)pImageData, data, count);
         //spin_lock_irqsave(&rgb133_devices[pImageData->device].imageDataEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Got image data for device(%d)\n", pImageData->device));
         if(pImageData->pData && pImageData->dataSize)
         {
            rgb133_devices[pImageData->device].imageData.pData = KernelVzalloc(pImageData->dataSize);
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to pData(0x%p)\n",
                  pImageData->dataSize, rgb133_devices[pImageData->device].imageData.pData));
            if(rgb133_devices[pImageData->device].imageData.pData == 0)
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to pData\n",
                     (int)pImageData->dataSize));
               rgb133_devices[pImageData->device].imageDataEv.signalled = 1;
               rgb133_devices[pImageData->device].imageData.status      = 997;
               rgb133_devices[pImageData->device].imageDataEv.signalled = 1;
               //spin_unlock_irqrestore(&rgb133_devices[pImageData->device].imageDataEv.lock, flags);
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
                     &rgb133_devices[pImageData->device].imageDataEv.q, pImageData->device));
               wake_up(&rgb133_devices[pImageData->device].imageDataEv.q);
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free pImageData(0x%p)\n", pImageData));
               KernelVfree(pImageData);
               pImageData = 0;
               return -ENOMEM;
            }
         }

         //spin_lock_irqsave(&rgb133_devices[pImageData->device].imageDataEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Returning status(%d) for device(%d)\n",
               pImageData->status, pImageData->device));
         //rgb133_devices[pImageData->device].imageDataEv.signalled = 1;
         rgb133_devices[pImageData->device].imageData.status      = pImageData->status;

         if(pImageData->pData && pImageData->dataSize &&
            rgb133_devices[pImageData->device].imageData.pData)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: copying %d bytes of image data from 0x%p to 0x%p\n",
                  pImageData->dataSize, pImageData->pData,
                  rgb133_devices[pImageData->device].imageData.pData));
            memcpy(rgb133_devices[pImageData->device].imageData.pData, pImageData->pData, pImageData->dataSize);
            rgb133_devices[pImageData->device].imageData.dataSize = pImageData->dataSize;
         }

         rgb133_devices[pImageData->device].imageDataEv.signalled = 1;
         //spin_unlock_irqrestore(&rgb133_devices[pImageData->device].imageDataEv.lock, flags);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Wake up q(0x%p) on device(%d)\n",
               &rgb133_devices[pImageData->device].imageDataEv.q, pImageData->device));
         wake_up(&rgb133_devices[pImageData->device].imageDataEv.q);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free pImageData(0x%p)\n", pImageData));
         KernelVfree(pImageData);
         pImageData = 0;
         break;
      }
#endif /* RGB133_USER_MODE */
      case VW_MAGIC_SET_DEVICE_PARMS:
      {
         // Get the current values
         VWDevice = KernelVzalloc(sizeof(sVWDevice));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to VWDevice(0x%p)\n",
               (int)sizeof(sVWDevice), VWDevice));
         if(VWDevice == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to VWDevice\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user((char*)VWDevice, data, count);

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWDevice->device];
            offset = VWDevice->input;
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWDevice->device);
         }

         // Fetch all the data for the device
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Write parms to dev[%d].channel[%d].client[%d], flags(0x%lx)\n",
               dev->index, offset, VWDevice->client, VWDevice->flags));

         CaptureSetDeviceParameters(dev, offset, VWDevice->client,
               &VWDevice->inputs[VWDevice->input].curDeviceParms);

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free VWDevice(0x%p)\n", VWDevice));
         KernelVfree(VWDevice);
         break;
      }
      case VW_MAGIC_WRITE_FLASH:
      {
         // Get the current values
         VWWriteFlash = KernelVzalloc(count);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to VWWriteFlash(0x%p)\n",
               (int)count, VWWriteFlash));
         if(VWWriteFlash == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to VWWriteFlash\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user((char*)VWWriteFlash, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: Write flash image into kernel for raw device(%d)\n",
               VWWriteFlash->device));

         if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
         {
            dev = &rgb133_devices[VWWriteFlash->device];
            offset = 0;
         }
         else
         {
            ReadControlMatchDevice(&dev, &offset, VWWriteFlash->device);
         }

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: resolved dev[%d].channel[%d]\n",
               dev->index, offset));

         if(offset == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: flash dev[%d].channel[%d]\n"));

            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_write: data(0x%p), size(%u), version(0x%x)\n",
                  &VWWriteFlash->data[0], VWWriteFlash->dataSize, VWWriteFlash->version));

#if 0
            {
               int i = 0;
               for(i=0; i<32; i++)
               {
                  if(!(i%16))
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_write: imageData(%p): 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
                           &VWWriteFlash->data[0],
                           VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++],
                           VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++],
                           VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++],
                           VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i++], VWWriteFlash->data[i]));
                  }
               }
            }
#endif

            CaptureWriteFlashImage(dev, &VWWriteFlash->data[0], VWWriteFlash->dataSize, VWWriteFlash->version);

            VWWriteFlash->completed = 1;
            VWWriteFlash->status = 0;

            rc = KernelCopyToUser((void *)data, (void *)VWWriteFlash, sizeof(sVWWriteFlash));
            count = sizeof(sVWWriteFlash);
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_write: skip write flash for dev[%d].channel[%d]\n",
                  dev->index, offset));
         }

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free VWWriteFlash(0x%p)\n", VWWriteFlash));
         KernelVfree(VWWriteFlash);
         break;
      }
      case VW_MAGIC_DEBUG_LEVEL:
      {
         // Get the current values
         VWDebugLevel = KernelVzalloc(sizeof(sVWDebugLevel));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: alloc %d bytes to VWDebugLevel(0x%p)\n",
               (int)sizeof(sVWDebugLevel), VWDebugLevel));
         if(VWDebugLevel == 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: failed to alloc %d bytes to VWDebugLevel\n",
                  (int)count));
            return -ENOMEM;
         }
         rc = copy_from_user((char*)VWDebugLevel, data, count);

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_write: set debug level(%d)\n", VWDebugLevel->level));
         CaptureSetDebugLevel(h, VWDebugLevel->level);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_write: free VWDebugLevel(0x%p)\n", VWDebugLevel));
         KernelVfree(VWDebugLevel);
         break;
      }
      default:
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_write: Invalid write(%d)(%d)\n", write_magic, (int)count));
         break;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_write: return count(%d)\n", (int)count));
   return count;
}

void QueueActiveDMABuffers(struct rgb133_handle* h, int lock_action)
{
   int i = -1;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "QueueActiveDMABuffers: START\n"));

   h->AcquireSpinLock(h, "QueueActiveDMABuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if(h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_ACTIVE)
            {
               RGB133PRINT((RGB133_LINUX_DBG_INOUT, "QueueActiveDMABuffers: Queue active buffer h->buffers[%d]\n", i));
               h->buffers[i]->state = RGB133_BUF_QUEUED;
               h->buffers[i]->notify = RGB133_NOTIFY_RESERVED;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "QueueActiveDMABuffers: h(0x%p)->buffers is NULL\n", h));
   }

   h->ReleaseSpinLock(h, "QueueActiveDMABuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "QueueActiveDMABuffers: END\n"));
}

unsigned int rgb133_poll(struct file* file, struct poll_table_struct* wait)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   int mask = 0;
   int i = 0, j = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_poll: START - [%d][%d][%d]\n",
         dev->index, h->channel, h->capture));

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_poll: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   if(!dev->control)
   {
      if(rgb133_is_mapped(h))
      {
         /* Polling only makes sense to us if we're streaming */
         if (CountActiveOrDoneDMABuffers(h, 0) > 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_poll: [%d][%d][%d] call CaptureWaitForMultiBufferEvent...\n",
                  dev->index, h->channel, h->capture));
            if(CaptureWaitForMultiBufferEvent(h) != STATUS_SUCCESS)
            {
               // We have waited long enough. If there is an active buffer, it needs to go back to queued
               QueueActiveDMABuffers(h, 1);
               DumpBufferStates(RGB133_LINUX_DBG_ERROR, h, "poll(Err)");
            }
            RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_poll: [%d][%d][%d] Waited for MultiBufferEvent\n",
                  dev->index, h->channel, h->capture));
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_poll: No active buffers on [%d][%d][%d]\n",
                  dev->index, h->channel, h->capture));
         }
      }
      mask = (POLLIN|POLLRDNORM);
   }
   else
   {
#ifndef RGB133_USER_MODE
      while(rgb133_devices[i].pDE)
      {
         for(j = 0; j < DeviceGetChannels(&rgb133_devices[i], 0); j++ )
         {
            POSEVENTAPI pEvent = DeviceGetSignalNotificationEvent(rgb133_devices[i].pDE, j);
            poll_wait(file, (wait_queue_head_t*)pEvent, wait);

            if(DeviceSignalNotificationEventSignalled(rgb133_devices[i].pDE, j))
            {
               sVWSignalEvent *pEvent = KernelVzalloc(sizeof(sVWSignalEvent));
               sVWSignalEvent *pList = dev->events;

               // Add the device and channel number into the list
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_poll: alloc %d bytes to pEvent(0x%p) add device[%d].channel[%d]\n",
                     sizeof(sVWSignalEvent), pEvent, i, j));
               pEvent->device = i;
               pEvent->channel = j;
               pEvent->next = 0;

               KernelMutexLock(dev->pLock);
               if(pList)
               {
                  while(pList->next) { pList = pList->next; }
                  RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_poll: add pEvent(0x%p) to back of list(0x%p)\n",
                        pEvent, pList));
                  pList->next = pEvent;
               }
               else
               {
                  RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_poll: add pEvent(0x%p) to front list(0x%p)\n",
                        pEvent, &dev->events));
                  dev->events = pEvent;
               }
               KernelMutexUnlock(dev->pLock);

               mask |= (POLLIN|POLLRDNORM);
            }
         }
         i++;
      }
#endif
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_poll: END - [%d][%d][%d] mask(0x%x)\n",
         dev->index, h->channel, h->capture, mask));
   return mask;
}

int rgb133_mmap(struct file* file, struct vm_area_struct* vma)
{
   struct rgb133_handle* h = file->private_data;

   unsigned long vm_base = vma->vm_start;
   unsigned long vm_size = vma->vm_end - vma->vm_start;

#ifdef RGB133_USER_MODE
   struct rgb133_dev* dev = h->dev;
   unsigned long pci_base = 0;
   int dev_num = -1;
#endif /* RGB133_USER_MODE */

   int retval = -ENOMEM;

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_mmap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_mmap: START - base(0x%lx), size(0x%lx)(%lu)\n",
      vm_base, vm_size, vm_size));

#ifdef RGB133_USER_MODE
   if(dev->control)
   {
      {
         //spin_lock_irqsave(&user_mode_lock, flags);
         spin_lock(&user_mode_lock);
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: device(%d)\n", mmap_dev_num));
         dev_num = mmap_dev_num++;
         spin_unlock(&user_mode_lock);
         pci_base = (unsigned long)rgb133_devices[dev_num].core.pci_base;
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: vm_base(0x%lx), vm_size(%lu)\n",
               vm_base, vm_size));
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: Map PCI Address space -> 0x%lx\n",
               pci_base));

         if(io_remap_pfn_range(vma, vm_base,
                               (pci_base >> PAGE_SHIFT),
                               vm_size, vma->vm_page_prot))
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap: io_remap_page_range failed\n"));
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_mmap: END\n"));
            return -EAGAIN;
         }

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: io_remap_page_range succeeded\n"));
         vma->vm_flags |= VM_IO | VM_RESERVED;

         RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_mmap: END\n"));
         return 0;
      }
   }
#endif /* RGB133_USER_MODE */

   h->EnterCriticalSection(h);

   /* Bail if we've been here before for this buffer! */
   {
      unsigned long size, offset;

      size = vma->vm_end - vma->vm_start;
      offset = (vma->vm_pgoff * PAGE_SIZE) / size;

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: buffer (%d), size(%u), vma->vm_pgoff(%u), page_size(%u)\n",
            offset, size, vma->vm_pgoff, PAGE_SIZE));
      if(rgb133_is_mapped_buffer(h->buffers[offset]))
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: buffer %d is mmapped already, IGNORE\n", offset));
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap: vma(0x%lx - 0x%lx), IGNORE\n",
            vma->vm_start, vma->vm_end));
         h->ExitCriticalSection(h);
         return 0;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_mmap: type(%s), base(0x%lx), size(0x%lx)(%lu)\n",
      v4l2_type_names[h->buffertype], vm_base, vm_size, vm_size));

   if(!(vma->vm_flags & VM_SHARED))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap: must have VM_SHARED(0x%x) in flags(0x%lx)\n",
         VM_SHARED, vma->vm_flags));
      goto out;
   }

   if((retval = __rgb133_buffer_mmap_mapper(h, vma)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap: __rgb133_buffer_mmap_mapper failed: %d\n",
            retval));
      goto out;
   }

   rgb133_set_mapped(h, 1);

   rgb133_vm_open(vma);

out:
   h->ExitCriticalSection(h);
   return retval;
}

/* Linux Kernel Module Ifc */

/* Exit the driver, and clean up */
static int rgb133_release(void)
{
   return 0;
}

#ifndef RGB133_USER_MODE
/* Dummy Windows SRB so we can keep the interface */
static PORT_CONFIGURATION_INFORMATION rgb133_configInfo[RGB133_MAX_DEVICES] = {
};

static HW_STREAM_REQUEST_BLOCK Srb[RGB133_MAX_DEVICES] = {
};

/* Dummy access ranges (we'll get this when we test with
 * real hardware!!
 */
static ACCESS_RANGE DummyAccessRange[RGB133_MAX_DEVICES] = {
};

/* Dummy Device Object
 */
static DEVICE_OBJECT rgb133_deviceObject[RGB133_MAX_DEVICES] = {
};

/* Dummy Interrupt Object */
static KINTERRUPT rgb133_interruptObject[RGB133_MAX_DEVICES];
#endif /* RGB133_USER_MODE */

int SetOverrideDevices(struct pci_dev* pdev, PDEAPI pDE)
{
   int override_devices = -1;

   switch(pdev->device)
   {
#ifdef INCLUDE_VISION
      case DGC133_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC133_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-X2 */
            case DGC139_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E1 */
            case DGC150_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E2 */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC139_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC1139_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E1S */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC144_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC144_PCI_SUBSYS_DEVICE_ID: /* VisionSD8 */
               override_devices = 8;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC150_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC1150_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E2S */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC151_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC151_PCI_SUBSYS_DEVICE_ID: /* VisionSD4+1 */
            case DGC1151_PCI_SUBSYS_DEVICE_ID: /* VisionSD4+1S */
               if(DeviceIsInputOrdinalZero(pDE))
                  override_devices = 1;
               else
                  override_devices = 4;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC153_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC1153_PCI_SUBSYS_DEVICE_ID: /* VisionDVI-DL */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC154_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC1154_PCI_SUBSYS_DEVICE_ID: /* VisionSDI2 */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC159_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC159_PCI_SUBSYS_DEVICE_ID: /* VisionAV */
               override_devices = 2;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC161_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC161_PCI_SUBSYS_DEVICE_ID: /* VisionHD4 */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC165_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC165_PCI_SUBSYS_DEVICE_ID: /* DGC165 */
               if(DeviceIsInputOrdinalZero(pDE))
                  override_devices = 2;
               else
                  override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC167_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC167_PCI_SUBSYS_DEVICE_ID: /* VisionAV-SDI */
               if(DeviceIsInputOrdinalZero(pDE))
                  override_devices = 2;
               else
                  override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC168_PCI_DEVICE_ID:
         switch(pdev->subsystem_device)
         {
            case DGC168_PCI_SUBSYS_DEVICE_ID: /* VisionAV-HD */
               if(DeviceIsInputOrdinalZero(pDE))
                  override_devices = 2;
               else
                  override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC179_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC179_PCI_SUBSYS_DEVICE_ID: /* VisionSC-DP2 */
               override_devices = 2;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC182_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC182_PCI_SUBSYS_DEVICE_ID: /* VisionSC-HD4+ */
               override_devices = 4;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC184_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC184_PCI_SUBSYS_DEVICE_ID: /* VisionSC-SDI4 */
               override_devices = 4;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC214_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
         case DGC214_PCI_SUBSYS_DEVICE_ID: /* VisionSC-UHD2 */
            override_devices = 2;
            break;
         default:
            override_devices = -1;
            break;
         }
         break;
      case DGC224_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
         case DGC224_PCI_SUBSYS_DEVICE_ID: /* VisionSC-S2 */
            override_devices = 2;
            break;
         default:
            override_devices = -1;
            break;
         }
         break;
#endif
#ifdef INCLUDE_VISIONLC
      case DGC186_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC186_PCI_SUBSYS_DEVICE_ID: /* VisionEC-SDI */
               override_devices = 4;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC199_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC199_PCI_SUBSYS_DEVICE_ID: /* VisionEC-HD */
               override_devices = 4;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC200_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC200_PCI_SUBSYS_DEVICE_ID: /* VisionLC-HD */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC204_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC204_PCI_SUBSYS_DEVICE_ID: /* VisionLC-SDI */
               override_devices = 1;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
      case DGC205_PCI_DEVICE_ID:
         switch (pdev->subsystem_device)
         {
            case DGC205_PCI_SUBSYS_DEVICE_ID: /* VisionLC-HD2 */
               override_devices = 2;
               break;
            default:
               override_devices = -1;
               break;
         }
         break;
#endif
      default:
         override_devices = -1;
         break;
   }

   if(rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS &&
      override_devices != -1)
      override_devices = 1;

   return override_devices;
}

#ifdef INCLUDE_VISION
const char* _DGC139   = "VisionRGB-E1";
const char* _DGC139S  = "VisionRGB-E1S";
const char* _DGC150   = "VisionRGB-E2";
const char* _DGC150S  = "VisionRGB-E2S";
const char* _DGC133   = "VisionRGB-X2";
const char* _DGC144   = "VisionSD8";
const char* _DGC151   = "VisionSD4+1";
const char* _DGC151S  = "VisionSD4+1S";
const char* _DGC153   = "VisionDVI-DL";
const char* _DGC154   = "VisionSDI2";
const char* _DGC159   = "VisionAV";
const char* _DGC161   = "VisionHD4";
const char* _DGC165   = "DGC165";
const char* _DGC167   = "VisionAV-SDI";
const char* _DGC168   = "VisionAV-HD";
const char* _DGC179   = "VisionSC-DP2";
const char* _DGC182   = "VisionSC-HD4+";
const char* _DGC184   = "VisionSC-SDI4";
const char* _DGC214   = "VisionSC-UHD2";
const char* _DGC224   = "VisionSC-S2";
#endif
#ifdef INCLUDE_VISIONLC
const char* _DGC186   = "VisionEC-SDI";
const char* _DGC199   = "VisionEC-HD";
const char* _DGC200   = "VisionLC-HD";
const char* _DGC204   = "VisionLC-SDI";
const char* _DGC205   = "VisionLC-HD2";
#endif

void SetCoreDeviceName(struct pci_dev* pdev, struct rgb133_dev* dev)
{
   int channel = 0;
   for(channel=0; channel<RGB133_MAX_CHANNEL_PER_CARD; channel++)
   {

      switch(pdev->device)
      {
#ifdef INCLUDE_VISION
         case DGC133_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC133_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-X2 */
                  sprintf(dev->core.name,"%s", _DGC133);
                  dev->core.type = DGC133;
                  break;
               case DGC139_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E1 */
                  sprintf(dev->core.name,"%s", _DGC139);
                  dev->core.type = DGC139;
                  break;
               case DGC150_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E2 */
                  sprintf(dev->core.name,"%s", _DGC150);
                  dev->core.type = DGC150;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC133");
                  break;
            }
            break;
         case DGC139_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC1139_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E1S */
                  sprintf(dev->core.name,"%s", _DGC139S);
                  dev->core.type = DGC139S;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC139");
                  break;
            }
            break;
         case DGC144_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC144_PCI_SUBSYS_DEVICE_ID: /* VisionSD8 */
                  sprintf(dev->core.name,"%s", _DGC144);
                  dev->core.type = DGC144;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC144");
                  break;
            }
            break;
         case DGC150_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC1150_PCI_SUBSYS_DEVICE_ID: /* VisionRGB-E2S */
                  sprintf(dev->core.name,"%s", _DGC150S);
                  dev->core.type = DGC150S;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC150");
                  break;
            }
            break;
         case DGC151_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC151_PCI_SUBSYS_DEVICE_ID: /* VisionSD4+1 */
                  sprintf(dev->core.name,"%s", _DGC151);
                  dev->core.type = DGC151;
                  break;
               case DGC1151_PCI_SUBSYS_DEVICE_ID: /* VisionSD4+1S */
                  sprintf(dev->core.name,"%s", _DGC151S);
                  dev->core.type = DGC151S;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC151");
                  break;
            }
            break;
         case DGC153_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC1153_PCI_SUBSYS_DEVICE_ID: /* VisionDVI-DL */
                  sprintf(dev->core.name,"%s", _DGC153);
                  dev->core.type = DGC153;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC153");
                  break;
            }
            break;
         case DGC154_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC1154_PCI_SUBSYS_DEVICE_ID: /* VisionSDI2 */
                  sprintf(dev->core.name,"%s", _DGC154);
                  dev->core.type = DGC154;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC154");
                  break;
            }
            break;
         case DGC159_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC159_PCI_SUBSYS_DEVICE_ID: /* VisionAV */
                  sprintf(dev->core.name,"%s", _DGC159);
                  dev->core.type = DGC159;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC159");
                  break;
            }
            break;
         case DGC161_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC161_PCI_SUBSYS_DEVICE_ID: /* VisionHD4 */
                  sprintf(dev->core.name,"%s", _DGC161);
                  dev->core.type = DGC161;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC161");
                  break;
            }
            break;
         case DGC165_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC165_PCI_SUBSYS_DEVICE_ID: /* DGC165 */
                  sprintf(dev->core.name,"%s", _DGC165);
                  dev->core.type = DGC165;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC165");
                  break;
            }
            break;
         case DGC167_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC167_PCI_SUBSYS_DEVICE_ID: /* VisionAV-SDI */
                  sprintf(dev->core.name,"%s", _DGC167);
                  dev->core.type = DGC167;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC167");
                  break;
            }
            break;
         case DGC168_PCI_DEVICE_ID:
            switch(pdev->subsystem_device)
            {
               case DGC168_PCI_SUBSYS_DEVICE_ID: /* VisionAV-HD */
                  sprintf(dev->core.name,"%s", _DGC168);
                  dev->core.type = DGC168;
                  break;
               default:
                  sprintf(dev->core.name,"UNKNOWN DGC168");
                  break;
            }
            break;
         case DGC179_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC179_PCI_SUBSYS_DEVICE_ID: /* VisionSC-DP2 */
                  sprintf(dev->core.name, "%s", _DGC179);
                  dev->core.type = DGC179;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC179");
                  break;
            }
            break;
         case DGC182_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC182_PCI_SUBSYS_DEVICE_ID: /* OEM_VISIONRGB_SCHD4 */
                  sprintf(dev->core.name, "%s", _DGC182);
                  dev->core.type = DGC182;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC182");
                  break;
            }
            break;
         case DGC184_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC184_PCI_SUBSYS_DEVICE_ID: /* VisionSC-SDI4 */
                  sprintf(dev->core.name, "%s", _DGC184);
                  dev->core.type = DGC184;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC184");
                  break;
            }
            break;
         case DGC214_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC214_PCI_SUBSYS_DEVICE_ID: /* VisionSC-UHD2 */
                  sprintf(dev->core.name, "%s", _DGC214);
                  dev->core.type = DGC214;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC214");
                  break;
            }
            break;
         case DGC224_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC224_PCI_SUBSYS_DEVICE_ID: /* VisionSC-S2 */
                  sprintf(dev->core.name, "%s", _DGC224);
                  dev->core.type = DGC224;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC224");
                  break;
            }
            break;
#endif
#ifdef INCLUDE_VISIONLC
         case DGC186_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC186_PCI_SUBSYS_DEVICE_ID: /* VisionEC-SDI */
                  sprintf(dev->core.name, "%s", _DGC186);
                  dev->core.type = DGC186;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC186");
                  break;
            }
            break;
         case DGC199_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
            case DGC199_PCI_SUBSYS_DEVICE_ID: /* VisionEC-HD */
               sprintf(dev->core.name, "%s", _DGC199);
               dev->core.type = DGC199;
               break;
            default:
               sprintf(dev->core.name, "UNKNOWN DGC199");
               break;
            }
            break;
         case DGC200_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC200_PCI_SUBSYS_DEVICE_ID: /* VisionLC-HD */
                  sprintf(dev->core.name, "%s", _DGC200);
                  dev->core.type = DGC200;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC200");
                  break;
            }
            break;
         case DGC204_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC204_PCI_SUBSYS_DEVICE_ID: /* VisionLC-SDI */
                  sprintf(dev->core.name, "%s", _DGC204);
                  dev->core.type = DGC204;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC204");
                  break;
            }
            break;
         case DGC205_PCI_DEVICE_ID:
            switch (pdev->subsystem_device)
            {
               case DGC205_PCI_SUBSYS_DEVICE_ID: /* VisionLC-HD2 */
                  sprintf(dev->core.name, "%s", _DGC205);
                  dev->core.type = DGC205;
                  break;
               default:
                  sprintf(dev->core.name, "UNKNOWN DGC205");
                  break;
            }
            break;
#endif
         default:
            sprintf(dev->core.name,"UNKNOWN DGC");
            break;
      }
#ifdef DRGB133_RELEASE_BUILD
      /* Truncate if necessary */
      if(strlen(dev->core.name) >= 16)
      {
         dev->core.name[15] = 0;
      }
#endif
   }
}

#ifdef RGB133_CONFIG_HAVE__DEVINIT
#define DEVINIT __devinit
#else
#define DEVINIT
#endif

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
void init_v4l2_dev(struct v4l2_device * v4l2_dev, struct device * dev, int num)
{
   INIT_LIST_HEAD(&v4l2_dev->subdevs);
   spin_lock_init(&v4l2_dev->lock);
#ifdef RGB133_CONFIG_HAVE_IOCTL_LOCK_IN_V4L2_DEV
   mutex_init(&v4l2_dev->ioctl_lock);
#endif
   v4l2_prio_init(&v4l2_dev->prio);
   kref_init(&v4l2_dev->ref);

   v4l2_dev->dev = dev;
   sprintf(v4l2_dev->name, "DGC-%d", num);
}

int rgb133_create_control_device(struct pci_dev * pci_dev);
#endif

#ifdef RGB133_CONFIG_HAVE_VFL_TYPE_VIDEO
#define __VFL_TYPE_VIDEO VFL_TYPE_VIDEO
#else
#define __VFL_TYPE_VIDEO VFL_TYPE_GRABBER
#endif

static int DEVINIT rgb133_probe(struct pci_dev* pdev,
   const struct pci_device_id* pci_id)
{
   int ret = 0;
   unsigned char lat;
   struct rgb133_dev* dev = 0;
   struct video_device* vfd = 0;
#ifndef RGB133_USER_MODE
   void* pDE = 0;
#endif /* RGB133_USER_MODE */
   int override_devices = -1;
   int i = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: START\n"));

   if(rgb133_device_index >= (RGB133_MAX_DEVICES-1))
   {
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: ENODEV END\n"));
      return -ENODEV;
   }

   /* Get the next driver device */
   dev = &rgb133_devices[rgb133_device_index];
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: %s card found - dev[%d](0x%p) - pdev(0x%p)\n",
         DRIVER_TAG, rgb133_device_index, dev, pdev));

   /* Two things to do in here
    * 1) All the device specific stuff (do once per device) like
    *    PCI setup, MMIO setup, IRQ setup.
    * 2) The video device setup.
    */

   /* Setup a new device structure */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: init rgb133(0x%p)\n", dev));
   memset(dev, 0, sizeof(*dev));
   dev->core.nr = rgb133_device_index;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_probe: device(0x%.4x), subsysdevice(0x%.4x)\n",
         pdev->device, pdev->subsystem_device));
   SetCoreDeviceName(pdev, dev);

   /* Assign debug level */
   dev->debug = rgb133_debug;

   /* Assign channel info */
   memset(&dev->init, 0, sizeof(unsigned int) * RGB133_MAX_CHANNEL_PER_CARD);
   dev->channels = -1;
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_probe: init dev(0x%p)->control to 0\n", dev));
   dev->control = 0;
   dev->index = rgb133_device_index;
   dev->events = 0;
   dev->bSetEquilisation = 1;
   dev->devices = -1;
   dev->ctrlUsers = 0;
   for(i=0; i<RGB133_MAX_CHANNEL_PER_CARD; i++)
      dev->ColourDomain[i] = -1;

   /* Assign PCI info */
   dev->core.pci = pdev;
   dev->id = pdev->device;

#ifdef RGB133_USER_MODE
   /* Init the IRQ event structure */
   init_waitqueue_head(&dev->irqEvQ.q);
   spin_lock_init(&dev->irqEvQ.lock);
   dev->irqEvQ.event = 0;
   dev->irqEvQ.exit = 0;

   /* Init the APP event structure */
   init_waitqueue_head(&dev->appEvQ.q);
   spin_lock_init(&dev->appEvQ.lock);
   dev->appEvQ.flags = 0;
   dev->appEvQ.event = 0;
   dev->appEvQ.exit = 0;

   /* Init the params event structure */
   init_waitqueue_head(&dev->parmsEv.q);
   spin_lock_init(&dev->parmsEv.lock);
   dev->parmsEv.signalled = false;

   /* Init the ctrl event structure */
   init_waitqueue_head(&dev->ctrlEv.q);
   spin_lock_init(&dev->ctrlEv.lock);
   dev->ctrlEv.signalled = false;

   /* Init the image data event structure */
   init_waitqueue_head(&dev->imageDataEv.q);
   spin_lock_init(&dev->imageDataEv.lock);
   dev->imageDataEv.signalled = false;

   /* Init the info event structure */
   init_waitqueue_head(&dev->infoEv.q);
   spin_lock_init(&dev->infoEv.lock);
   dev->infoEv.signalled = false;

#endif /* RGB133_USER_MODE */

   /* Init device structures */
   /* TODO: Are there any outside PDGC133DE? */

   if(pci_enable_device(pdev))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: Can't enable PCI device, pdev(0x%p)\n", pdev));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: Enable PCI dev EIO END\n"));
      return -EIO;
   }

   if(pci_set_dma_mask(pdev, DMA_BIT_MASK(64)))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: No suitable DMA available\n"));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: Set DMA Mask EIO END\n"));
      return -EIO;
   }

   /* Request PCI MMIO region */
   if(!request_mem_region(pci_resource_start(pdev,0),
            pci_resource_len(pdev,0),
            dev->core.name))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: Can't request iomem(0x%llx) for pdev(0x%p)\n",
            (unsigned long long)pci_resource_start(pdev,0), pdev));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: EBUSY END\n"));
      return -EBUSY;
   }

   /* Set PCI Master and Driver Data */
   pci_set_master(pdev);
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: Set drvdata(0x%p)\n", dev));
   pci_set_drvdata(pdev, dev);

   /* Read config registers from PCI device */
   pci_read_config_byte(pdev, PCI_CLASS_REVISION, &dev->revision);
   pci_read_config_byte(pdev, PCI_LATENCY_TIMER, &lat);

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: rgb%d rgb133-%d(rev: %d) at %s\n",
         rgb133_device_index, dev->id, dev->revision, pci_name(pdev)));
   dev->core.pci_base = (unsigned long long)pci_resource_start(pdev, 0);
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: irq(%d), latency(%d), mmio(0x%llx)\n",
         dev->core.pci->irq, lat,
         dev->core.pci_base));
   schedule();

   /* Memory Map the IO Region */
   /* TODO: Think this is where we'll get the access range for Windows pDE */
   dev->rgb133_mmio = ioremap(dev->core.pci_base, 0x80000);
   if(dev->rgb133_mmio == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: ioremap failed for pdev(0x%p)\n", pdev));
      ret = -EIO;
      goto err6;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: ioremap(0x%p)\n", dev->rgb133_mmio));
   }

   /* Create a Windows DeviceExtension Structure */
#ifndef RGB133_USER_MODE
   {
      int pDESize = 0;
      pDE = DeviceAllocMemory(&pDESize);
      if(pDE == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: failed alloc %d bytes to pDE\n",
               (int)sizeof(*pDE)));
         ret = -ENOMEM;
         goto err5;
      }
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: alloc %d bytes to pDE(0x%p) for dev[%d](0x%p)\n",
            pDESize, pDE, rgb133_device_index, dev));
      memset(pDE, 0, sizeof(*pDE));

      /* Assign pDE to our device */
      dev->pDE = pDE;

      /* Hardware initialisation
       * TODO: Per Device??
       */
      DeviceSetDevice(pDE, dev);

      /* Setup appropriate structs to keep the Win Ifc */
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: Set windows structs (config info)\n"));
      rgb133_configInfo[rgb133_device_index].HwDeviceExtension = pDE;
      rgb133_configInfo[rgb133_device_index].AccessRanges[0] = DummyAccessRange[rgb133_device_index];
      rgb133_configInfo[rgb133_device_index].NumberOfAccessRanges = 1;
      rgb133_configInfo[rgb133_device_index].AccessRanges[0].RangeStart.QuadPart = (LONGLONG)dev->rgb133_mmio;
      rgb133_configInfo[rgb133_device_index].AccessRanges[0].RangeLength = (512 * 1024);
      rgb133_configInfo[rgb133_device_index].AccessRanges[0].RangeInMemory = TRUE;
      rgb133_configInfo[rgb133_device_index].RealPhysicalDeviceObject = &rgb133_deviceObject[rgb133_device_index];

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: Set windows structs (srb)\n"));
      Srb[rgb133_device_index].Status = STATUS_SUCCESS;
      Srb[rgb133_device_index].Command = SRB_INITIALIZE_DEVICE;
      Srb[rgb133_device_index].CommandData.ConfigInfo = &rgb133_configInfo[rgb133_device_index];
      Srb[rgb133_device_index].HwDeviceExtension = pDE;

      /* Pretend we've received a Windows SRB packet
       *  - in this case use SRB_INITIALIZE_DEVICE to setup the
       *    hardware a la what happens in Windows
       */
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: HwInitialize(0x%p)\n", &Srb[rgb133_device_index]));
      DeviceInitializeHardware(&Srb[rgb133_device_index]);
      ret = Srb[rgb133_device_index].Status;

      if(ret < 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: Failed to init hardware(%lu)...\n", Srb[rgb133_device_index].Status));
         ret = Srb[rgb133_device_index].Status;
         goto err4;
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: init'd hardware(%d)...\n", ret));
      }

      /* Init the IRQ Stuff */
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: Disable interrupts using(0x%p)\n",
            pDE));
      DeviceDisableAllInterrupts(pDE);
   }
#else
   dev->pDE = (void*)0x12345678;
#endif /* RGB133_USER_MODE */

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: request_irq(%d)\n",
         dev->core.pci->irq));
   ret = request_irq(dev->core.pci->irq, rgb133_irq,
#ifndef RGB133_USER_MODE
         IRQF_SHARED, (const char*)dev->core.name, (void*)pDE);
#else
         IRQF_SHARED, (const char*)dev->core.name, (void*)dev);
#endif /* RGB133_USER_MODE */
   if(ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: rgb%d can't get IRQ(%d)\n",
         rgb133_device_index, dev->core.pci->irq));
      goto err3;
   }

#ifndef RGB133_USER_MODE
   DeviceSetInterruptObject(pDE, &rgb133_interruptObject[rgb133_device_index]);

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: init pInterruptObject(0x%p)->SpinLock(0x%p)\n",
      &rgb133_interruptObject[rgb133_device_index], &rgb133_interruptObject[rgb133_device_index].SpinLock));
   spin_lock_init(&rgb133_interruptObject[rgb133_device_index].SpinLock.lock);
#endif /* RGB133_USER_MODE */

   /* Initialise the mutex lock structure. */
   dev->pLock = KernelVzalloc(sizeof(struct mutex));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: assign 0x%p (%d bytes) to dev->pLock(0x%p)\n",
         dev->pLock, sizeof(struct mutex), &dev->pLock));
   KernelMutexInit(dev->pLock);

   /* Initialise the query controls structure. */
   for(i=0; i<RGB133_MAX_CHANNEL_PER_CARD; i++)
   {
      int j = 0;
      for(j=0; j<RGB133_NUM_CONTROLS; j++)
      {
         memcpy(&dev->V4L2Ctrls[i][j], &rgb133_qctrl_defaults[j], sizeof(struct v4l2_queryctrl));
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_probe: copy %d bytes from defaults(0x%p) to dev[%d]->V4L2Ctrls[%d][%d](0x%p)\n",
               sizeof(struct v4l2_queryctrl), &rgb133_qctrl_defaults[j],
               dev->index, i, j, &dev->V4L2Ctrls[i][j]));
      }
   }

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   if (rgb133_device_index == 0)
   {
      /* The first time we come through here, we need to create the control
      ** device.  This used to be done in rgb133_init, but now the registration
      ** requires the PCI device (even though this device most definitely doesn't
      ** actually need it) we've had to move things around.
      */
      ret = rgb133_create_control_device(dev->core.pci);
      if(ret < 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: Failed to create control device\n"));
         goto err;
      }
   }
#endif

   /* Default override devices value */
   override_devices = SetOverrideDevices(pdev, pDE);
   dev->devices = override_devices;

   /* Register all necessary devices with the V4L2 Ifc */
   {
      int local_device = 0;
      int local_channel = 0;

      while(override_devices > 0)
      {
         vfd = video_device_alloc();
         if(vfd == 0)
         {
            ret = -ENOMEM;
            goto err;
         }

         *vfd = rgb133_defaults;

         vfd->minor = -1;
#ifdef RGB133_CONFIG_HAVE_NEW_VDEV_NUM
         vfd->num = -2;
#endif
         vfd->release = video_device_release;
#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
         vfd->v4l2_dev = KernelVzalloc(sizeof(struct v4l2_device));
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: alloc %d bytes to vfd->v4l2_dev(0x%p)\n",
               sizeof(struct v4l2_device), vfd->v4l2_dev));
         /*
         ** We've rolled our own init function; we're not allowed to call
         ** the GPL-only function that initialises a mandatory structure.
         */
         init_v4l2_dev(vfd->v4l2_dev, &dev->core.pci->dev, rgb133_video_nr);

         vfd->dev_parent = &dev->core.pci->dev;

#else
#ifdef RGB133_CONFIG_HAVE_VDEV_PARENT
         vfd->parent = &dev->core.pci->dev;
#else
         vfd->dev = &dev->core.pci->dev;
#endif
#endif

         snprintf(vfd->name, sizeof(vfd->name), "%s (%i-%i)",
                  rgb133_defaults.name, rgb133_device_index, local_device);

         /* PG: registerig video device will fail if device_caps are not set (kernel v5.4) */
         rgb133V4L2AssignDeviceCaps(dev, vfd, NULL);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_probe: video_register_device(0x%p,%d)\n", vfd, rgb133_video_nr));
         ret = video_register_device(vfd, __VFL_TYPE_VIDEO, rgb133_video_nr);
         if(ret < 0)
         {
            goto err;
         }

         video_set_drvdata(vfd, dev);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_probe: vfd(0x%p)->minor(%d)\n", vfd, vfd->minor));

         if(rgb133_video_nr >= 0)
            rgb133_video_nr++;

         /* Assign video_device to rgb133_device */
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_probe: assign vfd(0x%p)\n", vfd));
         dev->pVfd[local_device] = vfd;

         /* All done for this device */
        RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: %s -> dev(0x%p)\n",
               dev->core.name, dev));
         sprintf(dev->core.node,"%s%d", "/dev/video",
#ifdef RGB133_CONFIG_HAVE_NEW_VDEV_NUM
               dev->pVfd[local_device]->num);
#else
               dev->pVfd[local_device]->minor);
#endif
         RGB133PRINT((RGB133_LINUX_DBG_CORE, "%s: %s capture driver...loaded as %s\n",
                  dev->core.name, DRIVER_TAG, dev->core.node));

         /* Add the name, node and minor number into the dev map */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_probe: (%s) (%s) minor(%d) dev[%d].channel(%d) added to dev[%d]->pDevMap[%d](0x%p)\n",
               dev->core.name, dev->core.node,
               dev->pVfd[local_device]->minor, local_device, local_channel,
               dev->index, local_device, &dev->pDevMap[local_device]));
         strcpy(dev->pDevMap[local_device].name, dev->core.name);
         strcpy(dev->pDevMap[local_device].node, dev->core.node);
         dev->pDevMap[local_device].index = rgb133_devices_count + local_device;
         dev->pDevMap[local_device].minor = dev->pVfd[local_device]->minor;
         dev->pDevMap[local_device].channel = local_channel;

         /* decrement override_devices */
         override_devices--;
         local_device++;
         local_channel++;
      }

      /* All fine and setup, increment the number of cards counter */
      rgb133_device_index++;
      rgb133_devices_count += local_device;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: END\n"));
   return 0;

err:
   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err(%d)\n", ret));
   {
      int local_device = 0;
      for(local_device=0; local_device<dev->devices; local_device++)
      {
         if(dev->pVfd[local_device])
         {
            /* Unregister the device */
            if(dev->pVfd[local_device]->minor != -1)
            {
               video_unregister_device(dev->pVfd[local_device]);
               /* video_device_release is called as part of unregister */
            }
            else
            {
               /* Release the device */
               video_device_release(dev->pVfd[local_device]);
            }

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: free dev(0x%p)->pVfd[%d](0x%p)->v4l2_dev(0x%p)\n",
                  dev, local_device, dev->pVfd[local_device], dev->pVfd[local_device]->v4l2_dev));
            KernelVfree(dev->pVfd[local_device]->v4l2_dev);
#endif
            dev->pVfd[local_device] = 0;
         }
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err2(%d) - free device structures(%d)\n",
         ret));
   if(dev->pLock)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: err2(%d) - free dev(0x%p)->pLock(0x%p)\n", ret, dev, dev->pLock));
      KernelVfree(dev->pLock);
      dev->pLock = 0;
   }

   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err2(%d) - free_irq(%d)\n",
         ret, dev->core.pci->irq));
#ifndef RGB133_USER_MODE
   free_irq(dev->core.pci->irq, pDE);
#else
   free_irq(dev->core.pci->irq, dev);
#endif /* RGB133_USER_MODE */

err3:
#ifndef RGB133_USER_MODE
   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err3(%d) - Uninit hardware\n", ret));
   DeviceUninitializeHardware( &Srb[rgb133_device_index] );

err4:
   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err4(%d)\n", ret));
   if(pDE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err4 - pDE found(%p)\n", pDE));
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_probe: free pDE(0x%p)\n", pDE));
      KernelVfree(pDE);
      pDE = 0;
   }

err5:
#endif /* RGB133_USER_MODE */
   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err5(%d)\n", ret));
   if(dev->rgb133_mmio)
      iounmap(dev->rgb133_mmio);
   release_mem_region(pci_resource_start(dev->core.pci, 0),
         pci_resource_len(dev->core.pci,0));

err6:
   RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_probe: err6(%d)\n", ret));
   pci_set_drvdata(pdev, NULL);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_probe: ERROR END\n"));
   return ret;
}

#ifdef RGB133_CONFIG_HAVE__DEVINIT
#define DEVEXIT __devexit
#else
#define DEVEXIT
#endif
int rgb133_destroy_control_device(void);

static void DEVEXIT rgb133_remove(struct pci_dev* pdev)
{
   struct rgb133_dev* dev = pci_get_drvdata(pdev);

#ifndef RGB133_USER_MODE
   int channel = dev->index;
   int ret;
   void* pDE = dev->pDE;
   int i = 0;
#endif /* RGB133_USER_MODE */

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_remove: START\n"));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: unregister pci_dev(0x%p)\n",
         pdev));
#ifndef RGB133_USER_MODE
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: rgb133(0x%p), pDE(0x%p)\n",
         dev, pDE));
#endif /* RGB133_USER_MODE */

   {
      int local_device = 0;
      for(local_device=0; local_device<dev->devices; local_device++)
      {
         if(dev->pVfd[local_device])
         {
            /* Unregister the device */
            if(dev->pVfd[local_device]->minor != -1)
            {
               video_unregister_device(dev->pVfd[local_device]);
               /* video_device_release is called as part of unregister */
            }
            else
            {
               /* Release the device */
               video_device_release(dev->pVfd[local_device]);
            }

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
            /* We don't have to do this if we're on a downlevel kernel where this structure
            ** wasn't mandatory.
            */
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_remove: free dev(0x%p)->pVfd[%d](0x%p)->v4l2_dev(0x%p)\n",
                  dev, local_device, dev->pVfd[local_device], dev->pVfd[local_device]->v4l2_dev));
            KernelVfree(dev->pVfd[local_device]->v4l2_dev);
#endif
            dev->pVfd[local_device] = 0;
         }
      }
   }

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   {
      struct rgb133_dev *dev;

      dev = &rgb133_devices[RGB133_MAX_DEVICES-1];
      if(dev->pVfd[0])
      {
         if(dev->pVfd[0]->minor != -1)
         {
            video_unregister_device(dev->pVfd[0]);
            /* video_device_release is called as part of unregister */
         }
         else
            video_device_release(dev->pVfd[0]);
         
         dev->pVfd[0] = 0;
         rgb133_destroy_control_device( );
      }
   }
#endif

   /* Free the Video Timing Dump buffers */
   for(i=0; i<RGB133_MAX_CHANNEL_PER_CARD; i++)
   {
      if(dev->pDetVdif[i])
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_remove: free dev[%d]->pDetVdif[%d](0x%p)\n", dev->index, i, dev->pDetVdif[i]));
         KernelVfree(dev->pDetVdif[i]);
         dev->pDetVdif[i] = 0;
      }
      if(dev->pCurVdif[i])
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_remove: free dev[%d]->pCurVdif[%d](0x%p)\n", dev->index, i, dev->pCurVdif[i]));
         KernelVfree(dev->pCurVdif[i]);
         dev->pCurVdif[i] = 0;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: rgb133(0x%p) - free device structures(%d)\n",
         dev));
   if(dev->pLock)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_remove: free dev(0x%p)->pLock(0x%p)\n", dev, dev->pLock));
      KernelVfree(dev->pLock);
      dev->pLock = 0;
   }

#ifndef RGB133_USER_MODE
   Srb[channel].Status = STATUS_SUCCESS;
   Srb[channel].Command = SRB_UNINITIALIZE_DEVICE;

   DeviceDisableAllInterrupts(pDE);
#endif /* RGB133_USER_MODE */

   /* Free IRQ */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: free_irq(%d)\n",
         dev->core.pci->irq));
#ifndef RGB133_USER_MODE
   free_irq(dev->core.pci->irq, pDE);
#else
   free_irq(dev->core.pci->irq, dev);
#endif /* RGB133_USER_MODE */

#ifndef RGB133_USER_MODE
   /* Pretend we've received a Windows SRB packet
    *  - in this case use SRB_INITIALIZE_DEVICE to setup the
    *    hardware a la what happens in Windows
    */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: HwUnInitialize(0x%p)\n", &Srb[channel]));
   DeviceUninitializeHardware(&Srb[channel]);
   ret = Srb[channel].Status;

   if(ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_remove: Failed to uninit hardware(%lu)...\n", Srb[channel].Status));
      ret = Srb[channel].Status;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: uninit'd hardware(%d)...\n", ret));
   }

   /* Clear the pDE */
   if(pDE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_remove: free pDE(0x%p)\n", pDE));
      KernelVfree(pDE);
      dev->pDE = 0;
   }
#endif /* RGB133_USER_MODE */

   /* Unmap MMIO */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: unmap MMIO(0x%p)\n", dev->rgb133_mmio));
   if(dev->rgb133_mmio)
   {
      iounmap(dev->rgb133_mmio);
   }

   release_mem_region(pci_resource_start(dev->core.pci, 0),
         pci_resource_len(dev->core.pci, 0));

   /* Clear any PCI Dev prvate data */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_remove: set pci_dev(0x%p) data to NULL\n", pdev));
   pci_set_drvdata(pdev, NULL);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_remove: END\n"));
}

#ifdef CONFIG_PM
static int rgb133_suspend(struct pci_dev* pci_dev, pm_message_t state)
{
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_suspend: TODO!!\n"));
   return 0;
}

static int rgb133_resume(struct pci_dev* pci_dev)
{
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_resume: TODO!!\n"));
   return 0;
}
#endif

static struct pci_device_id rgb133_pci_tbl[] = {
#ifdef INCLUDE_VISION
      {DGC_PCI_VENDOR_ID, DGC133_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC139_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC144_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC150_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC151_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC153_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC154_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC159_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC161_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC165_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC167_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC168_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC179_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC182_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC184_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC214_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC224_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
#endif
#ifdef INCLUDE_VISIONLC
      {DGC_PCI_VENDOR_ID, DGC186_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC199_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC200_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC204_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
      {DGC_PCI_VENDOR_ID, DGC205_PCI_DEVICE_ID, DGC_PCI_SUBSYSTEM_VENDOR_ID, PCI_ANY_ID, 0, 0, 0},
#endif
      {0},
};

MODULE_DEVICE_TABLE(pci, rgb133_pci_tbl);

#ifdef RGB133_CONFIG_HAVE__DEVINIT
#define DEVEXIT_P __devexit_p
#else
#define DEVEXIT_P
#endif

static struct pci_driver rgb133_pci_driver = {
   .name       = DRIVER_NAME,
   .id_table   = rgb133_pci_tbl,
   .probe      = rgb133_probe,
   .remove     = DEVEXIT_P(rgb133_remove),
#ifdef CONFIG_PM
   .suspend    = rgb133_suspend,
   .resume     = rgb133_resume,
#endif
};

int rgb133_create_control_device(struct pci_dev * pci_core_dev)
{
   struct rgb133_dev* dev = 0;
   int ret = -EINVAL;

   /* Get the control driver device and initialise */
   dev = &rgb133_devices[RGB133_MAX_DEVICES-1];
   memset(dev, 0, sizeof(*dev));
   dev->core.nr = RGB133_MAX_DEVICES-1;
   sprintf(dev->core.name,"%s Control", DRIVER_TAG);
   dev->debug = rgb133_debug;

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_create_control_device: init dev(0x%p)->control to 1\n", dev));
   dev->control = 1;
   dev->channels = -1;
   dev->index = RGB133_MAX_DEVICES-1;
   dev->core.pci = pci_core_dev;
   dev->id = RGB133_MAX_DEVICES-1;

   /* Initialise the mutex lock structure. */
   dev->pLock = KernelVzalloc(sizeof(struct mutex));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_create_control_device: assign 0x%p (%d bytes) to dev->pLock(0x%p)\n",
         dev->pLock, sizeof(struct mutex), &dev->pLock));
   KernelMutexInit(dev->pLock);

   dev->events = 0;
   dev->bSetEquilisation = 1;

   dev->pDE = 0;

#ifdef RGB133_USER_MODE
   /* Init the IRQ event structure */
   init_waitqueue_head(&dev->irqEvQ.q);
   spin_lock_init(&dev->irqEvQ.lock);
   dev->irqEvQ.event = 0;
   dev->irqEvQ.exit = 0;

   /* Init the APP event structure */
   init_waitqueue_head(&dev->appEvQ.q);
   spin_lock_init(&dev->appEvQ.lock);
   dev->appEvQ.flags = 0;
   dev->appEvQ.event = 0;
   dev->appEvQ.exit = 0;

   /* Init the Parms event structure */
   init_waitqueue_head(&dev->parmsEv.q);
   spin_lock_init(&dev->parmsEv.lock);
   dev->parmsEv.signalled = false;

   /* Init the ctrl event structure */
   init_waitqueue_head(&dev->ctrlEv.q);
   spin_lock_init(&dev->ctrlEv.lock);
   dev->ctrlEv.signalled = false;

   /* Init the image data event structure */
   init_waitqueue_head(&dev->imageDataEv.q);
   spin_lock_init(&dev->imageDataEv.lock);
   dev->imageDataEv.signalled = false;

   /* Init the info event structure */
   init_waitqueue_head(&dev->infoEv.q);
   spin_lock_init(&dev->infoEv.lock);
   dev->infoEv.signalled = false;

#endif /* RGB133_USER_MODE */

   /* Allocate video device and register with the V4L2 Ifc */
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_create_control_device: video_device_alloc()\n"));

   dev->pVfd[0] = video_device_alloc();
   if(dev->pVfd[0] == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_create_control_device: video_device_alloc() failed\n"));
      ret = -ENOMEM;
      goto exit;
   }

   /* Assign defaults */
   *dev->pVfd[0] = rgb133_defaults;

   dev->pVfd[0]->minor = RGB133_MAX_DEVICES-1;
   dev->pVfd[0]->release = video_device_release;

   snprintf(dev->pVfd[0]->name, sizeof(dev->pVfd[0]->name), "%s (%i)", rgb133_defaults.name, dev->pVfd[0]->minor);

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_create_control_device: video_register_device(0x%p,%d)\n", dev->pVfd[0], dev->pVfd[0]->minor));

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   dev->pVfd[0]->v4l2_dev = KernelVzalloc(sizeof(struct v4l2_device));

   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_create_control_device: alloc %d bytes to dev->pVfd[0]->v4l2_dev(0x%p)\n",
         sizeof(struct v4l2_device), dev->pVfd[0]->v4l2_dev));

   init_v4l2_dev(dev->pVfd[0]->v4l2_dev, &dev->core.pci->dev, rgb133_video_nr);
   dev->pVfd[0]->dev_parent = &dev->core.pci->dev;
#endif   

   /* PG: registerig video device will fail if device_caps are not set (kernel v5.4) */
   rgb133V4L2AssignDeviceCaps(dev, dev->pVfd[0], NULL);

   ret = video_register_device(dev->pVfd[0], __VFL_TYPE_VIDEO, dev->pVfd[0]->minor);
   if(ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_create_control_device: video_register_device() failed with ret 0x%x\n", ret));
      goto cleanup;
   }

   video_set_drvdata(dev->pVfd[0], dev);

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_create_control_device: vfd(0x%p)->num(%d)\n",
#ifdef RGB133_CONFIG_HAVE_NEW_VDEV_NUM
         dev->pVfd[0], dev->pVfd[0]->num));
#else
         dev->pVfd[0], dev->pVfd[0]->minor));
#endif

   sprintf(dev->core.node,"%s%d", "/dev/video",
#ifdef RGB133_CONFIG_HAVE_NEW_VDEV_NUM
         dev->pVfd[0]->num);
#else
         dev->pVfd[0]->minor);
#endif

   /* All done for this device */
   RGB133PRINT((RGB133_LINUX_DBG_CORE, "%s: %s capture driver...loaded as %s\n",
         dev->core.name, DRIVER_TAG, dev->core.node));

   /* Add the name, node and minor number into the dev map */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_create_control_device: (%s) (%s) minor(%d) dev[%d].channel(%d) added to dev[%d]->pDevMap[%d](0x%p)\n",
         dev->core.name, dev->core.node, dev->pVfd[0]->minor, 0, 0,
         dev->index, 0, &dev->pDevMap[0]));
   strcpy(dev->pDevMap[0].name, dev->core.name);
   strcpy(dev->pDevMap[0].node, dev->core.node);
   dev->pDevMap[0].index = 0;
   dev->pDevMap[0].minor = dev->pVfd[0]->minor;
   dev->pDevMap[0].channel = 0;

   return 0;

cleanup:
   if(dev->pVfd[0])
   {
      if(dev->pVfd[0]->minor != -1)
      {
         RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_create_control_device: cleanup - video_unregister_device()\n"));
         video_unregister_device(dev->pVfd[0]);
         /* video_device_release is called as part of unregister */
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_create_control_device: cleanup - video_device_release()\n"));
         video_device_release(dev->pVfd[0]);
      }

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_create_control_device: free dev(0x%p)->pVfd[0]->v4l2_dev(0x%p)\n",
            dev, dev->pVfd[0]->v4l2_dev));
      KernelVfree(dev->pVfd[0]->v4l2_dev);
#endif
      dev->pVfd[0] = 0;
   }

exit:
   return ret;
}

int rgb133_destroy_control_device(void)
{
   struct rgb133_dev* dev = 0;
   int ret = -EINVAL;

   /* Get the control driver device and initialise */
   dev = &rgb133_devices[RGB133_MAX_DEVICES-1];

   memset(&dev->init, 0, sizeof(unsigned int) * RGB133_MAX_CHANNEL_PER_CARD);
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_destroy_control_device: dev[%d](0x%p)\n",
         dev->index, dev));
   dev->control = 0;
   dev->channels = -1;
   dev->core.pci = 0;

#ifdef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   if(dev->v4l2_dev)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_destroy_control_device: free dev(0x%p)->v4l2_dev(0x%p)\n", dev, dev->v4l2_dev));
      KernelVfree(dev->v4l2_dev);
      dev->v4l2_dev = 0;
   }
#endif

   /* Free the mutex lock structure. */
   if(dev->pLock)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_destroy_control_device: free dev(0x%p)->pLock(0x%p)\n", dev, dev->pLock));
      KernelVfree(dev->pLock);
      dev->pLock = 0;
   }

   dev->pDE = 0;

   return 0;
}

/* Print the interlacing behaviour */
#ifdef RGB133_CONFIG_HAVE_INTERLACED_TB
const char* rgb133_interlacing_name[RGB133_MAX_INTERLACING_MODES] = {
      "Any", "None", "Top Field Only", "Bottom Field Only",
      "Weave", "Sequential Top Bottom", "Sequential Bottom Top",
      "Alternate Fields", "Weave Top Bottom", "Weave Bottom Top",
      "Bob", "Unknown"
};
#else
   const char* rgb133_interlacing_name[RGB133_MAX_INTERLACING_MODES] = {
         "Any", "None", "Top Field Only", "Bottom Field Only",
         "Weave", "Sequential Top Bottom", "Sequential Bottom Top",
         "Alternate Fields", "Bob", "Unknown"
   };
#endif

const char* PrintInterlacing(int type)
{
   switch(type)
   {
      case V4L2_FIELD_BOB:
         return rgb133_interlacing_name[RGB133_MAX_INTERLACING_MODES-2];
      case V4L2_FIELD_ANY:
      case V4L2_FIELD_NONE:
      case V4L2_FIELD_TOP:
      case V4L2_FIELD_BOTTOM:
      case V4L2_FIELD_INTERLACED:
      case V4L2_FIELD_SEQ_TB:
      case V4L2_FIELD_SEQ_BT:
      case V4L2_FIELD_ALTERNATE:
#ifdef RGB133_CONFIG_HAVE_INTERLACED_TB
      case V4L2_FIELD_INTERLACED_TB:
      case V4L2_FIELD_INTERLACED_BT:
#endif
         return rgb133_interlacing_name[type];
      default:
         return rgb133_interlacing_name[RGB133_MAX_INTERLACING_MODES-1];
   }
}

/* Print the message colour */
const char* rgb133_colour_names[RGB133_MAX_COLOURS] = {
      "Black", "White", "Yellow", "Cyan", "Green",
      "Magenta", "Red", "Blue", "Grey"
};

const char* PrintMessageColour(int colour)
{
   return rgb133_colour_names[colour];
}

/* Print the detection mode */
const char* PrintDetectMode(unsigned long mode)
{
   switch(mode)
   {
      case RGB133_DETECT_MODE_ANALOGUE:
         return "Analogue";
      case RGB133_DETECT_MODE_DVI:
         return "DVI";
      case RGB133_DETECT_MODE_DEFAULT:
      default:
         return "Default";
   }
}

/* Print the scaling aspect ratio */
const char* PrintScalingBehaviour(unsigned long mode)
{
   switch(mode)
   {
      case RGB133_SCALING_UP_ONLY:
         return "Upscale Only";
      case RGB133_SCALING_DOWN_ONLY:
         return "Downscale Only";
      case RGB133_SCALING_NONE:
         return "Never Scale";
      case RGB133_SCALING_DEFAULT:
      default:
         return "Default";
   }
}

/* Print the detection mode */
const char* PrintScalingAspectRatio(unsigned long ar)
{
   switch(ar)
   {
      case RGB133_SCALING_ASPECT_RATIO_3_2:
         return "3:2";
      case RGB133_SCALING_ASPECT_RATIO_4_3:
         return "4:3";
      case RGB133_SCALING_ASPECT_RATIO_5_3:
         return "5:3";
      case RGB133_SCALING_ASPECT_RATIO_5_4:
         return "5:4";
      case RGB133_SCALING_ASPECT_RATIO_8_5:
         return "8:5";
      case RGB133_SCALING_ASPECT_RATIO_16_9:
         return "16:9";
      case RGB133_SCALING_ASPECT_RATIO_16_10:
         return "16:10";
      case RGB133_SCALING_ASPECT_RATIO_17_9:
         return "17:9";
      case RGB133_SCALING_ASPECT_RATIO_SOURCE:
         return "Source";
      case RGB133_SCALING_ASPECT_RATIO_DEFAULT:
      default:
         return "Default";
   }
}

/* Print the detection mode */
const char* PrintInputGanging(unsigned long ganging_type)
{
   switch(ganging_type)
   {
      case RGB133_GANGING_TYPE_2x1:
         return "2x1";
      case RGB133_GANGING_TYPE_1x2:
         return "1x2";
      case RGB133_GANGING_TYPE_3x1:
         return "3x1";
      case RGB133_GANGING_TYPE_1x3:
         return "1x3";
      case RGB133_GANGING_TYPE_4x1:
         return "4x1";
      case RGB133_GANGING_TYPE_1x4:
         return "1x4";
      case RGB133_GANGING_TYPE_2x2:
         return "2x2";
      case RGB133_GANGING_TYPE_DISABLED:
      default:
         return "Disabled";
   }
}

/* Print the colour domain string */
const char* PrintColourDomain(unsigned long domain)
{
   switch(domain)
   {
      case RGB133_COLOUR_DOMAIN_AUTO:
         return "Auto";
      case RGB133_COLOUR_DOMAIN_RGB709_FULL:
         return "Full Range RGB BT.709";
      case RGB133_COLOUR_DOMAIN_YUV709_FULL:
         return "Full Range YUV BT.709";
      case RGB133_COLOUR_DOMAIN_YUV601_FULL:
         return "Full Range YUV BT.601";
      case RGB133_COLOUR_DOMAIN_YUV709_STUDIO:
         return "Studio Range YUV BT.709";
      case RGB133_COLOUR_DOMAIN_YUV601_STUDIO:
         return "Studio Range YUV BT.601";
      case RGB133_COLOUR_DOMAIN_RGB709_STUDIO:
         return "Studio Range RGB BT.709";
      case RGB133_COLOUR_DOMAIN_YUV2020_FULL:
         return "Full Range YUV BT.2020";
      case RGB133_COLOUR_DOMAIN_YUV2020_STUDIO:
         return "Studio Range YUV BT.2020";
      case RGB133_COLOUR_DOMAIN_RGB601_FULL:
         return "Full Range RGB BT.601";
      case RGB133_COLOUR_DOMAIN_RGB601_STUDIO:
         return "Studio Range RGB BT.601";
      case RGB133_COLOUR_DOMAIN_RGB2020_FULL:
         return "Full Range RGB BT.2020";
      case RGB133_COLOUR_DOMAIN_RGB2020_STUDIO:
         return "Studio Range RGB BT.2020";
      default:
         return "Unknown";
   }
}

/* Print the audio analog mute string */
const char* PrintAudioAnalogMute(unsigned long mute)
{
   switch(mute)
   {
      case RGB133_AUDIO_MUTE_NONE:
         return "Mute None";
      case RGB133_AUDIO_MUTE_UNBALANCED:
         return "Mute Unbalanced";
      case RGB133_AUDIO_MUTE_BALANCED:
         return "Mute Balanced";
      case RGB133_AUDIO_MUTE_BOTH:
         return "Mute Balanced and Unbalanced";
      default:
         return "Unknown";
   }
}

/* Output module load options for debug purposes */
void PrintModuleLoadOptions(void)
{
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "Module Load Options: Base Device Node            -> %d\n",
         rgb133_video_nr));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Debug Level                 -> %d\n",
         rgb133_debug));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Show FPS Output             -> %s\n",
         (rgb133_show_fps == RGB133_SHOW_FPS) ? "yes" : "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Show Coloured Buffers       -> %s\n",
         (rgb133_coloured_buffers == RGB133_COLOURED_BUFFERS) ? "yes" : "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Expose Inputs               -> %s\n",
         (rgb133_expose_inputs == RGB133_EXPOSE_INPUTS) ? "yes" : "no"));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Default Interlacing         -> %s\n",
         PrintInterlacing(rgb133_interlace)));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Non-V4L2 Pixel Formats      -> %s\n",
         (rgb133_non_vl42_pix_fmts == RGB133_ENABLE_NON_V4L2_PIX_FMTS) ? "yes": "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Default Byte Order          -> %s\n",
         (rgb133_flip_rgb == RGB133_RGB_BYTEORDER) ? "RGB": "BGR"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Colour Domain               -> %s\n",
         PrintColourDomain(rgb133_colour_domain)));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Full Pixel Range            -> %s\n",
         (rgb133_full_range == RGB133_FULL_RANGE_YUV) ? "yes" : "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Video Timings Limited       -> %s\n",
         (rgb133_limit_vid_timings == RGB133_VID_TIMINGS_LIMITED) ? "yes": "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     GTF Offset                  -> %s\n",
         (rgb133_offset_gtf == RGB133_GTF_OFFSET) ? "yes": "no"));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Dumb Buffer Width           -> %d\n",
         rgb133_dumb_buffer_width));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Dumb Buffer Height          -> %d\n",
         rgb133_dumb_buffer_height));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Dumb Buffer Rate            -> %d\n",
         rgb133_dumb_buffer_rate));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Timestamp Info              -> %s\n",
         (rgb133_timestamp_info == RGB133_TIMESTAMP_INFO_SILENT) ? "off": "on"));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Frame Debug                 -> %s\n",
         (rgb133_frame_debug == RGB133_NO_FRAME_DEBUG) ? "off": "on"));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     HDCP                        -> %s\n",
         (rgb133_hdcp == RGB133_HDCP_ON) ? "on" : "off"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Overscan Cropping           -> %s\n",
         (rgb133_overscan_cropping == RGB133_OVERSCAN_CROPPING_ON) ? "on" : "off"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Detection Mode              -> %s\n",
         PrintDetectMode(rgb133_detect_mode)));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Scaling Behaviour           -> %s\n",
         PrintScalingBehaviour(rgb133_scaling)));

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Scaling Aspect Ratio        -> %s\n",
         PrintScalingAspectRatio(rgb133_scaling_aspect_ratio)));
#ifdef INCLUDE_VISION
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Input Ganging               -> %s\n",
         PrintInputGanging(rgb133_ganging_type)));
#endif
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Show No Signal Message      -> %s\n",
         (rgb133_nosig == RGB133_DISPLAY_NO_SIGNAL_MSG) ? "yes" : "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     No Signal Message           -> %s\n",
         vdif_string_nosignal));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Show No Signal Counter      -> %s\n",
         (rgb133_nosig_counter == RGB133_SHOW_COUNTER) ? "yes" : "no"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Out Of Range Message        -> %s\n",
         vdif_string_outofrange));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Unrecognised Signal Message -> %s\n",
         vdif_string_unrec));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Message Colour              -> %s\n",
         PrintMessageColour(rgb133_msg_colour)));
#ifdef INCLUDE_VISIONLC
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Audio Source                -> %s\n",
         (rgb133_audio_source == RGB133_AUDIO_EMBEDDED) ? "Embedded" : "External"));
#endif
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Mute Digital Audio          -> %s\n",
         (rgb133_audio_mute_digital == RGB133_AUDIO_MUTE_OFF) ? "no" : "yes"));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Mute Analog Audio           -> %s\n",
         PrintAudioAnalogMute(rgb133_audio_mute_analog)));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Unbalanced Volume           -> %d%s\n",
         rgb133_audio_volume_unbalanced, (rgb133_audio_volume_unbalanced == RGB133_AUDIO_VOLUME_DEFAULT) ? " (default)" : ""));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     Balanced Volume             -> %d%s\n",
         rgb133_audio_volume_balanced, (rgb133_audio_volume_balanced == RGB133_AUDIO_VOLUME_DEFAULT) ? " (default)" : ""));
   RGB133PRINT((RGB133_LINUX_DBG_INIT, "                     In Diagnostics Mode         -> %s\n",
         (rgb133_diagnostics_mode == RGB133_DIAGNOSTICS_MODE) ? "yes" : "no"));
}

static void* HandleTable = 0;

/* Module Init/Exit Routines */
BOOL rgb133_driver_init = FALSE;

static struct timer_list rgb133_hw_tick_timer;

static int __init rgb133_init(void)
{
   int ret = 0;
   rgb133_driver_init = FALSE;

   /* Set the debug mask */
   CaptureSetDebugLevel(0, rgb133_debug);

   RGB133PRINT((RGB133_LINUX_DBG_CORE, "\n\n\n\n\n===== %s Driver Init =====\n\n", DRIVER_TAG));

   /* Upfront Init */
   HandleTable = DeviceAllocHandleTable();
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init: call InitHandleTable(0x%p)\n", HandleTable));
   DeviceInitHandleTable(HandleTable);

   DeviceDriverEntry( );

   PrintModuleLoadOptions( );

#ifndef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   /* Setup the control device */
   /* Older kernels without the dev_parent member of the dev structure can
   ** be setup this early.  Newer kernels, not so much.
   */
   ret = rgb133_create_control_device(NULL);
   if(ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_init: Failed to create control device\n"));
      goto cleanup;
   }
#endif
   rgb133_device_index = 0;

   /* Start a timer to reset the Tick count every 5s */
#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
   rgb133_timer_setup(&rgb133_hw_tick_timer, rgb133_clock_reset_passive_timer, 0);
#else
   rgb133_init_timer(&rgb133_hw_tick_timer);
   rgb133_setup_timer(&rgb133_hw_tick_timer, rgb133_clock_reset_passive_timer, (unsigned long)&rgb133_hw_tick_timer);
#endif
   rgb133_mod_timer(&rgb133_hw_tick_timer, RGB133_HW_TICK_TIMER_TIMEOUT);

   ret = pci_register_driver(&rgb133_pci_driver);

   if(ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_init: Error(%d) while loading %s driver - PCI reg driver\n", ret, DRIVER_TAG));
   }
   else
   {
      BOOL nios_alive = FALSE;
      long error = 0;
      int i;

      for(i=0; i<rgb133_device_index; i++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_init: Enable Interrupts for dev[%d](0x%p).pDE(0x%p)\n",
            i, &rgb133_devices[i], rgb133_devices[i].pDE));
         DeviceEnableAllInterrupts ( rgb133_devices[i].pDE );

         /* Wait with setting rgb133_driver_init until handshake with the card has completed 
         ** This is delayed to inhibit error messages in the system logs for when
         ** sysfs tries to open the new devices to get static information (controls,
         ** etc) but the handshake with the card isn't complete.
         ** Ideally, we'd like to stop sysfs open()s until after the handshake has occurred */
         nios_alive = DeviceIsNiosAlive(rgb133_devices[i].pDE, &error);
         if(nios_alive == FALSE && error == -EIO)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_init: pDE(0x%x), DeviceIsNiosAlive returned %d error(%d)\n",
                  rgb133_devices[i].pDE, nios_alive, error));
         }
      }

      rgb133_driver_init = TRUE;

      RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_init: Datapath Limited - %s "
         "Capture Driver ver. %s loaded ok.\n", DRIVER_TAG, RGB133_CHAR_VERSION));
   }

#ifndef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
cleanup:
#endif
   if (ret < 0)
   {
      DeviceCleanupHandleTable(&HandleTable);
      DeviceDGCDriverUnload( );
   }
   return ret;
}

static void __exit rgb133_exit(void)
{
#ifndef RGB133_CONFIG_HAVE_VDEV_DEV_PARENT
   struct rgb133_dev* dev = 0;

   dev = &rgb133_devices[RGB133_MAX_DEVICES-1];

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_exit: rgb133(0x%p) - vfd(0x%p)\n",
         dev, dev->pVfd[0]));

   if(dev->pVfd[0])
   {
      if(dev->pVfd[0]->minor != -1)
      {
         video_unregister_device(dev->pVfd[0]);
         /* video_device_release is called as part of unregister */
      }
      else
         video_device_release(dev->pVfd[0]);

      dev->pVfd[0] = 0;
      rgb133_destroy_control_device( );
   }

#endif

   rgb133_del_timer(&rgb133_hw_tick_timer);

   pci_unregister_driver(&rgb133_pci_driver);

   rgb133_release();

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_exit: call CleanupHandleTable()\n"));
   DeviceCleanupHandleTable(&HandleTable);
   DeviceDGCDriverUnload( );

   RGB133PRINT((RGB133_LINUX_DBG_INIT, "%s capture driver...unloaded\n", DRIVER_TAG));
   RGB133PRINT((RGB133_LINUX_DBG_CORE, "\n===== %s Driver Exit =====\n\n\n\n\n", DRIVER_TAG));
}

module_init(rgb133_init);
module_exit(rgb133_exit);

/* modinfo stuff */
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(RGB133_CHAR_VERSION);

/** Device Config **/
//module_param(rgb133_captures, uint, 0444);
//MODULE_PARM_DESC(rgb133_captures, "number of video captues each device is allowed to use");

module_param(rgb133_video_nr, uint, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_video_nr, "base video device number");

module_param(rgb133_expose_inputs, uint, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_expose_inputs, "expose device channels as v4l2 device nodes");

/** No signal message behaviour **/
/** Writeable by root and device owner group.  Everyone else reads **/
module_param(rgb133_nosig, int, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_nosig, "changes no signal message generation behaviour");

/** Debug Config **/
module_param(rgb133_debug, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(debug, "outputs debug info at appropriate user set level");

/** FPS Output **/
module_param(rgb133_show_fps, uint, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_show_fps, "outputs FPS info");

/** Coloured Buffers **/
module_param(rgb133_coloured_buffers, uint, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_coloured_buffers, "outputs coloured buffers");

/** Default Deinterlacing Method **/
module_param(rgb133_interlace, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_interlace, "sets up the default interlacing type\n");

/** Behaviour on non-v4l2 pixel formats **/
module_param(rgb133_non_vl42_pix_fmts, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_non_vl42_pix_fmts, "Enable or disable pixel formats unknown to v4l2 core\n");

/** Default RGB Byteorder **/
module_param(rgb133_flip_rgb, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_flip_rgb, "flip the order of RGB bytes (RGB | BGR)\n");

/** Default Colour Domain **/
module_param(rgb133_colour_domain, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_colour_domain, "Globally set the detected colour domain (AUTO, (BT.601 | BT.709 | BT.2020)RGB, (BT.601 | BT.709 | BT.2020)YUV\n");

/** Default Vid Timing Limit Behaviour **/
module_param(rgb133_limit_vid_timings, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_limit_vid_timings, "limit the maximum values on some or all of the analog video timing parameters\n");

/** Parameters to make Skype work **/
module_param(rgb133_dumb_buffer_width, int, S_IRUGO|S_IWUSR|S_IWGRP);
module_param(rgb133_dumb_buffer_height, int, S_IRUGO|S_IWUSR|S_IWGRP);
module_param(rgb133_dumb_buffer_rate, int, S_IRUGO|S_IWUSR|S_IWGRP);

/** Timestamp info **/
module_param(rgb133_timestamp_info, ulong, S_IRUGO|S_IWUSR|S_IWGRP);

/** Frame info **/
module_param(rgb133_frame_debug, ulong, S_IRUGO|S_IWUSR|S_IWGRP);

/* Default detection mode */
module_param(rgb133_detect_mode, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_detect_mode, "Set the detection mode in the firmware");

/* No upscale */
module_param(rgb133_scaling, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_scaling, "Set the behaviour for scaling");

/* Scaling Aspect Ratio */
module_param(rgb133_scaling_aspect_ratio, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_scaling_aspect_ratio, "Set the behaviour for scaling based on a fixed aspect ratio");

/* HDCP */
module_param(rgb133_hdcp, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_hdcp, "Set the behaviour for HDCP sources");

/* Overscan Cropping */
module_param(rgb133_overscan_cropping, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_overscan_cropping, "Enable overscan cropping");

/** Special GTF input behaviour **/
module_param(rgb133_offset_gtf, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_offset_gtf, "Offset the Vertical Porches for Analog GTF modes");

/**********    Signal texts    *********/
module_param_string(rgb133_no_signal_text, vdif_string_nosignal, 64, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb_133_no_signal_text, "The string to display when there is a no signal condition\n");

module_param_string(rgb133_out_of_range_text, vdif_string_outofrange, 64, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(vdif_string_outofrange, "The string to display when the signal is out of range\n");

module_param_string(rgb133_unrecognisable_text, vdif_string_unrec, 64, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(vdif_string_unrec, "The string to display when the signal is unrecognisable\n");

module_param(rgb133_msg_colour, int, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_msg_colour, "The colour that the message should be written in");

/* No signal counter behaviour */
module_param(rgb133_nosig_counter, int, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_nosig_counter, "changes no signal counter generation behaviour");

/* Diagnostics Mode */
module_param(rgb133_diagnostics_mode, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_diagnostics_mode, "Enable diagnostics mode");

/* Full Range YUV */
module_param(rgb133_full_range, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_full_range, "Disable range reduction of pixel values");

#ifdef INCLUDE_VISION
/* Input Ganging Type */
module_param(rgb133_ganging_type, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_ganging_type, "Enable ganging inputs together on supported cards");
#endif

#ifdef INCLUDE_VISIONLC
/* Audio source */
module_param(rgb133_audio_source, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_audio_source, "Select audio source on supported cards");
#endif

/* Audio Digital Mute */
module_param(rgb133_audio_mute_digital, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_audio_mute_digital, "Mute digital audio interfaces on/off");

/* Audio Analog Mute */
module_param(rgb133_audio_mute_analog, ulong, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_audio_mute_analog, "Mute analog audio interfaces");

/* Audio Unbalanced Volume */
module_param(rgb133_audio_volume_unbalanced, int, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_audio_volume_unbalanced, "Set volume on unbalanced analog interfaces");

/* Audio Balanced Volume */
module_param(rgb133_audio_volume_balanced, int, S_IRUGO|S_IWUSR|S_IWGRP);
MODULE_PARM_DESC(rgb133_audio_volume_balanced, "Set volume on balanced analog interfaces");

/********** Registry Variables **********/
module_param(regFirmwarePath25, charp, 0444);
MODULE_PARM_DESC(regFirmwarePath25, "path in which the v25 fw is stored\n");

module_param(regFirmwarePath27, charp, 0444);
MODULE_PARM_DESC(regFirmwarePath27, "path in which the v27 fw is stored\n");
