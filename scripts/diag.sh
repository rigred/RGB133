#!/bin/sh

###################################################################
# Datapath Limited Vision / VisionLC Diagnostics Script
# diag.sh
# Date: 04/10/2013 
# support@datapath.co.uk
###################################################################

LIMIT_SIZE="NO"

usage ( )
{
   echo
   echo "Usage: $0 [-s] [<archive_dir>]"
   echo
   echo "Options: -s            Limit for size."
   echo "         <archive_dir> A directory into which the output diagnostics"
   echo "                       archive will be saved."
}

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:$PATH

SCRIPTS_DIR="$( cd "$( dirname "$0" )" && pwd)"
INSTALL_DIR=`dirname $SCRIPTS_DIR`

if [ "x$INSTALL_DIR" = "x" -o ! -d $INSTALL_DIR ] ; then
   echo "Failed to determine install directory!"
   echo "Using default..."
   INSTALL_DIR="./"
fi

DIAG_DIR="$INSTALL_DIR/diag"
TMP_DIAG_DIR="/tmp/diag"
PARMS_DIR_RGB133="/sys/module/rgb133/parameters"
PARMS_DIR_RGB200="/sys/module/rgb200/parameters"


# our guard for modifying / not modifying module parameter rgb133_diagnostics_mode under /sys/...
# initialize to 1 means: do not modify
DIAG_MODE_RGB133=1
DIAG_MODE_RGB200=1


mkdir -p $TMP_DIAG_DIR > /dev/null 2>&1
if [ ! -d "$TMP_DIAG_DIR" ] ; then
   echo "Failed to create diagnostics directory: $TMP_DIAG_DIR"
   echo "Please contact support@datapath.co.uk for further assistance."
   exit
fi

echo "=============== Vision / VisionLC Diagnostics Tool ==============="
echo

if [ `id -u` -ne 0 ] ; then
   echo "Please re-run this tool as the superuser."
   exit
fi

if [ "x" != "x$1" ] ; then
   if [ "$1" = "-s" ] ; then
      LIMIT_SIZE="YES"
      if [ "x" != "x$2" ] ; then
         if [ "$2" = "-d" ] ; then
            if [ -d $PARMS_DIR_RGB133 ] ; then
               DIAG_MODE_RGB133=`cat $PARMS_DIR_RGB133/rgb133_diagnostics_mode`
               if [ $DIAG_MODE_RGB133 = 0 ] ; then   
                  sudo -i echo -n "1" > $PARMS_DIR_RGB133/rgb133_diagnostics_mode
               fi
            fi
            if [ -d $PARMS_DIR_RGB200 ] ; then
               DIAG_MODE_RGB200=`cat $PARMS_DIR_RGB200/rgb133_diagnostics_mode`
               if [ $DIAG_MODE_RGB200 = 0 ] ; then   
                  sudo -i echo -n "1" > $PARMS_DIR_RGB200/rgb133_diagnostics_mode
               fi
            fi
            if [ "x" != "x$3" ] ; then 
               if [ ! -d $3 ] ; then
                  echo "Provided archive directory \"$3\" doesn't exist."
                  usage
                  exit
               else
                  DIAG_DIR=$3
               fi
            fi
         elif [ ! -d $2 ] ; then
            echo "Provided archive directory \"$2\" doesn't exist."
            usage
            exit
         else
            DIAG_DIR=$2
         fi
      fi
   elif [ "$1" = "-d" ] ; then
      if [ -d $PARMS_DIR_RGB133 ] ; then
         DIAG_MODE_RGB133=`cat $PARMS_DIR_RGB133/rgb133_diagnostics_mode`
         if [ $DIAG_MODE_RGB133 = 0 ] ; then   
            sudo -i echo -n "1" > $PARMS_DIR_RGB133/rgb133_diagnostics_mode
         fi
      fi
      if [ -d $PARMS_DIR_RGB200 ] ; then
         DIAG_MODE_RGB200=`cat $PARMS_DIR_RGB200/rgb133_diagnostics_mode`
         if [ $DIAG_MODE_RGB200 = 0 ] ; then   
            sudo -i echo -n "1" > $PARMS_DIR_RGB200/rgb133_diagnostics_mode
         fi
      fi
      if [ "x" != "x$2" ] ; then
         if [ "$2" = "-s" ] ; then
            LIMIT_SIZE="YES"
            if [ "x" != "x$3" ] ; then
               if [ ! -d $3 ] ; then
                  echo "Provided archive directory \"$3\" doesn't exist."
                  usage
                  exit
               else
                  DIAG_DIR=$3
               fi
            fi
         elif [ ! -d $2 ] ; then
            echo "Provided archive directory \"$2\" doesn't exist."
            usage
            exit
         else
            DIAG_DIR=$2
         fi
      fi   
   else 
      if [ "x" != "x$1" ] ; then
         if [ ! -d $1 ] ; then
            echo "Provided archive directory \"$1\" doesn't exist."
            usage
            exit
         else
            DIAG_DIR=$1
         fi
      fi
   fi
