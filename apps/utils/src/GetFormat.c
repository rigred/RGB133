#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#include <linux/videodev2.h>

#include "rgb133control.h"
#include "rgb133v4l2.h"

#define NUM_INPUTS    1
#define DEVICE_OFFSET 0

int main(int argc, char* argv[])
{
   int fd = 0;
   int i = 0;
   struct v4l2_format f;

   char device_name[128];

   memset(device_name, 0, 128);

   for(i=0; i<NUM_INPUTS; i++)
   {
      sprintf(device_name, "/dev/video%d", i+DEVICE_OFFSET);
      printf("\n\nDevice: %s\n", device_name);
      fd = open(device_name, O_RDWR);
      if(fd < 0)
      {
         printf("Failed to open device: %s\n", device_name);
         exit(0);
      }

      memset (&f, 0, sizeof(struct v4l2_format));
      f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if(ioctl(fd, VIDIOC_G_FMT, &f) < 0)
      {
         perror("Failed to get capture buffer format: ");
         close(fd);
         exit(0);
      }
      printf("Capture buffer format: %dx%d\n",
         f.fmt.pix.width, f.fmt.pix.height);

      f.fmt.pix.width = 640;
      f.fmt.pix.height = 480;
      printf("Set Capture buffer format: %dx%d\n",
         f.fmt.pix.width, f.fmt.pix.height);
      if(ioctl(fd, VIDIOC_S_FMT, &f) < 0)
      {
         perror("Failed to get capture buffer format: ");
         close(fd);
         exit(0);
      }

      memset (&f, 0, sizeof(struct v4l2_format));
      f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if(ioctl(fd, VIDIOC_G_FMT, &f) < 0)
      {
         perror("Failed to get capture buffer format: ");
         close(fd);
         exit(0);
      }
      printf("New Capture buffer format: %dx%d\n",
         f.fmt.pix.width, f.fmt.pix.height);

      memset (&f, 0, sizeof(struct v4l2_format));
      f.type = V4L2_BUF_TYPE_CAPTURE_SOURCE;
      {
         int err = 0;
         if((err = ioctl(fd, RGB133_VIDIOC_G_SRC_FMT, (void*)&f)) < 0)
         {
            perror("Failed to get source input format through propietary ioctl: ");
            if(ioctl(fd, VIDIOC_G_FMT, &f) < 0)
            {
               perror("Failed to get source input format using private ioctl and type: ");
               close(fd);
               exit(0);
            }
            else
            {
               printf("Input format (private ioctl): %dx%d\n",
                  f.fmt.pix.width, f.fmt.pix.height);
            }
         }
         else
         {
            printf("Input format (propietary ioctl): %dx%d\n",
               f.fmt.pix.width, f.fmt.pix.height);
         }
      }

      close(fd);
   }

   return 0;
}
