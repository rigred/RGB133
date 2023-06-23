/*
 * rgb133ioctl.c
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>

// Need PCI info for querycaps.
#include <linux/pci.h>

/* V4L2 includes */
#include <linux/videodev2.h>
#include <media/v4l2-common.h>

#include "rgb133config.h"

#ifdef RGB133_CONFIG_HAVE_V4L2_IOCTL
#include <media/v4l2-ioctl.h>
#endif /* RGB133_CONFIG_HAVE_V4L2_IOCTL */

#include <media/v4l2-dev.h>

#include "rgb133.h"
#include "rgb133api.h"
#include "rgb133forcedetect.h"
#include "rgb133capapi.h"
#include "rgb133deviceapi.h"
#include "rgb_api_types.h"
#include "rgb133kernel.h"
#include "rgb133v4l2.h"
#include "rgb133capparms.h"
#include "rgb133ioctl.h"
#include "rgb133mapping.h"
#include "rgb133peekpoke.h"
#include "rgb133timestamp.h"
#include "rgb133vidbuf.h"
#include "rgb133vdif.h"
#include "rgb133control.h"
#include "devicetypes.h"
#include "rgb133debug.h"

#include "OSAPI_TYPES.h"
#include "OSAPI.h"

// For initialising Multiline Messages buffers
#define MAX_LINE_LENGTH 512

extern unsigned long rgb133_non_vl42_pix_fmts;
extern unsigned long rgb133_flip_rgb;
extern unsigned long rgb133_timestamp_info;

extern unsigned long rgb133_debug;
unsigned long old_debug_level = -1;

extern int rgb133_expose_inputs;

extern unsigned long rgb133_hdcp;

extern unsigned long rgb133_diagnostics_mode;

/* Static Control Data - used to copy into the dev->qctrl structure when it's
 allocated */
struct v4l2_queryctrl rgb133_qctrl_defaults[RGB133_NUM_CONTROLS] =
{
      {
            .id = V4L2_CID_BRIGHTNESS,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Brightness",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = V4L2_CID_CONTRAST,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name =
            "Contrast",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = V4L2_CID_SATURATION,
            .type =  V4L2_CTRL_TYPE_INTEGER,
            .name = "Saturation",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = V4L2_CID_HUE,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Hue",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = V4L2_CID_BLACK_LEVEL,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Black Level",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = V4L2_CID_VFLIP,
            .type = V4L2_CTRL_TYPE_BOOLEAN,
            .name = "Vertical Flip",
            .minimum = 0,
            .maximum = 1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_HOR_POS,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Horizontal Position",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_HOR_SIZE,
            .type =  V4L2_CTRL_TYPE_INTEGER,
            .name = "Horizontal Size",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_PHASE,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Phase",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_VERT_POS,
            .type =  V4L2_CTRL_TYPE_INTEGER,
            .name = "Vertical Position",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_R_COL_BRIGHTNESS,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Red Brightness",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_R_COL_CONTRAST,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Red Contrast",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_G_COL_BRIGHTNESS,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Green Brightness",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_G_COL_CONTRAST,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Green Contrast",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id =  RGB133_V4L2_CID_B_COL_BRIGHTNESS,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Blue Brightness",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_B_COL_CONTRAST,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Blue Contrast",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_FORCE_DETECT,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Detect Type",
            .minimum = 0,
            .maximum = RGB133_NUM_FORCE_DETECT_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_SCALING,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Scaling Function",
            .minimum = 0,
            .maximum = RGB133_NUM_SCALING_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Scaling AR",
            .minimum = 0,
            .maximum = RGB133_NUM_SCALING_AR_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_HOR_TIME,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Horizontal Timing",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_VER_TIME,
            .type = V4L2_CTRL_TYPE_INTEGER,
            .name = "Vertical Timing",
            .minimum = 0,
            .maximum = 0,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_HDCP,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "HDCP",
            .minimum = 0,
            .maximum = RGB133_NUM_HDCP_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Colour Domain",
            .minimum = 0,
            .maximum = RGB133_NUM_COLOURDOMAIN_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_LIVESTREAM,
            .type = V4L2_CTRL_TYPE_BOOLEAN,
            .name = "LiveStream",
            .minimum = 0,
            .maximum = 1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Input Ganging",
            .minimum = 0,
            .maximum = RGB133_NUM_INPUT_GANGING_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Signal Type",
            .minimum = 0,
            .maximum = RGB133_NUM_SIGNAL_TYPE_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .type = V4L2_CTRL_TYPE_MENU,
            .name = "Video Standard",
            .minimum = 0,
            .maximum = RGB133_NUM_VIDEO_STANDARD_ITEMS-1,
            .step = 1,
            .default_value = 0,
            .flags = 0,
      },
};

#ifdef RGB133_CONFIG_HAVE_V4L2_QUERYMENU_NAME_IN_UNION
#define NAME_EQUALS(x) { .name = x, }
#else
#define NAME_EQUALS(x) .name = x
#endif

struct v4l2_querymenu rgb133_qmenu_force_detect[RGB133_NUM_FORCE_DETECT_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_FORCE_DETECT,
            .index = 0,
            NAME_EQUALS("Default"),
      },
      {
            .id = RGB133_V4L2_CID_FORCE_DETECT,
            .index = 1,
            NAME_EQUALS("Analogue"),
      },
      {
            .id = RGB133_V4L2_CID_FORCE_DETECT,
            .index = 2,
            NAME_EQUALS("DVI"),
      },
};

struct v4l2_querymenu rgb133_qmenu_scaling[RGB133_NUM_SCALING_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_SCALING,
            .index = 0,
            NAME_EQUALS("Default"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING,
            .index = 1,
            NAME_EQUALS("Upscale Only"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING,
            .index = 2,
            NAME_EQUALS("Downscale Only"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING,
            .index = 3,
            NAME_EQUALS("No Scaling"),
      },
};

struct v4l2_querymenu rgb133_qmenu_scaling_ar[RGB133_NUM_SCALING_AR_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 0,
            NAME_EQUALS("Default"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 1,
            NAME_EQUALS("Source"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 2,
            NAME_EQUALS("3:2"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 3,
            NAME_EQUALS("4:3"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 4,
            NAME_EQUALS("5:3"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 5,
            NAME_EQUALS("5:4"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 6,
            NAME_EQUALS("8:5"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 7,
            NAME_EQUALS("16:9"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 8,
            NAME_EQUALS("16:10"),
      },
      {
            .id = RGB133_V4L2_CID_SCALING_AR,
            .index = 9,
            NAME_EQUALS("17:9"),
      },
};

struct v4l2_querymenu rgb133_qmenu_hdcp[RGB133_NUM_HDCP_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_HDCP,
            .index = 0,
            NAME_EQUALS("Off"),
      },
      {
            .id = RGB133_V4L2_CID_HDCP,
            .index = 1,
            NAME_EQUALS("On"),
      },
};

struct v4l2_querymenu rgb133_qmenu_colourdomain[RGB133_NUM_COLOURDOMAIN_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 0,
            NAME_EQUALS("Auto"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 1,
            NAME_EQUALS("Full Range RGB BT.709"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 2,
            NAME_EQUALS("Full Range YUV BT.709"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 3,
            NAME_EQUALS("Full Range YUV BT.601"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 4,
            NAME_EQUALS("Studio Range YUV BT.709"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 5,
            NAME_EQUALS("Studio Range YUV BT.601"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 6,
            NAME_EQUALS("Studio Range RGB BT.709"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 7,
            NAME_EQUALS("Full Range YUV BT.2020"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 8,
            NAME_EQUALS("Studio Range YUV BT.2020"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 9,
            NAME_EQUALS("Full Range RGB BT.601"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 10,
            NAME_EQUALS("Studio Range RGB BT.601"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 11,
            NAME_EQUALS("Full Range RGB BT.2020"),
      },
      {
            .id = RGB133_V4L2_CID_COLOURDOMAIN,
            .index = 12,
            NAME_EQUALS("Studio Range RGB BT.2020"),
      },
};

struct v4l2_querymenu rgb133_qmenu_inputganging[RGB133_NUM_INPUT_GANGING_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 0,
            NAME_EQUALS("Disabled"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 1,
            NAME_EQUALS("2x2"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 2,
            NAME_EQUALS("4x1"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 3,
            NAME_EQUALS("1x4"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 4,
            NAME_EQUALS("2x1"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 5,
            NAME_EQUALS("1x2"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 6,
            NAME_EQUALS("3x1"),
      },
      {
            .id = RGB133_V4L2_CID_INPUT_GANGING,
            .index = 7,
            NAME_EQUALS("1x3"),
      },
};

struct v4l2_querymenu rgb133_qmenu_signal_type[RGB133_NUM_SIGNAL_TYPE_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 0,
            NAME_EQUALS("No Signal"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 1,
            NAME_EQUALS("DVI"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 2,
            NAME_EQUALS("DVI Dual Link"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 3,
            NAME_EQUALS("SDI"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 4,
            NAME_EQUALS("Video"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 5,
            NAME_EQUALS("3-Wire Sync On Green"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 6,
            NAME_EQUALS("4-Wire Composite Sync"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 7,
            NAME_EQUALS("5-Wire Separate Syncs"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 8,
            NAME_EQUALS("YPRPB"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 9,
            NAME_EQUALS("CVBS"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 10,
            NAME_EQUALS("YC"),
      },
      {
            .id = RGB133_V4L2_CID_SIGNAL_TYPE,
            .index = 11,
            NAME_EQUALS("Unknown"),
      },
};

struct v4l2_querymenu rgb133_qmenu_video_standard[RGB133_NUM_VIDEO_STANDARD_ITEMS] =
{
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 0,
            NAME_EQUALS("Unknown"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 1,
            NAME_EQUALS("NTSC-M"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 2,
            NAME_EQUALS("NTSC-J"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 3,
            NAME_EQUALS("NTSC-4-43-50"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 4,
            NAME_EQUALS("NTSC-4-43-60"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 5,
            NAME_EQUALS("PAL-I"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 6,
            NAME_EQUALS("PAL-M"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 7,
            NAME_EQUALS("PAL-NC"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 8,
            NAME_EQUALS("PAL-4-43-60"),
      },
      {
            .id = RGB133_V4L2_CID_VIDEO_STANDARD,
            .index = 9,
            NAME_EQUALS("SECAM-L"),
      },
};

/* v4l2 ioctl handling definitions */
int rgb133_querycap(struct file* file, void* priv, struct v4l2_capability* cap)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   unsigned long devCaps = 0;
   char businfo[255];

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querycap: START\n"));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_querycap: query cap into(0x%p)\n", cap));

   /* Better tell the caller who we are and what we can do! */
   memset(cap->driver, 0, 16);
   strncpy(cap->driver, DRIVER_TAG, 15); // Avoid buffer overrun, and leave final \0 value to terminate the string.
   strcpy(cap->card, dev->core.name);
   cap->version = RGB133_VERSION;

   if (!DeviceIsControl(dev->control))
   {
#ifdef INCLUDE_VISION
      if (dev->core.type == DGC133)
      {
         strcpy(cap->bus_info, "PCI:");
      }
      else
#endif
      {
         strcpy(cap->bus_info, "PCIe:");
      }
      snprintf(businfo, sizeof(businfo), "%04.4X:%02.2d:%03.3d",
            dev->core.pci->device, dev->core.pci->bus->number, dev->index);
      strcat(cap->bus_info, businfo);
   }

   rgb133V4L2AssignDeviceCaps(dev, NULL, cap);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querycap: END\n"));

   return 0;
}

int rgb133_enum_fmt_vid_cap(struct file* file, void* priv, struct v4l2_fmtdesc* f)
{
   struct rgb133_handle* h = file->private_data;
   const char* desc = 0;
   unsigned int real_index = 0;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_enum_fmt_vid_cap: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_enum_fmt_vid_cap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_enum_fmt_vid_cap: START\n"));

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_fmt_vid_cap: enumerate format[%d](0x%p) (if(%d) < %d)\n",
         f->index, f, f->index, v4lSupportedPixelFormats()));

   /* Only return existing formats (non-NULL) */
   if(f->index < v4lSupportedPixelFormatsReal(h))
   {
      real_index = v4lGetExistingFormatIndex(h, f->index);
      f->pixelformat = mapUItoUI(__PIXELFORMAT__, __MAP_TO_V4L__, real_index);
      desc = mapUItoCC(__PIXELFORMATDESC__, __MAP_TO_V4L__, real_index);
      strlcpy(f->description, desc, sizeof(f->description));
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_fmt_vid_cap: return pixelformat[%d](0x%x)\n",
            f->index, f->pixelformat));
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_fmt_vid_cap: return -1\n"));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_enum_fmt_vid_cap: END\n"));

   return 0;
}

int rgb133_g_fmt_vid_cap(struct file* file, void* priv, struct v4l2_format* f)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   PRGBCAPTUREAPI pRGBCapture = 0;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_fmt_vid_cap: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_fmt_vid_cap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_fmt_vid_cap: START\n"));

   /* Check the requested format type*/
   if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       f->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_fmt_vid_cap: invalid type requested for capture[%d] on device(0x%p)\n",
            h->capture, dev));
      return -EINVAL;
   }

   pRGBCapture = CaptureGetCapturePtr(h);
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_fmt_vid_cap: returning fmt data for capture[%d](0x%p) on device(0x%p)\n",
         h->capture, pRGBCapture, dev));

   /* Get current parameters */
   if ((rc = CaptureGetVidCapFormat(h, f)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_fmt_vid_cap: CaptureGetVidCapFormat failed: %d\n", rc));
      return rc;
   }

   if (CaptureVidMeasIsInterlaced(CaptureGetVidMeasFlags(h)))
   {
      if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_fmt_vid_cap: INTERLACED input...\n"));
         if (CaptureClientInterlaced(h))
         {
            unsigned long type = 0;

            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_fmt_vid_cap: WEAVE/ODD/EVEN/ALT\n"));
            CaptureGetClientInterlaced(h, 0, 0, (unsigned int*)&f->fmt.pix.field, 0);
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_fmt_vid_cap: BOB\n"));
            f->fmt.pix.field = V4L2_FIELD_BOB;
         }
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_fmt_vid_cap: source interlaced\n"));
         f->fmt.pix.field = V4L2_FIELD_INTERLACED;
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_fmt_vid_cap: source/client not interlaced\n"));
      f->fmt.pix.field = V4L2_FIELD_NONE;
   }

   if (f->fmt.pix.width == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_g_fmt_vid_cap: divide by zero risk...\n"));
      f->fmt.pix.width = -1;
   }
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_fmt_vid_cap: END - w(%d), h(%d), pfmt(0x%lx), si(%d), bpl(%d)\n",
         f->fmt.pix.width, f->fmt.pix.height, f->fmt.pix.pixelformat, f->fmt.pix.sizeimage, f->fmt.pix.bytesperline));

   return 0;
}

int rgb133_try_fmt_vid_cap(struct file* file, void* priv, struct v4l2_format* f)
{
   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_try_fmt_vid_cap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_try_fmt_vid_cap: TODO(0x%p)\n", f));

   return 0;
}

int rgb133_s_fmt_vid_cap(struct file* file, void* priv, struct v4l2_format* f)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   unsigned int alignMask = DeviceGetAlignmentMask(h->dev->pDE);
   unsigned int CaptureWidth = 0;
   unsigned int CaptureHeight = 0;
   unsigned int ComposeLeft = 0;
   unsigned int ComposeTop = 0;
   unsigned int ComposeWidth = 0;
   unsigned int ComposeHeight = 0;
   unsigned int ImageSize = 0;
   unsigned int CropTop = 0, CropBottom = 0;
   unsigned int CropLeft = 0, CropRight = 0;
   unsigned int PixelFormat = 0;
   unsigned int Bpp = 0;
   char* fourcc = 0;

   PGETPARMSOUTAPI pParmsOut = 0;
   sVWDeviceParms *maxDeviceParms;

   int rc = -EINVAL;

   maxDeviceParms = KernelVzalloc(sizeof(sVWDeviceParms));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_s_fmt_vid_cap: alloc %d bytes to maxDeviceParms(0x%p)\n",
         sizeof(sVWDeviceParms), maxDeviceParms));

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_fmt_vid_cap: Device is control\n"));
      rc = -EINVAL;
      goto error;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_fmt_vid_cap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: START h(0x%p) [%d][%d][%d] pRGBCapture(0x%p)[%d]\n",
         h, h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h), CaptureGetCaptureNumber(CaptureGetCapturePtr(h))));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: set fmt data for capture[%d] on device(0x%p) - pDE(0x%p)\n",
         h->capture, dev, dev->pDE));
   rc = rgb133_try_fmt_vid_cap(file, priv, f);
   if (rc != 0)
   {
      goto error;
   }

   PixelFormat = f->fmt.pix.pixelformat;
   fourcc = (char*) &PixelFormat;
   if(((PixelFormat == V4L2_PIX_FMT_RGB10 || PixelFormat == V4L2_PIX_FMT_Y410) && !DeviceIs10BitSupported(h)) ||
       ((PixelFormat == V4L2_PIX_FMT_YVU420) && !DeviceIsYV12Supported(h)))
   {
      /* 10bit colour or YV12 supported on some cards */
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Pixel format(0x%x - %c%c%c%c) not supported h(0x%p) [%d][%d][%d]\n",
            PixelFormat, fourcc[0], fourcc[1], fourcc[2], fourcc[3], h, h->dev->index, h->channel, h->capture));
      rc = -EINVAL;
      goto error;
   }

   if (f->type == V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      struct _rgb133ForceDetect* pFD = (struct _rgb133ForceDetect*) &f->fmt;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: Set default detect mode on dev[%d].chanel[%d].capture[%d] for input[%d] and mode(%d)\n",
            h->dev->index, h->channel, h->capture, pFD->input, pFD->mode));
      return CaptureIoctlSetDetectMode(h, pFD);
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap:      source(%u x %u) using pf(0x%x)\n",
         h->sCapture.SourceWidth, h->sCapture.SourceHeight, h->sCapture.pixelformat));
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: current aoi(%u x %u)\n",
         h->sCapture.CaptureWidth, h->sCapture.CaptureHeight));
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap:     set aoi(%u x %u) using pf(0x%x), field(%d)\n",
         f->fmt.pix.width, f->fmt.pix.height, f->fmt.pix.pixelformat, f->fmt.pix.field));

   /* Get the min/max values for HorAddrTime & VerAddrTime */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_s_fmt_vid_cap: get min/max device parms\n"));
   if ((pParmsOut = CaptureGetDeviceParametersReturnDeviceParms(dev, 0,
         0, maxDeviceParms, 0, 0, h->channel)) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Failed to get min/max vid timing parms\n"));
      rc = -EINVAL;
      goto error;
   }

   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_s_fmt_vid_cap: free pParmsOut(0x%p)\n", pParmsOut));
   KernelVfree(pParmsOut);

   /* V4L2 format struct can have width & height set to 0 or 0xffffffff,
    * in this case make sure we don't send that to the hardware
    */
   if (h->sCapture.ScalingMode == RGB133_SCALING_DEFAULT)
   {
      if (f->fmt.pix.width && f->fmt.pix.width <= maxDeviceParms->VideoTimings.HorAddrTime)
      {
         CaptureWidth = f->fmt.pix.width;
      }
      else /* use defaults */
      {
         CaptureWidth = maxDeviceParms->VideoTimings.HorAddrTime;
      }
      if (f->fmt.pix.height && f->fmt.pix.height <= maxDeviceParms->VideoTimings.VerAddrTime)
      {
         CaptureHeight = f->fmt.pix.height;
      }
      else /* use defaults */
      {
         CaptureHeight = maxDeviceParms->VideoTimings.VerAddrTime;
      }
   }
   else
   {
      CaptureWidth = f->fmt.pix.width;
      CaptureHeight = f->fmt.pix.height;
   }

   /* Make sure source w x h is initialised */
   if (h->sCapture.SourceWidth == 0 || h->sCapture.SourceWidth > maxDeviceParms->VideoTimings.HorAddrTime)
   {
      h->sCapture.SourceWidth = CaptureWidth;
   }
   if (h->sCapture.SourceHeight == 0 || h->sCapture.SourceHeight > maxDeviceParms->VideoTimings.VerAddrTime)
   {
      h->sCapture.SourceHeight = CaptureHeight;
   }
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_fmt_vid_cap: Call CaptureSetPixelFormat(pDE(0x%p), capture(%d), PixelFormat(0x%x - %c%c%c%c)\n", 
         dev->pDE, h->capture, PixelFormat, fourcc[0], fourcc[1], fourcc[2], fourcc[3]));
   if ((rc = CaptureSetPixelFormat(h, PixelFormat)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Failed to set pixel format(0x%x - %c%c%c%c): (%d)\n",
            PixelFormat, fourcc[0], fourcc[1], fourcc[2], fourcc[3], rc));
      goto error;
   }

   /* If we haven't officially set the pixel format successfully yet, we still need a
    * bpp value to make a stab at the scaling, cropping and pitching
    */
   if(PixelFormat != 0)
   {
      Bpp = v4lBytesPerPixel(PixelFormat);
      if(Bpp == -1)
         Bpp = 0;
   }

   ComposeLeft = 0;
   ComposeTop = 0;
   ComposeWidth = CaptureWidth;
   ComposeHeight = CaptureHeight;
   if(CaptureCalculateScalingAndCropping(h,
         h->sCapture.ScalingMode, h->sCapture.ScalingAR,
         h->sCapture.SourceWidth, h->sCapture.SourceHeight, CaptureWidth, CaptureHeight,
         &CropTop, &CropLeft, &CropBottom, &CropRight,
         &h->sCapture.BufferOffset, Bpp, DeviceGetAlignmentMask(h->dev->pDE), PixelFormat,
         &ComposeLeft, &ComposeTop, &ComposeWidth, &ComposeHeight, TRUE) == RGB133_SCALING_CHANGED)
   {
      /* Invalidate No Signal Buffers */
      rgb133_invalidate_buffers(h);
   }

   /* Data alignment constraints - this is a final point beyond which we should not proceed if data is not aligned for DMA */
#ifdef INCLUDE_VISIONLC
   /* Additional constraint for VisionLC cards
    * For NV12, YV12, capture width must be multiple of 16 (due to hardware limitation) */
   if((PixelFormat == V4L2_PIX_FMT_NV12) || (PixelFormat == V4L2_PIX_FMT_YVU420))
   {
      if(CaptureWidth & 15)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Incorrect capture width(%d) for PixFmt: 0x%x. "
               "Must be a multiple of 16\n",
               CaptureWidth, f->fmt.pix.pixelformat));
         rc = -ERANGE;
         goto error;
      }
   }
#endif
   /* Pitch needs to be DWORD aligned too, apart from (scaled width * Bpp) being DWORD aligned
    * Note that for Aspect Ratios other than default, testing only (scaled width * Bpp) DWORD alignment may not be enough
    * because capture width may not equal scaled width in such cases
    */
   if(((CaptureWidth * Bpp) & alignMask) || ((ComposeWidth * Bpp) & alignMask))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Incorrect capture width(%d) or scaled width(%d) for PixFmt: 0x%x. width x Bpp(%d) must be DWORD aligned\n",
            CaptureWidth, ComposeWidth, f->fmt.pix.pixelformat, Bpp));
      rc = -ERANGE;
      goto error;
   }

   /* NV12 and YV12 require even number of lines */
   if(f->fmt.pix.pixelformat == V4L2_PIX_FMT_NV12 || f->fmt.pix.pixelformat == V4L2_PIX_FMT_YVU420)
   {
      if((ComposeHeight | CaptureHeight) & 1)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Incorrect scaled height(%d) or capture height(%d) for pixel format 0x%x. Must be even\n",
               ComposeHeight, CaptureHeight, f->fmt.pix.pixelformat));
         rc = -ERANGE;
         goto error;
      }
   }

   {
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: h(0x%p), dev[%d](0x%p)\n", h, dev->index, dev));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: Call CaptureSetScaleAndAOI(channel[%d].capture[%d], source_h(%d), source_w(%d)\n",
            h->channel, h->capture, h->sCapture.SourceHeight, h->sCapture.SourceWidth));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: Call CaptureSetScaleAndAOI(height_scaled(%d), width_scaled(%d))\n",
            ComposeHeight, ComposeWidth));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: Call CaptureSetScaleAndAOI(Top(%d) Left(%d) Bottom(%d) Right(%d))\n",
            CropTop, CropLeft, CropBottom, CropRight));
      if ((rc = CaptureSetScaleAndAOI(h,
            h->sCapture.SourceHeight, h->sCapture.SourceWidth, ComposeHeight, ComposeWidth,
            CropTop, CropLeft, CropBottom, CropRight)) != 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_fmt_vid_cap: Failed to set scaling(%d)\n", rc));
         goto error;
      }

      /* Only assign if setting scaling worked */
      f->fmt.pix.width = h->sCapture.CaptureWidth = CaptureWidth;
      f->fmt.pix.height = h->sCapture.CaptureHeight = CaptureHeight;
      h->sCapture.ReqComposeLeft = 0;
      h->sCapture.ReqComposeTop = 0;
      h->sCapture.ReqComposeWidth = CaptureWidth;
      h->sCapture.ReqComposeHeight = CaptureHeight;
      h->sCapture.ComposeLeft = ComposeLeft;
      h->sCapture.ComposeTop = ComposeTop;
      h->sCapture.ComposeWidth = ComposeWidth;
      h->sCapture.ComposeHeight = ComposeHeight;
      h->sCapture.CropTop = CropTop;
      h->sCapture.CropLeft = CropLeft;
      h->sCapture.CropBottom = CropBottom;
      h->sCapture.CropRight = CropRight;

      /* Tweak the rgb133_flip_rgb flag here if pixel format is not RGB24/32 and was set correctly */
      if (rgb133_flip_rgb)
      {
         if (PixelFormat != V4L2_PIX_FMT_RGB24 && PixelFormat != V4L2_PIX_FMT_RGB32 &&
             PixelFormat != V4L2_PIX_FMT_RGB10)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: PixelFormat(0x%lx) is not V4L2_PIX_FMT_RGB24(0x%lx) or"
                  "V4L2_PIX_FMT_32(0x%lx) or V4L2_PIX_FMT_RGB10(0x%lx)\n",
                  PixelFormat, V4L2_PIX_FMT_RGB24, V4L2_PIX_FMT_RGB32, V4L2_PIX_FMT_RGB10));
            CaptureSetRGBFlip(h, 0);
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: PixelFormat(0x%lx) is V4L2_PIX_FMT_RGB24(0x%lx) or"
                  " V4L2_PIX_FMT_32(0x%lx) or V4L2_PIX_FMT_RGB10(0x%lx)\n",
                  PixelFormat, V4L2_PIX_FMT_RGB24, V4L2_PIX_FMT_RGB32, V4L2_PIX_FMT_RGB10));
            CaptureSetRGBFlip(h, 1);
         }
      }

      if (CaptureVidMeasIsInterlaced(CaptureGetVidMeasFlags(h)))
      {
         if (f->fmt.pix.field != V4L2_FIELD_INTERLACED &&
             f->fmt.pix.field != V4L2_FIELD_BOB &&
             f->fmt.pix.field != V4L2_FIELD_ALTERNATE &&
             f->fmt.pix.field != V4L2_FIELD_TOP &&
             f->fmt.pix.field != V4L2_FIELD_BOTTOM)
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: Interlacing type(%d) is not supported yet\n", f->fmt.pix.field));
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_fmt_vid_cap: Interlacing type set to field(%d)!\n", f->fmt.pix.field));
            CaptureSetClientInterlaced(h, (u32*) &f->fmt.pix.field);
         }
      }
      else
      {
         f->fmt.pix.field = V4L2_FIELD_NONE;
      }

      /* Only assign if the setting pixel format worked */
      h->sCapture.pixelformat = PixelFormat;
      h->sCapture.bpp = v4lBytesPerPixel(h->sCapture.pixelformat);

      switch(PixelFormat)
      {
         case V4L2_PIX_FMT_NV12:
            ImageSize = (h->sCapture.CaptureWidth * h->sCapture.bpp) * h->sCapture.CaptureHeight +
                        ((h->sCapture.CaptureWidth * h->sCapture.bpp) / 2) * h->sCapture.CaptureHeight;
            h->sCapture.planes = 2;
            break;
         case V4L2_PIX_FMT_YVU420:
            ImageSize = (h->sCapture.CaptureWidth * h->sCapture.bpp) * h->sCapture.CaptureHeight +
                        ((h->sCapture.CaptureWidth * h->sCapture.bpp) / 2) * h->sCapture.CaptureHeight;
            h->sCapture.planes = 3;
            break;
         default:
            ImageSize = (h->sCapture.CaptureWidth * h->sCapture.bpp) * h->sCapture.CaptureHeight;
            h->sCapture.planes = 1;
            break;
      }
      f->fmt.pix.sizeimage = h->sCapture.imageSize = ImageSize;

      RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_s_fmt_vid_cap: [%d][%d][%d]: Buffer: %dx%d - "
                                          "Crop Rect: %d,%d,%d,%d - Compose Rect: %d,%d,%d,%d (Offset: %d)\n",
            h->dev->index, h->channel, h->capture,
            h->sCapture.CaptureWidth, h->sCapture.CaptureHeight,
            h->sCapture.CropLeft, h->sCapture.CropTop, (h->sCapture.CropRight - h->sCapture.CropLeft), (h->sCapture.CropBottom - h->sCapture.CropTop),
            h->sCapture.ComposeLeft, h->sCapture.ComposeTop, h->sCapture.ComposeWidth, h->sCapture.ComposeHeight, h->sCapture.BufferOffset));
      RGB133PRINT((RGB133_LINUX_DBG_INIT, "\t\t\t\t     PixFmt: 0x%x (%c%c%c%c) bpp: %d\n",
            h->sCapture.pixelformat, fourcc[0], fourcc[1], fourcc[2], fourcc[3], h->sCapture.bpp));
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_fmt_vid_cap: END h(0x%p) [%d][%d][%d] pRGBCapture(0x%p)[%d]\n",
         h, h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h), CaptureGetCaptureNumber(CaptureGetCapturePtr(h))));

