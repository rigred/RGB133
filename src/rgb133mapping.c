/*
 * rgb133mapping.c
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

#include <linux/videodev2.h>

#include "rgb133config.h"
#include "rgb133capapi.h"
#include "rgb133mapping.h"
#include "rgb133vdif.h"
#include "rgb133v4l2.h"
#include "rgb133deviceapi.h"
#include "rgb133kernel.h"
#include "rgb133debug.h"

extern unsigned long rgb133_non_vl42_pix_fmts;

/* Indexed pixel formats match indexes of Windows pixel formats */
unsigned int fourccPixelFormats[] = {
   0,                            /*  0 */
   V4L2_PIX_FMT_RGB555,          /*  1 */
   V4L2_PIX_FMT_RGB565,          /*  2 */
   V4L2_PIX_FMT_RGB32,           /*  3 */
   0,                            /*  4 */
   V4L2_PIX_FMT_GREY,            /*  5 */
   0,                            /*  6 */
   V4L2_PIX_FMT_YUYV,            /*  7 */
#ifdef RGB133_CONFIG_HAVE_V4L2_EXT_PIX_FMTS
   V4L2_PIX_FMT_YVYU,            /*  8 */
#else
   0,
#endif
   V4L2_PIX_FMT_UYVY,            /*  9 */
   V4L2_PIX_FMT_R8,              /* 10 */
   V4L2_PIX_FMT_G8,              /* 11 */
   V4L2_PIX_FMT_B8,              /* 12 */
   V4L2_PIX_FMT_RGB24,           /* 13 */
   0,                            /* 14 */
   V4L2_PIX_FMT_NV12,            /* 15 */
   V4L2_PIX_FMT_YVU420,          /* 16 */
   0,                            /* 17 */
   0,                            /* 18 */
#ifdef INCLUDE_VISION
   V4L2_PIX_FMT_RGB10,           /* 19 */
   V4L2_PIX_FMT_Y410,            /* 20 */
#endif
   V4L2_PIX_FMT_BGR24,           /* Additional Linux pixel formats from here */
   V4L2_PIX_FMT_BGR32,
   V4L2_PIX_FMT_Y8,
   0
};

const char* pixelFormatsDesc[] = {
   "NOT USED",                   /*  0 */
   "555, packed, RGBO",          /*  1 */
   "565, packed, RGBP",          /*  2 */
   "888, packed, RGB4",          /*  3 */
   "NOT USED",                   /*  4 */
   "4:0:0, packed, GREY",        /*  5 */
   "NOT USED",                   /*  6 */
   "4:2:2, packed, YUY2",        /*  7 */
#ifdef RGB133_CONFIG_HAVE_V4L2_EXT_PIX_FMTS
   "4:2:2, packed, YVYU",        /*  8 */
#else
   "NOT USED",
#endif
   "4:2:2, packed, UYVY",        /*  9 */
   "8, packed, R",               /* 10 */
   "8, packed, G",               /* 11 */
   "8, packed, B",               /* 12 */
   "888, packed, RGB3",          /* 13 */
   "NOT USED",                   /* 14 */
   "4:2:0, semi-planar, NV12",   /* 15 */
   "4:2:0, planar, YV12",        /* 16 */
   "NOT USED",                   /* 17 */
   "NOT USED",                   /* 18 */
#ifdef INCLUDE_VISION
   "A2R10G10B10, packed, RGB10", /* 19 */
   "4:4:4, packed, Y410",        /* 20 */
#endif
   "888, packed, BGR3",
   "888, packed, BGR4",
   "4:0:0, packed, Y8",
};

/* Technically, these aren't const, as they're exposed in /sys/modules/ */
char vdif_string_nosignal[64] = { "No Signal" };
char vdif_string_outofrange[64] = { "Out Of Range" };
char vdif_string_unrec[64] = { "Unrecognisable" };

/* Return bytes per pixel from
 * a V4L enumeration
 */
