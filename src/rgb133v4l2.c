/*
 * rgb133v4l2.c
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

#include "rgb133config.h"
#include "rgb133api.h"
#include "rgb133mapping.h"
#include "rgb133v4l2.h"
#include "rgb133capapi.h"
#include "rgb133deviceapi.h"
#include "rgb133ioctl.h"
#include "rgb133kernel.h"

#ifndef RGB133_CONFIG_HAVE_V4L2_IOCTL_OPS
struct video_device rgb133_defaults = {
   .owner          = THIS_MODULE,
   .name         = DRIVER_NAME,
   .fops         = &rgb133_fops,
#ifdef RGB133_CONFIG_ASSIGN_TV_NORMS
   .tvnorms      = RGB133_NORMS,
#endif
   .current_norm = V4L2_STD_PAL,
   .minor        = -1,
   .vidioc_querycap            = rgb133_querycap,
   .vidioc_enum_fmt_cap        = rgb133_enum_fmt_vid_cap,
   .vidioc_g_fmt_cap           = rgb133_g_fmt_vid_cap,
#ifdef RGB133_CONFIG_HAVE_G_FMT_PRIVATE
   .vidioc_g_fmt_type_private  = rgb133_g_fmt_vid_cap,
#endif/* RGB133_CONFIG_HAVE_G_FMT_PRIVATE */
   .vidioc_try_fmt_cap         = rgb133_try_fmt_vid_cap,
   .vidioc_s_fmt_cap           = rgb133_s_fmt_vid_cap,
#ifdef RGB133_CONFIG_HAVE_S_FMT_PRIVATE
   .vidioc_s_fmt_type_private  = rgb133_s_fmt_vid_cap,
#endif/* RGB133_CONFIG_HAVE_S_FMT_PRIVATE */
   .vidioc_reqbufs             = rgb133_reqbufs,
   .vidioc_querybuf            = rgb133_querybuf,
   .vidioc_qbuf                = rgb133_qbuf,
   .vidioc_dqbuf               = rgb133_dqbuf,
   .vidioc_s_std               = rgb133_s_std,
   .vidioc_enum_input          = rgb133_enum_input,
   .vidioc_g_input             = rgb133_g_input,
   .vidioc_s_input             = rgb133_s_input,
   .vidioc_queryctrl           = rgb133_queryctrl,
   .vidioc_querymenu           = rgb133_querymenu,
   .vidioc_g_ctrl              = rgb133_g_ctrl,
   .vidioc_s_ctrl              = rgb133_s_ctrl,
   .vidioc_streamon            = rgb133_streamon,
   .vidioc_streamoff           = rgb133_streamoff,
   .vidioc_g_parm              = rgb133_g_parm,
   .vidioc_s_parm              = rgb133_s_parm,
#ifdef RGB133_CONFIG_HAVE_CROP_API
   .vidioc_cropcap             = rgb133_cropcap,
   .vidioc_g_crop              = rgb133_g_crop,
   .vidioc_s_crop              = rgb133_s_crop,
#endif /* RGB133_CONFIG_HAVE_CROP_API */
#ifdef RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS
   .vidioc_enum_framesizes = rgb133_enum_framesizes,
   .vidioc_enum_frameintervals = rgb133_enum_frameintervals,
#endif /* RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS */
#ifdef RGB133_CONFIG_HAVE_VIDIOC_DEFAULT
   .vidioc_default             = rgb133_default_ioctl,