fi

if [ -z "$DIAG_DIR" -o ! -d "$DIAG_DIR" ] ; then 
   echo "Problem using archive directory \"$DIAG_DIR\"."
   usage
   exit
fi

cd $DIAG_DIR > /dev/null 2> /dev/null
DIAG_DIR=`pwd`
cd - > /dev/null 2> /dev/null

if [ ! -d $DIAG_DIR ] ; then
   echo "Please re-run this script at the root of the Vision / VisionLC"
   echo "installation directory.  Or alternatively provide a location"
   echo "to save the output diagnostics archive into."
   usage
   exit
fi

DIAGFILE="diag-`date +%Y-%m-%d_%H:%M:%S`.tar.bz2"

get_sys_info ( )
{
   SYSINFO=`uname -a`
   KERNELVERSION=`uname -r`
   ARCH=`uname -m | sed -e 's/i.86/i386/'`
   DRVVER=`cat include/rgb133.h 2> /dev/null | grep RGB133_MODULE_NAME | sed -e 's/.*-//g' | sed -e 's/".*//g'`
   MODDIR=`ls -l /lib/modules/$KERNELVERSION`
   LSHAL=`lshal 2> /dev/null`
   DEVNODES=`ls -l /dev/video*`
   ALLDEVNODES=`find /dev -iname "*" -exec ls -l {} \; | grep video`
   echo "===== Vision / VisionLC System Information=====" > $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
   echo "SYSTEM INFO:       $SYSINFO" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "KERNEL VERSION:    $KERNELVERSION" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "ARCH:              $ARCH" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "DRIVER VERSION:    $DRVVER" >> $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
   echo "Modules Directory:" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "$MODDIR" >> $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
   echo "lshal:" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "$LSHAL" >> $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
   echo "V4L2 Device Nodes:" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "$DEVNODES" >> $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
   echo "Video Device Nodes:" >> $TMP_DIAG_DIR/sysinfo.txt
   echo "$ALLDEVNODES" >> $TMP_DIAG_DIR/sysinfo.txt
   echo >> $TMP_DIAG_DIR/sysinfo.txt
}

get_os_info ( )
{
   PS=`ps -ef`
   echo "===== Vision / VisionLC OS Information=====" > $TMP_DIAG_DIR/osinfo.txt
   echo >> $TMP_DIAG_DIR/osinfo.txt
   echo "PS:" >> $TMP_DIAG_DIR/osinfo.txt
   echo "$PS" >> $TMP_DIAG_DIR/osinfo.txt
}

get_sys_messages ( )
{
   if [ "$LIMIT_SIZE" = "YES" ] ; then
      MESSAGES=`find /var/log -maxdepth 1 -iname "messages" 2> /dev/null`
      SYSLOG=`find /var/log -maxdepth 1 -iname "syslog" 2> /dev/null`
      KERNLOG=`find /var/log -maxdepth 1 -iname "kern.log" 2> /dev/null`
      DMESGLOG=`find /var/log -maxdepth 1 -iname "dmesg" 2> /dev/null`
      MESSAGES1=`find /var/log -maxdepth 1 -iname "messages.1" 2> /dev/null`
      SYSLOG1=`find /var/log -maxdepth 1 -iname "syslog.1" 2> /dev/null`
      KERNLOG1=`find /var/log -maxdepth 1 -iname "kern.log.1" 2> /dev/null`
      DMESGLOG1=`find /var/log -maxdepth 1 -iname "dmesg.0" 2> /dev/null`
      FILES="$MESSAGES $SYSLOG $KERNLOG $DMESGLOG $MESSAGES1 $SYSLOG1 $KERNLOG1 $DMESGLOG1"
   else
      MESSAGES=`find /var/log -maxdepth 1 -iname "messages*" 2> /dev/null`
      SYSLOG=`find /var/log -maxdepth 1 -iname "syslog*" 2> /dev/null`
      KERNLOG=`find /var/log -maxdepth 1 -iname "kern*" 2> /dev/null`
      DMESGLOG=`find /var/log -maxdepth 1 -iname "dmesg*" 2> /dev/null`
      FILES="$MESSAGES $SYSLOG $KERNLOG $DMESGLOG"
   fi
   tar -cjf $TMP_DIAG_DIR/sys_messages.tar.bz2 $FILES > /dev/null 2>&1
}

