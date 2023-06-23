#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# vwconfig.sh
# Date: 10/12/2020
# support@datapath.co.uk
################################################################

CC="$1"
ARCH="$2"
SYSINC=`$CC -print-file-name=include 2> /dev/null`
KERNELDIR="$3"
OUTPUTDIR="$4"

BASE_CFLAGS=""
VW_INC=""
VW_FLAGS=""

echo "COMPILER:          $CC" > config.vw.log
echo "ARCH:              $ARCH" >> config.vw.log
echo "SYSTEM INCLUDE:    $SYSINC" >> config.vw.log
echo "KERNEL SOURCE:     $KERNELDIR" >> config.vw.log
echo "OUTPUT DIR:        $OUTPUTDIR" >> config.vw.log

create_cflags()
{
  BASE_CFLAGS="-Os -isystem $SYSINC \
      -Werror-implicit-function-declaration"

  VW_INC="-I./include"
  VW_FLAGS="$VW_INC"
  
  CFLAGS="$VW_FLAGS $BASE_CFLAGS"
}

create_cflags

FEATURES="v4l2_selection_api"

feature_test()
{
  case "$1" in
    v4l2_selection_api)
      # Do we have g_selection and s_selection ioctls?
      echo >> config.vw.log
      echo "Testing for VIDIOC_G_SELECTION and for VIDIOC_S_SELECTION..." >> config.vw.log

      echo "
#include <linux/videodev2.h>
void dummy(void)
{
  struct v4l2_selection s;
}
" > conftest$$.c
      echo "$CC $CFLAGS -c conftest$$.c" >> config.vw.log
      $CC $CFLAGS -c conftest$$.c >> config.vw.log 2>&1
      rm -f conftest$$.c
      
      if [ -f conftest$$.o ] ; then
        echo "#define VW_CONFIG_HAVE_SELECTION_API" >> conftest.h
        rm -f conftest$$.o
        return
      fi
    ;;
    *)  echo "Invalid test: $1";;
  esac
}

if [ -e "$OUTPUTDIR/VWConfig.h" ] ; then
  rm -f $OUTPUTDIR/VWConfig.h
fi

case $5 in
  test_features)
    echo >> config.vw.log
    echo "$0 testing for features: $FEATURES" >> config.vw.log
    echo >> config.vw.log
    rm -f conftest.h
    echo "#ifndef VWCONFIG_H_" >> conftest.h
    echo "#define VWCONFIG_H_" >> conftest.h
    echo >> conftest.h
    for test in $FEATURES ; do
      feature_test $test
      rm -f conftest*.c
      rm -f conftest*.o
    done
    echo >> conftest.h
    echo "#endif /*VWCONFIG_H_*/" >> conftest.h
  ;;
  *) echo "Invalid argument: $5";;
esac

if [ -f conftest.h ] ; then
  mv conftest.h $OUTPUTDIR/VWConfig.h
fi