#endif /* RGB133_CONFIG_HAVE_VIDIOC_DEFAULT */
   .release                    = video_device_release,
};
#else
const struct v4l2_ioctl_ops rgb133_ioctl_ops = {
   .vidioc_querycap            = rgb133_querycap,
   .vidioc_enum_fmt_vid_cap    = rgb133_enum_fmt_vid_cap,
   .vidioc_g_fmt_vid_cap       = rgb133_g_fmt_vid_cap,
#ifdef RGB133_CONFIG_HAVE_G_FMT_PRIVATE
   .vidioc_g_fmt_type_private  = rgb133_g_fmt_vid_cap,
#endif /* RGB133_CONFIG_HAVE_G_FMT_PRIVATE */
   .vidioc_try_fmt_vid_cap     = rgb133_try_fmt_vid_cap,
   .vidioc_s_fmt_vid_cap       = rgb133_s_fmt_vid_cap,
#ifdef RGB133_CONFIG_HAVE_S_FMT_PRIVATE
   .vidioc_s_fmt_type_private  = rgb133_s_fmt_vid_cap,
#endif /* RGB133_CONFIG_HAVE_S_FMT_PRIVATE */
   .vidioc_reqbufs             = rgb133_reqbufs,
   .vidioc_querybuf            = rgb133_querybuf,
   .vidioc_qbuf                = rgb133_qbuf,
   .vidioc_dqbuf               = rgb133_dqbuf,
   .vidioc_s_std               = rgb133_s_std,
   .vidioc_enum_input          = rgb133_enum_input,
   .vidioc_g_input             = rgb133_g_input,
   .vidioc_s_input             = rgb133_s_input,
   .vidioc_queryctrl           = rgb133_queryctrl,
   .vidioc_querymenu           = rgb133_querymenu,
   .vidioc_g_ctrl              = rgb133_g_ctrl,
   .vidioc_s_ctrl              = rgb133_s_ctrl,
   .vidioc_streamon            = rgb133_streamon,
   .vidioc_streamoff           = rgb133_streamoff,
   .vidioc_g_parm              = rgb133_g_parm,
   .vidioc_s_parm              = rgb133_s_parm,
#ifdef RGB133_CONFIG_HAVE_CROP_API
   .vidioc_cropcap             = rgb133_cropcap,
   .vidioc_g_crop              = rgb133_g_crop,
   .vidioc_s_crop              = rgb133_s_crop,
#endif /* RGB133_CONFIG_HAVE_CROP_API */
#ifdef RGB133_CONFIG_HAVE_SELECTION_API
   .vidioc_g_selection         = rgb133_g_selection,
   .vidioc_s_selection         = rgb133_s_selection,
#endif /* RGB133_CONFIG_HAVE_SELECTION_API */
#ifdef RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS
   .vidioc_enum_framesizes = rgb133_enum_framesizes,
   .vidioc_enum_frameintervals = rgb133_enum_frameintervals,
#endif /* RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS */
#ifdef RGB133_CONFIG_HAVE_VIDIOC_DEFAULT
   .vidioc_default             = rgb133_default_ioctl,
#endif /* RGB133_CONFIG_HAVE_VIDIOC_DEFAULT */
};

struct video_device rgb133_defaults = {
   .name         = DRIVER_NAME,
   .fops         = &rgb133_fops,
   .ioctl_ops    = &rgb133_ioctl_ops,
   .tvnorms      = RGB133_NORMS,
#ifndef RGB133_CONFIG_NO_CURRENT_NORM
   .current_norm = V4L2_STD_PAL,
#endif
   .minor        = -1,
   .release      = video_device_release,
};
#endif /* !RGB133_CONFIG_HAVE_V4L2_IOCTL_OPS */

const char* rgb133_v4l2_io_types[RGB133_NUM_V4L2_IO_TYPES] = {
   "UNKNOWN", "READ", "MMAP", "USERPTR" };

int rgb133V4L2IsCapture(PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   if(f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
      f->type == V4L2_BUF_TYPE_CAPTURE_SOURCE)
      return 1;
   return 0;
}