int v4lBytesPerPixel(unsigned int PixelFormat)
{
   switch (PixelFormat)
   {
      case V4L2_PIX_FMT_RGB555:
      case V4L2_PIX_FMT_RGB555X:
      case V4L2_PIX_FMT_RGB565:
      case V4L2_PIX_FMT_RGB565X:
#ifdef RGB133_CONFIG_HAVE_V4L2_EXT_PIX_FMTS
      case V4L2_PIX_FMT_RGB444:
      case V4L2_PIX_FMT_YVYU:
      case V4L2_PIX_FMT_Y16:
#endif
      case V4L2_PIX_FMT_YUYV:
      case V4L2_PIX_FMT_UYVY:
         return 2;
      case V4L2_PIX_FMT_RGB24:
      case V4L2_PIX_FMT_BGR24:
         return 3;
      case V4L2_PIX_FMT_RGB32:
      case V4L2_PIX_FMT_BGR32:
      case V4L2_PIX_FMT_RGB10:
      case V4L2_PIX_FMT_Y410:
         return 4;
      case V4L2_PIX_FMT_R8:
      case V4L2_PIX_FMT_G8:
      case V4L2_PIX_FMT_B8:
      case V4L2_PIX_FMT_GREY:
      case V4L2_PIX_FMT_Y8:
      case V4L2_PIX_FMT_RGB332:
      case V4L2_PIX_FMT_NV12:
      case V4L2_PIX_FMT_YVU420:
         return 1;
      default:
         return -1;
   }
}

int v4lSupportedPixelFormats(void)
{
   return RGB133_SPPT_PIXEL_FORMATS;
}

int IsV4L2PixelFormat(unsigned int PixelFormat)
{
   switch (PixelFormat)
   {
      case V4L2_PIX_FMT_RGB555:
      case V4L2_PIX_FMT_RGB555X:
      case V4L2_PIX_FMT_RGB565:
      case V4L2_PIX_FMT_RGB565X:
#ifdef RGB133_CONFIG_HAVE_V4L2_EXT_PIX_FMTS
      case V4L2_PIX_FMT_RGB444:
      case V4L2_PIX_FMT_YVYU:
      case V4L2_PIX_FMT_Y16:
#endif
      case V4L2_PIX_FMT_YUYV:
      case V4L2_PIX_FMT_UYVY:
      case V4L2_PIX_FMT_RGB24:
      case V4L2_PIX_FMT_BGR24:
      case V4L2_PIX_FMT_RGB32:
      case V4L2_PIX_FMT_BGR32:
      case V4L2_PIX_FMT_GREY:
      case V4L2_PIX_FMT_RGB332:
      case V4L2_PIX_FMT_NV12:
      case V4L2_PIX_FMT_YVU420:
         return 1;
      case V4L2_PIX_FMT_R8:
      case V4L2_PIX_FMT_G8:
      case V4L2_PIX_FMT_B8:
      case V4L2_PIX_FMT_RGB10:
      case V4L2_PIX_FMT_Y410:
      case V4L2_PIX_FMT_Y8:
         return 0;
      default:
         return 0;
   }
}

int v4lSupportedPixelFormatsReal(struct rgb133_handle* h)
{
   unsigned int fmt = 0;
   int i = 0;
   int j = 0;

   for(i=0; i<=RGB133_SPPT_PIXEL_FORMATS; i++)
   {
      fmt = fourccPixelFormats[i];
      if(((fmt == V4L2_PIX_FMT_RGB10 || fmt == V4L2_PIX_FMT_Y410) && !DeviceIs10BitSupported(h)) ||
         ((fmt == V4L2_PIX_FMT_YVU420) && !DeviceIsYV12Supported(h)) ||
         (!(IsV4L2PixelFormat(fmt)) && (rgb133_non_vl42_pix_fmts == RGB133_DISABLE_NON_V4L2_PIX_FMTS)) ||
         fmt == 0)
         continue;

      j++;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "v4lSupportedPixelFormatsReal: return %d\n", j));

   return j;
}

unsigned int v4lGetExistingFormatIndex(struct rgb133_handle* h, unsigned int index)
{
   unsigned int formats_found = 0;
   unsigned int real_index = 0;
   unsigned int fmt = 0;
   unsigned int i = 0;

   while((formats_found <= index) && (i <= v4lSupportedPixelFormats()))
   {
      fmt = fourccPixelFormats[i];
      i++;

      if(((fmt == V4L2_PIX_FMT_RGB10 || fmt == V4L2_PIX_FMT_Y410) && !DeviceIs10BitSupported(h)) ||
         ((fmt == V4L2_PIX_FMT_YVU420) && !DeviceIsYV12Supported(h)) ||
         (!(IsV4L2PixelFormat(fmt)) && (rgb133_non_vl42_pix_fmts == RGB133_DISABLE_NON_V4L2_PIX_FMTS)) ||
         fmt == 0)
         continue;

      formats_found++;
   }
   real_index = i - 1;

   return real_index;
}

