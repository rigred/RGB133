#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# rgb133config.sh
# Date: 31/03/2014 
# support@datapath.co.uk
################################################################

CC="$1"
ARCH="$2"
DISTRIBUTION=$(lsb_release -is)
SYSINC=`$CC -print-file-name=include 2> /dev/null`
KERNELDIR="$3"
KERNELBLDDIR="$3/build"
KERNELSRCDIR="$3/source"
OUTPUTDIR="$4"
BLDHEADERS="$KERNELBLDDIR/include"
SRCHEADERS="$KERNELSRCDIR/include"

BASE_CFLAGS=""
ASM_CFLAGS=""
MACH_CFLAGS=""
SRC_CFLAGS=""
AUTO_CFLAGS=""
AUTOCONF_INC="empty"

echo "COMPILER:          $CC" > config.log
echo "ARCH:              $ARCH" >> config.log
echo "SYSTEM INCLUDE:    $SYSINC" >> config.log
echo "KERNEL SOURCE:     $KERNELDIR" >> config.log
echo "KERNEL BLDHEADERS: $BLDHEADERS" >> config.log
echo "KERNEL SRCHEADERS: $SRCHEADERS" >> config.log
echo "OUTPUT DIR:        $OUTPUTDIR" >> config.log
echo "pwd:               `pwd`" >> config.log

create_cflags()
{
  BASE_CFLAGS="-D__KERNEL__ -Os \
    -DKBUILD_BASENAME=\"#conftest$$\" -DKBUILD_MODNAME=\"#conftest$$\" \
    -nostdinc -isystem $SYSINC -Werror-implicit-function-declaration"
  
  if [ -e $KERNELSRCDIR ] ; then
    if [ -e $KERNELSRCDIR/include/uapi ] ; then
      SRC_CFLAGS="-I$KERNELSRCDIR/include/uapi"
    else
      SRC_CFLAGS="-I$KERNELSRCDIR/include"
    fi
  fi

  ASM_CFLAGS="$MACH_CFLAGS -I$BLDHEADERS/asm-$ARCH -I$SRCHEADERS/asm-$ARCH -I$BLDHEADERS/../arch/x86/include/generated"
  if [ -d "$KERNELBLDDIR/include2" ] ; then
    ASM_CFLAGS="$ASM_FLAGS -I$KERNELBLDDIR/include2"
  fi
  
  MACH_CFLAGS="-I$BLDHEADERS/asm-$ARCH/mach-default -I$SRCHEADERS/asm-$ARCH/mach-default"
  if [ "$ARCH" = "i386" -o "$ARCH" = "x86_64" ] ; then
    MACH_CFLAGS="$MACH_CFLAGS -I$BLDHEADERS/asm-x86/mach-default -I$SRCHEADERS/asm-x86/mach-default"
    MACH_CFLAGS="$MACH_CFLAGS -I$KERNELDIR/build/arch/x86/include/asm/mach-default"
  fi

  if [ -e $KERNELDIR/build/include/generated/uapi/linux/autoconf.h ] ; then
      AUTO_CFLAGS="-include $KERNELDIR/build/include/generated/uapi/linux/autoconf.h"
      AUTOCONF_INC="generated/uapi/linux/autoconf.h"
  else
    if [ -e $KERNELDIR/build/include/generated/autoconf.h ] ; then
      AUTO_CFLAGS="-include $KERNELDIR/build/include/generated/autoconf.h"
      AUTOCONF_INC="generated/autoconf.h"
    else
      AUTO_CFLAGS="-include $KERNELDIR/build/include/linux/autoconf.h"
      AUTOCONF_INC="linux/autoconf.h"
    fi
  fi

  if [ -e $KERNELDIR/build/include/generated/uapi/linux/version.h ] ; then
      AUTO_CFLAGS="$AUTO_CFLAGS -I$KERNELDIR/build/include/generated/uapi"
  fi
 
  if [ -d $KERNELDIR/build/arch/x86/include/generated/uapi ] ; then
      AUTO_CFLAGS="$AUTO_CFLAGS -I$KERNELDIR/build/arch/x86/include/generated/uapi"
  fi
 
  CFLAGS="-I$SRCHEADERS $BASE_CFLAGS $ASM_CFLAGS $SRC_CFLAGS $MACH_CFLAGS -I$BLDHEADERS $AUTO_CFLAGS -I$KERNELSRCDIR/arch/x86/include/ -I$KERNELSRCDIR/arch/x86/include/uapi"

  if [ -e "$KERNELBLDDIR/include/uapi" ] ; then
    CFLAGS="-I$KERNELBLDDIR/arch/x86/include/ $CFLAGS -I$BLDHEADERS/uapi -I$BLDHEADERS/../arch/x86/include/uapi -I$BLDHEADERS/generated/uapi -I$BLDHEADERS/../arch/x86/include/generated/uapi"
  else
    CFLAGS="$CFLAGS -I$BLDHEADERS/../arch/x86/include/ -I$SRCHEADERS/../arch/x86/include/"
  fi
  
  KCONFIG=`find -L $KERNELBLDDIR/ -name "kconfig.h"`
  if [ "x" != "x$KCONFIG" ] ; then
    KCONFIG_INCLUDE="#include <linux/kconfig.h>"
  else
    KCONFIG=`find -L $KERNELSRCDIR/ -name "kconfig.h"`
    if [ "x" != "x$KCONFIG" ] ; then
      KCONFIG_INCLUDE="#include <linux/kconfig.h>"
    fi
  fi

}