int rgb133V4L2IsCaptureVideo(PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   if(f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
      return 1;
   return 0;
}

int rgb133V4L2IsCaptureSource(PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   if(f->type == V4L2_BUF_TYPE_CAPTURE_SOURCE)
      return 1;
   return 0;
}

int rgb133V4L2GetCaptureVideo(void)
{
   return V4L2_BUF_TYPE_VIDEO_CAPTURE;
}

int rgb133V4L2GetPixFmtRGB24(void)
{
   return V4L2_PIX_FMT_RGB24;
}

int rgb133V4L2GetPixFmtBGR24(void)
{
   return V4L2_PIX_FMT_BGR24;
}

int rgb133V4L2GetPixFmtRGB32(void)
{
   return V4L2_PIX_FMT_RGB32;
}

int rgb133V4L2GetPixFmtBGR32(void)
{
   return V4L2_PIX_FMT_BGR32;
}

int rgb133V4L2GetPixFmtRGB10(void)
{
   return V4L2_PIX_FMT_RGB10;
}

const char* rgb133V4L2GetIOType(PV4L2EMEMAPI _pMemory)
{
   enum v4l2_memory* pMemory = (enum v4l2_memory*)_pMemory;

   if(pMemory)
   {
      switch(*pMemory)
      {
         case 0:
            return rgb133_v4l2_io_types[RGB133_READ_IO];
         case V4L2_MEMORY_MMAP:
            return rgb133_v4l2_io_types[RGB133_MMAP_IO];
         case V4L2_MEMORY_USERPTR:
            return rgb133_v4l2_io_types[RGB133_USERPTR_IO];
         default:
            return rgb133_v4l2_io_types[RGB133_UNKNOWN_IO];
      }
   }
   else
      return rgb133_v4l2_io_types[RGB133_UNKNOWN_IO];
}

int rgb133V4L2GetFieldNone(void)
{
   return V4L2_FIELD_NONE;
}

int rgb133V4L2GetFieldBob(void)
{
   return V4L2_FIELD_BOB;
}

int rgb133V4L2GetFieldTop(void)
{
   return V4L2_FIELD_TOP;
}

int rgb133V4L2GetFieldBottom(void)
{
   return V4L2_FIELD_BOTTOM;
}

int rgb133V4L2GetFieldInterlaced(void)
{
   return V4L2_FIELD_INTERLACED;
}

int rgb133V4L2GetFieldAlternate(void)
{
   return V4L2_FIELD_ALTERNATE;
}

int rgb133V4L2GetCIDForceDetect(void)
{
   return RGB133_V4L2_CID_FORCE_DETECT;
}

int rgb133V4L2GetCIDHDCP(void)
{
   return RGB133_V4L2_CID_HDCP;
}

int rgb133V4L2GetCIDInputGanging(void)
{
   return RGB133_V4L2_CID_INPUT_GANGING;
}

/***************************************/
void rgb133V4L2AssignDimensions(struct rgb133_handle* h, PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   if(rgb133V4L2IsCaptureVideo(f))
   {
      if(h->sCapture.CaptureWidth &&
         h->sCapture.CaptureWidth != h->sCapture.SourceWidth)
         f->fmt.pix.width = h->sCapture.CaptureWidth;
      else
         f->fmt.pix.width     = h->sCapture.SourceWidth;

      if(h->sCapture.CaptureHeight &&
         h->sCapture.CaptureHeight != h->sCapture.SourceHeight)
         f->fmt.pix.height = h->sCapture.CaptureHeight;
      else
         f->fmt.pix.height     = h->sCapture.SourceHeight;
   }
   else
   {
      f->fmt.pix.width     = h->sCapture.SourceWidth;
      f->fmt.pix.height     = h->sCapture.SourceHeight;
   }
}

void rgb133V4L2AssignPixFmtFromWin(struct rgb133_handle* h, PV4L2FMTAPI _f, unsigned long PixelFormat)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   f->fmt.pix.pixelformat = mapUItoUI(__PIXELFORMAT__, __MAP_TO_V4L__, PixelFormat);
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133V4L2AssignPixFmtFromWin: pixelformat(0x%x), h->sCapture.pixelformat(0x%x)\n",
         f->fmt.pix.pixelformat, h->sCapture.pixelformat));
}

void rgb133V4L2CorrectBGRPixFmt(struct rgb133_handle* h, PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   if(f->fmt.pix.pixelformat == rgb133V4L2GetPixFmtRGB24() &&
      h->sCapture.pixelformat == rgb133V4L2GetPixFmtBGR24())
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133V4L2CorrectBGRPixFmt: Report 24-bit BGR colourspace 0x%x\n", rgb133V4L2GetPixFmtBGR24()));
      f->fmt.pix.pixelformat = rgb133V4L2GetPixFmtBGR24();
   }
   else if(f->fmt.pix.pixelformat == rgb133V4L2GetPixFmtRGB32() &&
      h->sCapture.pixelformat == rgb133V4L2GetPixFmtBGR32())
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133V4L2CorrectBGRPixFmt: Report 32-bit BGR colourspace 0x%x\n", rgb133V4L2GetPixFmtBGR32()));
      f->fmt.pix.pixelformat = rgb133V4L2GetPixFmtBGR32();
   }
}

void rgb133V4L2AssignImageSize(struct rgb133_handle* h, PV4L2FMTAPI _f)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   f->fmt.pix.bytesperline = f->fmt.pix.width * v4lBytesPerPixel(f->fmt.pix.pixelformat);

   switch(f->fmt.pix.pixelformat)
   {
      case V4L2_PIX_FMT_NV12:
      case V4L2_PIX_FMT_YVU420:
         f->fmt.pix.sizeimage = (f->fmt.pix.height * f->fmt.pix.bytesperline) + (f->fmt.pix.height * f->fmt.pix.bytesperline / 2);
      break;
      default:
         f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
   }

   /* Only set if it's a request for the output capture buffer format */
   if(rgb133V4L2IsCaptureVideo(f))
   {
      h->sCapture.pixelformat = f->fmt.pix.pixelformat;
      h->sCapture.imageSize = f->fmt.pix.sizeimage;
   }
}

void rgb133V4L2AssignPriv(PV4L2FMTAPI _f, unsigned long VerFrequency, unsigned long DefVerFrequency)
{
   struct v4l2_format* f = (struct v4l2_format*)_f;
   f->fmt.pix.priv =  VerFrequency ? VerFrequency : DefVerFrequency;
}