/* Map pixel formats
 * from - windows value
 * to   - address of linux value
 */
void pixelFormatMappingWinToV4L(unsigned int from, unsigned int* to)
{
   int i, found = FALSE;
   /* */
   for(i=0; i<=v4lSupportedPixelFormats(); i++)
   {
      if(from == i)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingWinToV4L: Set to fourccPixelFormats[%d](0x%x)\n",
            i, fourccPixelFormats[i]));
         *to = fourccPixelFormats[i];
         found = TRUE;
         break;
      }
   }

   if(!found)
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingWinToV4L: Failed to match pixelFormat(0x%x)\n", from));
}

/*unsigned int v4lSwapPixelFormat(unsigned int PixelFormat)
{
   switch(PixelFormat)
   {
      case V4L2_PIX_FMT_BGR24:
         RGB133PRINT((RGB133_LINUX_DBG_TODO, "v4lSwapPixelFormat: Return V4L2_PIX_FMT_RGB24(0x%x)\n", V4L2_PIX_FMT_RGB24)
         return V4L2_PIX_FMT_RGB24;
      case V4L2_PIX_FMT_RGB24:
         RGB133PRINT((RGB133_LINUX_DBG_TODO, "v4lSwapPixelFormat: Return V4L2_PIX_FMT_BGR24(0x%x)\n", V4L2_PIX_FMT_BGR24)
         return V4L2_PIX_FMT_BGR24;
      case V4L2_PIX_FMT_BGR32:
         RGB133PRINT((RGB133_LINUX_DBG_TODO, "v4lSwapPixelFormat: Return V4L2_PIX_FMT_RGB32(0x%x)\n", V4L2_PIX_FMT_RGB32)
         return V4L2_PIX_FMT_RGB32;
      case V4L2_PIX_FMT_RGB32:
         RGB133PRINT((RGB133_LINUX_DBG_TODO, "v4lSwapPixelFormat: Return V4L2_PIX_FMT_BGR32(0x%x)\n", V4L2_PIX_FMT_BGR32)
         return V4L2_PIX_FMT_BGR32;
      default:
         return PixelFormat;
   }
}*/

/* Map pixel formats
 * from - V4L value
 * to   - address of win value
 */
void pixelFormatMappingV4LToWin(unsigned int from, unsigned int* to)
{
   int i, found = FALSE;
   /* */
   for(i=0; i<=v4lSupportedPixelFormats(); i++)
   {
      if(from == fourccPixelFormats[i])
      {
         /* Additional Linux pixel formats */
         if(fourccPixelFormats[i] == V4L2_PIX_FMT_BGR32)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingV4LToWin: Set to %d\n", 3));
            *to = 3;
            found = TRUE;
            break;
         }
         else if(fourccPixelFormats[i] == V4L2_PIX_FMT_BGR24)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingV4LToWin: Set to %d\n", 13));
            *to = 13;
            found = TRUE;
            break;
         }
         else if(from == V4L2_PIX_FMT_Y8)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingV4LToWin: Force set to Grey(%d)\n", 5));
            *to = 5;
            found = TRUE;
            break;
         }
         else /* 1to1 match to Windows pixel formats */
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingV4LToWin: Set to %d\n", i));
            *to = i;
            found = TRUE;
            break;
         }
      }
   }

   if(!found)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatMappingV4LToWin: Failed to match pixelFormat(0x%x)\n", from));
      *to = 0;
   }
}

/* Map pixel formats description
 * from     - windows value
 * returns  - description of the pixel format
 */
const char* pixelFormatDescMappingWinToV4L(unsigned int from)
{
   int i;
   /* */
   for(i=0; i<=v4lSupportedPixelFormats(); i++)
   {
      if(from == i)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "pixelFormatDescMappingWinToV4L: return pixelFmtDesc[%d](%s)\n",
               i, pixelFormatsDesc[i]));
         return pixelFormatsDesc[i];
      }
   }
   return "UNKNOWN PIXFMT";
}

/* Map pixel formats description
 * from     - windows value
 * returns  - description of the pixel format
 */