error:
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_s_fmt_vid_cap: free maxDeviceParms(0x%p)\n", maxDeviceParms));
   KernelVfree(maxDeviceParms);

   return rc;
}

/*!
 ** StopDMAStreamingAndFreeBuffers - called by reqbufs in response to the 0 buffer request.
 ** V4L2 docs say that a request for 0 buffers is the lazy man's stop any DMAs in flight,
 ** set streaming to off, and free all the buffers in the system.
 */
void StopDmaStreamingAndFreeBuffers(struct rgb133_handle * h)
{
   int i;
   rgb133_set_streaming(h, RGB133_STRM_STOPPED);

   CaptureWaitForData(h);  // Wait for anything to finish.

   if (rgb133_is_mapped(h))
   {
      /* Free IO request structures */
      if (h->ioctlsin != NULL)
      {
         for (i = 0; i < h->numbuffers; i++)
         {
            if (h->ioctlsin[i] != NULL)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "StopDmaStreamingAndFreeBuffers: free h->ioctlsin[%d](0x%p)\n", i, h->ioctlsin[i]));
               KernelVfree(h->ioctlsin[i]);
               h->ioctlsin[i] = NULL;
            }
         }

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "StopDmaStreamingAndFreeBuffers: free h->ioctlsin(0x%p) main structure\n", h->ioctlsin));
         KernelVfree(h->ioctlsin);
         h->ioctlsin = NULL;
      }

      if (h->buffers != NULL)
      {
         for (i = 0; i < h->numbuffers; i++)
         {
            if (h->buffers[i] != NULL)
            {
               if (h->buffers[i]->memory == V4L2_MEMORY_MMAP)
               {
                  __rgb133_buffer_mmap_dma_free(&h->buffers[i]);
               }
            }
         }

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "StopDmaStreamingAndFreeBuffers: free h(0x%p)->buffers(0x%p)\n", h, h->buffers));
         KernelVfree(h->buffers);
         h->buffers = 0;
         h->numbuffers = 0;
         rgb133_set_mapped(h, FALSE);
      }
   }
}

/*!
 ** ioctl VIDIOC_REQBUFS routine.
 ** This is now the function which allocates memory for the buffer and sets up all the
 ** DMA structures for the buffers.
 */
int rgb133_reqbufs(struct file* file, void* priv, struct v4l2_requestbuffers* req)
{
   struct rgb133_handle* h = file->private_data;
   unsigned int count = 0, size = 0;
   int i = 0;
   int retval = -EINVAL;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_reqbufs: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_reqbufs: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_reqbufs: START [%d][%d][%d]- h(0x%p) req(0x%p) count(%d)\n",
         h->dev->index, h->channel, h->capture, h, req, req->count));

   if (req->count == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_reqbufs: requested 0 buffers: Freeing buffers...\n"));

      StopDmaStreamingAndFreeBuffers(h);

      return 0;
   }
   else if (req->count < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_reqbufs: requested invalid buffers: %d\n", req->count));
      return -EINVAL;
   }

   if (req->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: Invalid buffer type(%d)\n", req->type));
      return -EINVAL;
   }
   if (req->memory != V4L2_MEMORY_MMAP && req->memory != V4L2_MEMORY_USERPTR)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: Invalid memory type(%d)\n", req->memory));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_reqbufs: check streaming status for h(0x%p)\n", h));
   if (rgb133_is_streaming(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: streaming already exists for h(0x%p)\n", h));
      retval = -EBUSY;
      goto out;
   }

   if (req->memory == V4L2_MEMORY_MMAP)
   {
      if (h->buffers != NULL)
      {
         int i;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_reqbufs: reset mmapped buffers in h(0x%p)\n", h));
         for (i = 0; i < h->numbuffers; i++)
         {
            if (h->buffers[i] != NULL)
            {
               __rgb133_buffer_mmap_dma_free(&h->buffers[i]);
            }
         }
         h->numbuffers = 0;
         rgb133_set_mapped(h, FALSE);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_reqbufs: reset buffer h(0x%p)\n", h));
         // TODO: sort this out
         // rgb133_buffer_q_free(q, RGB133_BUF_MMAP, KernelMode);
      }
   }

   count = req->count;

   size = 0;

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_reqbufs: setup mmap buffer count and size for h(0x%p)\n", h));
   retval = rgb133_mmap_buffer_setup(h, &count, &size);
   if(retval < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: failed to setup h(0x%p) for mmap/userptr io mmap buffer setup\n", h));
      retval = -EINVAL;
      goto out;
   }

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_reqbufs: setup %u buffers of size(%u) of type(%s)\n", count, size, v4l2_type_names[req->type]));
   retval = __rgb133_buffer_reqbuf_setup(h, count, size, req->memory);
   if (retval < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: failed to setup h(0x%p) for mmap/userptr io\n", h));
      goto out;
   }

   /* Set up IO request structures */
   {
      h->ioctlsin = KernelVzalloc(sizeof(void*));
      if(h->ioctlsin == NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: failed to allocate memory for h(0x%p)->ioctlsin\n", h));
         retval = -ENOMEM;
         goto out;
      }
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_reqbufs: alloc %d bytes to h(0x%p)->ioctlsin(0x%p)\n",
            sizeof(void*), h, h->ioctlsin));

      for (i = 0; i < count; i++)
      {
         h->ioctlsin[i] = KernelVzalloc(sizeof(CaptureGetSizeIoctlIn()));
         if(h->ioctlsin[i] == NULL)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_reqbufs: failed to allocate %d bytes for h(0x%p)->ioctlsin[%d]\n",
                  (int)sizeof(void*), h, i));
            retval = -ENOMEM;
            break;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_reqbufs: alloc %d bytes to h(0x%p)->ioctlsin[%d](0x%p)\n",
               sizeof(void*), h, i, h->ioctlsin[i]));
      }

      if (retval != 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_reqbufs: at least 1 allocation of IO request structs failed - cleaning up\n"));
         for ( i = 0; i < count; i++)
         {
            if (h->ioctlsin[i])
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_reqbufs: free h(0x%p)->ioctlsin[%d](0x%p)\n", h, i, h->ioctlsin[i]));
               KernelVfree(h->ioctlsin[i]);
               h->ioctlsin[i] = NULL;
            }
         }

         goto out;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_reqbufs: intiialised %u buffers of size(%u) of type(%s)\n", count, size, v4l2_type_names[req->type]));

   if (req->memory == V4L2_MEMORY_USERPTR)
   {
      /* Manually set the mapped flags for USERPTR IO so that
       * the buffers get torn down on close */
      rgb133_set_mapped(h, TRUE);
   }

   h->memorytype = req->memory;
   h->buffertype = req->type;

   h->numbuffers = count;

   /* We do now support more than one buffer. */
   req->count = h->numbuffers;

out:
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_reqbufs: END [%d][%d][%d] - buffers[%d] size(%d)\n",
         h->dev->index, h->channel, h->capture, count, size));

   return retval;
}