get_sys_pcicfg ( )
{
   lspci -xxx -v > $TMP_DIAG_DIR/lspci.txt
}

get_kernel_info ( )
{
   KERNELVERSION=`uname -r`
   DOTCONFIG=`cat /boot/config-$KERNELVERSION`
   SYMBOLS=`cat /proc/kallsyms`
   echo "===== Vision / VisionLC Kernel Information =====" > $TMP_DIAG_DIR/kernelinfo.txt
   echo >> $TMP_DIAG_DIR/kernelinfo.txt
   echo "KERNEL VERSION: $KERNELVERSION" >> $TMP_DIAG_DIR/kernelinfo.txt
   echo >> $TMP_DIAG_DIR/kernelinfo.txt
   echo ".config:" >> $TMP_DIAG_DIR/kernelinfo.txt
   echo "$DOTCONFIG" >> $TMP_DIAG_DIR/kernelinfo.txt
   echo >> $TMP_DIAG_DIR/kernelinfo.txt
   echo "Symbols:" >> $TMP_DIAG_DIR/kernelinfo.txt
   echo "$SYMBOLS" >> $TMP_DIAG_DIR/kernelinfo.txt
   echo >> $TMP_DIAG_DIR/kernelinfo.txt
}

get_installed_module_info ( )
{
   MODLOC=`find /lib -iname "rgb$1.ko" 2> /dev/null`
   if [ "x" != "x$MODLOC" ] ; then
      MODINFO=`modinfo rgb$1 2>> $TMP_DIAG_DIR/driverinfo.txt`
      echo "Module Location:" >> $TMP_DIAG_DIR/driverinfo.txt
      echo "$MODLOC" >> $TMP_DIAG_DIR/driverinfo.txt
      echo >> $TMP_DIAG_DIR/driverinfo.txt
      echo "$MODINFO" >> $TMP_DIAG_DIR/driverinfo.txt
      echo >> $TMP_DIAG_DIR/driverinfo.txt
   fi
}

get_driver_info ( )
{
   if [ -e $TMP_DIAG_DIR/driverinfo.txt ] ; then
      rm -f $TMP_DIAG_DIR/driverinfo.txt > /dev/null 2>&1
   fi
   get_installed_module_info 133
   get_installed_module_info 200
}