const char* pixelFormatDescMappingV4LToWin(unsigned int from)
{
   /*
   int i;
   for(i=RGBPIXELFORMAT_UNKNOWN; i<RGBPIXELFORMAT_COUNT; i++)
   {
      if(from == i)
      {
         return pixelFormatsDesc[i];
      }
   }*/
   return "UNKNOWN";
}

const char* vdifFlagsString(unsigned int from)
{
   if(from & RGB133_VDIF_FLAG_NOSIGNAL)
      return vdif_string_nosignal;
   else if(from & RGB133_VDIF_FLAG_OUTOFRANGE)
      return vdif_string_outofrange;
   else if(from & RGB133_VDIF_FLAG_UNRECOGNISABLE)
      return vdif_string_unrec;

   return "UNKNOWN";
}


const char string_dma_overflow[64] = { "DMA Overflow" };

const char* bufferNotificationString(unsigned int from)
{
   if(from == RGB133_NOTIFY_DMA_OVERFLOW)
      return string_dma_overflow;

   return "UNKNOWN";
}

/* Map windows values to linux equivalents
 * mapping_type - the type of conversion to use
 * win_value    - windows value to convert
 * returns      - The linux equivalent of the windos value
 */
unsigned int mapUItoUI(enum rgb133_mapping_type type, enum rgb133_mapping_dir dir, unsigned int from)
{
   unsigned int to = 0;

   switch(type)
   {
      case __PIXELFORMAT__:
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "mapUItoUI: __PIXELFORMAT__ mapping\n"));
         if(dir == __MAP_TO_WIN__)
            pixelFormatMappingV4LToWin(from, &to);
         else
            pixelFormatMappingWinToV4L(from, &to);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "mapUItoUI: __PIXELFORMAT__ from(%d) -> to(%d)\n",
               from, to));
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "mapUItoUI: unknown mapping\n"));
         break;
   }

   return to;
}

/* Map windows values to linux equivalents
 * mapping_type - the type of conversion to use
 * win_value    - windows value to convert
 * returns      - The linux equivalent of the windos value
 */
const char* mapUItoCC(enum rgb133_mapping_type type, enum rgb133_mapping_dir dir, unsigned int from)
{
   return __mapUItoCC(type, dir, from, 0, 0, 0);
}

const char* __mapUItoCC(enum rgb133_mapping_type type, enum rgb133_mapping_dir dir, unsigned int from, char* messagesIn[], char* messagesOut[], int* pNumMessages)
{
   const char* to = "UNKNOWN";
   int i = 0;

   switch(type)
   {
      case __PIXELFORMATDESC__:
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "mapUItoCC: __PIXELFORMATDESC__ mapping\n"));
         if(dir == __MAP_TO_WIN__)
            to = pixelFormatDescMappingV4LToWin(from);
         else
            to = pixelFormatDescMappingWinToV4L(from);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "mapUItoCC: __PIXELFORMAT__ from(%d) -> to(%s)\n",
               from, to));
         break;
      case __VDIFFLAGS__:
         if(dir != __MAP_TO_STRING__)
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "mapUItoCC: unknown VDIF flags mapping\n"));
         }
         else
         {
            to = vdifFlagsString(from);

            // Always copy first message
            strncpy(messagesOut[0], to, strlen(to));
            // Conditionally add more messages
            if(*pNumMessages > 0)
            {
               for(i=1; i<=(*pNumMessages); i++)
               {
                  strncpy(messagesOut[i], messagesIn[i-1], strlen(messagesIn[i-1]));
               }
            }
            (*pNumMessages)++;
         }
         break;
      case __BUFFER_NOTIFICATIONS__:
         if(dir != __MAP_TO_STRING__)
         {
            RGB133PRINT((RGB133_LINUX_DBG_WARNING, "mapUItoCC: unknown buffer notifications mapping\n"));
         }
         else
         {
            to = bufferNotificationString(from);

            // Always copy first message
            strncpy(messagesOut[0], to, strlen(to));
            // Conditionally add more messages
            if(*pNumMessages > 0)
            {
               for(i=1; i<=(*pNumMessages); i++)
               {
                  strncpy(messagesOut[i], messagesIn[i-1], strlen(messagesIn[i-1]));
               }
            }
            (*pNumMessages)++;
         }
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "mapUItoCC: unknown mapping\n"));
         break;
   }

   return to;
}