/*!
 ** ioctl VIDIOC_QUERYBUF routine. sends back to the user the parameters which describe the MMAP'ed (only!)
 ** buffer requested. Note that we work in the buffer oriented mode, not in the planar mode.
 */
int rgb133_querybuf(struct file* file, void* priv, struct v4l2_buffer* b)
{
   struct rgb133_handle* h = file->private_data;
   int ret = 0;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_querybuf: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_querybuf: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querybuf: START - b[%d]\n", b->index));

   if (rgb133_streaming_valid(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_querybuf: streaming already exists for h(0x%p): %d\n", h, rgb133_get_streaming(h)));
      return -EBUSY;
   }

   h->EnterCriticalSection(h);

   if (b->type != h->buffertype)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_querybuf: invalid q type(%d) or v4l buffer type(%d)\n", h->buffertype, b->type));
      goto out;
   }

   if (h->buffers == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_querybuf: invalid buffer (none allocated), h(0x%p)->buffers(0x%p)\n", h, h->buffers));
      ret = -ENOBUFS;
      goto out;
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_querybuf: Query buffer(0x%p)(%d)\n", b, b->index));

   // Flags are passed back to the user in *b; the return value is irrelevant.
   ret = rgb133_mmap_buffer_status(h, b);

out:
   h->ExitCriticalSection(h);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querybuf: END - b->length(%d)\n", b->length));

   return ret;
}

void InitBufferAndCopyUser(struct rgb133_handle* h, char* data, int bsize,
      int pixelformat, enum __rgb133_colour colour);

struct rgb133_unified_buffer * GetNextAvailableActiveBuffer(struct rgb133_handle *h, int lock_action)
{
   int i = -1;
   struct rgb133_unified_buffer * buf = NULL;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GNAAB: START\n"));

   h->AcquireSpinLock(h, "GNAAB", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_ACTIVE)
            {
               buf = h->buffers[i];
               break;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GNAAB: h(0x%p)->buffers is NULL\n", h));
   }

   h->ReleaseSpinLock(h, "GNAAB", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GNAAB: Returning buffer %p (%d) as next available\n", buf, buf ? i : -1));
   return buf;
}

static struct v4l2_buffer sBuf;

/*!
 ** Does what it says on the tin; get the next buffer that's available for being a DMA target.
 ** "next" is open to interpretation - this routine *always* selects the lowest numbered
 ** available buffer.  With a well-behaved client, this should end up pingponging just two
 ** buffers all the time.
 */
struct rgb133_unified_buffer * GetNextAvailableQueuedBuffer(struct rgb133_handle *h, int lock_action)
{
   int i = -1;
   struct rgb133_unified_buffer * buf = NULL;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "GNAQB: START\n"));

   h->AcquireSpinLock(h, "GNAQB", lock_action);

   /* Send all buffers as they come */
   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_QUEUED)
            {
               buf = h->buffers[i];
               break;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GNAQB: h(0x%p)->buffers is NULL\n", h));
   }

   if (buf != NULL)
   {
      RGB133PRINTNS((RGB133_LINUX_DBG_INOUT, "GNAQB: Buffer[%d] 0x%p is now a DMA target for handle 0x%p\n",
            buf->index, buf, h));
      buf->state = RGB133_BUF_ACTIVE;
   }

   h->ReleaseSpinLock(h, "GNAQB", lock_action);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GNAQB: Returning buffer[%d] %p as next available\n",
         buf ? i : -1, buf));
   return buf;
}

int rgb133_startdma(struct rgb133_handle* h, struct rgb133_unified_buffer* b);

#define CASE(STATE, level)  case STATE: RGB133PRINT((level, "%s: Buffer %d (memory %p, bid: %p) is in state " #STATE " notify(%d)\n", pMsg, i, b->pMMapMemory, b, b->notify)); break;
#define DEFAULT(level) default: RGB133PRINT((level, "%s: Buffer %d (memory %p, bid: %p) is corrupt. State is %d\n", pMsg, i, b->pMMapMemory, b)); break;

void DumpBufferStates(unsigned int dbgLvl, struct rgb133_handle * h, char * pMsg)
{
   struct rgb133_unified_buffer * b;
   int i;

   if (h->buffers)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         b = h->buffers[i];
         if (b != NULL)
         {
            switch (b->state)
            {
               CASE(RGB133_BUF_NEEDS_INIT, dbgLvl);
               CASE(RGB133_BUF_PREPARED, dbgLvl);
               CASE(RGB133_BUF_QUEUED, dbgLvl);
               CASE(RGB133_BUF_ACTIVE, dbgLvl);
               CASE(RGB133_BUF_DONE, dbgLvl);
               CASE(RGB133_BUF_ERROR, dbgLvl);
               CASE(RGB133_BUF_IDLE, dbgLvl);
               DEFAULT(dbgLvl);
            }
         }
      }
   }
}

/*!
 ** locks and semaphores cannot be held when entering this function, it performs all the locking
 ** necessary for correct operation.
 */
void AttemptDMAStart(struct rgb133_handle *h)
{
   struct rgb133_dev* dev = h->dev;
   struct rgb133_unified_buffer * pBuf;
   BOOL bRequeue = FALSE;
   PRGBCAPTUREAPI pRGBCapture = 0;
   unsigned int found = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "AttemptDMAStart: [%d][%d][%d] START\n",
         h->dev->index, h->channel, h->capture));

   pRGBCapture = CaptureGetCapturePtr(h);
   if (pRGBCapture == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "AttemptDMAStart: pRGBCapture for h(0x%p) is NULL\n", h));
      return;
   }

   if (!rgb133_is_streaming(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "AttemptDMAStart: Handle %p is not streaming.\n", h));
      CaptureSetMultiBufferEvent(h, IO_NO_INCREMENT);
      return;
   }

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "AttemptDMAStart (START)");

   /* Send all queued buffers to the DMA engine */
   while ((pBuf = GetNextAvailableQueuedBuffer(h, LOCK_SPINLOCK)) != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "AttemptDMAStart: Got buffer %d off the incoming queue - rgb133_startdma\n", pBuf->index));
      rgb133_startdma(h, pBuf);
      found = 1;
   }

   /* No queued buffer(s) */
   if (!found)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "AttemptDMAStart: Not attempting start of DMA into NULL buffer\n"));
      CaptureSetMultiBufferEvent(h, IO_NO_INCREMENT);
      return;
   }

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "AttemptDMAStart (END)");

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "AttemptDMAStart: [%d][%d][%d] END\n",
         h->dev->index, h->channel, h->capture));
}

static atomic_t CapNum[RGB133_MAX_DEVICES][RGB133_MAX_CAP_PER_CHANNEL];

void CompleteActiveDMABuffer(struct rgb133_handle *h, void *pRGBCapture, int index, rgb133_notify__t notify, u32 seq, u32 fieldId)
{
   struct rgb133_unified_buffer* pActiveDmaBuffer = NULL;

   h->AcquireSpinLock(h, "CompleteActiveDMABuffer", LOCK_SPINLOCK);
   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "CompleteActiveDMABuffer(START)");

   if (h->buffers == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CompleteActiveDMABuffer: h(0x%p)->buffers is NULL\n", h));
      h->ReleaseSpinLock(h, "CompleteActiveDMABuffer", UNLOCK_SPINLOCK);
      return;
   }

   pActiveDmaBuffer = h->buffers[index];

   if (pActiveDmaBuffer == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CompleteActiveDMABuffer: pRGBCapture[%d].[%d].[%d].(0x%p)->h(0x%p) "
            "pActiveDmaBuffer is NULL - MEM(%d)\n",
            CaptureGetDeviceIndex(pRGBCapture), CaptureGetChannelNumber(pRGBCapture), CaptureGetCaptureNumber(pRGBCapture),
            pRGBCapture, h, h->memorytype));

      DumpBufferStates(RGB133_LINUX_DBG_ERROR, h, "CompleteActiveDMABuffer(Err)");
      h->ReleaseSpinLock(h, "CompleteActiveDMABuffer", UNLOCK_SPINLOCK);
      return;
   }

   /* Complete buffer only if it has not already been selected for dq
    * If we are No Signal, poll may have already returned and buffer may already be given to the application
    * If so, buffer's state is now IDLE and and we should not complete it here
    * If we complete IDLE buffer and set its state to DONE, that same buffer will be selected for DQ next time! */
   if(pActiveDmaBuffer->state != RGB133_BUF_IDLE)
   {
      pActiveDmaBuffer->CapNum = atomic_inc_return(&CapNum[CaptureGetDeviceIndex(pRGBCapture)][CaptureGetCaptureNumber(pRGBCapture)]);
      pActiveDmaBuffer->state = RGB133_BUF_DONE;
      pActiveDmaBuffer->notify = notify;
      pActiveDmaBuffer->seq = seq;
      pActiveDmaBuffer->fieldId = fieldId;
   }

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "CompleteActiveDMABuffer(END)");

   h->ReleaseSpinLock(h, "CompleteActiveDMABuffer", UNLOCK_SPINLOCK);
}

void CompleteActiveDMABuffers(struct rgb133_handle* h, int lock_action, rgb133_notify__t notify)
{
   int i = -1;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CompleteActiveDMABuffers: START\n"));

   h->AcquireSpinLock(h, "CompleteActiveDMABuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_ACTIVE)
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "CompleteActiveDMABuffers: Complete active buffer h->buffers[%d]\n", i));
               h->buffers[i]->state = RGB133_BUF_DONE;
               h->buffers[i]->notify = notify;
               h->buffers[i]->seq = 0;
               h->buffers[i]->fieldId = 0;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CompleteActiveDMABuffers: h(0x%p)->buffers is NULL\n", h));
   }

   h->ReleaseSpinLock(h, "CompleteActiveDMABuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CompleteActiveDMABuffers: END\n"));
}

int rgb133_qbuf(struct file* file, void* priv, struct v4l2_buffer* b)
{
   struct rgb133_handle* h = file->private_data;
   PRGBCAPTUREAPI pRGBCapture = 0;
   PCHANNELAPI pChannel = 0;

   int retval = 0;

   if (old_debug_level != rgb133_debug)
   {
      old_debug_level = rgb133_debug;
      CaptureSetDebugLevel(h, rgb133_debug);
   }

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_qbuf: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_qbuf: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_qbuf: START [%d][%d][%d] pRGBCapture(0x%p) - buffer[%d].length(%d)\n",
         h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h), b->index, b->length));

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "Queue(START)");

   if ((h->memorytype & b->memory) != b->memory)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_qbuf: Buffer memory type(%d) (MMAP/USER) mismatch(%d)\n",
            h->memorytype, b->memory));
      return -EINVAL;
   }

   if ((b->index > h->numbuffers))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_qbuf: buffer index %d exceeds stated limit of %d" "for handle %p\n",
            b->index, h->numbuffers, h));
      return -EINVAL;
   }

   pRGBCapture = CaptureGetCapturePtr(h);
   pChannel = CaptureGetChannelPtr(h);

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_qbuf: [%d][%d][%d] queue buffer(0x%p)\n",
         h->dev->index, h->channel, h->capture, b));

   h->AcquireSpinLock(h, "qbuf", LOCK_SPINLOCK);

   if (rgb133_buffer_queued(h, b))
   {
      h->ReleaseSpinLock(h, "qbuf", UNLOCK_SPINLOCK);

      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_qbuf: buffer %d already queued; AttemptDMAStart\n", b->index));
      AttemptDMAStart(h);

      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_qbuf: REQUEUE END - buffer[%d].length(%d)\n", b->index, b->length));

      return 0;
   }
   else
   {
      RGB133PRINTNS((RGB133_LINUX_DBG_LOG, "rgb133_qbuf: buffer %d not queued, PROCESS\n", b->index));
   }

   h->ReleaseSpinLock(h, "qbuf", UNLOCK_SPINLOCK);

   if (b->memory == V4L2_MEMORY_USERPTR)
   {
      /* Create internal buffer structure */
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_qbuf: setup userptr buffer[%d](0x%p), userptr(0x%lx), size(%d)\n",
            b->index, h->buffers[b->index], b->m.userptr, h->sCapture.imageSize));
      retval = rgb133_userptr_buffer_setup(&h->buffers[b->index], b->m.userptr, b->length, UserMode);
      if (retval < 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_qbuf: failed to init userptr buffer: %d\n", b->length));
         return retval;
      }
      h->sCapture.imageSize = b->length;

      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_qbuf: assign b->m.userptr(0x%lx) to buffer[%d](0x%p)\n",
            b->m.userptr, b->index, h->buffers[b->index]));
      h->buffers[b->index]->pUserMemory = (void*) b->m.userptr;

      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_qbuf: assign pMdl from buffer[%d](0x%p) into pIrp(0x%p)\n",
            b->index, h->buffers[b->index], h->buffers[b->index]->pIrp));

      {
         int i;
         for(i=0; i<(h->sCapture.planes); i++)
            h->buffers[b->index]->pIrp->MdlPlanarAddress[i] = rgb133_userptr_buffer_get_mdl(&h->buffers[b->index], i);
      }
      // we still need this one (used by ReadFlash_V27)
      h->buffers[b->index]->pIrp->MdlAddress = rgb133_userptr_buffer_get_mdl(
            &h->buffers[b->index], 0);
   }
   
   // Setting buffer's state to queued is now moved here so that for USER PTR I/O
   // buffer setup is finished before we set buffer's state to queued
   h->AcquireSpinLock(h, "qbuf", LOCK_SPINLOCK);

   // DMJ TODO: Should this be a call to "set_queued" instead?
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_qbuf: assign h(0x%p)->buffers[%d](0x%p)->state(%d)\n",
         h, b->index, h->buffers[b->index], RGB133_BUF_QUEUED));
   h->buffers[b->index]->CapNum = 0;
   h->buffers[b->index]->ACQ.sec = 0;
   h->buffers[b->index]->ACQ.usec = 0;
   h->buffers[b->index]->state = RGB133_BUF_QUEUED;
   h->buffers[b->index]->notify = RGB133_NOTIFY_RESERVED;
   h->buffers[b->index]->seq = 0;
   h->buffers[b->index]->fieldId = 0;

   h->ReleaseSpinLock(h, "qbuf", UNLOCK_SPINLOCK);

   if (rgb133_streaming_on(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_qbuf: [%d][%d][%d] pRGBCapture(0x%p) buffer[%d].length(%d) AttemptDMAStart\n",
            h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h), b->index, b->length));
      AttemptDMAStart(h);
   }

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "Queue(END)");

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_qbuf: END [%d][%d][%d] pRGBCapture(0x%p) - buffer[%d] length(%d)\n",
         h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h), b->index, b->length));

   return 0;
}

int rgb133_startdma(struct rgb133_handle* h, struct rgb133_unified_buffer* buf)
{
   PRGBCAPTUREAPI pRGBCapture = 0;
   PCHANNELAPI pChannel = 0;

   int retval = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_startdma: [%d][%d][%d] buffer[%d] START\n",
         h->dev->index, h->channel, h->capture, buf->index));

   pRGBCapture = CaptureGetCapturePtr(h);
   pChannel = CaptureGetChannelPtr(h);

   /* If streaming has been stopped don't start the DMA */
   if (!rgb133_is_streaming(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_startdma: [%d][%d][%d] streaming stopped; complete active buffer[%d]\n",
            h->dev->index, h->channel, h->capture, buf->index));
      CompleteActiveDMABuffer(h, pRGBCapture, buf->index, RGB133_NOTIFY_RESERVED, 0, 0);
      CaptureSetMultiBufferEvent(h, IO_INCREMENT);
      return -EIO;
   }

   if (buf->memory != V4L2_MEMORY_MMAP && buf->memory != V4L2_MEMORY_USERPTR)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_startdma: unknown V4L2 memory id(%d)\n", buf->memory));
      CaptureSetMultiBufferEvent(h, IO_INCREMENT);
      return -EINVAL;
   }

   if(!h->pNoSignalBufIn)
   {
      h->pNoSignalBufIn = KernelVzalloc(h->sCapture.imageSize);
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_startdma: [%d][%d][%d] alloc %d bytes to pNoSignalBufIn(0x%p)\n",
            h->dev->index, h->channel, h->capture, (int)h->sCapture.imageSize, h->pNoSignalBufIn));

      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_startdma: [%d][%d][%d] init io pNoSignalBufIn(0x%p) with %s\n",
            h->dev->index, h->channel, h->capture, h->pNoSignalBufIn, (buf->memory == V4L2_MEMORY_MMAP) ? "BLUE" : "YELLOW"));
      CaptureInitBuffer(h, 0, h->pNoSignalBufIn, h->sCapture.imageSize, h->sCapture.pixelformat,
                        ((buf->memory == V4L2_MEMORY_MMAP) ? RGB133_BLUE : RGB133_YELLOW), RGB133_CAP_NO_COPY_USER);
   }

   /* Init the output buffer */
   if (buf->memory == V4L2_MEMORY_MMAP)
   {
      if (buf->pMMapMemory == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_startdma: [%d][%d][%d] buf(0x%p)->pMMapMemory is NULL\n",
                h->dev->index, h->channel, h->capture, buf));
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_startdma: init mmap io signal buffer[%d] with BLUE\n", buf->index));
         memcpy((void *)buf->pMMapMemory, (const void *)h->pNoSignalBufIn, (size_t)h->sCapture.imageSize);
      }
   }
   else if (buf->memory == V4L2_MEMORY_USERPTR)
   {
      if (buf->pUserMemory == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_startdma: dev[%d].channel[%d].capture[%d]." "buf->pUserMemory is NULL\n",
               h->dev->index, h->channel, h->capture));
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_startdma: init userptr io signal buffer with YELLOW\n"));
         KernelCopyToUser((char*)buf->pUserMemory, h->pNoSignalBufIn, h->sCapture.imageSize);
      }
   }

   /* Log the Q timestamp */
   {
      sTimeval tv_q;
      KernelGetCurrentTime((PTIMEVALAPI)&tv_q, NULL, NULL);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_startdma: Timestamp(%lu.%06lu)\n",
            tv_q.tv_sec, tv_q.tv_usec));
      KernelSetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_Q, tv_q.tv_sec, tv_q.tv_usec);
   }

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_startdma: [%d][%d][%d] call CaptureRequestForData buffer[%d]...\n",
         h->dev->index, h->channel, h->capture, buf->index));
   CaptureRequestForData(h, buf);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_startdma: END - [%d][%d][%d] buffer[%d]\n",
         h->dev->index, h->channel, h->capture, buf->index));

   return retval;
}