int rgb133V4L2GetSampleRate(PV4L2CAPPARMAPI _pCapParm, unsigned long* pSampleRate)
{
   struct v4l2_captureparm* pCapParm = (struct v4l2_captureparm*)_pCapParm;

   /* Only interested in frame time or now
    * scale by 1000 to increase precision in the division */
   if(pCapParm->timeperframe.numerator == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133V4L2GetSampleRate: Invalid timeperframe.numerator(%u)\n",
            pCapParm->timeperframe.numerator));
      return 1;
   }
   else if(pCapParm->timeperframe.denominator == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133V4L2GetSampleRate: Invalid timeperframe.denominator(%u)\n",
            pCapParm->timeperframe.denominator));
      return 1;
   }

   *pSampleRate =  (pCapParm->timeperframe.denominator * 1000) / pCapParm->timeperframe.numerator;

   return 0;
}

void rgb133V4L2AssignTimePerFrame(PV4L2CAPPARMAPI _pCapParm, unsigned long SampleRate, unsigned long VerFrequency)
{
   struct v4l2_captureparm* pCapParm = (struct v4l2_captureparm*)_pCapParm;

   pCapParm->timeperframe.numerator = 1001;

   /* Calculate denominator
    *
    * n = t and d = fps
    * -         -
    * d         n
    *
    * d = (n * fps)
    *
    */
   if(SampleRate == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignTimePerFrame: Set SampleRate to default (%lu mHz)\n",
            VerFrequency));
      SampleRate = VerFrequency;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133V4L2AssignTimePerFrame: denom = ( 1001 * SR(%lu) ) / 1000 = %lu\n",
      SampleRate, (1001 * SampleRate) / 1000));
   pCapParm->timeperframe.denominator = (1001 * SampleRate) / 1000;
}

void rgb133V4L2AssignControlLimits(struct rgb133_handle* h, PDEAPI _pDE, eControls ctrls)
{
   int id=0, min=0, max=10, def=5;
   int i=0, j=0;

   for(i=V4L2_CID_BASE; i<V4L2_CID_LASTP1; i++)
   {
      unsigned long flags = 0;

      switch(i)
      {
         case V4L2_CID_BRIGHTNESS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetBrightnessLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignControlLimits: Brightness - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= RGB133_ALL_CONTROLS;
            }
            break;

         case V4L2_CID_CONTRAST:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetContrastLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignControlLimits: Contrast - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= RGB133_ALL_CONTROLS;
            }
            break;
         case V4L2_CID_SATURATION:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetSaturationLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignControlLimits: Saturation - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= RGB133_ALL_CONTROLS;
            }
            break;
         case V4L2_CID_HUE:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetHueLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignControlLimits: Hue - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= RGB133_ALL_CONTROLS;
            }
            break;
         case V4L2_CID_BLACK_LEVEL:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetBlackLevelLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133V4L2AssignControlLimits: BlackLevel - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= RGB133_ALL_CONTROLS;
            }
            break;
         default:
            id = 0;
      }

      if(flags)
      {
         for(j=0; j<RGB133_NUM_CONTROLS; j++)
         {
            if(h->dev->V4L2Ctrls[h->channel][j].id == id)
            {
               RGB133PRINT((RGB133_LINUX_DBG_STUPID, "rgb133V4L2AssignControlLimits: hit V4L2 control(%d), local control(%d), with id(0x%x)\n",
                  i, j, h->dev->V4L2Ctrls[h->channel][j].id));
               h->dev->V4L2Ctrls[h->channel][j].minimum       = min;
               h->dev->V4L2Ctrls[h->channel][j].maximum       = max;
               h->dev->V4L2Ctrls[h->channel][j].default_value = def;
               break;
            }
         }
      }
   }
}