get_install_info ( )
{
   STARTUP=`find /etc -iname "*DGC133*" 2> /dev/null`
   FIRMWARE=`find /lib/firmware -iname "*dgc*" 2> /dev/null`
   FLASHIMG=`find $INSTALL_DIR/firmware -iname "flashimg*" 2> /dev/null`
   LSMOD133=`lsmod | grep rgb133 2> /dev/null`
   LSMOD200=`lsmod | grep rgb200 2> /dev/null`
   echo "===== Vision & VisionLC Driver Information =====" > $TMP_DIAG_DIR/installinfo.txt 
   if [ "x" != "x$LSMOD133" ] ; then
      if [ -e $INSTALL_DIR/bin/Flash133 ] ; then
         FLASH133=`$INSTALL_DIR/bin/Flash133 -l`
      fi
   fi
   if [ "x" != "x$LSMOD200" ] ; then
      if [ -e $INSTALL_DIR/bin/Flash200 ] ; then
         FLASH200=`$INSTALL_DIR/bin/Flash200 -l`
      fi
   fi
   USERSVC=`find /usr/local -iname "*DGC133*" 2> /dev/null`
   SAMPAPP=`find /usr/local -iname "*Vision*" 2> /dev/null`
   DADA_MAPPINGS=`ls -l /dev | grep dada 2> /dev/null`
   echo "Firmware:" >> $TMP_DIAG_DIR/installinfo.txt
   if [ "x" != "x$FIRMWARE" ] ; then
      for fw in $FIRMWARE ; do
         echo "`md5sum $fw 2> /dev/null`" >> $TMP_DIAG_DIR/installinfo.txt
      done
   fi 
   echo >> $TMP_DIAG_DIR/installinfo.txt
   echo "Flash Image:" >> $TMP_DIAG_DIR/installinfo.txt
   if [ "x" != "x$FLASHIMG" ] ; then
      for img in $FLASHIMG ; do
         echo "`md5sum $img 2> /dev/null`" >> $TMP_DIAG_DIR/installinfo.txt
      done
   fi 
   echo >> $TMP_DIAG_DIR/installinfo.txt
   echo "Flash Info:" >> $TMP_DIAG_DIR/installinfo.txt
   if [ "x" != "x$FLASH133" ] ; then
      echo "$FLASH133" >> $TMP_DIAG_DIR/installinfo.txt
      echo >> $TMP_DIAG_DIR/installinfo.txt
   fi
   if [ "x" != "x$FLASH200" ] ; then
      echo "$FLASH200" >> $TMP_DIAG_DIR/installinfo.txt
      echo >> $TMP_DIAG_DIR/installinfo.txt
   fi
   echo "Startup Info:" >> $TMP_DIAG_DIR/installinfo.txt
   echo "$STARTUP" >> $TMP_DIAG_DIR/installinfo.txt
   echo >> $TMP_DIAG_DIR/installinfo.txt
   echo "User Service Info:" >> $TMP_DIAG_DIR/installinfo.txt
   echo "$USERSVC" >> $TMP_DIAG_DIR/installinfo.txt
   echo >> $TMP_DIAG_DIR/installinfo.txt
   echo "Vision Info:" >> $TMP_DIAG_DIR/installinfo.txt
   echo "$SAMPAPP" >> $TMP_DIAG_DIR/installinfo.txt
   echo >> $TMP_DIAG_DIR/installinfo.txt
   echo "Dada Devices Mappings Info:" >> $TMP_DIAG_DIR/installinfo.txt
   echo "$DADA_MAPPINGS" >> $TMP_DIAG_DIR/installinfo.txt
   echo >> $TMP_DIAG_DIR/installinfo.txt
}

get_module_info ( )
{
   lsmod > $TMP_DIAG_DIR/lsmod.txt
}

get_interrupt_info ( )
{
   cat /proc/interrupts > $TMP_DIAG_DIR/interrupts.txt
}

get_install_log_files ( )
{
   cp -f $INSTALL_DIR/install.log $TMP_DIAG_DIR > /dev/null 2>&1
   cp -f $INSTALL_DIR/config.log $TMP_DIAG_DIR > /dev/null 2>&1
}

get_service_log_files ( )
{
   LOCALS=`find $INSTALL_DIR/ -iname "service.log*"`
   INSTALLED=`find /usr/local/DGC133 -iname "service.log*"`
   if [ -n "$LOCALS" ] ; then
      mkdir -p $TMP_DIAG_DIR/local_service
      cp -f $LOCALS $TMP_DIAG_DIR/local_service/ > /dev/null 2>&1
   fi
   if [ -n "$INSTALLED" ] ; then
      mkdir -p $TMP_DIAG_DIR/installed_service
      cp -f $INSTALLED $TMP_DIAG_DIR/installed_service/ > /dev/null 2>&1
   fi
}

get_config_info ( )
{
   RGB133CONFIG=`find $INSTALL_DIR/ -iname "rgb133config.h"`
   if [ -n "$RGB133CONFIG" ] ; then
      mkdir -p $TMP_DIAG_DIR/config > /dev/null 2>&1
      cp $RGB133CONFIG $TMP_DIAG_DIR/config > /dev/null 2>&1
   fi
}