unsigned long CalculateLostReadTime(struct rgb133_handle *h)
{
   signed long delta;
   unsigned long adjust = 0;

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "CalculateLostReadTime: START\n"));

   if (h->rate.tv_now.tv_sec == 0 && h->rate.tv_now.tv_usec == 0)
   {
      KernelGetCurrentTime((PTIMEVALAPI)&h->rate.tv_now, NULL, NULL);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "CalculateLostReadTime: tv_now (%lu.%lu)\n",
            h->rate.tv_now.tv_sec, h->rate.tv_now.tv_usec));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CalculateLostReadTime: END  - now == then\n"));
      return 0;
   }
   else
   {
      unsigned long long now = 0;
      unsigned long long later = 0;

      KernelGetCurrentTime((PTIMEVALAPI)&h->rate.tv_later, NULL, NULL);

      now = (unsigned long long) (h->rate.tv_now.tv_sec * 1000000LL);
      now += h->rate.tv_now.tv_usec;
      later = (unsigned long long) (h->rate.tv_later.tv_sec * 1000000LL);
      later += h->rate.tv_later.tv_usec;

      delta = (later - now);
      delta -= h->rate.adjust;
      if (h->rate.timeout >= delta)
         adjust = h->rate.timeout - delta;
      else
         adjust = 0;

      h->rate.tv_now.tv_sec = h->rate.tv_later.tv_sec;
      h->rate.tv_now.tv_usec = h->rate.tv_later.tv_usec;
      h->rate.tv_later.tv_sec = 0;
      h->rate.tv_later.tv_usec = 0;
   }
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "CalculateLostReadTime: END\n"));

   return adjust;
}

void NoSignalBufferWait(struct rgb133_handle* h)
{
   unsigned long rate = CaptureRateSet(h);
   unsigned long msdelay = 0;
   unsigned long usdelay = 0;

   if (rate == 0)
   {

      if (h->rate.source)
      {
         /* Use intput rate if no rate set, capped at input rate */
         rate = h->rate.source;
      }
      else
      {
         /* Use default rate if no rate set and not seen an input */
         rate = CaptureGetInputRate(h);

         /* If the rate is still invalid, use total default */
         if (rate == 0) {
            rate = 15000;
         }
      }
   }

   /* Use last known input source rate (or default) */
   /* Rate in in mHz, work out us */
   if (rate == 0)
   {
      // Dont exit, go pop
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "NoSignalBufferWait: rate is zero\n"));
   }

   h->rate.timeout = (10000000000LL / rate) + 5;
   h->rate.timeout /= 10;

   h->rate.adjust = CalculateLostReadTime(h);
   msdelay = h->rate.adjust / 1000;
   usdelay = (h->rate.adjust - (msdelay * 1000));

   if (msdelay > 10000 && msdelay != h->rate.msdelay)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "NoSignalBufferWait: rate capped at 1/10 fps\n"));
      h->rate.msdelay = msdelay;
      msdelay = 10000;
   }
   /* Wait */
   RGB133WAIT(msdelay, usdelay);
}

int NoSignalDrawMessage(struct rgb133_handle* h, struct rgb133_unified_buffer * buf, struct v4l2_buffer* b)
{
   if (buf->pMemory)
   {
      unsigned long flags = CaptureGetDetectedFlags(h, h->channel, NULL);
      int numMessages = 0;
      int i = 0;

      /* Initialise messages buffers in order to not write on top of old messages */
      for(i=0; i<(sizeof(h->pMessagesIn)/sizeof(char*)); i++)
      {
         KernelMemset(h->pMessagesIn[i], 0, MAX_LINE_LENGTH);
      }
      for(i=0; i<(sizeof(h->pMessagesOut)/sizeof(char*)); i++)
      {
         KernelMemset(h->pMessagesOut[i], 0, MAX_LINE_LENGTH);
      }

      if(flags & RGB133_VDIF_FLAG_UNRECOGNISABLE)
      {
         numMessages = 3;
      }
      else
      {
         numMessages = 0;
      }

      //Add messages
      if(numMessages != 0)
      {
         unsigned long VerFreq = 0;
         unsigned long HorFreq = 0;
         unsigned long HTotal = 0;
         unsigned long ActivePixels = 0;
         unsigned long ActiveLines = 0;
         unsigned long LineDuration = 0;
         unsigned long FrameDuration = 0;
         unsigned long HSyncDuration = 0;
         unsigned long VSyncDuration = 0;

         CaptureGetParmsSignalUnrecognisable(h, &VerFreq, &HorFreq, &HTotal, &ActivePixels, &ActiveLines,
                                             &LineDuration, &FrameDuration, &HSyncDuration, &VSyncDuration);

         snprintf(h->pMessagesIn[0], MAX_LINE_LENGTH, "VerFreq: %-u  HorFreq: %-u  HTotal: %-u\n",
                  VerFreq, HorFreq, HTotal);
         snprintf(h->pMessagesIn[1], MAX_LINE_LENGTH, "ActivePixs: %-u  ActiveLns: %-u  LineDur: %-u\n",
                  ActivePixels, ActiveLines, LineDuration);
         snprintf(h->pMessagesIn[2], MAX_LINE_LENGTH, "FrameDur: %-u  HSyncDur: %-u  VSyncDur: %-u\n",
                  FrameDuration, HSyncDuration, VSyncDuration );
      }

      {
         enum rgb133_mapping_type mappingType = (buf->notify == RGB133_NOTIFY_DMA_OVERFLOW) ? __BUFFER_NOTIFICATIONS__ : __VDIFFLAGS__;

         if (!rgb133_is_reading(h))
         {
            if (b->memory == V4L2_MEMORY_MMAP)
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalDrawMessage: show MMAP message on buffer[%d]->pMemory(0x%p)...\n",
                     buf->index, buf->pMemory));
               __mapUItoCC(mappingType, __MAP_TO_STRING__, flags, h->pMessagesIn, h->pMessagesOut, &numMessages);
               __CaptureShowMessage(h, h->pMessagesOut, numMessages, buf->pMemory, 0, h->sCapture.imageSize, RGB133_CAP_NO_COPY_USER);
            }
            else if (b->memory == V4L2_MEMORY_USERPTR)
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalDrawMessage: show USERPTR message on buf[%d](0x%p)->pMemory(0x%p) size(%d - %d)\n",
                     buf->index, buf, buf->pMemory, buf->bsize, h->sCapture.imageSize));
               __mapUItoCC(mappingType, __MAP_TO_STRING__, flags, h->pMessagesIn, h->pMessagesOut, &numMessages);
               __CaptureShowMessage(h, h->pMessagesOut, numMessages, buf->pMemory, buf->pMessageBuffer, h->sCapture.imageSize, RGB133_CAP_COPY_USER);
            }
         }
         else /* READ_IO */
         {
            /* (1) Add one of: "No Signal", "Out Of Range", "Unrecognisable",
               (2) If there are messages in pMessagesIn[], process messages from pMessagesIn[] to pMessagesOut[] */
            __mapUItoCC(mappingType, __MAP_TO_STRING__, flags, h->pMessagesIn, h->pMessagesOut, &numMessages);
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalDrawMessage: show READ IO message on buf[%d](0x%p)->pMemory(0x%p) size(%d - %d)\n",
                  buf->index, buf, buf->pMemory, buf->bsize, h->sCapture.imageSize));
            __CaptureShowMessage(h, h->pMessagesOut, numMessages, buf->pMemory, 0, h->sCapture.imageSize, RGB133_CAP_COPY_USER);
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "NoSignalDrawMessage: [%d][%d][%d] don't show message\n",
            h->dev->index, h->channel, h->capture));
   }

   return 0;
}

void RequeueDMAdBuffers(struct rgb133_handle* h, int lock_action)
{
   int i = -1;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "RequeueDMAdBuffers: START\n"));

   h->AcquireSpinLock(h, "RequeueDMAdBuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_DONE)
            {
               RGB133PRINT((RGB133_LINUX_DBG_INOUT, "RequeueDMAdBuffers: Reset h->buffers[%d]->state to QUEUED\n", i));
               h->buffers[i]->state = RGB133_BUF_QUEUED;
               h->buffers[i]->notify = RGB133_NOTIFY_RESERVED;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "RequeueDMAdBuffers: h(0x%p)->buffers is NULL\n", h));
   }

   h->ReleaseSpinLock(h, "RequeueDMAdBuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "RequeueDMAdBuffers: END\n"));
}

struct rgb133_unified_buffer * GetEarliestDoneBuffer(struct rgb133_handle *h)
{
   struct rgb133_unified_buffer Buf, *pBuf = &Buf;
   int i, SelectedBuffer = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GetEarliestDoneBuffer: START\n"));

   Buf.CapNum = -1;

   for (i = 0; i < h->numbuffers; i++)
   {
      if (h->buffers[i] != NULL)
      {
         if (h->buffers[i]->state & RGB133_BUF_DONE)
         {
            if (h->buffers[i]->CapNum < pBuf->CapNum)
            {
               pBuf = h->buffers[i];
               SelectedBuffer = 1;
            }
         }
         else if (h->buffers[i]->CapNum == pBuf->CapNum)
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "GEDB: Duplicate buffer[%d]->CapNum(%d)\n", i, h->buffers[i]->CapNum));
         }
      }
   }
   if ((pBuf->CapNum == (unsigned long long) -1) && (SelectedBuffer == 0))
   {
      pBuf = NULL;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GEDB selects a NULL buffer.\n"));
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GetEarliestDoneBuffer: END Returning buffer [%d] %p\n",
         (pBuf ? pBuf->index : -1), pBuf));
   return pBuf;
}

#define GNIB_NO_ERROR 0

struct rgb133_unified_buffer * GetNonIdleBuffer(struct rgb133_handle *h)
{
   struct rgb133_unified_buffer Buf, *pBuf = &Buf;
   int i, SelectedBuffer = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GetNonIdleBuffer: START\n"));

   Buf.CapNum = -1;

   for (i = 0; i < h->numbuffers; i++)
   {
      if (h->buffers[i] != NULL)
      {
         if (h->buffers[i]->state == RGB133_BUF_ACTIVE ||
             h->buffers[i]->state == RGB133_BUF_QUEUED ||
             h->buffers[i]->state == RGB133_BUF_DONE)
         {
            pBuf = h->buffers[i];
            pBuf->state = RGB133_BUF_IDLE;
            SelectedBuffer = 1;
            break;
         }
      }
   }
   if ((pBuf->CapNum == (unsigned long long) -1) && (SelectedBuffer == 0))
   {
      pBuf = NULL;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GNIB selects a NULL buffer.\n"));
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GetNonIdleBuffer: END Returning buffer [%d] %p\n",
         (pBuf ? pBuf->index : -1), pBuf));
   return pBuf;
}

#define GEFB_NO_ERROR                          0
#define GEFB_NO_SIGNAL_FILL_FAILED            -1
#define GEFB_FAILED                           -2

int GetEarliestFilledBuffer(struct rgb133_handle * h, struct rgb133_unified_buffer** ppBuf, struct v4l2_buffer* b)
{
   struct rgb133_unified_buffer * pBuf;
   int ret = GEFB_NO_ERROR;
   int i;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GEFB: START\n"));
   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "GEFB(START)");

   h->AcquireSpinLock(h, "GEFB", LOCK_SPINLOCK);

   pBuf = GetEarliestDoneBuffer(h);
   if (pBuf)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "GEFB: returning buffer[%d]\n", pBuf->index));
      pBuf->state = RGB133_BUF_IDLE;

      h->ReleaseSpinLock(h, "GEFB", UNLOCK_SPINLOCK);

      /* Init the no signal buffer */
      if (pBuf->notify != RGB133_NOTIFY_FRAME_CAPTURED)
      {
         if(!h->pNoSignalBufOut)
         {
            h->pNoSignalBufOut = KernelVzalloc(h->sCapture.imageSize);
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "GEFB: [%d][%d][%d] alloc %d bytes to pNoSignalBufOut(0x%p)\n",
                  h->dev->index, h->channel, h->capture, (int)h->sCapture.imageSize, h->pNoSignalBufOut));

            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "GEFB: [%d][%d][%d] init pNoSignalBufOut(0x%p) with %s\n",
                  h->dev->index, h->channel, h->capture, h->pNoSignalBufOut, (pBuf->memory == V4L2_MEMORY_MMAP) ? "CYAN" : "GREY"));
            CaptureInitBuffer(h, 0, h->pNoSignalBufOut, h->sCapture.imageSize, h->sCapture.pixelformat,
                              ((pBuf->memory == V4L2_MEMORY_MMAP) ? RGB133_CYAN : RGB133_GREY), RGB133_CAP_NO_COPY_USER);
         }

         /* Init the output buffer */
         if (pBuf->memory == V4L2_MEMORY_MMAP)
         {
            if (pBuf->pMMapMemory == 0)
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "GEFB: [%d][%d][%d] pBuf(0x%p)->pMMapMemory is NULL\n",
                     h->dev->index, h->channel, h->capture, pBuf));
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "GEFB: init mmap io signal buffer[%d] with CYAN\n", pBuf->index));
               memcpy((void *)pBuf->pMMapMemory, (const void *)h->pNoSignalBufOut, (size_t)h->sCapture.imageSize);
            }
         }
         else if (pBuf->memory == V4L2_MEMORY_USERPTR)
         {
            if (pBuf->pUserMemory == 0)
            {
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "GEFB: [%d][%d][%d] pBuf->pUserMemory is NULL\n",
                     h->dev->index, h->channel, h->capture));
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "GEFB: init userptr io signal buffer with GREY\n"));
               KernelCopyToUser((char*) pBuf->pUserMemory, h->pNoSignalBufOut, h->sCapture.imageSize);
            }
         }

         /* Manual wait, no DMA IRQ to limit the rate for "No Signal" cases */
         NoSignalBufferWait(h);
         /* Add the counter now that we are ready to give the buffer to the user */
         NoSignalDrawMessage(h, pBuf, b);
      }
   }
   else
   {
      h->ReleaseSpinLock(h, "GEFB", UNLOCK_SPINLOCK);
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GEFB: No buffers available\n"));
   }

   *ppBuf = pBuf;

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "GEFB(END)");
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "GEFB: END\n"));
   return ret;
}

int CountQueuedBuffers(struct rgb133_handle* h, int lock_action)
{
   int i = -1;
   int count = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountQueuedBuffers: START\n"));

   h->AcquireSpinLock(h, "CountQueuedBuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_QUEUED)
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "CountQueuedBuffers: Active buffer h->buffers[%d]\n", i));
               count++;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CountQueuedBuffers: h(0x%p)->buffers is NULL\n", h));
      count = -1;
   }

   h->ReleaseSpinLock(h, "CountQueuedBuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountQueuedBuffers: END - count(%d)\n", count));
   return count;
}

int CountActiveDMABuffers(struct rgb133_handle* h, int lock_action)
{
   int i = -1;
   int count = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountActiveDMABuffers: START\n"));

   h->AcquireSpinLock(h, "CountActiveDMABuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if (h->buffers[i]->state & RGB133_BUF_ACTIVE)
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "CountActiveDMABuffers: Active buffer h->buffers[%d]\n", i));
               count++;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CountActiveDMABuffers: h(0x%p)->buffers is NULL\n", h));
      count = -1;
   }

   h->ReleaseSpinLock(h, "CountActiveDMABuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountActiveDMABuffers: END - count(%d)\n", count));
   return count;
}

int CountActiveOrDoneDMABuffers(struct rgb133_handle* h, int lock_action)
{
   int i = -1;
   int count = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountActiveOrDoneDMABuffers: START\n"));

   h->AcquireSpinLock(h, "CountActiveOrDoneDMABuffers", lock_action);

   if (h->buffers != 0)
   {
      for (i = 0; i < h->numbuffers; i++)
      {
         if (h->buffers[i] != NULL)
         {
            // Simple search-for-first algorithm.
            if ((h->buffers[i]->state & RGB133_BUF_ACTIVE) || (h->buffers[i]->state & RGB133_BUF_DONE))
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "CountActiveOrDoneDMABuffers: Active or Done buffer h->buffers[%d]\n", i));
               count++;
            }
         }
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "CountActiveOrDoneDMABuffers: h(0x%p)->buffers is NULL\n", h));
      count = -1;
   }

   h->ReleaseSpinLock(h, "CountActiveOrDoneDMABuffers", lock_action);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CountActiveOrDoneDMABuffers: END - count(%d)\n", count));
   return count;
}

void SignalTransition(struct rgb133_handle* h)
{
   h->no_signal = FALSE;

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "SignalTransition: Channel[%d].Capture[%d] call APIFromNoSignal\n",
         h->channel, h->capture));
   KernelMutexLock(h->dev->pLock);
   h->dev->transition[h->channel] = TRUE;
   if(APIFromNoSignal(h) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "SignalTransition: Channel[%d].Capture[%d] APIFromNoSignal failed\n",
            h->channel, h->capture));
   }
   KernelMutexUnlock(h->dev->pLock);

   /* re-initialise no signal stuff */
   h->no_sig_jiffies = 0;
   h->day = 0;
   h->hr = 0;
   h->m  = 0;
   h->s  = 0;
   h->ms = 0;

   /* re-initialise rate stuff */
   h->rate.timeout = 0;
   h->rate.source = 0;
   h->rate.tv_now.tv_sec = 0;
   h->rate.tv_now.tv_usec = 0;
   h->rate.tv_later.tv_sec = 0;
   h->rate.tv_later.tv_usec = 0;

   /* re-initialise fps debug stuff */
   h->frame_count = 0;

   /* re-initialise frame info */
   h->frame.seq = -1;
   h->frame.ts.tv_sec = -1;
   h->frame.ts.tv_usec = -1;

   /* Invalidate No Signal Buffers */
   rgb133_invalidate_buffers(h);
}

void NoSignalTransition(struct rgb133_handle* h)
{
   h->no_signal = TRUE;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalTransition: Channel[%d].Capture[%d] Has a transition been signalled?\n",
         h->channel, h->capture));
   if(h->dev->transition[h->channel] == FALSE)
   {
      KernelMutexLock(h->dev->pLock);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalTransition: Channel[%d].Capture[%d] Locked mutex, has transition been signalled?\n",
            h->channel, h->capture));
      if(h->dev->transition[h->channel] == FALSE)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalTransition: Channel[%d].Capture[%d] No transition signalled, signal\n",
               h->channel, h->capture));
         h->dev->transition[h->channel] = TRUE;
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalTransition: Channel[%d].Capture[%d] Transition signalled, no need to signal\n",
               h->channel, h->capture));
      }
      KernelMutexUnlock(h->dev->pLock);
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "NoSignalTransition: Channel[%d].Capture[%d] Transition signalled, don't attempt transition signal\n",
            h->channel, h->capture));
   }

   /* re-initialise rate stuff */
   h->rate.timeout = 0;

   /* re-initialise no signal stuff */
   h->frame_count = 0;

   /* Invalidate No Signal Buffers */
   rgb133_invalidate_buffers(h);
}