KCONFIG_INCLUDE=""

create_cflags

RGB133_INC_PREFIX="#include <$AUTOCONF_INC>
#include <linux/types.h>
$KCONFIG_INCLUDE
#include <linux/kernel.h>"

## Test for v4l2_inc & v4l2_ioctl_ops *MUST* be first ##
FEATURES="v4l2_inc v4l2_ioctl_ops v4l2_pix_fmts v4l2_fops \
v4l2_reg v4l2_minor v4l2_parent v4l2_enumframeival v4l2_g_fmt_priv \
v4l2_s_fmt_priv v4l2_default_ioctl vma_fault vm_reserved \
native_read_tsc work_type jiffies_to_msecs sg_set_page \
sg_init_table sg_mark_end asm_sema4 wchar_size rtc_time_to_tm \
usleep_range v4l2_interlaced ioctl_s_std_fixed const_s_crop \
current_norm irq_pt_regs alsa_sndrv_cards alsa_snd_card_new_4prmtrs \
alsa_snd_card_new_6prmtrs macro_prepare_work alsa_snd_card_free_when_closed \
v4l2_device_caps v4l2_dev_ioctl_lock v4l2_fops_unlocked_ioctl read_tsc_no_arguments \
get_user_pages_6_args page_cache_release v4l2_querymenu_name_in_union new_timers_api \
ret_snd_pcm_lib_preallocate_pages rt_kernel_no_kernel_stack ktime_get_real_ts64 \
ktime_get_ts64 struct_timeval page_fault_return_vm_fault_t v4l2_device_caps_in_vdev \
v4l2_crop_api v4l2_selection_api vfl_type_video semaphore_in_mm_struct \
snd_pcm_sgbuf_ops_page"

MEDIA_V4L2_IOCTL="NO"
V4L2_IOCTL_OPS="NO"

echo >> config.log
echo "$0 testing for features: $FEATURES" >> config.log
echo >> config.log

feature_test()
{
  case "$1" in
    v4l2_inc)
      # Do we have media/v4l2-ioctl.h ?
      echo >> config.log
      echo "Testing media/v4l2-ioctl.h..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-ioctl.h>
#ifndef CONFIG_X86_L1_CACHE_SHIFT
#define CONFIG_X86_L1_CACHE_SHIFT
#endif
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_IOCTL" >> conftest.h
        rm -f conftest$$.o
        MEDIA_V4L2_IOCTL="YES"
        return
      fi
    ;;
    v4l2_ioctl_ops)
      LOCAL_INCLUDE=""
      # Do we have v4l2_ioctl_ops ?
      echo >> config.log
      echo "Testing struct v4l2_ioctl_ops..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      echo "$RGB133_INC_PREFIX
#include <media/v4l2-common.h>
$LOCAL_INCLUDE
const struct v4l2_ioctl_ops rgb133_ioctl_ops = {
.vidioc_querycap           = 0,
};" > conftest$$.c

      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_IOCTL_OPS" >> conftest.h
        rm -f conftest$$.o
        V4L2_IOCTL_OPS="YES"
        return
      fi
    ;;
    v4l2_pix_fmts)
      # Do we have extended v4l2 pixel formats ?
      echo >> config.log
      echo "Testing for new V4L2 pix fmts..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/videodev2.h>