get_rgbhelper_info ( )
{
   VIDEONODES=`find /dev/ -iname "dada*"`
   echo "=================================================" > $TMP_DIAG_DIR/RGBHelper.txt
   if [ -n "$VIDEONODES" ] ; then
      for node in $VIDEONODES ; do
         if [ -e $INSTALL_DIR/bin/RGBHelper ] ; then
            if [ "$node" != "/dev/video63" -a "$node" != "/dev/video64" ] ; then
               echo "RGBHelper - $node - `date`" >> $TMP_DIAG_DIR/RGBHelper.txt
               $INSTALL_DIR/bin/RGBHelper -d $node -s ".*" >> $TMP_DIAG_DIR/RGBHelper.txt
               echo "-------------------------------------------------" >> $TMP_DIAG_DIR/RGBHelper.txt
               sleep 1
               echo "RGBHelper - $node - `date`" >> $TMP_DIAG_DIR/RGBHelper.txt
               $INSTALL_DIR/bin/RGBHelper -d $node -s ".*" >> $TMP_DIAG_DIR/RGBHelper.txt
               echo "=================================================" >> $TMP_DIAG_DIR/RGBHelper.txt
            fi
         else
            echo "Failed to find RGBHelper!" >> $TMP_DIAG_DIR/RGBHelper.txt
         fi 
      done
   else
      echo "Failed to find any V4L2 device nodes!" >> $TMP_DIAG_DIR/RGBHelper.txt
   fi
}

get_edid_info ( )
{
   VIDEONODES=`find /dev/ -iname "dada*"`
   echo "=================================================" > $TMP_DIAG_DIR/Edid.dump
   if [ -n "$VIDEONODES" ] ; then
      for node in $VIDEONODES ; do
         if [ -e $INSTALL_DIR/bin/Edid ] ; then
            if [ "$node" != "/dev/video63" -a "$node" != "/dev/video64" ] ; then
               echo "Edid - $node - `date`" >> $TMP_DIAG_DIR/Edid.dump
               $INSTALL_DIR/bin/Edid -rd:$node -x >> $TMP_DIAG_DIR/Edid.dump 2> /dev/null
               echo "=================================================" >> $TMP_DIAG_DIR/Edid.dump
            fi
         else
            echo "Failed to find Edid!" >> $TMP_DIAG_DIR/Edid.dump
         fi 
      done
   else
      echo "Failed to find any V4L2 device nodes!" >> $TMP_DIAG_DIR/Edid.dump
   fi
}

create_archive ( )
{
   cd $TMP_DIAG_DIR > /dev/null 2>&1
   rm -vf $DIAG_DIR/diag-*.tar.bz2
   tar -cjf /tmp/$DIAGFILE *
   mv /tmp/$DIAGFILE $DIAG_DIR
   cd - > /dev/null 2>&1
}

echo -n "Retrieving System Info..."
get_sys_info
echo "DONE"
echo -n "Retrieving OS Info..."
get_os_info
echo "DONE"
echo -n "Retrieving System Messages..."
get_sys_messages
echo "DONE"
echo -n "Retrieving PCI Configuration Info..."
get_sys_pcicfg
echo "DONE"
echo -n "Retrieving Kernel Info..."
get_kernel_info
echo "DONE"
echo -n "Retrieving Driver Info..."
get_driver_info
echo "DONE"
echo -n "Retrieving Install Info..."
get_install_info
echo "DONE"
echo -n "Retrieving Kernel Module Info..."
get_module_info
echo "DONE"
echo -n "Retrieving Interrupt Info..."
get_interrupt_info
echo "DONE"
echo -n "Retrieving Install Log Files..."
get_install_log_files
echo "DONE"
echo -n "Retrieving Service Log Files..."
get_service_log_files
echo "DONE"
echo -n "Retrieving RGBHelper Info..."
get_rgbhelper_info
echo "DONE"
echo -n "Retrieving EDID Info..."
get_edid_info
echo "DONE"
echo -n "Retrieving Config Information..."
get_config_info
echo "DONE"
echo -n "Creating Diagnostic Archive..."
create_archive
echo "DONE"

# We have set rgb133_diagnostics_mode parameter "on the fly". Now we need to clear it back.
if [ $DIAG_MODE_RGB133 = 0 ] ; then
   sudo -i echo -n "0" > $PARMS_DIR_RGB133/rgb133_diagnostics_mode
fi
if [ $DIAG_MODE_RGB200 = 0 ] ; then
   sudo -i echo -n "0" > $PARMS_DIR_RGB200/rgb133_diagnostics_mode
fi

echo
echo " Please email $DIAG_DIR/$DIAGFILE"
echo " to support@datapath.co.uk."
echo 
echo "=========== Vision / VisionLC Diagnostics Tool Finished ==========="
echo