int GetFrameInfo(struct rgb133_handle* h, struct rgb133_unified_buffer * pBuf, unsigned int* pSeq, unsigned int* pField)
{
   unsigned int fieldId = 0;
   unsigned long type = 0;

   if(h == NULL || pBuf == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "GetFrameInfo: Invalid capture handle(0x%p) or buffer(0x%p)\n", h, pBuf));
      return -1;
   }
   if(pBuf->notify == RGB133_NOTIFY_FRAME_CAPTURED)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "GetFrameInfo: Get frame info for capture[%d]\n",
            h->capture));

      *pSeq = pBuf->seq;
      fieldId = pBuf->fieldId;

      *pField = rgb133V4L2GetFieldNone();
      if(CaptureVidMeasIsInterlaced(CaptureGetVidMeasFlags(h)))
      {
         CaptureGetClientInterlaced(h, &type, pSeq, pField, &fieldId);
      }
   }
   else
   {
      /* Initialise sequence count if necessary and add one */
      if(h->frame.seq == -1)
         h->frame.seq = 0;
      *pSeq = h->frame.seq + 1;

      /* Initialise timestamp if necessary and add one frame time*/
      if(h->frame.ts.tv_usec == -1)
      {
         unsigned long sec, usec;

         KernelDoGettimeofday(&sec, &usec);

         h->frame.ts.tv_sec = sec;
         h->frame.ts.tv_usec = usec;
      }
   }

   return 0;
}

#define RGBDEINTERLACE_FRAME        0
#define RGBDEINTERLACE_FIELD_0      1
#define RGBDEINTERLACE_FIELD_1      2
#define RGBDEINTERLACE_ALTERNATE    4
#define RGBDEINTERLACE_HARDWARE_BOB 8

extern unsigned long rgb133_frame_debug;

int ReportFrameInfo(struct rgb133_handle* h, int seq, unsigned int field, sTimeval ts)
{
   unsigned long rate, sourceRate;
   unsigned long sourceTime, sourceSec, sourceUsec;
   unsigned long time, sec, usec;
   unsigned long type = 0;
   int nonSeqFramesGuard = 1;

   if(rgb133_frame_debug == RGB133_FRAME_DEBUG)
   {
      /* Get deinterlace type */
      CaptureGetClientInterlaced(h, &type, 0, 0, 0);

      /* Non-sequential frames */
      if(h->frame.seq != -1 && (seq - h->frame.seq) != 1)
      {
         /* Also, check for alternate deinterlacing mode */
         if(type != RGBDEINTERLACE_ALTERNATE || (h->field - field == 0) || (seq - h->frame.seq != 0))
         {
            RGB133PRINT((RGB133_LINUX_DBG_TODO, "ReportFrameInfo: [%d][%d][%d] Non-sequential Seq - Now(%d) Prev(%d) Field(%d)\n",
                  h->dev->index, h->channel, h->capture,
                  seq, h->frame.seq, field));
         }

         nonSeqFramesGuard = 0;
      }

      /* Check if just started capture */
      if(h->frame.ts.tv_sec != -1 && h->frame.ts.tv_usec != -1)
      {
         /* Get the input source rate */
         sourceRate = CaptureGetInputRate(h);

         /* If the rate is still invalid, use total default */
         if(sourceRate == 0)
         {
            sourceRate = 15000;
         }

         /* Get the capture (decimated) rate */
         rate = CaptureRateSet(h);
         if(rate == 0)
         {

            if(h->rate.source)
            {
               /* Use input rate if no rate set, capped at input rate */
               rate = h->rate.source;
            }
            else
            {
               /* Use default rate if no rate set and not seen an input */
               rate = CaptureGetInputRate(h);

               /* If the rate is still invalid, use total default */
               if(rate == 0)
               {
                  rate = 15000;
               }
            }
         }

         /* Non-sequential timestamps */
         if(nonSeqFramesGuard)
         {
            unsigned long diff_s, diff_us;
            unsigned long exp_s, exp_us;
            unsigned long min_us;
            unsigned long us_overflow = 0;
            unsigned long tolerance;

            /* Adjust if Bobbing or Alternating */
            if ((type & RGBDEINTERLACE_HARDWARE_BOB) | (type & RGBDEINTERLACE_ALTERNATE))
            {
               sourceRate *= 2;
               rate *= 2;
            }

            /* Turn source rate into time per frame */
            sourceTime = 1000000000 / sourceRate;
            sourceSec = sourceTime / 1000000;
            sourceUsec = sourceTime % 1000000;

            /* Turn capture rate into time per frame */
            time = 1000000000 / rate;
            sec = time / 1000000;
            usec = time % 1000000;

            diff_s = ts.tv_sec - h->frame.ts.tv_sec;
            if(h->frame.ts.tv_usec > ts.tv_usec)
            {
               diff_s--;
               us_overflow = 1000000;
            }
            diff_us = (us_overflow + ts.tv_usec) - h->frame.ts.tv_usec;

            min_us = ((sourceUsec-50)/100); /* Minimum diff is one frame time, due to decimation effects */
            min_us = (min_us/10)*100;
            exp_s = sec;
            exp_us = ((usec+50)/100);    /* Allow for precision in the driver timestamp number */
            tolerance = (exp_us/10)*100; /* Tolerance is a tenth of a frame time */
            exp_us *= 100;               /* Multiply back up into us units */
            if((diff_s != exp_s) ||
               /*(diff_us < min_us) ||*/
               (diff_us > (exp_us + tolerance)) ||
               (diff_us < min_us))
            {
               RGB133PRINT((RGB133_LINUX_DBG_TODO, "ReportFrameInfo: [%d][%d][%d] Non-sequential TS - Now(%u.%06u) Prev(%u.%06u)\n",
                     h->dev->index, h->channel, h->capture,
                     ts.tv_sec, ts.tv_usec,
                     h->frame.ts.tv_sec, h->frame.ts.tv_usec));
               RGB133PRINT((RGB133_LINUX_DBG_TODO, "ReportFrameInfo: [%d][%d][%d] Non-sequential TS - Diff(%u.%06u) Exp(%u.%06u) Tol(%u.%06u) Min(%u.%06u)\n",
                     h->dev->index, h->channel, h->capture,
                     diff_s, diff_us,
                     exp_s, exp_us,
                     exp_s, (exp_us + tolerance),
                     0, min_us));
            }
         }
      }
   }

   h->frame.seq = seq;
   h->field = field;
   h->frame.ts.tv_sec = ts.tv_sec;
   h->frame.ts.tv_usec = ts.tv_usec;
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "ReportFrameInfo: [%d][%d][%d] Assign Prev(%u.%06u) to frame(%d)\n",
         h->dev->index, h->channel, h->capture,
         h->frame.ts.tv_sec, h->frame.ts.tv_usec, h->frame.seq));

   return 0;
}

#define RGB133_FPS_FRAME_COUNT 500

int ReportFPS(struct rgb133_handle* h)
{
   int fps = -1;
   int fps_rem = -1;

   /* Log start of frame count if necessary */
   h->frame_count++;

   if(h->frame_count == 1)
   {
      // Log time of first frame
      KernelGetCurrentTime((PTIMEVALAPI)&h->now, NULL, NULL);
   }
   else if(h->frame_count == RGB133_FPS_FRAME_COUNT)
   {
      // Log the time of the 500th frame
      KernelGetCurrentTime((PTIMEVALAPI)&h->later, NULL, NULL);
   }

   if(h->frame_count == RGB133_FPS_FRAME_COUNT)
   {
      int diff_sec = h->later.tv_sec - h->now.tv_sec;
      int diff_usec = h->later.tv_usec - h->now.tv_usec;
      unsigned long diff = (diff_sec * 1000) + (diff_usec / 1000);
      if(diff != 0)
      {
         fps = ((RGB133_FPS_FRAME_COUNT-1)*1000) / diff;
         fps_rem = (((RGB133_FPS_FRAME_COUNT-1)*1000) % diff);
         if(fps_rem >= 500)
            fps++;
      }
      else
      {
         fps = 0;
         fps_rem = 0;
      }

      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "ReportFPS: dev[%d]->Channel[%d].Capture[%d] -> fps(%d) -> IO(%s)\n",
            h->dev->index, h->channel, h->capture, fps, rgb133V4L2GetIOType(&h->memorytype)));
      h->frame_count = 0;
   }

   return fps;
}

extern int rgb133_show_fps;

int rgb133_dqbuf(struct file* file, void* priv, struct v4l2_buffer* b)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_unified_buffer * buf = 0;
   PRGBCAPTUREAPI pRGBCapture = 0;

   int retval = 0, rc;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_dqbuf: START [%d][%d][%d] pRGBCapture(0x%p)\n",
         h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h)));

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "Dequeue (START)");

   pRGBCapture = CaptureGetCapturePtr(h);

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: [%d][%d][%d] dq buffer(0x%p)\n",
         h->dev->index, h->channel, h->capture, b));

   h->EnterCriticalSection(h);

   /* GetEarliestFilledBuffer will return the earliest filled buffer *and* dequeue it */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: Calling GetEarliestFilledBuffer\n"));
   rc = GetEarliestFilledBuffer(h, &buf, b);
   if (buf)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GEFB returns buffer %d as returnable\n", buf->index));
      if (rc != GEFB_NO_ERROR)
      {
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GEFB returned buffer %d with error %d\n",
               buf->index, rc));
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GEFB returned NULL buffer\n"));
      if (file->f_flags & O_NONBLOCK)
      {
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: Non-blocking set, so returning EAGAIN\n"));
         h->ExitCriticalSection(h);
         return -EAGAIN;
      }
      else
      {
         int count = 0;
         NTSTATUS status = STATUS_SUCCESS;
         LARGE_INTEGER timeout;
         timeout.QuadPart = -10000000L;

         /* Are there any active buffers, if so we need to wait...or if streaming has been turned off */
         count = CountActiveDMABuffers(h, LOCK_SPINLOCK);
         if (count > 0)
         {
            // Wait for DMA to finish, *then* re-inspect.
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: Waiting for pRGBCapture(0x%p) DMA to finish on [%d][%d][%d]\n",
                  pRGBCapture, h->dev->index, h->channel, h->capture));
            if ((status = OSWaitForSingleObject(CaptureGetMultiBufferEvent(pRGBCapture), Executive, KernelMode, FALSE, &timeout)) == STATUS_TIMEOUT)
            {
               RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: pRGBCapture(0x%p) timeout waiting for DMA on [%d][%d][%d]\n",
                     pRGBCapture, h->dev->index, h->channel, h->capture));
               h->ExitCriticalSection(h);
               return -EINVAL;
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: pRGBCapture(0x%p) DMA has finished on [%d][%d][%d]\n",
                     pRGBCapture, h->dev->index, h->channel, h->capture));

               rc = GetEarliestFilledBuffer(h, &buf, b);
               if (buf)
               {
                  RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GEFB returns buffer %d as returnable on 2nd attempt\n",
                        buf->index));
                  if (rc != GEFB_NO_ERROR)
                  {
                     RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GEFB returned buffer %d with error %d on 2nd attempt\n",
                           buf->index, rc));
                  }
               }
               else
               {
                  if (rgb133_get_streaming(h) == RGB133_STRM_OFF)
                  {
                     /* If we're dequeuing after turning streaming off we need to dequeue all the buffers */
                     if ((buf = GetNonIdleBuffer(h)) == NULL)
                     {
                        h->ExitCriticalSection(h);
                        return -EINVAL;
                     }
                     else
                     {
                        RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GetNonIdleBuffer(active) returned buf[%d] for [%d][%d][%d]\n",
                              buf->index, h->dev->index, h->channel, h->capture));
                     }
                  }
                  else
                  {
                     // Oh.
                     RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: DMA has finished but no available buffers [%d][%d][%d] return -EAGAIN\n",
                           h->dev->index, h->channel, h->capture));
                     h->ExitCriticalSection(h);
                     return -EAGAIN;
                  }
               }
            }
         }
         else if (count == 0 && rgb133_get_streaming(h) == RGB133_STRM_ON)
         {
            if ((buf = GetNonIdleBuffer(h)) == NULL)
            {
               h->ExitCriticalSection(h);
               return -EINVAL;
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GetNonIdleBuffer(no active) returned buf[%d] for [%d][%d][%d]\n",
                     buf->index, h->dev->index, h->channel, h->capture));
            }
         }
         else if (rgb133_get_streaming(h) == RGB133_STRM_OFF)
         {
            if ((buf = GetNonIdleBuffer(h)) == NULL)
            {
               h->ExitCriticalSection(h);
               return -EINVAL;
            }
            else
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: GetNonIdleBuffer(streaming off) returned buf[%d] for [%d][%d][%d]\n",
                     buf->index, h->dev->index, h->channel, h->capture));
            }
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_dqbuf: Counting Active DMA buffers failed [%d][%d][%d]\n",
                  h->dev->index, h->channel, h->capture));
            h->ExitCriticalSection(h);
            return -EINVAL;
         }
      }
   }

   /* Log the DQ timestamp */
   if (rgb133_timestamp_info != RGB133_TIMESTAMP_INFO_SILENT)
   {
      sTimeval tv_dq;
      KernelGetCurrentTime((PTIMEVALAPI)&tv_dq, NULL, NULL);
      KernelSetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_DQ, tv_dq.tv_sec, tv_dq.tv_usec);

      if (rgb133_timestamp_info == RGB133_TIMESTAMP_INFO_NOISY)
      {
         unsigned long acq_sec, acq_usec;
         unsigned long q_sec, q_usec;
         unsigned long dma_sec, dma_usec;
         unsigned long dq_sec, dq_usec;
         signed long lat_sec, lat_usec;

         KernelGetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_ACQ, &acq_sec, &acq_usec);
         KernelGetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_DQ, &dq_sec, &dq_usec);
         if (rgb133_timestamp_info == RGB133_TIMESTAMP_INFO_NOISY)
         {
            KernelGetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_Q, &q_sec, &q_usec);
            KernelGetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_DMA, &dma_sec, &dma_usec);
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_dqbuf: ACQ(%lu.%06lu)\n", acq_sec, acq_usec));
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_dqbuf: Q  (%lu.%06lu)\n", q_sec, q_usec));
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_dqbuf: DMA(%lu.%06lu)\n", dma_sec, dma_usec));
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_dqbuf: DQ (%lu.%06lu)\n", dq_sec, dq_usec));
         }
         lat_sec = dq_sec - acq_sec;
         lat_usec = dq_usec - acq_usec;
         if (lat_usec < 0)
         {
            lat_sec--;
            lat_usec *= -1;
         }
         RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_dqbuf: LAT(%lu.%06lu)\n", lat_sec, lat_usec));
      }
   }

   if (b->memory == V4L2_MEMORY_USERPTR)
   {
      b->index = buf->index;
      b->m.userptr = (unsigned long) buf->pUserMemory;
      b->length = buf->bsize;
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_dqbuf: dequeuing userptr buffer[%d](0x%lx) of size(%d)\n",
            b->index, b->m.userptr, b->length));

      rgb133_userptr_buffer_free(&buf, UserMode);
   }
   else
   {
      b->length = buf->bsize;
   }

   GetFrameInfo(h, buf, &b->sequence, &b->field);
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_dqbuf: Seq(%lu), field(%lu)\n", b->sequence, b->field));

   if (rgb133_show_fps == RGB133_SHOW_FPS)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_dqbuf: Call ReportFPS()\n"));
      h->fps = ReportFPS(h);
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_dqbuf: [%d][%d][%d] buffer[%d] Set bytesused(%u)\n",
         h->dev->index, h->channel, h->capture, buf->index, h->sCapture.imageSize));
   b->bytesused = h->sCapture.imageSize;
   b->index = buf->index; // Need to tell the user which buffer is being used!

   {
      unsigned long sec, usec;

      KernelGetTimestamp(h, buf->index, RGB133_LINUX_TIMESTAMP_ACQ, &sec, &usec);
      b->timestamp.tv_sec = sec;
      b->timestamp.tv_usec = usec;
   }

   ReportFrameInfo(h, b->sequence, b->field, b->timestamp);

   h->ExitCriticalSection(h);

   DumpBufferStates(RGB133_LINUX_DBG_INOUT, h, "Dequeue (END)");

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_dqbuf: END - [%d][%d][%d] - buffer[%d] seq(%lu) - retval(%d)\n",
         h->dev->index, h->channel, h->capture, b->index, b->sequence, retval));

#if defined(DELAY_EACH_FRAME)
   msleep(10000);
#endif
   return retval;
}

int rgb133_streamon(struct file* file, void* priv, enum v4l2_buf_type i)
{
   struct rgb133_handle* h = priv;
   PGETPARMSOUTAPI pParmsOut = 0;

   int retval = -EINVAL;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_streamon: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_streamon: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamon: START [%d][%d][%d] pRGBCapture(0x%p)\n",
         h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h)));

   retval = -EBUSY;
   if (rgb133_is_reading(h))
      return -EBUSY;

   retval = 0;

   h->EnterCriticalSection(h);

   if (rgb133_is_streaming(h))
   {
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamon: STREAMING END [%d][%d][%d] pRGBCapture(0x%p)\n", h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h)));
      h->ExitCriticalSection(h);
      return 0;
   }

   rgb133_set_streaming(h, RGB133_STRM_ON);
   CaptureClearMultiBufferEvent(h);

   /* Get the min/max values for HorAddrTime & VerAddrTime */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_streamon: get min/max device parms\n"));
   if ((pParmsOut = CaptureGetDeviceParametersReturnDeviceParms(h->dev, 0,
         0, 0, 0, 0, h->channel)) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_streamon: Failed to get min/max vid timing parms\n"));
   }
   else
   {
      /* Dump out the video timings */
      CaptureDumpVideoTimingsFromDeviceParms(h, pParmsOut);

      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_streamon: free pParmsOut(0x%p)\n", pParmsOut));
      KernelVfree(pParmsOut);
   }

   h->ExitCriticalSection(h);

   AttemptDMAStart(h);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamon: END [%d][%d][%d] pRGBCapture(0x%p)\n",
         h->dev->index, h->channel, h->capture, CaptureGetCapturePtr(h)));

   return retval;
}