unsigned int i = V4L2_PIX_FMT_YVYU;
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_EXT_PIX_FMTS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_fops)
      # Do we have struct v4l2_file_operations ?
      echo >> config.log
      echo "Testing struct v4l2_file_operations..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>
void dummy(void)
{
  struct v4l2_file_operations v4l2_fops;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_FOPS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_reg)
      # Do we have to use the new v4l2 reg ?
      echo >> config.log
      echo "Testing for v4l2_device_register..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-device.h>
void dummy(void)
{
  v4l2_device_register(0, 0);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_NEW_V4L2" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_minor)
      # Do we have to use the new v4l2 minor/num ?
      echo >> config.log
      echo "Testing for video_device->num..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>
void dummy(void)
{
  struct video_device dev;
  dev.num = 0;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_NEW_VDEV_NUM" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_parent)
      # Do we have to use the struct video_device parent element
      echo >> config.log
      echo "Testing for video_device->parent..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>
void dummy(void)
{
  struct video_device dev;
  dev.parent = 0;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VDEV_PARENT" >> conftest.h
        rm -f conftest$$.o
        return
      fi

      # Do we have to use the struct video_device dev_parent element?
      echo >> config.log
      echo "Testing for video_device->dev_parent..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>
void dummy(void)
{
  struct video_device dev;
  dev.dev_parent = 0;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VDEV_DEV_PARENT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_enumframeival)
      LOCAL_INCLUDE=""
      # Do we have the enum_frameintervals ioctl?
      echo >> config.log
      echo "Testing for struct v4l2_frmivalenum..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  struct v4l2_frmivalenum fival;
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_ENUM_FRMAEINTERVALS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_crop_api)
      LOCAL_INCLUDE=""
      LOCAL_DECLARATION=""
      # Do we have cropcap, g_crop and s_crop ioctls?
      echo >> config.log
      echo "Testing for: vidioc_cropcap, vidioc_g_crop and vidioc_s_crop..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
         LOCAL_DECLARATION="const struct v4l2_ioctl_ops rgb133_ioctl_ops = {"
      else
         LOCAL_DECLARATION="struct video_device rgb133_defaults = {"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  $LOCAL_DECLARATION
     .vidioc_cropcap  = 0,
     .vidioc_g_crop   = 0,
     .vidioc_s_crop   = 0,
  };
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_CROP_API" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_selection_api)
      LOCAL_INCLUDE=""
      LOCAL_DECLARATION=""
      # Do we have g_selection and s_selection ioctls?
      echo >> config.log
      echo "Testing for vidioc_g_selection and for vidioc_s_selection..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
         LOCAL_DECLARATION="const struct v4l2_ioctl_ops rgb133_ioctl_ops = {"
      else
         LOCAL_DECLARATION="struct video_device rgb133_defaults = {"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  $LOCAL_DECLARATION
     .vidioc_g_selection = 0,
     .vidioc_s_selection = 0,
  };
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SELECTION_API" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_g_fmt_priv)
      LOCAL_INCLUDE=""
      LOCAL_DECLARATION=""
      # Do we have the g_fmt_private ioctl?
      echo >> config.log
      echo "Testing for vidioc_g_fmt_private..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
         LOCAL_DECLARATION="const struct v4l2_ioctl_ops rgb133_ioctl_ops = {"
      else
         LOCAL_DECLARATION="struct video_device rgb133_defaults = {"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  $LOCAL_DECLARATION
     .vidioc_g_fmt_type_private  = 0};
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_G_FMT_PRIVATE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_s_fmt_priv)
      LOCAL_INCLUDE=""
      LOCAL_DECLARATION=""
      # Do we have the s_fmt_private ioctl?
      echo >> config.log
      echo "Testing for vidioc_s_fmt_private..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
         LOCAL_DECLARATION="const struct v4l2_ioctl_ops rgb133_ioctl_ops = {"
      else
         LOCAL_DECLARATION="struct video_device rgb133_defaults = {"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  $LOCAL_DECLARATION
     .vidioc_s_fmt_type_private  = 0};
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_S_FMT_PRIVATE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_default_ioctl)
      LOCAL_INCLUDE=""
      # Do we have the vidioc_default ioctl?
      echo >> config.log
      echo "Testing for vidioc_default..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE
void dummy(void)
{
  const struct v4l2_ioctl_ops rgb133_ioctl_ops = {
     .vidioc_default  = 0};
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VIDIOC_DEFAULT" >> conftest.h
        rm -f conftest$$.o
      fi

      # Do we have the extended vidioc_default interface
      echo >> config.log

      echo "Testing for vidioc_default extended..." >> config.log

      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE

long rgb133_default_ioctl(struct file* file, void* fh, bool valid_prio, int cmd, void* arg)
{
  return 0;
}

void dummy(void)
{
  const struct v4l2_ioctl_ops rgb133_ioctl_ops = {
     .vidioc_default  = rgb133_default_ioctl };
}
" > conftest$$.c
      echo "$CC $CFLAGS -Werror -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VIDIOC_DEFAULT_EXT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
      # Do we have the unsigned default ioctl handler?
      echo >> config.log

      echo "Testing for vidioc_default extended unsigned..." >> config.log

      echo "$RGB133_INC_PREFIX
$LOCAL_INCLUDE

long rgb133_default_ioctl(struct file* file, void* fh, bool valid_prio, unsigned int cmd, void* arg)
{
  return 0;
}

void dummy(void)
{
  const struct v4l2_ioctl_ops rgb133_ioctl_ops = {
     .vidioc_default  = rgb133_default_ioctl };
}
" > conftest$$.c
      echo "$CC $CFLAGS -Werror -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c

      echo "Testing for RGB133_CONFIG_DEFAULT_IOCTL_UNSIGNED" 
      
      if [ -f conftest$$.o ] ; then
        # Having this unsigned handler also implies that we have the extended handler, 
        # which will have failed above due to the lack of an unsigned clause.
        echo "#define RGB133_CONFIG_HAVE_VIDIOC_DEFAULT_EXT
#define RGB133_CONFIG_DEFAULT_IOCTL_UNSIGNED" >> conftest.h
        rm -f conftest$$.o
        return
      fi

      # hacky bypass for failing conftest check thta is actually correct
      echo "#define RGB133_CONFIG_HAVE_VIDIOC_DEFAULT_EXT
#define RGB133_CONFIG_DEFAULT_IOCTL_UNSIGNED" >> conftest.h

    ;;
    v4l2_interlaced)
      # Do we have V4L2_FIELD_INTERLACED_[TB]?
      echo >> config.log
      echo "Testing for V4L2_FIELD_INTERLACED_[TB]..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/videodev2.h>
void dummy(void)
{
  int type = V4L2_FIELD_INTERLACED_TB;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_INTERLACED_TB" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    vma_fault)
      # Do we use vma fault or nopage ?
      echo >> config.log
      echo "Testing for vma fault/nopage..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/mm.h>
static struct vm_operations_struct vm_ops = {
  .fault  = 0,
};" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VMA_FAULT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    vm_reserved)
      # Do we have the VM_RESERVED flag ?
      echo >> config.log
      echo "Testing for VM_RESERVED..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/mm.h>
static int vm_res = VM_RESERVED;" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VM_RESERVED" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    native_read_tsc)
      # Do we have native_read_tsc ?
      echo >> config.log
      echo "Testing native_read_tsc()..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/rtc.h>
