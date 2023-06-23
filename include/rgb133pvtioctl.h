/*
 * rgb133pvtioctl.h
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

#ifndef __RGB133_PVTIOCTL__H
#define __RGB133_PVTIOCTL__H

#include <asm/ioctl.h>

#define RGB133_PRIVATE_VIDIOC_BASE  (BASE_VIDIOC_PRIVATE+20)
#define RGB133_PRIVATE_IOCTL_BASE   0

/* Private V4L2 VIDIOC IOCTLS */
#define RGB133_PRIVATE_IOCTL_G_FMT            _IOWR('V', RGB133_PRIVATE_VIDIOC_BASE+0, struct v4l2_format)
#define RGB133_PRIVATE_IOCTL_S_FMT            _IOWR('V', RGB133_PRIVATE_VIDIOC_BASE+1, struct v4l2_format)

/* Private RGB133 IOCTLS */
#define RGB133_PRIVATE_IOCTL_PEEK             _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+0,  struct _srgb133_peek)
#define RGB133_PRIVATE_IOCTL_POKE             _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+1,  struct _srgb133_poke)
#define RGB133_PRIVATE_IOCTL_DUMP             _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+2,  struct _srgb133_dump)

#define RGB133_PRIVATE_IOCTL_SET_FORCE_DETECT _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+3,  struct _rgb133ForceDetect)

#define RGB133_PRIVATE_IOCTL_GET_EDID         _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+4,  struct _rgb133EdidOps)
#define RGB133_PRIVATE_IOCTL_SET_EDID         _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+5,  struct _rgb133EdidOps)

#define RGB133_PRIVATE_IOCTL_GET_DIAG_NAMES   _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+6,  struct _rgb133Diags)
#define RGB133_PRIVATE_IOCTL_GET_DIAG         _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+7,  struct _rgb133count)

#define RGB133_PRIVATE_IOCTL_G_VIDEO_TIMINGS  _IOWR('D', RGB133_PRIVATE_IOCTL_BASE+8, struct _srgb133VideoTimings)

#endif /* __RGB133_PVTIOCTL__H */