int rgb133_streamoff(struct file* file, void* priv, enum v4l2_buf_type i)
{
   struct rgb133_handle* h = priv;
   PRGBCAPTUREAPI pRGBCapture = 0;
   int retval = -EINVAL;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_streamoff: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_streamoff: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   pRGBCapture = CaptureGetCapturePtr(h);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamoff: START [%d][%d][%d] pRGBCapture(0x%p)\n",
         h->dev->index, h->channel, h->capture, pRGBCapture));

   h->EnterCriticalSection(h);
   if (!rgb133_is_streaming(h))
   {
      h->ExitCriticalSection(h);
      goto out;
   }
   h->ExitCriticalSection(h);

   rgb133_set_streaming(h, RGB133_STRM_OFF);

   APIDisableCapture(h);

   CaptureWaitForData(h);  // Wait for anything to finish.

   if (CountActiveDMABuffers(h, 1) > 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamoff: wait for AutoCap [%d][%d][%d] pRGBCapture(0x%p)\n",
            h->dev->index, h->channel, h->capture, pRGBCapture));
      OSWaitForSingleObject(CaptureGetMultiBufferEvent(pRGBCapture), Executive, KernelMode, FALSE, NULL);
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_streamoff: waited for AutoCap [%d][%d][%d] pRGBCapture(0x%p)\n",
            h->dev->index, h->channel, h->capture, pRGBCapture));
   }

   retval = 0;

out:
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_streamoff: END [%d][%d][%d] pRGBCapture(0x%p)\n",
      h->dev->index, h->channel, h->capture, pRGBCapture));

   return retval;
}

#ifdef RGB133_CONFIG_HAVE_VIDIOC_S_STD_FIX
int rgb133_s_std(struct file* file, void* priv, v4l2_std_id norm)
#else
int rgb133_s_std(struct file* file, void* priv, v4l2_std_id* norm)
#endif
{
#ifdef RGB133_CONFIG_HAVE_VIDIOC_S_STD_FIX
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_std: TODO: %d\n", (int)norm));
#else
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_std: TODO: %d\n", (int)*norm));
#endif
   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_std: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }
   return 0;
}

int rgb133_g_std(struct file* file, void* priv, v4l2_std_id* norm)
{
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_std: TODO: %d\n", (int)*norm));
   return 0;
}

int rgb133_enum_input(struct file* file, void* priv, struct v4l2_input* inp)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;
   const char* modestr = NULL;
   unsigned short flags;
   int channels = dev->channels;
   int __channel = -1;
   unsigned int n;
   SIZE_T size;
   SIZE_T str_len;
   int error = 0;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT(
            (RGB133_LINUX_DBG_WARNING, "rgb133_enum_input: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_enum_input: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_enum_input: START\n"));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_enum_input: enumerate input[%d][%d](0x%p) [max(%d)]\n",
         inp->index, h->channel, inp, channels));

   n = inp->index;

   if (n >= channels)
   {
      if (n == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_input: can't enumerate input[%d] - max(%d)\n", n, channels));
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_input: can't enumerate input[%d] - max(%d)\n", n, channels));
      }
      return -EINVAL;
   }

   memset(inp, 0, sizeof(*inp));

   if (rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_input: no expose inputs, set index to n(%d)\n", n));
      inp->index = n;
      __channel = n;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_input: expose inputs, force index to 0\n"));
      inp->index = 0;
      __channel = h->channel;
   }
   inp->type = V4L2_INPUT_TYPE_CAMERA;
   inp->audioset = 0;
   inp->std = RGB133_NORMS;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_input: get detected mode for h(0x%p)->channel(%d), index(%u)", 
         h, __channel, inp->index))

   modestr = CaptureGetVidMeasString(h, __channel, &error);
   if (error)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_input: Invalid detected mode - error(%d)\n", error));
      return -EINVAL;
   }
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_input: Got detected mode (%s)", modestr))

   sprintf(inp->name, modestr);

   size = sizeof(inp->name);
   str_len = strlen(inp->name);

   if (CaptureGetDetectedFlags(h, __channel, NULL) & RGB133_VDIF_FLAG_VIDEO)
   {
      snprintf(inp->name + str_len, size - str_len, " Input%d", __channel);
      str_len = strlen(inp->name);
   }

   snprintf(inp->name + str_len, size - str_len, " (%s)", DeviceGetConnectorType(h->dev->pDE, __channel));

   if (CaptureIsBadSignal(h, __channel))
   {
      inp->status |= V4L2_IN_ST_NO_SIGNAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_enum_input: END\n"));

   return 0;
}

int rgb133_g_input(struct file* file, void* priv, unsigned int* i)
{
   struct rgb133_handle* h = file->private_data;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_input: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_input: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_input: START\n"));

   if (rgb133_expose_inputs == RGB133_NO_EXPOSE_INPUTS)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_input: put input(%d) into i(0x%p)\n", h->channel, i));
      *i = h->channel;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_input: force(0) into i - exposed inputs\n"));
      *i = 0;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_input: END\n"));

   return 0;
}

int rgb133_s_input(struct file* file, void* priv, unsigned int i)
{
   struct rgb133_handle* h = file->private_data;
   struct rgb133_dev* dev = h->dev;
   PIRP pIrp = NULL;

   int ret = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_input: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_input: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_input: START - h(0x%p) [%d][%d][%d] - [%d] - pRGBCapture[%d][%d](0x%p)\n",
         h, h->dev->index, h->channel, h->capture, i, CaptureGetChannelNumber(CaptureGetCapturePtr(h)), CaptureGetCaptureNumber(CaptureGetCapturePtr(h)), CaptureGetCapturePtr(h)));

   KernelMutexLock(dev->pLock);

   if (i >= dev->channels)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_input: requested channel(%d) > max channels(%d)\n", i, dev->channels));
      KernelMutexUnlock(dev->pLock);
      return -EINVAL;
   }

   if (h->channel == i)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Requested channel(%d) same as current channel(%d)\n", i, h->channel));
      RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_input: END - Same Input\n"));
      KernelMutexUnlock(dev->pLock);
      return 0;
   }
   else
   {
      if (rgb133_expose_inputs == RGB133_EXPOSE_INPUTS)
      {
         if (i != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_input: Requested input(%d) non-zero and inputs exposed.\n", i));
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_input: END - Exposed non-0\n"));
            KernelMutexUnlock(dev->pLock);
            return -EINVAL;
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Always input 0 when exposing inputs\n"));
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_input: END - Exposed 0\n"));
            KernelMutexUnlock(dev->pLock);
            return 0;
         }
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Close video device on dev[%d].channel(%d).capture(%d)\n", dev->index, h->channel, h->capture));

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_input: Disable dev[%d].channel[%d].capture[%d]\n", dev->index, h->channel, h->capture));
   APIDisableCapture(h);

   /* Drop into Windows driver close */
   if ((ret = APICloseCapture(h)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_input: APICloseCapture failed\n"));
      KernelMutexUnlock(dev->pLock);
      return ret;
   }

   if (!h->bSysFSOpen)
   {
      h->EnterCriticalSection(h);

      if (rgb133_is_mapped(h))
      {
         if (h->buffers)
         {
            int j;
            for (j = 0; j < h->numbuffers; j++)
            {
               RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: free mmapped buffers in h(0x%p)->buffers[%d](0x%p)(0x%p)\n", h, j, &h->buffers[j], h->buffers[j]));
               if (h->buffers[j] != NULL)
               {
                  __rgb133_buffer_mmap_dma_free(&h->buffers[j]);
               }
            }
            rgb133_set_mapped(h, FALSE);
            h->numbuffers = 0;
         }
      }

      h->ExitCriticalSection(h);

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Uninit handle q\n"));
      rgb133_q_uninit(h);
   }

   /* Free up the (no) signal buffer */
   if (h->pNoSignalBufIn)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_s_input: free pNoSignalBufIn(0x%p)\n", h->pNoSignalBufIn));
      KernelVfree(h->pNoSignalBufIn);
      h->pNoSignalBufIn = NULL;
   }
   if (h->pNoSignalBufOut)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_s_input: free pNoSignalBufOut(0x%p)\n", h->pNoSignalBufOut));
      KernelVfree(h->pNoSignalBufOut);
      h->pNoSignalBufOut = NULL;
   }

   /* Assign new channel */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Set channel(%d)\n", i));
   h->channel = i;

   if (rgb133_expose_inputs == RGB133_EXPOSE_INPUTS)
   {
      /* Lookup the default channel number, only 1 channel per device when
       * running in exposed input mode. */
      int loop = 0;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Override channel(%d)\n", i));
      for (loop = 0; loop < RGB133_MAX_CHANNEL_PER_CARD; loop++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_input: dev[%d](0x%p)->pDevMap[%d].minor(%d) h->minor(%d)\n",
               dev->index, dev, loop, dev->pDevMap[loop].minor, h->minor));
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_input: dev[%d](0x%p)->pDevMap[%d].minor(%d) h->minor(%d)\n",
                     dev->index, dev, loop, dev->pDevMap[loop].minor, h->minor));
         if (dev->pDevMap[loop].minor == h->minor)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_input: matched minor(%d) @ (%d)\n", h->minor, loop));
            break;
         }
      }

      if (loop == RGB133_MAX_CHANNEL_PER_CARD)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_input: failed to match h->minor(%d)\n", h->minor));
         KernelMutexUnlock(dev->pLock);
         return -ENODEV;
      }

      h->channel = dev->pDevMap[loop].channel;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: using h->channel(%d)\n", h->channel));
   }

   /* Open the capture */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_s_input: Open new channel(%d).capture(%d)\n", h->channel, h->capture));
   /* Set up !transitory! Irp for this open */
   pIrp = AllocAndSetupIrp(h);
   if(NULL == pIrp)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_input: failed to allocate IRP\n"));
      KernelMutexUnlock(dev->pLock);
      return -ENOMEM;
   }

   if ((ret = APIOpenCapture(h, pIrp)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_input: APIOpenCapture failed(%d)\n", ret));
      FreeIrp(pIrp);
      KernelMutexUnlock(dev->pLock);
      return ret;
   }
   /* Free transitory Irp  - it's not needed any more */
   FreeIrp(pIrp);

   /* Enable the capture */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: enable dev[%d].channel[%d].capture(%d)\n", h->dev->index, h->channel, h->capture));
   if ((ret = APIEnableCapture(h)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_input: APIEnableCapture failed(%d)\n", ret));
      KernelMutexUnlock(dev->pLock);
      return ret;
   }

   /* Initialise the file handle data */
   /* Initialise no signal stuff */
   h->no_sig_jiffies = 0;
   h->day = 0;
   h->hr = 0;
   h->m = 0;
   h->s = 0;
   h->ms = 0;

   /* Initialise rate stuff */
   h->rate.timeout = 0;
   h->rate.tv_now.tv_sec = 0;
   h->rate.tv_now.tv_usec = 0;
   h->rate.tv_later.tv_sec = 0;
   h->rate.tv_later.tv_usec = 0;

   /* Initialise fps stuff */
   h->frame_count = 0;

   /* Initialise frame info */
   h->frame.seq = -1;
   h->frame.ts.tv_sec = -1;
   h->frame.ts.tv_usec = -1;

   if(!h->dev->init[h->channel])
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_input: Setup initial control values for channel[%d].capture[%d]\n",
            h->channel, h->capture));
      CaptureSetControls(h);

      /* Flag channel on device as initialised */
      h->dev->init[h->channel] = 1;
   }

   KernelMutexUnlock(dev->pLock);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_input: END - h(0x%p) [%d][%d][%d] - pRGBCapture[%d][%d](0x%p)\n",
         h, h->dev->index, h->channel, h->capture, CaptureGetChannelNumber(CaptureGetCapturePtr(h)),
         CaptureGetCaptureNumber(CaptureGetCapturePtr(h)), CaptureGetCapturePtr(h)));

   return 0;
}

int rgb133_ctrl_disabled(unsigned int id, eVWSignalType type)
{
   if (type == VW_TYPE_DVI ||
       type == VW_TYPE_SDI ||
       type == VW_TYPE_DVI_DUAL_LINK)
   {
      if (id != V4L2_CID_BRIGHTNESS &&
          id != V4L2_CID_CONTRAST &&
          id != V4L2_CID_VFLIP &&
          id != RGB133_V4L2_CID_COLOURDOMAIN)
      {
         return V4L2_CTRL_FLAG_DISABLED;
      }
   }
   else
   {
      if (type != VW_TYPE_NOSIGNAL)
      {
         if (type != VW_TYPE_VIDEO)
         {
            if (id != V4L2_CID_BRIGHTNESS &&
                id != V4L2_CID_CONTRAST &&
                id != V4L2_CID_SATURATION &&
                id != V4L2_CID_HUE &&
                id != V4L2_CID_VFLIP &&
                id != RGB133_V4L2_CID_HOR_POS &&
                id != RGB133_V4L2_CID_VERT_POS &&
                id != RGB133_V4L2_CID_VIDEO_STANDARD)
            {
               return V4L2_CTRL_FLAG_DISABLED;
            }
         }
         else
         {
            if (id != V4L2_CID_BRIGHTNESS &&
                id != V4L2_CID_CONTRAST &&
                id != V4L2_CID_BLACK_LEVEL &&
                id != V4L2_CID_VFLIP &&
                id != RGB133_V4L2_CID_HOR_POS &&
                id != RGB133_V4L2_CID_HOR_SIZE &&
                id != RGB133_V4L2_CID_PHASE &&
                id != RGB133_V4L2_CID_VERT_POS &&
                id != RGB133_V4L2_CID_R_COL_BRIGHTNESS &&
                id != RGB133_V4L2_CID_R_COL_CONTRAST &&
                id != RGB133_V4L2_CID_G_COL_BRIGHTNESS &&
                id != RGB133_V4L2_CID_G_COL_CONTRAST &&
                id != RGB133_V4L2_CID_B_COL_BRIGHTNESS &&
                id != RGB133_V4L2_CID_B_COL_CONTRAST &&
                id != RGB133_V4L2_CID_HOR_TIME &&
                id != RGB133_V4L2_CID_VER_TIME &&
                id != RGB133_V4L2_CID_COLOURDOMAIN)
            {
               return V4L2_CTRL_FLAG_DISABLED;
            }
         }
      }
      else
      {
         return V4L2_CTRL_FLAG_DISABLED;
      }
   }

   return 0;
}