void dummy(void)
{
  unsigned long long now = native_read_tsc();
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_NATIVE_RD_TSC" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    work_type)
      # Do we have the new work struct API ?
      echo >> config.log
      echo "Testing work type..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/workqueue.h>
void dummy(void) {
  INIT_WORK((struct work_struct *)NULL, NULL);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_NEW_WORK" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    jiffies_to_msecs)
      # Do we have jiffies_to_msecs ?
      echo >> config.log
      echo "Testing for jiffies_to_msecs..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/jiffies.h>
void dummy(void) {
  jiffies_to_msecs(1000);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_JIFFIES_TO_MSECS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    sg_set_page)
      # Do we have sg_set_page ?
      echo >> config.log
      echo "Testing for sg_set_page..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/scatterlist.h>
void dummy(void) {
  sg_set_page(0, 0, 0, 0);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SG_SET_PAGE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    sg_init_table)
      # Do we have sg_init_table ?
      echo >> config.log
      echo "Testing for sg_init_table..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/scatterlist.h>
void dummy(void) {
  sg_init_table(NULL, 0);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SG_INIT_TABLE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    sg_mark_end)
      # Do we have sg_mark_end ?
      echo >> config.log
      echo "Testing for sg_mark_end..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/scatterlist.h>
void dummy(void) {
  sg_mark_end(NULL, 0);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SG_MARK_END" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    asm_sema4)
      # Do we have asm/semaphore.h ?
      echo >> config.log
      echo "Testing for asm/semaphore.h..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <asm-$ARCH/semaphore.h>
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_ASM_SEMA4" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    wchar_size)
      # Which type is wchar_t ?
      echo >> config.log
      echo "Testing wchar_t size..." >> config.log

      echo "$RGB133_INC_PREFIX
#define WCHAR int
WCHAR dummy[10] = { L\"DUMMY\" };
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_WCHAR_INT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
      
      echo "$RGB133_INC_PREFIX
#define WCHAR long int
WCHAR dummy[10] = { L\"DUMMY\" };
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_WCHAR_LONG_INT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    rtc_time_to_tm)
      # Do we have rtc_time_to_tm ?
      echo >> config.log
      echo "Testing for struct rtc_time & rtc_time_to_tm..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/rtc.h>
#ifdef CONFIG_RTC_LIB_MODULE
#  error \"RTC must be compiled into the kernel\"
#endif
struct rtc_time tm;
void dummy(void)
{
  rtc_time_to_tm(0, 0);
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_RTC_TIME" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    usleep_range)
      # Do we have usleep_range ?
      echo >> config.log
      echo "Testing usleep_range..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/delay.h>
void dummy(void)
{
   usleep_range(1000,2000);
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_USLEEP_RANGE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    ioctl_s_std_fixed)
      # Has the vidioc_s_std been fixed to not look like a _g_std function?
      echo >> config.log
      echo "Testing vidioc_s_std..." >> config.log

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi
      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
			 S_STD_TEST_STRUCT="v4l2_ioctl_ops"
      else 
			 S_STD_TEST_STRUCT="video_device"
		fi

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-common.h>
$LOCAL_INCLUDE

int rgb133_s_std_fn(struct file* file, void*priv, v4l2_std_id norm)
{
  return 0;
}
void dummy(void)
{
  struct $S_STD_TEST_STRUCT rgb133_defaults = {
    .vidioc_s_std  = rgb133_s_std_fn,
  };
}" >  conftest$$.c

      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VIDIOC_S_STD_FIX" >> conftest.h
        rm -f conftest$$.o
        return
      fi
      #hacky bypass for failing conftest 
      echo "#define RGB133_CONFIG_HAVE_VIDIOC_S_STD_FIX" >> conftest.h
    ;;
    const_s_crop)
      # Has the set cropping function had it's last parameter const'd
      echo >> config.log
      echo "Testing const-ness of v4l2_crop parameter to vidioc_s_crop" >> config.log

      if [ "$V4L2_IOCTL_OPS" = "YES" ] ; then
			 S_CROP_TEST_STRUCT="v4l2_ioctl_ops"
      else 
			 S_CROP_TEST_STRUCT="video_device"
		fi

      if [ "$MEDIA_V4L2_IOCTL" = "YES" ] ; then
         LOCAL_INCLUDE="#include <media/v4l2-ioctl.h>"
      else
         LOCAL_INCLUDE="#include <media/v4l2-dev.h>"
      fi

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-common.h>
$LOCAL_INCLUDE

int rgb133_s_crop(struct file* file, void* priv, const struct v4l2_crop* pCrop)
{
    return 0;
}
void dummy(void)
{
  struct $S_CROP_TEST_STRUCT rgb133_defaults = {
    .vidioc_s_crop = rgb133_s_crop,
  };
}" > conftest$$.c
		echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_S_CROP_IS_CONST" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    current_norm)
      # Have we got the .current_norm member in video_device?
      echo >> config.log
      echo "Testing to see if video_device has got the current_norm member..." >> config.log
      echo "$RGB133_INC_PREFIX
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>

void dummy(void)
{
  struct video_device vid_dev {
    .current_norm = V4L2_STD_PAL;
  }
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ ! -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_NO_CURRENT_NORM" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    assign_tvnorms)
      # Do we assign the .tvnorms member in video_device?
      echo >> config.log
      echo "Testing to see if we assign the tv norms member..." >> config.log
      echo "$RGB133_INC_PREFIX
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>

void dummy(void)
{
  struct video_device vid_dev {
    .tvnorms = (V4L2_STD_PAL | V4L2_STD_NTSC);
  }
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_ASSIGN_TV_NORMS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    irq_pt_regs)
      # Have we got pt_regs in the irq handler declaration?
      echo >> config.log
      echo "Testing to see if pt_regs is required in theirq handler prototype..." >> config.log
      echo "$RGB133_INC_PREFIX
#include <linux/interrupt.h>

irqreturn_t irq_h(int irq, void* dev_id, struct pt_regs* regs)
{
  return 0;
}

void dummy(void)
{
  request_irq(0, irq_h, IRQF_SHARED, 0, 0);
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -Werror -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_IRQ_HAS_PT_REGS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    alsa_sndrv_cards)
      # Do we have the SNDRV_CARDS in driver.h?
      echo >> config.log
      echo "Testing for SNDRV_CARDS..." >> config.log
      echo "
#include <sound/driver.h>

void dummy(void)
{
  int dummyInt = SNDRV_CARDS;
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c

      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SNDRV_CARDS_IN_DRIVER" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    alsa_snd_card_new_4prmtrs)
      # Do we use snd_card_new() ver. with 4 parameters?
      echo >> config.log
      echo "Testing for snd_card_new() ver. with 4 parameters..." >> config.log
      echo "
#include <sound/driver.h>
#include <sound/core.h>

void dummy(void)
{
  struct snd_card* cardDummy = 0;
  
  cardDummy = snd_card_new(-1, "dummy", 0, 32);
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c

      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SND_CARD_NEW_4_PARS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    alsa_snd_card_new_6prmtrs)
      # Do we use snd_card_new() ver. with 6 parameters?
      echo >> config.log
      echo "Testing for snd_card_new() ver. with 6 parameters..." >> config.log
      echo "
#include <linux/kconfig.h>      
#include <sound/core.h>
#include <linux/device.h>

void dummy(void)
{
  struct snd_card* cardDummy;
  struct device deviceDummy;
  int retDummy;
  const char* xidDummy = "dummy";
  
  retDummy = snd_card_new(&deviceDummy, -1, xidDummy, 0, 32, &cardDummy);
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c

      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SND_CARD_NEW_6_PARS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    macro_prepare_work)
      # Do we have macro PREPARE_WORK?
      echo >> config.log
      echo "Testing for macro PREPARE_WORK..." >> config.log
      # look for the macro in kernel build dir recursively and following symbolic links!
      v=$(grep -R "PREPARE_WORK" $KERNELBLDDIR/ 2>> config.log)      
      if [ -n "$v" ]; then
        echo "#define RGB133_CONFIG_HAVE_MACRO_PREPARE_WORK" >> conftest.h
        return
      else
        v=$(grep -R "PREPARE_WORK" $KERNELSRCDIR/ 2>> config.log)
        if [ -n "$v" ]; then
          echo "#define RGB133_CONFIG_HAVE_MACRO_PREPARE_WORK" >> conftest.h
          return
        fi
      fi
    ;;
    alsa_snd_card_free_when_closed)
      # Do we have snd_card_free_when_closed ?
      echo >> config.log
      echo "Testing snd_card_free_when_closed..." >> config.log

      echo " 
#include <linux/kconfig.h>      
#include <sound/core.h>
void dummy(void)
{
   int retval = 0;
   struct snd_card* cardDummy = 0;
   
   retval = snd_card_free_when_closed(cardDummy);
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SND_CARD_FREE_WHEN_CLOSED" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_device_caps)
      # Do we have device_caps in struct v4l2_capability?
      echo >> config.log
      echo "Testing device_caps in struct v4l2_capability..." >> config.log
      
      echo " 
#include <linux/kconfig.h>
#include <linux/videodev2.h>      
void dummy(void)
{
   struct v4l2_capability capDummy;
   unsigned int dev_caps = 1;
   
   capDummy.device_caps = dev_caps;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_DEVICE_CAPS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_device_caps_in_vdev)
      # Do we have device_caps in struct video_device?
      echo >> config.log
      echo "Testing device_caps in struct video_device..." >> config.log
      
      echo " 
#include <linux/kconfig.h>
#include <media/v4l2-dev.h>
void dummy(void)
{
   struct video_device vdev;
   unsigned int dev_caps = 1;
   
   vdev.device_caps = dev_caps;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_DEVICE_CAPS_IN_VIDEO_DEVICE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_dev_ioctl_lock)
      # Do we have ioctl_lock in v4l2_dev?
      echo >> config.log
      echo "Testing for v4l2_dev->ioctl_lock..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-device.h>
void dummy(void)
{
  struct v4l2_device v4l2_dev;
  v4l2_dev.ioctl_lock = 0;
}
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_IOCTL_LOCK_IN_V4L2_DEV" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;  
    v4l2_fops_unlocked_ioctl)
      # Do we have unlocked_ioctl in struct v4l2_file_operations ?
      echo >> config.log
      echo "Testing unlocked_ioctl in struct v4l2_file_operations..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>
void dummy(void)
{
  struct v4l2_file_operations v4l2_fops;
  v4l2_fops.unlocked_ioctl = 0;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_UNLOCKED_IOCTL_IN_V4L2_FOPS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    read_tsc_no_arguments)
      # Do we have read_tsc_no_arguments ?
      echo >> config.log
      echo "Testing read_tsc_no_arguments()..." >> config.log

      echo "$RGB133_INC_PREFIX

#include <asm/atomic.h>
#include <asm/msr.h>
void dummy(void)
{
  unsigned long long now = rdtsc();
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_READ_TSC_NO_ARGUMENTS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    get_user_pages_6_args)
      # Do we have get_user_pages_6_args ?
      echo >> config.log
      echo "Testing get_user_pages_6_args()..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/mm.h>
void dummy(void)
{
  struct page** map = 0;
  unsigned long addr = 0;
  
  if(addr)
     get_user_pages(addr, 1, 0, 1, map, 0);
}" > conftest$$.c
     
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_GET_USER_PAGES_6_ARGUMENTS" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    page_cache_release)
      # Do we have page_cache_release ?
      echo >> config.log
      echo "Testing page_cache_release()..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/pagemap.h>
void dummy(void)
{
  struct page* page = 0;
 
  if(page)
     page_cache_release(page);
}" > conftest$$.c
    
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
     
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_PAGE_CACHE_RELEASE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    v4l2_querymenu_name_in_union)
      # Do we have name in union in struct v4l2_querymenu ?
      echo >> config.log
      echo "Testing name in union in struct v4l2_querymenu..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/videodev2.h>
void dummy(void)
{
  struct v4l2_querymenu dummy_menu;

  dummy_menu.value = 1;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_V4L2_QUERYMENU_NAME_IN_UNION" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    new_timers_api)
      # Do we have new timers API ?
      echo >> config.log
      echo "Testing new timers API..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/timer.h>
void dummy(struct timer_list* pTimer)
{
  struct timer_list timer;
  timer_setup(&timer, dummy, 0);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_NEW_TIMERS_API" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    ret_snd_pcm_lib_preallocate_pages)
      # Do we have a return value from snd_pcm_lib_preallocate_pages_for_all?
      echo >> config.log
      echo "Testing return value from snd_pcm_lib_preallocate_pages_for_all..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/pci.h>
#include <sound/pcm.h>
void dummy()
{
  struct snd_pcm pcm;
  struct pci_dev pci;
  int ret = snd_pcm_lib_preallocate_pages_for_all(&pcm, 3, &pci, 64*1024, 64*1024);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_RETURN_FROM_SND_PCM_PREALLOCATE_PAGES" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    rt_kernel_no_kernel_stack)
      # Are we RT kernel and have an undefined symbol kernel_stack ?
      echo >> config.log
      echo "Testing RT kernel and kernel_stack..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/module.h>
void dummy(void)
{
  unsigned int dummy = (unsigned int)kernel_stack;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ ! -f conftest$$.o ] ; then
        ARCH=$(uname -m | sed -e 's/i.86/i386/')
        if [ "$ARCH" != "i386" ] ; then
          KVER=$(uname -r)

          if [ "$DISTRIBUTION" = "Debian" ]; then
            PREEMPT_RT=$(cat /boot/config-$KVER | grep "PREEMPT_RT")
          elif [ "$DISTRIBUTION" = "Arch" ]; then
            echo "Detected Arch Linux!"
            PREEMPT_RT=$(zcat /proc/config.gz | grep "PREEMPT_RT")
          fi

          if [ -n "$PREEMPT_RT" ] ; then
            echo "#define RGB133_CONFIG_RT_AND_UNDEFINED_KERNEL_STACK" >> conftest.h
          fi
        fi
      else
        rm -f conftest$$.o
      fi
    ;;
    ktime_get_real_ts64)
      # Do we have ktime_get_real_ts64 for reading current time ?
      echo >> config.log
      echo "Testing for ktime_get_real_ts64..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/time64.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
void dummy(void)
{
  struct timespec64 tv;
  ktime_get_real_ts64(&tv);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_KTIME_GET_REAL_TS64" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    ktime_get_ts64)
      # Do we have ktime_get_ts64 for reading current time ?
      echo >> config.log
      echo "Testing for ktime_get_ts64..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/time64.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
void dummy(void)
{
  struct timespec64 tv;
  ktime_get_ts64(&tv);
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_KTIME_GET_TS64" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    struct_timeval)
      # Do we have struct timeval to use in kernel space?
      echo >> config.log
      echo "Testing for struct timeval in kernel space..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/time.h>
void dummy(struct timeval *tv)
{
  tv->tv_sec = 0;
  tv->tv_usec = 0;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_STRUCT_TIMEVAL" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    page_fault_return_vm_fault_t)
      # Does page fault handler return vm_fault_t ?
      echo >> config.log
      echo "Testing for page fault handler returning vm_fault_t..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/mm_types.h>
#include <linux/mm.h>

static vm_fault_t dummy_handler(struct vm_fault *vmf)
{
   return VM_FAULT_ERROR;
}

struct vm_operations_struct dummy_operations = { .fault = dummy_handler, };
" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_PAGE_FAULT_HANDLER_RETURN_VM_FAULT_T" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    vfl_type_video)
      # Do we have VFL_TYPE_VIDEO in V4L2 device node type enumeration?
      echo >> config.log
      echo "Testing for VFL_TYPE_VIDEO..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <media/v4l2-dev.h>

void dummy(void)
{
  int dummy = VFL_TYPE_VIDEO;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_VFL_TYPE_VIDEO" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    semaphore_in_mm_struct)
      # Do we have mmap_sem or mmap_lock in struct mm_struct?
      echo >> config.log
      echo "Testing for name of semaphore in struct mm_struct..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <linux/rwsem.h>      
#include <linux/mm_types.h>
void dummy(struct mm_struct *mm)
{
  struct rw_semaphore* sem = &mm->mmap_lock;
}" > conftest$$.c
      
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_MMAP_LOCK_IN_MM_STRUCT" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    snd_pcm_sgbuf_ops_page)
      # Do we have exported snd pcm get page fault callback?
      echo >> config.log
      echo "Testing for snd_pcm_sgbuf_ops_page..." >> config.log

      echo "$RGB133_INC_PREFIX
#include <sound/pcm.h>

void dummy(void)
{
  struct snd_pcm_ops dummy_ops = {
   .page    =     snd_pcm_sgbuf_ops_page,
  };
}" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.log
      $CC $CFLAGS -c conftest$$.c >> config.log 2>&1
      rm -f conftest$$.c

      if [ -f conftest$$.o ] ; then
        echo "#define RGB133_CONFIG_HAVE_SND_PCM_SGBUF_OPS_PAGE" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    *)  echo "Invalid test: $1";;
  esac
}

if [ -e "$OUTPUTDIR/rgb133config.h" ] ; then
  rm -f $OUTPUTDIR/rgb133config.h
fi

case $5 in
  test_linux_features)
    echo "Testing Linux kernel features for kernel source: $KERNELDIR"
    rm -f conftest.h
    for test in $FEATURES ; do
      feature_test $test
      rm -f conftest*.c
      rm -f conftest*.o
    done
  ;;
  *) echo "Invalid argument: $4";;
esac

if [ -f conftest.h ] ; then
  mv conftest.h $OUTPUTDIR/rgb133config.h
fi
