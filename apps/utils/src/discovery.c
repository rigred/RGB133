
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <errno.h>
#include "rgb133control.h"
#include "rgb133v4l2.h"

const char * deviceFormat = "/dev/video%d";

#define CAPS(x) if (caps & x) { printf("%s" #x "\n", prefix); }

void DumpCaps(__u32 caps, char * prefix)
{
#if defined(V4L2_CAP_VIDEO_CAPTURE)
	CAPS(V4L2_CAP_VIDEO_CAPTURE);
#endif
#if defined(V4L2_CAP_VIDEO_CAPTURE_MPLANE)
	CAPS(V4L2_CAP_VIDEO_CAPTURE_MPLANE);
#endif
#if defined(V4L2_CAP_VIDEO_OUTPUT)
	CAPS(V4L2_CAP_VIDEO_OUTPUT);
#endif
#if defined(V4L2_CAP_VIDEO_OUTPUT_MPLANE)
	CAPS(V4L2_CAP_VIDEO_OUTPUT_MPLANE);
#endif
#if defined(V4L2_CAP_VIDEO_M2M)
	CAPS(V4L2_CAP_VIDEO_M2M);
#endif
#if defined(V4L2_CAP_VIDEO_M2M_MPLANE)
	CAPS(V4L2_CAP_VIDEO_M2M_MPLANE);
#endif
#if defined(V4L2_CAP_VIDEO_OVERLAY)
	CAPS(V4L2_CAP_VIDEO_OVERLAY);
#endif
#if defined(V4L2_CAP_VBI_CAPTURE)
	CAPS(V4L2_CAP_VBI_CAPTURE);
#endif
#if defined(V4L2_CAP_VBI_OUTPUT)
	CAPS(V4L2_CAP_VBI_OUTPUT);
#endif
#if defined(V4L2_CAP_SLICED_VBI_CAPTURE)
	CAPS(V4L2_CAP_SLICED_VBI_CAPTURE);
#endif
#if defined(V4L2_CAP_SLICED_VBI_OUTPUT)
	CAPS(V4L2_CAP_SLICED_VBI_OUTPUT);
#endif
#if defined(V4L2_CAP_RDS_CAPTURE)
	CAPS(V4L2_CAP_RDS_CAPTURE);
#endif
#if defined(V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
	CAPS(V4L2_CAP_VIDEO_OUTPUT_OVERLAY);
#endif
#if defined(V4L2_CAP_HW_FREQ_SEEK)
	CAPS(V4L2_CAP_HW_FREQ_SEEK);
#endif
#if defined(V4L2_CAP_RDS_OUTPUT)
	CAPS(V4L2_CAP_RDS_OUTPUT);
#endif
#if defined(V4L2_CAP_TUNER)
	CAPS(V4L2_CAP_TUNER);
#endif
#if defined(V4L2_CAP_AUDIO)
	CAPS(V4L2_CAP_AUDIO);
#endif
#if defined(V4L2_CAP_RADIO)
	CAPS(V4L2_CAP_RADIO);
#endif
#if defined(V4L2_CAP_MODULATOR)
	CAPS(V4L2_CAP_MODULATOR);
#endif
#if defined(V4L2_CAP_READWRITE)
	CAPS(V4L2_CAP_READWRITE);
#endif
#if defined(V4L2_CAP_ASYNCIO)
	CAPS(V4L2_CAP_ASYNCIO);
#endif
#if defined(V4L2_CAP_STREAMING)
	CAPS(V4L2_CAP_STREAMING);
#endif
#if defined(V4L2_CAP_DEVICE_CAPS)
	CAPS(V4L2_CAP_DEVICE_CAPS);
#endif
}

int main(int argc, char * argv[])
{
	int i = 0;
	char device[16];
	int file_descriptor;

	sprintf(device, deviceFormat, i);

	while ((file_descriptor=open(device, O_RDWR)) != -1)
	{
		/* we opened a device, find its properties */
		int retval;
		struct v4l2_capability caps;

		if ((retval = ioctl(file_descriptor, VIDIOC_QUERYCAP, &caps)) >= 0)
		{
			printf("Device %d (%s) is a v4l2 device\n", i, device);
			printf("Driver name is: %s\n", caps.driver);
			printf("Card name is: %s\n", caps.card);
			printf("Bus position: %s\n", caps.bus_info);
			printf("Version: %d.%d.%d\n", caps.version >> 16 & 0xFF, caps.version >> 8 & 0xFF, caps.version & 0xFF);

#if defined(V4L2_CAP_DEVICE_CAPS)
			printf("Driver capabilities:\n");
			DumpCaps(caps.capabilities, "  ");
			if (caps.capabilities & V4L2_CAP_DEVICE_CAPS)
			{
				printf("Device capabilities:\n");
				DumpCaps(caps.device_caps, "  ");
			}
#endif
			printf("\n");
			// Enumerate inputs:
			{
				struct v4l2_input ip;

				ip.index = 0;
				while (ioctl(file_descriptor, VIDIOC_ENUMINPUT, &ip) == 0)
				{
					ip.index++;
					printf("Input %d has name: %s\n", ip.index, ip.name);
				}
				printf("There are %d input(s) on this device.\n\n", ip.index);
			}
			// Enumerate standard controls
			{
				struct v4l2_queryctrl qc;
				int i = 0;
				qc.id = V4L2_CID_BASE;
				while (qc.id <= V4L2_CID_LASTP1)
				{
					if (ioctl(file_descriptor, VIDIOC_QUERYCTRL, &qc) == 0)
					{
						i++;
						if (qc.flags & V4L2_CTRL_FLAG_DISABLED)
						{
							// Control is disabled, move onto the next.
							printf("Control %s is disabled\n", qc.name);
							qc.id++;
							continue;
						}
						printf ("Control %s has minimum of %d and maximum of %d\n", qc.name, qc.minimum, qc.maximum);
					}
					qc.id++;
				}
				printf("There are a total of %d standard controls\n\n", i);
			}
			// Enumerate custom controls (i.e. generic parameters which aren't in the V4L2 control list)
			{
				struct v4l2_queryctrl qc;
				int i=0;
				qc.id = V4L2_CID_PRIVATE_BASE;
				while (ioctl(file_descriptor, VIDIOC_QUERYCTRL, &qc) == 0)
				{
					i++;
					if (qc.flags & V4L2_CTRL_FLAG_DISABLED)
					{
						// Control is disabled, move onto the next.
						printf ("Private Control %s is disabled\n", qc.name);
						qc.id++;
						continue;
					}
					printf ("Private Control %s has minimum of %d and maximum of %d\n", qc.name, qc.minimum, qc.maximum);
					qc.id++;
				}
				printf ("There are a total of %d custom controls\n", i);
			}


		} else {
			printf("Retval was %d with errno as %d\n", retval, errno);
			printf("Device %d (%s) isn't a v4l2 device\n", i, device);
			goto next;
		}
next:
		printf ("\n");

		close(file_descriptor);
		i++;
		sprintf(device, deviceFormat, i);
	}
	printf("\nEnumerated %d capture devices\n", i);
}