int rgb133_queryctrl(struct file* file, void* priv, struct v4l2_queryctrl* qc)
{
   int i;
   int invalid_ctrl = 0;
   sVWDeviceParms curDeviceParms;

   struct rgb133_handle* h = file->private_data;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_queryctrl: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_queryctrl: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_queryctrl: START - check control qc->id(0x%x)\n", qc->id));

   if (qc->id < V4L2_CID_BASE)
   {
      invalid_ctrl = 1;
   }
   else if (qc->id >= V4L2_CID_LASTP1)
   {
      if (qc->id < V4L2_CID_PRIVATE_BASE)
      {
         invalid_ctrl = 1;
      }
      else if (qc->id >= V4L2_CID_PRIVATE_LASTP1)
      {
         invalid_ctrl = 1;
      }
   }

   if (invalid_ctrl)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_queryctrl: invalid control query for id(0x%x)\n", qc->id));
      return -EINVAL;
   }

   for (i = 0; i < RGB133_NUM_CONTROLS; i++)
   {
      if (qc->id && qc->id == h->dev->V4L2Ctrls[h->channel][i].id)
      {
         eVWSignalType type = 0;

         /* Get the current device parms */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_queryctrl: call CaptureGetDeviceParameters\n"));
         CaptureGetDeviceParameters(h->dev, &curDeviceParms, 0, 0, 0, 0,
               h->channel, 0);

         type = curDeviceParms.type;
         memcpy(qc, &(h->dev->V4L2Ctrls[h->channel][i]), sizeof(*qc));

         CaptureSetDynamicControls(h);
 
         /* Handle signal dependent controls */
         if(qc->id != RGB133_V4L2_CID_FORCE_DETECT &&
            qc->id != RGB133_V4L2_CID_SCALING &&
            qc->id != RGB133_V4L2_CID_SCALING_AR &&
            qc->id != RGB133_V4L2_CID_LIVESTREAM &&
            qc->id != RGB133_V4L2_CID_HDCP &&
            qc->id != RGB133_V4L2_CID_INPUT_GANGING &&
            qc->id != RGB133_V4L2_CID_SIGNAL_TYPE)
         {
            if (rgb133_ctrl_disabled(qc->id, type))
            {
               qc->flags = V4L2_CTRL_FLAG_INACTIVE | V4L2_CTRL_FLAG_DISABLED;
            }
         }
         else if(qc->id == RGB133_V4L2_CID_FORCE_DETECT ||  /* Handle device specific controls */
                 qc->id == RGB133_V4L2_CID_HDCP ||
                 qc->id == RGB133_V4L2_CID_INPUT_GANGING)
         {
            if(!CaptureControlEnabled(h, qc->id))
            {
               qc->flags = V4L2_CTRL_FLAG_INACTIVE | V4L2_CTRL_FLAG_DISABLED;
            }
         }
         else { } /* Anything else is never disabled */

         RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_queryctrl: END\n"));
         return 0;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_queryctrl: END (BAD)\n"));

   return -EINVAL;
}

int rgb133_querymenu(struct file* file, void* priv, struct v4l2_querymenu* qm)
{
   int i;
   int invalid_ctrl = 0;
   sVWDeviceParms curDeviceParms;

   struct rgb133_handle* h = file->private_data;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_querymenu: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_querymenu: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querymenu: START - check menu item qm->id(0x%x) index(%d)\n",
         qm->id, qm->index));

   if(qm->id && qm->id == RGB133_V4L2_CID_FORCE_DETECT)
   {
      for(i=0; i<RGB133_NUM_FORCE_DETECT_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_force_detect[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_force_detect[i].name, sizeof(rgb133_qmenu_force_detect[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_SCALING)
   {
      for(i=0; i<RGB133_NUM_SCALING_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_scaling[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_scaling[i].name, sizeof(rgb133_qmenu_scaling[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_SCALING_AR)
   {
      for(i=0; i<RGB133_NUM_SCALING_AR_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_scaling_ar[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_scaling_ar[i].name, sizeof(rgb133_qmenu_scaling_ar[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_HDCP)
   {
      for(i=0; i<RGB133_NUM_HDCP_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_hdcp[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_hdcp[i].name, sizeof(rgb133_qmenu_hdcp[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_COLOURDOMAIN)
   {
      for(i=0; i<RGB133_NUM_COLOURDOMAIN_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_colourdomain[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_colourdomain[i].name, sizeof(rgb133_qmenu_colourdomain[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_INPUT_GANGING)
   {
      for(i=0; i<RGB133_NUM_INPUT_GANGING_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_inputganging[i].index)
         {
            if(CaptureGangingTypeSupported(h->dev, qm->index))
            {
               memcpy(&qm->name, &rgb133_qmenu_inputganging[i].name, sizeof(rgb133_qmenu_inputganging[i].name));
               return 0;
            }
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_SIGNAL_TYPE)
   {
      for(i=0; i<RGB133_NUM_SIGNAL_TYPE_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_signal_type[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_signal_type[i].name, sizeof(rgb133_qmenu_signal_type[i].name));
            return 0;
         }
      }
   }
   else if(qm->id && qm->id == RGB133_V4L2_CID_VIDEO_STANDARD)
   {
      for(i=0; i<RGB133_NUM_VIDEO_STANDARD_ITEMS; i++)
      {
         if(qm->index == rgb133_qmenu_video_standard[i].index)
         {
            memcpy(&qm->name, &rgb133_qmenu_video_standard[i].name, sizeof(rgb133_qmenu_video_standard[i].name));
            return 0;
         }
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_querymenu: END (BAD)\n"));

   return -EINVAL;
}

int rgb133_g_ctrl(struct file* file, void* priv, struct v4l2_control* ctrl)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;

   sVWDeviceParms curDeviceParms;
   eVWSignalType type = 0;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT(
            (RGB133_LINUX_DBG_WARNING, "rgb133_g_ctrl: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_ctrl: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_ctrl: START\n"));

   /* Check if control is disabled */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_ctrl: call CaptureGetDeviceParameters\n"));
   CaptureGetDeviceParameters(h->dev, &curDeviceParms, 0, 0, 0, 0, h->channel,
         0);
   type = curDeviceParms.type;

   /* Handle signal dependent controls */
   if(ctrl->id != RGB133_V4L2_CID_FORCE_DETECT &&
      ctrl->id != RGB133_V4L2_CID_SCALING &&
      ctrl->id != RGB133_V4L2_CID_SCALING_AR &&
      ctrl->id != RGB133_V4L2_CID_LIVESTREAM &&
      ctrl->id != RGB133_V4L2_CID_HDCP &&
      ctrl->id != RGB133_V4L2_CID_INPUT_GANGING &&
      ctrl->id != RGB133_V4L2_CID_SIGNAL_TYPE)
   {
      if (rgb133_ctrl_disabled(ctrl->id, type))
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: ctrl(%u) is disabled and can't be read.\n", ctrl->id));
         return -EINVAL;
      }
   }
   else if(ctrl->id == RGB133_V4L2_CID_FORCE_DETECT || /* Handle device specific controls */
           ctrl->id == RGB133_V4L2_CID_HDCP ||
           ctrl->id == RGB133_V4L2_CID_INPUT_GANGING)
   {
      if(!CaptureControlEnabled(h, ctrl->id))
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: ctrl(%u) is disabled and can't be read.\n", ctrl->id));
         return -EINVAL;
      }
   }
   else { } /* Anything else is never disabled */

   switch (ctrl->id)
   {
      case V4L2_CID_BRIGHTNESS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_BRIGHTNESS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Brightness\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Brightness should be (%d)\n", ctrl->value));
         }
         break;
      case V4L2_CID_CONTRAST:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_CONTRAST);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Contrast\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Contrast should be (%d)\n", ctrl->value));
         }
         break;
      case V4L2_CID_SATURATION:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_SATURATION);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Saturation\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Saturation should be (%d)\n", ctrl->value));
         }
         break;
      case V4L2_CID_HUE:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_HUE);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Hue\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Hue should be (%d)\n", ctrl->value));
         }
         break;
      case V4L2_CID_BLACK_LEVEL:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_BLACKLEVEL);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get BlackLevel\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: BlackLevel should be (%d)\n", ctrl->value));
         }
         break;
      case V4L2_CID_VFLIP:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_VFLIP);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Vertical Flip\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Vertical Flip should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_HOR_TIME:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_HOR_TIME);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Horizontal Timing\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Horizontal Timing should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_VER_TIME:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_VER_TIME);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Vertical Timing\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Vertical Timing should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_HOR_POS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_HOR_POS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Horizontal Position\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Horizontal Position should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_HOR_SIZE:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_HOR_SIZE);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Horizontal Size\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Horizontal Size should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_PHASE:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_PHASE);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Phase\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Phase should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_VERT_POS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_VERT_POS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Vertical Position\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Vertical Position should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_R_COL_BRIGHTNESS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_R_COL_BRIGHTNESS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Red Brightness\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Red Brightness should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_R_COL_CONTRAST:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_R_COL_CONTRAST);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Red Contrast\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Red Contrast should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_G_COL_BRIGHTNESS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_G_COL_BRIGHTNESS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Green Brightness\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Green Brightness should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_G_COL_CONTRAST:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_G_COL_CONTRAST);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Green Contrast\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Green Contrast should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_B_COL_BRIGHTNESS:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_B_COL_BRIGHTNESS);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Blue Brightness\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Blue Brightness should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_B_COL_CONTRAST:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_B_COL_CONTRAST);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Blue Contrast\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Blue Contrast should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_FORCE_DETECT:
         rc = CaptureGetDetectMode(h, h->channel, (unsigned long*)&ctrl->value);
         if(rc != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Detection Method\n"));
         }
         else
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Detection Method should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_SCALING:
         ctrl->value = h->sCapture.ScalingMode;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Scaling should be (%d)\n", ctrl->value));
         rc = 0;
         break;
      case RGB133_V4L2_CID_SCALING_AR:
         ctrl->value = h->sCapture.ScalingAR;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Scaling AR should be (%d)\n", ctrl->value));
         rc = 0;
         break;
      case RGB133_V4L2_CID_HDCP:
         ctrl->value = rgb133_hdcp;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: HDCP should be (%d)\n", ctrl->value));
         rc = 0;
         break;
      case RGB133_V4L2_CID_COLOURDOMAIN:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_COLOURDOMAIN);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Colour Domain: %d\n", rc));
         }
         else
         {
            ctrl->value = CaptureMapColourDomainFromWin(rc);
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Colour Domain should be (%d %d)\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_LIVESTREAM:
         rc = CaptureGetParameterCurrent(h, RGB133_CAP_PARM_LIVESTREAM);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get LiveStream\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: LiveStream should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_INPUT_GANGING:
         rc = CaptureGetGangingType(h);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Ganging Type\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Ganging Type should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_SIGNAL_TYPE:
         rc = CaptureGetVWSignalType(h);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Signal Type\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Signal Type should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_VIDEO_STANDARD:
         rc = CaptureGetVideoStandard(h);
         if (rc < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: Failed to get Video Standard\n"));
         }
         else
         {
            ctrl->value = rc;
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: Video Standard should be (%d)\n", ctrl->value));
         }
         break;
      case RGB133_V4L2_CID_CLIENT_ID:
         ctrl->value = h->capture;
         rc = 0;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_ctrl: get Client ID(%d)\n", ctrl->value));
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_ctrl: Invalid ctrl(0x%x)\n", ctrl->id));
         return -EINVAL;
   }

   if (rc < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_ctrl: CaptureGetParameterCurrent failed\n"));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_ctrl: END\n"));

   return 0;
}

int rgb133_s_ctrl(struct file* file, void* priv, struct v4l2_control* ctrl)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;

   sVWDeviceParms curDeviceParms;
   eVWSignalType type = 0;

   int rc = 0, i;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_ctrl: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_ctrl: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: START - ctrl(%u - 0x%lx)\n", ctrl->id, ctrl->id));

   /* Check if control is disabled */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_ctrl: call CaptureGetDeviceParameters\n"));
   CaptureGetDeviceParameters(h->dev, &curDeviceParms, 0, 0, 0, 0, h->channel, 0);

   type = curDeviceParms.type;

   /* Handle signal dependent controls */
   if(ctrl->id != RGB133_V4L2_CID_FORCE_DETECT &&
      ctrl->id != RGB133_V4L2_CID_SCALING &&
      ctrl->id != RGB133_V4L2_CID_SCALING_AR &&
      ctrl->id != RGB133_V4L2_CID_LIVESTREAM &&
      ctrl->id != RGB133_V4L2_CID_HDCP &&
      ctrl->id != RGB133_V4L2_CID_INPUT_GANGING)
   {
      if (rgb133_ctrl_disabled(ctrl->id, type))
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_ctrl: ctrl(%u) is disabled and can't be set.\n", ctrl->id));
         return -EINVAL;
      }
   }
   else if(ctrl->id == RGB133_V4L2_CID_FORCE_DETECT || /* Handle device specific controls */
           ctrl->id == RGB133_V4L2_CID_HDCP ||
           ctrl->id == RGB133_V4L2_CID_INPUT_GANGING)
   {
      if(!CaptureControlEnabled(h, ctrl->id))
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_ctrl: ctrl(%u) is disabled and can't be set.\n", ctrl->id));
         return -EINVAL;
      }
   }
   else { } /* Anything else is never disabled */

   /* It may actually be a wise move to police the min/max values now; stop the user shooting
    ** the kernel module in the head.  This is especially true of HorPos/VerPos.
    */
   for (i = 0; i < RGB133_NUM_CONTROLS; i++)
   {
      if (ctrl->id == h->dev->V4L2Ctrls[h->channel][i].id)
      {
         if ((ctrl->value > h->dev->V4L2Ctrls[h->channel][i].maximum) ||
             (ctrl->value < h->dev->V4L2Ctrls[h->channel][i].minimum))
         {
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: ERANGE END - ctrl %u (%s) cannot be set with out of range value %d.\n",
                  ctrl->id, h->dev->V4L2Ctrls[h->channel][i].name, ctrl->value));
            return -ERANGE;
         }
      }
   }

   /* Now we've got the current settings, change the required value
    * and set them into the capture */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_ctrl: Set v4l2 param(0x%x) to value(%d)\n", ctrl->id, ctrl->value));
   switch (ctrl->id)
   {
      case V4L2_CID_BRIGHTNESS:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_BRIGHTNESS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Brightness(%d): %d\n", ctrl->value, rc));
         }
         break;
      case V4L2_CID_CONTRAST:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_CONTRAST, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Contrast(%d): %d\n", ctrl->value, rc));
         }
         break;
      case V4L2_CID_SATURATION:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_SATURATION, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Saturation(%d): %d\n", ctrl->value, rc));
         }
         break;
      case V4L2_CID_HUE:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_HUE, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Hue(%d): %d\n", ctrl->value, rc));
         }
         break;
      case V4L2_CID_BLACK_LEVEL:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_BLACKLEVEL, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set BlackLevel(%d): %d\n", ctrl->value, rc));
         }
         break;
      case V4L2_CID_VFLIP:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_VFLIP, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Vertical Flip(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_HOR_TIME:
         if ((rc = CaptureSetVideoTimingParameter(h, RGB133_CAP_PARM_HOR_TIME, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Horizontal Timing(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_VER_TIME:
         if ((rc = CaptureSetVideoTimingParameter(h, RGB133_CAP_PARM_VER_TIME, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Vertical Timing(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_HOR_POS:
         if ((rc = CaptureSetVideoTimingParameter(h, RGB133_CAP_PARM_HOR_POS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Horizontal Position(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_HOR_SIZE:
         if ((rc = CaptureSetVideoTimingParameter(h, RGB133_CAP_PARM_HOR_SIZE, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Horizontal Size(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_PHASE:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_PHASE, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Phase(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_VERT_POS:
         if ((rc = CaptureSetVideoTimingParameter(h, RGB133_CAP_PARM_VERT_POS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Vertical Position(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_R_COL_BRIGHTNESS:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_R_COL_BRIGHTNESS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Red Brightness(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_R_COL_CONTRAST:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_R_COL_CONTRAST, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Red Contrast(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_G_COL_BRIGHTNESS:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_G_COL_BRIGHTNESS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Green Brightness(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_G_COL_CONTRAST:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_G_COL_CONTRAST, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Green Contrast(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_B_COL_BRIGHTNESS:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_B_COL_BRIGHTNESS, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Blue Brightness(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_B_COL_CONTRAST:
         if ((rc = CaptureSetColourBalanceParameter(h, RGB133_CAP_PARM_B_COL_CONTRAST, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Blue Contrast(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_FORCE_DETECT:
         if((rc = CaptureSetDetectMode(h, 0, h->channel, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Detection Method(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_SCALING:
         h->sCapture.ScalingMode = ctrl->value;
         if((rc = CaptureRedetectAndSetVideoParams(h)) < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: Failed to redetect and set new video params: %d\n", rc));
            return rc;
         }
         break;
      case RGB133_V4L2_CID_SCALING_AR:
         h->sCapture.ScalingAR = ctrl->value;
         if((rc = CaptureRedetectAndSetVideoParams(h)) < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: Failed to redetect and set new video params: %d\n", rc));
            return rc;
         }
         break;
      case RGB133_V4L2_CID_HDCP:
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_ctrl: Setting HDCP currently unsupported, reset ctrl->value(%lu)\n",
               rgb133_hdcp));
         rgb133_hdcp = ctrl->value;
         rc = 0;
         break;
      case RGB133_V4L2_CID_COLOURDOMAIN:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_COLOURDOMAIN, CaptureMapColourDomainToWin(ctrl->value))) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Colour Domain(%d): %d\n",
                  ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_LIVESTREAM:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_LIVESTREAM, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set LiveStream(%d): %d\n", ctrl->value, rc));
         }
         break;
      case RGB133_V4L2_CID_INPUT_GANGING:
         if ((rc = CaptureSetGangingType(h->dev->pDE, h->channel, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Ganging Type(%d): %d\n", ctrl->value, rc));
         }
         break;

      case RGB133_V4L2_CID_VIDEO_STANDARD:
         if ((rc = CaptureSetParameter(h, RGB133_CAP_PARM_VIDEO_STD, ctrl->value)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_ctrl: Failed to set Video Standard(%d): %d\n", ctrl->value, rc));
         }
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: END - Invalid ctrl(0x%x)\n", ctrl->id));
         return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_ctrl: END\n"));

   return rc;
}

int rgb133_g_parm(struct file* file, void* priv, struct v4l2_streamparm* strm)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;
   struct v4l2_captureparm* pCapParm = &strm->parm.capture;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_parm: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_parm: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_parm: START\n"));

   /* Check the requested format type*/
   if (strm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       strm->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_parm: invalid type requested for capture[%d] on device(0x%p)\n", h->capture, dev));
      return -EINVAL;
   }

   /* Get current parameters */
   if ((rc = CaptureGetStreamingParms(h, pCapParm)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_parm: rgb133_g_parm failed: %d\n", rc));
   }

   /* Add in the flag to declare we support setting frame rate */
   pCapParm->capability |= V4L2_CAP_TIMEPERFRAME;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_parm: timeperframe(%u/%u)\n",
      strm->parm.capture.timeperframe.numerator, strm->parm.capture.timeperframe.denominator));

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_parm: END\n"));

   return rc;
}

int rgb133_s_parm(struct file* file, void* priv, struct v4l2_streamparm* strm)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;
   struct v4l2_captureparm* pCapParm = &strm->parm.capture;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_parm: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_parm: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_parm: START\n"));

   /* Check the requested format type*/
   if (strm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       strm->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_parm: invalid type requested for capture[%d] on device(0x%p)\n", h->capture, dev));
      return -EINVAL;
   }

   /* Set current parameters */
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_parm: set rate(%u/%u) for channel[%d].capture[%d]\n",
         pCapParm->timeperframe.numerator, pCapParm->timeperframe.denominator ,h->channel, h->capture));
   if ((rc = CaptureSetStreamingParms(h, pCapParm)) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_parm: rgb133_s_parm failed: %d\n", rc));
   }

   /* re-initialise rate stuff */
   h->rate.timeout = 0;
   h->rate.tv_now.tv_sec = 0;
   h->rate.tv_now.tv_usec = 0;
   h->rate.tv_later.tv_sec = 0;
   h->rate.tv_later.tv_usec = 0;

   /* re-initialise no signal stuff */
   h->frame_count = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_parm: END\n"));

   return rc;
}

#ifdef RGB133_CONFIG_HAVE_CROP_API
int rgb133_cropcap(struct file* file, void* priv, struct v4l2_cropcap* pCropcap)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;

   rgb133_cap_parms_aoi MinAOI, MaxAOI;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_cropcap: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_cropcap: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_cropcap: START\n"));

   /* Check the requested format type*/
   if (pCropcap->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       pCropcap->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_cropcap: invalid type requested for capture[%d] on device(0x%p)\n", h->capture, dev));
      return -EINVAL;
   }

   if ((rc = CaptureGetMinMaxCropping(h, &MinAOI, &MaxAOI)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_cropcap: Failed to get Min/Max Cropping: %d\n", rc));
   }

   /* Fill the default bounds */
   pCropcap->bounds.top = MinAOI.Top;
   pCropcap->bounds.left = MinAOI.Left;
   pCropcap->bounds.width = MaxAOI.Right;
   pCropcap->bounds.height = MaxAOI.Bottom;

   /* Fill the default cropping rect */
   pCropcap->defrect.top = 0;
   pCropcap->defrect.left = 0;
   pCropcap->defrect.width = h->sCapture.SourceWidth;
   pCropcap->defrect.height = h->sCapture.SourceHeight;

   /* Fill the default cropping rect */
   pCropcap->pixelaspect.numerator = 1;
   pCropcap->pixelaspect.denominator = 1;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_cropcap: END\n"));

   return rc;
}

int rgb133_g_crop(struct file* file, void* priv, struct v4l2_crop* pCrop)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;

   int* pTop = &pCrop->c.top;
   int* pLeft = &pCrop->c.left;
   int* pWidth = &pCrop->c.width;
   int* pHeight = &pCrop->c.height;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_crop: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_crop: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_crop: START\n"));

   /* Check the requested format type*/
   if (pCrop->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       pCrop->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_crop: invalid type requested for capture[%d] on device(0x%p)\n", h->capture, dev));
      return -EINVAL;
   }

   if ((rc = CaptureGetCropping(h, pTop, pLeft, pWidth, pHeight)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_crop: Failed to get cropping(%d)\n", rc));
      return rc;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_g_crop: Top(%d), Left(%d), Width(%d), Height(%d)\n", *pTop, *pLeft, *pWidth, *pHeight));

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_crop: END\n"));

   return rc;
}

#ifdef RGB133_CONFIG_S_CROP_IS_CONST
int rgb133_s_crop(struct file* file, void* priv, const struct v4l2_crop* pCrop)
#else
int rgb133_s_crop(struct file* file, void* priv, struct v4l2_crop* pCrop)
#endif
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;

   int Top = pCrop->c.top;
   int Left = pCrop->c.left;
   int Width = pCrop->c.width;
   int Height = pCrop->c.height;

   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_crop: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_crop: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_crop: START\n"));

   /* Check the requested format type*/
   if (pCrop->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       pCrop->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_crop: invalid type requested for capture[%d] on device(0x%p)\n", h->capture, dev));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_s_crop: Top(%d), Left(%d), Width(%d), Height(%d)\n", Top, Left, Width, Height));
   if ((rc = CaptureSetCropping(h, Top, Left, Width, Height)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_crop: Failed to set cropping(%d)\n", rc));
      return rc;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_crop: END\n"));

   return rc;
}
#endif /* RGB133_CONFIG_HAVE_CROP_API */

#ifdef RGB133_CONFIG_HAVE_SELECTION_API
int rgb133_g_selection(struct file* file, void* priv, struct v4l2_selection* pSelection)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;
   int rc;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_selection: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_selection: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_selection: START buffer type(%d) target(0x%x)\n", 
         pSelection->type, pSelection->target));

   /* Check the requested format type*/
   if (pSelection->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       pSelection->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_selection: invalid type requested for capture[%d] on device(0x%p)\n", 
            h->capture, dev));
      return -EINVAL;
   }

   switch (pSelection->target)
   {
      case V4L2_SEL_TGT_CROP_BOUNDS:
      {
         rgb133_cap_parms_aoi MinAOI, MaxAOI;

         if ((rc = CaptureGetMinMaxCropping(h, &MinAOI, &MaxAOI)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_selection: Failed to get Min/Max Cropping: %d\n", rc));
            return rc;
         }

         /* Fill with the bounds of the cropping rect */
         pSelection->r.left = MinAOI.Left;
         pSelection->r.top = MinAOI.Top;
         pSelection->r.width = MaxAOI.Right;
         pSelection->r.height = MaxAOI.Bottom;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Crop Bounds "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));
         break;
      }
      case V4L2_SEL_TGT_CROP_DEFAULT:
         /* Fill the default cropping rect */
         pSelection->r.left = 0;
         pSelection->r.top = 0;
         pSelection->r.width = h->sCapture.SourceWidth;
         pSelection->r.height = h->sCapture.SourceHeight;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Crop Default "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));
         break;
      case V4L2_SEL_TGT_CROP:
      {
         int* pTop = &pSelection->r.top;
         int* pLeft = &pSelection->r.left;
         int* pWidth = &pSelection->r.width;
         int* pHeight = &pSelection->r.height;

         if ((rc = CaptureGetCropping(h, pTop, pLeft, pWidth, pHeight)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_selection: Failed to get cropping(%d)\n", rc));
            return rc;
         }

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Crop "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));
         break;
      }
      case V4L2_SEL_TGT_COMPOSE_PADDED:
         /* PG TODO: A padded target would require leaving non-active-video area in buffers untouched.
          * Current behaviour is that we initialise buffers before DMA. */
      case V4L2_SEL_TGT_COMPOSE_BOUNDS:
      case V4L2_SEL_TGT_COMPOSE_DEFAULT:
         /* Fill with the bounds of the memory buffer */
         pSelection->r.left = 0;
         pSelection->r.top = 0;
         pSelection->r.width = h->sCapture.CaptureWidth;
         pSelection->r.height = h->sCapture.CaptureHeight;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Target(0x%x) "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->target,
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));
         break;
      case V4L2_SEL_TGT_COMPOSE:
         /* This target is used to configure scaling and composition of the image in memory */
         pSelection->r.left = h->sCapture.ComposeLeft;
         pSelection->r.top = h->sCapture.ComposeTop;
         pSelection->r.width = h->sCapture.ComposeWidth;
         pSelection->r.height = h->sCapture.ComposeHeight;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Compose "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_selection: Invalid target(0x%x)\n", pSelection->target));
         return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_selection: END\n"));

   return 0;
}

int rgb133_s_selection(struct file* file, void* priv, struct v4l2_selection* pSelection)
{
   struct rgb133_handle* h = priv;
   struct rgb133_dev* dev = h->dev;
   int rc = -EINVAL;

   if (DeviceIsControl(dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_selection: Device is control\n"));
      return -EINVAL;
   }

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_s_selection: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_selection: START\n"));

   /* Check the requested format type*/
   if (pSelection->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
       pSelection->type != V4L2_BUF_TYPE_CAPTURE_SOURCE)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_selection: invalid type requested for capture[%d] on device(0x%p)\n", 
            h->capture, dev));
      return -EINVAL;
   }

   switch (pSelection->target)
   {
      case V4L2_SEL_TGT_CROP:
      {
         int Top = pSelection->r.top;
         int Left = pSelection->r.left;
         int Width = pSelection->r.width;
         int Height = pSelection->r.height;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_selection: Crop Left(%d), Top(%d), Width(%d), Height(%d)\n", 
               Left, Top, Width, Height));

         if ((rc = CaptureSetCropping(h, Top, Left, Width, Height)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_selection: Failed to set cropping(%d)\n", rc));
            return rc;
         }

         break;
      }
      case V4L2_SEL_TGT_COMPOSE:
      {
         int Top = pSelection->r.top;
         int Left = pSelection->r.left;
         int Width = pSelection->r.width;
         int Height = pSelection->r.height;

         /* This target is used to configure scaling and composition of the image in memory */
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_selection: Compose IN "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));

         if ((rc = CaptureSetImageComposition(h, &Left, &Top, &Width, &Height)) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_s_selection: Failed to set compose rect(%d,%d,%d,%d) ret(%d)\n", 
                  Left, Top, Width, Height, rc));
            return rc;
         }

         pSelection->r.left = Left;
         pSelection->r.top = Top;
         pSelection->r.width = Width;
         pSelection->r.height = Height;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_selection: Compose OUT "
                  "Left(%d), Top(%d), Width(%d), Height(%d)\n",
               pSelection->r.left, pSelection->r.top, pSelection->r.width, pSelection->r.height));

         break;
      }
      default:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_s_selection: Invalid target(0x%x)\n", pSelection->target));
         return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_s_selection: END\n"));

   return 0;
}
#endif /* RGB133_CONFIG_HAVE_SELECTION_API */

extern int rgb133_dumb_buffer_width;
extern int rgb133_dumb_buffer_height;

#ifdef RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS
int rgb133_enum_framesizes(struct file* file, void* priv, struct v4l2_frmsizeenum* fsize)
{
   struct rgb133_handle* h = priv;
   sVWDeviceParms *maxDeviceParms;
   sVWDeviceParms *curDeviceParms;

   unsigned long BytesPerPixel, ImageWidthBytes;

   int rc = 0;

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_enum_framesizes: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   maxDeviceParms = KernelVzalloc(sizeof(sVWDeviceParms));
   curDeviceParms = KernelVzalloc(sizeof(sVWDeviceParms));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_enum_framesizes: alloc %d bytes to maxDeviceParms(0x%p) %d bytes to curDeviceParms(0x%p)\n",
         sizeof(sVWDeviceParms), maxDeviceParms, sizeof(sVWDeviceParms), curDeviceParms));

   if (fsize->index != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: index(%d) is not zero\n", fsize->index));
      rc = -EINVAL;
      goto error;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: enumerate index(%d)\n", fsize->index));

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: get min/max device parms\n"));
   if ((rc = CaptureGetDeviceParameters(h->dev, curDeviceParms, 0, maxDeviceParms, 0, 0, h->channel, 0)) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_framesizes: Failed to get min/max vid timing parms: %d\n", rc));
      rc = -EINVAL;
      goto error;
   }

   fsize->type = V4L2_FRMIVAL_TYPE_CONTINUOUS;

   if (rgb133_dumb_buffer_width == -1 && rgb133_dumb_buffer_height == -1)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: Using CONT width/height from (%lu-%lu) x (%lu-%lu)\n",
            1, maxDeviceParms->VideoTimings.HorAddrTime, 1, maxDeviceParms->VideoTimings.VerAddrTime));
      fsize->stepwise.min_width = 1;
      fsize->stepwise.max_width = maxDeviceParms->VideoTimings.HorAddrTime;
      fsize->stepwise.step_width = 1;
      fsize->stepwise.min_height = 1;
      fsize->stepwise.max_height = maxDeviceParms->VideoTimings.VerAddrTime;
      fsize->stepwise.step_height = 1;
   }
   else if (rgb133_dumb_buffer_width > -1 && rgb133_dumb_buffer_width > -1)
   {
      fsize->type = V4L2_FRMIVAL_TYPE_DISCRETE;
      if (rgb133_dumb_buffer_width == 0 && rgb133_dumb_buffer_height == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: Using cur DISC width/height %lu x %lu\n",
               curDeviceParms->VideoTimings.HorAddrTime, curDeviceParms->VideoTimings.VerAddrTime));
         fsize->discrete.width = curDeviceParms->VideoTimings.HorAddrTime;
         fsize->discrete.height = curDeviceParms->VideoTimings.VerAddrTime;
      }
      else if (rgb133_dumb_buffer_width > 0 && rgb133_dumb_buffer_height > 0)
      {
         if (rgb133_dumb_buffer_width
               > maxDeviceParms->VideoTimings.HorAddrTime)
            rgb133_dumb_buffer_width = maxDeviceParms->VideoTimings.HorAddrTime;
         if (rgb133_dumb_buffer_height
               > maxDeviceParms->VideoTimings.VerAddrTime)
            rgb133_dumb_buffer_height =
                  maxDeviceParms->VideoTimings.VerAddrTime;
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_framesizes: Using DISC width/height %lu x %lu\n",
               rgb133_dumb_buffer_width, rgb133_dumb_buffer_height));
         fsize->discrete.width = rgb133_dumb_buffer_width;
         fsize->discrete.height = rgb133_dumb_buffer_height;
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_framesizes: Invalid DISC width/height %d x %d: mismatch\n",
               rgb133_dumb_buffer_width, rgb133_dumb_buffer_height));
         rc = -EINVAL;
         goto error;
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_framesizes: Invalid DISC width/height %d x %d: negative\n",
                  rgb133_dumb_buffer_width, rgb133_dumb_buffer_height));
      rc = -EINVAL;
      goto error;
   }

   if (fsize->type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
   {
      BytesPerPixel = v4lBytesPerPixel(fsize->pixel_format);
      ImageWidthBytes = fsize->stepwise.min_width * BytesPerPixel;

      while ((ImageWidthBytes & 0x03) != 0)
      {
         fsize->stepwise.min_width++;
         ImageWidthBytes = fsize->stepwise.min_width * BytesPerPixel;
      }

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_enum_framesizes: width[min(%lu),max(%lu)], height[min(%lu),max(%lu)]\n",
            fsize->stepwise.min_width, fsize->stepwise.max_width, fsize->stepwise.min_height, fsize->stepwise.max_height));
   } else {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_enum_framesizes: width(%lu)x height(%lu)\n",
            fsize->discrete.width, fsize->discrete.height));
   }

error:
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_enum_framesizes: free maxDeviceParms(0x%p) curDeviceParms(0x%p)\n",
         maxDeviceParms, curDeviceParms));
   KernelVfree(maxDeviceParms);
   KernelVfree(curDeviceParms);

   return rc;
}