void rgb133V4L2AssignPrivateControlLimits(struct rgb133_handle* h, PDEAPI _pDE, PDEVPARMSAPI pCurrent, PDEVPARMSAPI pDetect,
      eControls ctrls)
{
   int id=0, min=0, max=10, def=5;
   int i=0, j=0;

   for(i=V4L2_CID_PRIVATE_BASE; i<V4L2_CID_PRIVATE_LASTP1; i++)
   {
      unsigned long flags = 0;

      switch(i)
      {
         case RGB133_V4L2_CID_HOR_TIME:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS ||
               ctrls == RGB133_DYNAMIC_CONTROLS)
            {
               CaptureGetHorTimeLimits(_pDE, &min, &max, &def, pDetect);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Horizontal Timing - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_VER_TIME:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS ||
               ctrls == RGB133_DYNAMIC_CONTROLS)
            {
               CaptureGetVerTimeLimits(_pDE, &min, &max, &def, pDetect);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Vertical Timing - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_HOR_POS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS ||
               ctrls == RGB133_DYNAMIC_CONTROLS)
            {
               CaptureGetHorPosLimits(_pDE, &min, &max, &def, pCurrent, pDetect);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Horizontal Position - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_HOR_SIZE:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS ||
               ctrls == RGB133_DYNAMIC_CONTROLS)
            {
               CaptureGetHorSizeLimits(_pDE, &min, &max, &def, pCurrent, pDetect);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Horizontal Size - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_PHASE:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetPhaseLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Phase - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_VERT_POS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS ||
               ctrls == RGB133_DYNAMIC_CONTROLS)
            {
               CaptureGetVerPosLimits(_pDE, &min, &max, &def, pCurrent, pDetect);
               RGB133PRINT((RGB133_LINUX_DBG_TRACE, "RealCaptureSetControls: Vertical Position - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_R_COL_BRIGHTNESS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetRedBrightnessLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Red Brightness - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_R_COL_CONTRAST:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetRedContrastLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Red Contrast - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_G_COL_BRIGHTNESS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetGreenBrightnessLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Green Brightness - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_G_COL_CONTRAST:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetGreenContrastLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Green Contrast - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_B_COL_BRIGHTNESS:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetBlueBrightnessLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Blue Brightness - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         case RGB133_V4L2_CID_B_COL_CONTRAST:
            id    = i;
            if(ctrls == RGB133_ALL_CONTROLS)
            {
               CaptureGetBlueContrastLimits(_pDE, &min, &max, &def);
               RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RealCaptureSetControls: Blue Contrast - id(0x%x) min(%d) max(%d) def(%d)\n",
                  id, min, max, def));
               flags |= ctrls;
            }
            break;
         default:
            id = 0;
      }

      if(flags)
      {
         for(j=0; j<RGB133_NUM_CONTROLS; j++)
         {
            if(h->dev->V4L2Ctrls[h->channel][j].id == id)
            {
               RGB133PRINT((RGB133_LINUX_DBG_STUPID, "RealCaptureSetControls: hit private V4L2 control(%d), local control(%d), with id(0x%x)\n",
                  i, j, h->dev->V4L2Ctrls[h->channel][j].id));
               h->dev->V4L2Ctrls[h->channel][j].minimum       = min;
               h->dev->V4L2Ctrls[h->channel][j].maximum       = max;
               h->dev->V4L2Ctrls[h->channel][j].default_value = def;
               break;
            }
         }
      }
   }
}

void rgb133V4L2AssignDeinterlace(u32* pField, PSETPARMSINAPI pSetParmsIn)
{
   switch(*pField)
   {
      case V4L2_FIELD_BOB:
         CaptureSetDeinterlaceBob(pSetParmsIn);
         break;
      case V4L2_FIELD_ALTERNATE:
         CaptureSetDeinterlaceAlternate(pSetParmsIn);
         break;
      case V4L2_FIELD_TOP:
         CaptureSetDeinterlaceField0(pSetParmsIn);
         break;
      case V4L2_FIELD_BOTTOM:
         CaptureSetDeinterlaceField1(pSetParmsIn);
         break;
      case V4L2_FIELD_ANY:
      case V4L2_FIELD_INTERLACED:
      case V4L2_FIELD_NONE:
      default:
         CaptureSetDeinterlaceFrame(pSetParmsIn);
         break;
   }
}

void rgb133V4L2AssignDeviceCaps(struct rgb133_dev* dev,
                        struct video_device *vdev,
                        struct v4l2_capability* pCap)
{
   unsigned long devCaps = 0;

   if (!DeviceIsControl(dev->control))
      devCaps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE | V4L2_CAP_STREAMING;
   else
      /* read() and write() system calls are actually supported on a control device */
      devCaps = V4L2_CAP_READWRITE;

   if (pCap)
   {
#ifdef RGB133_CONFIG_HAVE_V4L2_DEVICE_CAPS
      /* V4L2 ioctl core expects capabilities to be a superset of
       * device_caps and V4L2_CAP_DEVICE_CAPS
       */
      pCap->device_caps = devCaps;
      pCap->capabilities = devCaps | V4L2_CAP_DEVICE_CAPS;
#else
      pCap->capabilities = devCaps;
#endif
   }

#ifdef RGB133_CONFIG_HAVE_V4L2_DEVICE_CAPS_IN_VIDEO_DEVICE
   if (vdev)
      vdev->device_caps = devCaps;
#endif
}