extern int rgb133_dumb_buffer_rate;

int rgb133_enum_frameintervals(struct file* file, void* priv, struct v4l2_frmivalenum* fival)
{
   struct rgb133_handle* h = priv;
   int rc = 0;

   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_enum_frameintervals: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }

   if (fival->index != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_frameintervals: index(%d) is not zero\n", fival->index));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enum_frameintervals: enumerate index(%d)\n", fival->index));

   if (rgb133_dumb_buffer_rate == -1)
   {
      //RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_enum_frameintervals: Using CONT rate from (%lu/%lu) to (%lu/%lu)\n",
      //      1, ((CaptureGetInputRate(h) + 500) / 1000),
      //      1, 1));
      fival->type = V4L2_FRMIVAL_TYPE_CONTINUOUS;
      fival->stepwise.min.numerator = 1;
      fival->stepwise.min.denominator = ((CaptureGetInputRate(h) + 500) / 1000);
      fival->stepwise.max.numerator = 1;
      fival->stepwise.max.denominator = 1;
      fival->stepwise.step.numerator = 1;
      fival->stepwise.step.denominator =
            ((CaptureGetInputRate(h) + 500) / 1000);
   }
   else if (rgb133_dumb_buffer_rate == 0)
   {
      //RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_enum_frameintervals: Using cur DISC rate 1/%lu\n",
      //      CaptureGetInputRate(h)));
      fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
      fival->discrete.numerator = 1;
      fival->discrete.denominator = CaptureGetInputRate(h) / 1000;
   }
   else if (rgb133_dumb_buffer_rate > 0)
   {
      if (rgb133_dumb_buffer_rate > CaptureGetInputRate(h))
         rgb133_dumb_buffer_rate = CaptureGetInputRate(h);

      //RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_enum_frameintervals: Using DISC rate 1/%lu\n",
      //      rgb133_dumb_buffer_rate));
      fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
      fival->discrete.numerator = 1;
      fival->discrete.denominator = rgb133_dumb_buffer_rate;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_enum_frameintervals: Invalid dumb buffer rate(%d)\n", rgb133_dumb_buffer_rate));
      return -EINVAL;
   }

   return rc;
}
#endif /* RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS */

int rgb133_g_video_timings(struct file* file, void* priv, struct _srgb133VideoTimings* pTimings)
{
   struct rgb133_handle* h = file->private_data;
   sVWDeviceParms* curDeviceParms;
   sVWDeviceParms* detDeviceParms;
   int rc = 0;

   if (DeviceIsControl(h->dev->control))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_video_timings: Device is control\n"));
      return -EINVAL;
   }
   if (IsDiagnosticsMode(rgb133_diagnostics_mode))
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_g_video_timings: We are in diagnostics mode! Returning...\n"));
      return -EBADRQC;
   }
   if (pTimings->size == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_video_timings: Invalid size(0)\n"));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_video_timings: START\n"));

   curDeviceParms = KernelVzalloc(sizeof(sVWDeviceParms));
   detDeviceParms = KernelVzalloc(sizeof(sVWDeviceParms));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_g_video_timings: alloc %d bytes to curDeviceParms(0x%p)\n",
         sizeof(sVWDeviceParms), curDeviceParms));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_g_video_timings: alloc %d bytes to detDeviceParms(0x%p)\n",
         sizeof(sVWDeviceParms), detDeviceParms));

   if (CaptureGetDeviceParameters(h->dev, curDeviceParms, 0, 0, 0, detDeviceParms, h->channel, 0) != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_video_timings: Failed to get current video timings\n"));
      rc = -EINVAL;
      goto out;
   }

   pTimings->size = min(pTimings->size, sizeof(sVWDeviceParms));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_g_video_timings: copy %lu bytes for video timings type(%d)\n",
               pTimings->size, pTimings->type));

   switch (pTimings->type)
   {
      case RGB133_PARMS_CURRENT:
         memcpy(&pTimings->VideoTimings, &curDeviceParms->VideoTimings, pTimings->size);
         break;
      case RGB133_PARMS_DETECTED:
         memcpy(&pTimings->VideoTimings, &detDeviceParms->VideoTimings, pTimings->size);
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_g_video_timings: Invalid type(%lu)\n", pTimings->type));
         rc = -EINVAL;
   }

out:
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_g_video_timings: free detDeviceParms(0x%p)\n", detDeviceParms));
   KernelVfree(detDeviceParms);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_g_video_timings: free curDeviceParms(0x%p)\n", curDeviceParms));
   KernelVfree(curDeviceParms);

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_g_video_timings: END return %d\n", rc));

   return rc;
}

#ifdef RGB133_CONFIG_HAVE_VIDIOC_DEFAULT
#ifdef RGB133_CONFIG_HAVE_VIDIOC_DEFAULT_EXT
#ifdef RGB133_CONFIG_DEFAULT_IOCTL_UNSIGNED
long rgb133_default_ioctl(struct file* file, void* fh, bool priv, unsigned int cmd, void* arg)
#else
long rgb133_default_ioctl(struct file* file, void* fh, bool priv, int cmd, void* arg)
#endif
#else
long rgb133_default_ioctl(struct file* file, void* fh, int cmd, void* arg)
#endif /* RGB133_CONFIG_HAVE_VIDIOC_DEFAULT_EXT */
{
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_default_ioctl: START\n"));

   switch (cmd)
   {
      case RGB133_PRIVATE_IOCTL_G_FMT:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_VIDIOC_G_SRC_FMT\n"));
         return rgb133_g_fmt_vid_cap(file, fh, arg);
      case RGB133_PRIVATE_IOCTL_PEEK:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_PEEK\n"));
         return rgb133_peekpoke_peek(file, fh, arg);
      case RGB133_PRIVATE_IOCTL_POKE:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_POKE\n"));
         return rgb133_peekpoke_poke(file, fh, arg);
      case RGB133_PRIVATE_IOCTL_DUMP:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_DUMP\n"));
         return rgb133_peekpoke_dump(file, fh, arg);
      case RGB133_PRIVATE_IOCTL_SET_FORCE_DETECT:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_SET_FORCE_DETECT\n"));
         return CaptureIoctlSetDetectMode((struct rgb133_handle*) file->private_data, (prgb133ForceDetect) arg);
      case RGB133_PRIVATE_IOCTL_GET_EDID:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_GET_EDID\n"));
         return CaptureGetEdid((struct rgb133_handle*) file->private_data, (prgb133EdidOps) arg);
      case RGB133_PRIVATE_IOCTL_SET_EDID:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_SET_EDID\n"));
         return CaptureSetEdid((struct rgb133_handle*) file->private_data, (prgb133EdidOps) arg);
      case RGB133_PRIVATE_IOCTL_GET_DIAG_NAMES:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_GET_DIAG_NAMES\n"));
         return CaptureIoctlGetDiagNames((struct rgb133_handle*) file->private_data, NULL, (prgb133Diags) arg);
      case RGB133_PRIVATE_IOCTL_GET_DIAG:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_PRIVATE_IOCTL_GET_DIAG\n"));
         return CaptureIoctlGetDiag((struct rgb133_handle*) file->private_data, NULL, (prgb133count) arg);
      case RGB133_PRIVATE_IOCTL_G_VIDEO_TIMINGS:
         RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_default_ioctl: RGB133_VIDIOC_G_VIDEO_TIMINGS\n"));
         return rgb133_g_video_timings(file, fh, arg);
      default:
         RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_default_ioctl: Invalid ioctl(0x%x)\n", cmd));
         break;
   }
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_default_ioctl: END\n"));
   return -EINVAL;
}
#endif /* RGB133_CONFIG_HAVE_VIDIOC_DEFAULT */
