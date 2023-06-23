#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# install.sh
# Date: 03/10/2013 
# support@datapath.co.uk
################################################################

SILENT="NO"
USER_MODE="NO"
VERSION="NO"
DISTRIBUTION=$(lsb_release -is)

# TYPE is set to appropriate driver in release build
TYPE=133
if [ "$TYPE" != "133" -a "$TYPE" != "200" ] ; then
   echo "Invalid TYPE: $TYPE"
   echo "Invalid TYPE: $TYPE" >> install.log
   exit
else
   if [ "$TYPE" = "133" ]; then
      DRIVER_NAME=Vision
      CONTROLDEV="/dev/video63"
      VERSION="7.27.1.40643"
      SAMPLE_APP="Vision"
   else
      DRIVER_NAME=VisionLC
      CONTROLDEV="/dev/video64"
      VERSION="1.4.0.40643"
      SAMPLE_APP="VisionLC"
   fi
fi

INSTALL_RGB133_CONF="NO"

KERNELVERSION=`uname -r`
KVERSION=`echo $KERNELVERSION | cut -d . -f 1`
KPATCHLEVEL=`echo $KERNELVERSION | cut -d . -f 2`
KSUBLEVEL=`echo $KERNELVERSION | cut -d . -f 3`

ARCH=`uname -m | sed -e 's/i.86/i386/'`
if [ "$ARCH" = "i386" ] ; then
   ARCH_BITS=32
   NOT_ARCH_BITS=64
else
   ARCH_BITS=64
   NOT_ARCH_BITS=32
fi

RT=""
if [ "$ARCH_BITS" = "64" ] ; then
  KVER=$(uname -r)
  
  if [ "$DISTRIBUTION" = "Debian" ]; then
    PREEMPT_RT=$(cat /boot/config-$KVER | grep "PREEMPT_RT")
  elif [ "$DISTRIBUTION" = "Arch" ]; then
    PREEMPT_RT=$(zcat /proc/config.gz | grep "PREEMPT_RT")
  fi
  
  if [ -n "$PREEMPT_RT" ] ; then
    RT="RT."
  fi
fi


SAMPLE_DIR="apps/${SAMPLE_APP}"

print_short_version ( )
{
   echo -n "$VERSION"
   echo -n "$VERSION" >> install.log
}

print_header ( )
{
   echo
   echo >> install.log
   echo -n "Datapath Limited ${DRIVER_NAME} Linux Driver (v"
   echo -n "Datapath Limited ${DRIVER_NAME} Linux Driver (v" >> install.log
   print_short_version
   echo ")"
   echo ")" >> install.log
   echo
   echo >> install.log
}

print_sample_header ( )
{
   echo
   echo >> install.log
   echo -n "Datapath Limited ${DRIVER_NAME} Linux Sample Application (v"
   echo -n "Datapath Limited ${DRIVER_NAME} Linux Sample Application (v" >> install.log
   print_short_version
   echo ")"
   echo ")" >> install.log
   echo
   echo >> install.log
}

usage ( )
{
   print_header
   echo "Usage: $0 [-u] [-s]"
   echo
   echo "-u: Install the user mode driver."
   echo "    Default mode is to install kernel mode"
   echo "    driver when flag is not present."
   if [ "$1" = "USE_LICENCE" ] ; then
      echo "-s: Silently install, automatically accept"
      echo "    the Datapath Limited Software License."
   else
      echo "-s: Silently install."
   fi
   exit
}

while getopts "suvx" flag ; do
   case "$flag" in
      s)SILENT="YES";;
      u)USER_MODE="YES";;
      v)VERSION="YES";;
      x)usage;;
      *)usage;;
   esac
done

print_long_version ( )
{
   echo
   echo "Datapath Limited ${DRIVER_NAME} Linux Driver"
   echo "Version: $VERSION"
   echo 
}

print_license ( )
{
   echo "By installing this driver package you agree to be bound to the"
   echo "Datapath Limited Software Licence.  Full terms and conditions can be"
   echo "read in the file: `pwd`/docs/LICENCE"
   echo
   echo "If this is not acceptable please exit now and remove this"
   echo "driver installation package (`pwd`)."
   echo

   if [ "$SILENT" = "YES" ] ; then
      echo "Datapath Limited Software License has been automatically accepted."
      echo
      echo "[Press CTRL+C to cancel installation]"
      echo -n "Installation will commence in 10 seconds..."
      echo -n "\033[13D"
      COUNTDOWN="10"
      while [ $COUNTDOWN -gt 0 ] ; do
        sleep 1
        COUNTDOWN=$(($COUNTDOWN-1))
        echo -n " $COUNTDOWN"
        echo -n "\033[2D"
      done
      echo
      echo
   else
      echo "Do you wish to accept the Datapath Limited Software License and"
      echo -n "continue with the installation (Y/N) [N]: "
      read cont

      if [ "$cont" != "YES" -a "$cont" != "yes" -a \
         "$cont" != "Y" -a "$cont" != "y" ] ; then
         echo
         echo "Cancelling installation, please remove this driver installation"
         echo "package from [`pwd`]."
         exit
      fi
   fi

   echo "Commencing installation..."
   echo
}

print_sample_license ( )
{
   echo "By installing this application you agree to be bound to the"
   echo "Datapath Limited Software Licence. Full terms and conditions can be"
   echo "read in the file: `pwd`/$SAMPLE_DIR/docs/LICENCE"
   echo
   echo "If this is not acceptable please decline the licence agreement now and remove"
   echo "this application installation package (`pwd`/$SAMPLE_DIR)"
   echo "once the installer has exited."
   echo

   if [ "$SILENT" = "YES" ] ; then
      echo "Datapath Limited Software License has been automatically accepted."
      echo
      echo
      echo
   else
      echo "Do you wish to accept the Datapath Limited Software License and"
      echo -n "continue with the installation (Y/N) [N]: "
      read cont

      if [ "$cont" != "YES" -a "$cont" != "yes" -a \
         "$cont" != "Y" -a "$cont" != "y" ] ; then
         echo
         echo "Datapath Limited ${DRIVER_NAME} Linux Sample Application will not be installed."
         echo "Datapath Limited ${DRIVER_NAME} Linux Sample Application will not be installed." >> install.log
         echo "Please remove this application installation files from"
         echo "[`pwd`/$SAMPLE_DIR]."
         echo
         return 1
      fi
   fi

   echo "Commencing installation..."
   echo
}

print_help ( )
{
   echo
   echo "========================================================================="
   echo
   echo "If you are experiencing difficulty with this installation"
   echo "please contact support@datapath.co.uk."
   echo
   echo "Please include details of computer system such as motherboard"
   echo "type, processor type, RAM and the number of (and type of)"
   echo "Datapath Limited cards including positions in the system."
   echo
   echo "Please include details of Linux kernel version, and if"
   echo "appropriate, OS Distribution (Ubuntu, Fedora Core,"
   echo "Red Hat Enterprise etc)."
   echo
   echo "Please also run the provided diagnostics harvesting script"
   echo "(for more details please see the DIAG file included with this"
   echo "release)."
   echo
   echo " # cd <install_dir>"
   echo " # ./scripts/diag.sh"
   echo
   echo "========================================================================="
   echo
}

print_qt_libs_help ( )
{
   echo
   echo "========================================================================="
   echo
   echo "The ${DRIVER_NAME} Linux Sample Application has not been installed because"
   echo "the required Qt5 libraries are not installed in this system."
   echo "At the very least you will need libQtCore and libQtWidgets."
   echo
   echo "For more information please contact support@datapath.co.uk."
   echo
   echo "Note: You may just need to add the location of your Qt libraries to"
   echo "      the installation users LD_LIBRARY_PATH environment variable."
   echo
   echo "========================================================================="
   echo
}

print_v4l2_help ( )
{
   echo
   echo "========================================================================="
   echo
   echo "The ${DRIVER_NAME} sample application has not been installed because"
   echo "the required v4l2 libraries are not installed in this system."
   echo "At the very least you will need libv4l2.so and libv4lconvert.so."
   echo
   echo "For more information please contact support@datapath.co.uk."
   echo
   echo "========================================================================="
   echo
}

print_shutdown ( )
{
   echo
   echo "========================================================================="
   echo "=========================== SHUTDOWN REQUIRED ==========================="
   echo "========================================================================="
   echo "  In order to use the upgraded firmware this"
   echo "  machine must be fully powered off."
   echo
   echo -n "  Shutdown now? [Y/N] (default Y): "
   read option
   if [ "$option" = "y" -o "$option" = "Y" -o "$option" = "" ] ; then
      if [ -e /sbin/poweroff ] ; then
         /sbin/poweroff
      else
         echo "Unable to automatically poweroff, please shutdown manually."
      fi
   fi
   echo "========================================================================="
}

print_missing_tools ( )
{
   echo
   echo "Your system has one or more system tools missing which are"
   echo "required to compile and load the ${DRIVER_NAME} Linux driver."
   echo
   echo "Required packages: make, gcc, ld"
   print_help
   exit
}

print_missing_qt_tools ( )
{
   echo
   echo "Your system has one or more system tools missing which are"
   echo "required to compile and load the ${DRIVER_NAME} Linux Sample Application."
   echo
   echo "Required packages: qmake make g++"
   print_help
}

print_missing_kernel_devel ( )
{
   echo
   echo "Your system is missing one or more kernel development packages which"
   echo "is required to build and load the ${DRIVER_NAME} Linux driver."
   echo
   echo "Required packages: kernel-source, kernel-devel"
   echo
   echo "Please make sure that the correct versions of these packages are"
   echo "installed.  Versions required: `uname -r`"
   print_help
   exit
}

start_progress ( )
{
   touch .$1_lock > /dev/null 2>&1
   ./scripts/progress.sh .$1_lock &
}

stop_progress ( )
{
   rm -f .$1_lock > /dev/null 2>&1
}

test_tools ( )
{
   # Check important tools
   IMPORTANT_TOOLS="make gcc ld"
   for tool in $IMPORTANT_TOOLS ; do
      $tool --help > /dev/null 2>&1
      RET=$?
      if [ $RET -ne 0 ] ; then
         print_missing_tools
      fi
   done
}

test_qt_tools ( )
{
   # Check important tools
   IMPORTANT_QT_TOOLS="qmake make g++"
   for tool in $IMPORTANT_QT_TOOLS ; do
      $tool --help > /dev/null 2>&1
      RET=$?
      if [ $RET -ne 0 ] ; then
         echo "FAILED"
         echo "FAILED" >> install.log
         print_missing_qt_tools
         return 1
      fi
   done
}

test_qt_libs ( )
{
   # Check for Qt libraries
   TMP_DIR=tmp
   if [ -d $TMP_DIR ] ; then
      rm -vrf $TMP_DIR > /dev/null 2>&1
   fi
   mkdir -p $TMP_DIR >> install.log 2>&1
   cd $TMP_DIR >> install.log 2>&1

   echo "int main(void) { return 0; }" > test_qt.c
   qmake -project -o test_qt.pro >> ../install.log 2>&1
   qmake >> ../install.log 2>&1
   make >> ../install.log 2>&1
   RET=$?
   if [ $RET -ne 0 ] ; then
      echo "FAILED"
      echo "FAILED" >> ../install.log
      print_qt_libs_help
      cd .. >> ../install.log 2>&1
      rm -vrf $TMP_DIR > /dev/null 2>&1
      return 1
   fi
   cd .. >> ../install.log 2>&1
   rm -vrf $TMP_DIR > /dev/null 2>&1
}

sanity_check ( )
{
   # Check important packages
   IMPORTANT_PACKAGE_FILES="scripts/mod/modpost"
   IMPORTANT_PACKAGE_DIRS="include/linux/media"
   KERNERL_BASE="/lib/modules/`uname -r`"
   KERNEL_BUILD="$KERNEL_BASE/build"
   KERNEL_SOURCE="$KERNEL_BASE/source"
   
   IMPORTANT_FILES_FOUND="NO"
   for file in $IMPORTANT_PACKAGE_FILES ; do
      if [ -f $KERNEL_BUILD/$file ] ; then
         IMPORTANT_FILES_FOUND="YES"
      elif [ -f $KERNEL_SOURCE/$file ] ; then
         IMPORTANT_FILES_FOUND="YES"
      fi
   done 

   IMPORTANT_DIRS_FOUND="NO"
   for dir in $IMPORTANT_PACKAGE_DIRS ; do
      if [ -f $KERNEL_BUILD/$dir ] ; then
         IMPORTANT_DIRS_FOUND="YES"
      elif [ -f $KERNEL_SOURCE/$dir ] ; then
         IMPORTANT_DIRS_FOUND="YES"
      fi
   done 
   
   if [ "$IMPORTANT_FILES_FOUND" != "YES" -o "$IMPORTANT_DIRS_FOUND" != "YES" ] ; then
      print_missing_kernel_devel
   fi
}

install_sample_app ( )
{
   if [ "$SILENT" != "YES" ] ; then
      echo
      echo -n "Do you wish to install Datapath Limited ${DRIVER_NAME} Linux Sample Application (Y/N) [N]: "
      read cont

      if [ "$cont" != "YES" -a "$cont" != "yes" -a \
         "$cont" != "Y" -a "$cont" != "y" ] ; then
         echo
         echo >> install.log
         echo "Datapath Limited ${DRIVER_NAME} Linux Sample Application will not be installed."
         echo "Datapath Limited ${DRIVER_NAME} Linux Sample Application will not be installed." >> install.log
         echo
         echo >> install.log
         return
      fi
   fi

   print_sample_header

   SAMPLE_LIC_TEXT=`find ./$SAMPLE_DIR/docs -iname "LICENCE"`
   USE_SAMPLE_LICENCE="NO"
   if [ -n "$SAMPLE_LIC_TEXT" ] ; then
      USE_SAMPLE_LICENCE="YES"
      print_sample_license
      RET=$?
      if [ $RET -ne 0 ] ; then
         return
      fi
   fi

   echo -n "Checking for important modules..."
   echo -n "Checking for important modules..." >> install.log
   if [ "$TYPE" = "133" ]; then
      IMPORTANT_MODULES="rgb133"
   else
      IMPORTANT_MODULES="rgb200"
   fi

   for module in $IMPORTANT_MODULES ; do
     FOUND=`find . -iname "$module"`
     LOADED=`lsmod | grep $module`
     if [ -z "$LOADED" ] ; then
        echo "FAILED"
        echo "FAILED" >> install.log
        echo "$0: Driver module $module is not loaded."
        echo "Please install the missing driver module $module."
        echo "$0: Driver module $module is not loaded." >> install.log
        echo "Please install the missing driver module $module." >> install.log
        print_help
        return
     fi
   done
   echo "DONE"
   echo "DONE" >> install.log

   echo -n "Checking for important Qt tools..."
   echo -n "Checking for important Qt tools..." >> install.log
   test_qt_tools
   RET=$?
   if [ $RET -ne 0 ] ; then
      return
   fi
   echo "DONE"
   echo "DONE" >> install.log

   echo -n "Checking presence of Qt libraries..."
   echo -n "Checking presence of Qt libraries..." >> install.log
   test_qt_libs
   RET=$?
   if [ $RET -ne 0 ] ; then
      return
   fi
   echo "DONE"
   echo "DONE" >> install.log

   echo -n "Generating Makefile for sample application with qmake..."
   echo -n "Generating Makefile for sample application with qmake..." >> install.log
   cd $SAMPLE_DIR > /dev/null 2>&1
   qmake >> ../../install.log 2>&1
   RET=$?
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> ../../install.log
     echo "Failed to generate Makefile for sample application: $RET"
     echo "Failed to generate Makefile for sample application: $RET" >> ../../install.log
     print_help
     cd - > /dev/null 2>&1
     return
   fi
   echo "DONE"
   echo "DONE" >> ../../install.log

   echo -n "Building sample application..."
   echo -n "Building sample application..." >> ../../install.log
   make >> ../../install.log 2>&1
   RET=$?
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> ../../install.log
     echo "Failed to build sample application: $RET"
     echo "Failed to build sample application: $RET" >> ../../install.log
     print_help
     cd - > /dev/null 2>&1
     return
   fi
   echo "DONE"
   echo "DONE" >> ../../install.log

   # Copy exes to usr local
   echo -n "Installing sample application into /usr/local/Vision..."
   echo -n "Installing sample application into /usr/local/Vision..." >> ../../install.log
   cp -f ./${SAMPLE_APP} /usr/local/Vision/${SAMPLE_APP} > /dev/null 2>&1
   echo "DONE"
   echo "DONE" >> ../../install.log

   cd - > /dev/null 2>&1
}

copy_firmware( )
{
   FW=`find ./firmware -iname "dgc*fw*.bin"`
   if [ "x" = "x$FW" ] ; then
     echo "Failed to find any firmware images: $FW"
     echo "Failed to find any firmware images: $FW" >> install.log
     print_help
     exit
   fi
   echo -n "Copying ${DRIVER_NAME} firmware..."
   echo -n "Copying ${DRIVER_NAME} firmware..." >> install.log
   cp $FW /lib/firmware/ >> install.log 2>&1
   RET=$?
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "Failed to copy $FW into /lib/firmware : $RET"
     echo "Failed to copy $FW into /lib/firmware : $RET" >> install.log
     print_help
     exit
   fi
   echo "DONE"
   echo "DONE" >> install.log
}

build_module( )
{
   echo -n "Building module for kernel: $KVERSION.$KPATCHLEVEL.$KSUBLEVEL..."
   echo -n "Building module for kernel: $KVERSION.$KPATCHLEVEL.$KSUBLEVEL..." >> install.log
   start_progress build
   make RELEASE_BUILD=YES modules >> install.log 2>&1
   RET=$?
   stop_progress build
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "Failed to build module: $RET"
     echo "Failed to build module: $RET" >> install.log
     print_help
     exit
   fi
   echo "DONE"
   echo "DONE" >> install.log
}

install_module( )
{
   if [ -e /etc/init.d/DGC133 ] ; then
      echo "Stopping User Mode service..."
      echo "Stopping User Mode service..." >> install.log
      /etc/init.d/DGC133 stop >> install.log
   fi
   
   echo -n "Installing module..."
   echo -n "Installing module..." >> install.log
   make modules_install >> install.log 2>&1
   RET=$?
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "Failed to install module: $RET"
     echo "Failed to install module: $RET" >> install.log
     print_help
     exit
   fi
   echo "DONE"
   echo "DONE" >> install.log
}

build_module_and_install( )
{
   build_module
   install_module
}

build_user_module_and_install( )
{
   build_module "RGB133_USER_MODE=YES"
   install_module
}

load_module( )
{
   echo -n "Loading module..."
   echo -n "Loading module..." >> install.log
   start_progress load
   make DEPMOD=$1 load >> install.log 2>&1
   RET=$?
   stop_progress load
   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "Failed to load module: $RET"
     echo "Failed to load module: $RET" >> install.log
     print_help
     exit
   fi
   echo "DONE"
   echo "DONE" >> install.log
}

start_user_mode_service( )
{
   TRIES=0
   echo -n "Start User Mode Service..."
   echo -n "Start User Mode Service..." >> install.log

   start_progress ctrl
   while [ ! -c /dev/video63 -a $TRIES -ne 10 ] ; do
      sleep 1
      TRIES=$(($TRIES + 1))
   done
   stop_progress ctrl

   if [ ! -c /dev/video63 ] ; then
      echo "FAILED"
      echo "FAILED" >> install.log
      echo "${DRIVER_NAME} control device failed to initialise..."
      echo "${DRIVER_NAME} control device failed to initialise..." >> install.log
   else
      if [ -e /etc/init.d/DGC133 ] ; then
         start_progress ums
         /etc/init.d/DGC133 start >> install.log

         # Give the app init time
         sleep 20
         stop_progress ums
         echo "DONE"
         echo "DONE" >> install.log
      else
         echo "FAILED"
         echo "FAILED" >> install.log
         echo "User Mode service script missing."
         echo "User Mode service script missing." >> install.log
      fi
   fi
}

install_user_mode_service ( )
{
   echo -n "Install User Mode Service..."
   echo -n "Install User Mode Service..." >> install.log

   UPDATERCD=`which update-rc.d > /dev/null 2>&1`

   # Copy exe to usr local
   mkdir -p /usr/local/DGC133
   cp ./bin/DGC133 /usr/local/DGC133/DGC133

   # Copy startup script
   cp -vf ./scripts/DGC133 /etc/init.d/ >> install.log
   chmod uga+x /etc/init.d/DGC133

   MANUAL="NO"
   if [ -z "$UPDATERCD" ] ; then
      MANUAL="YES"
   elif [ ! -e $UPDATERCD ] ; then
      MANUAL="YES"
   fi

   if [ "$MANUAL" = "YES" ] ; then
      echo -n "Manually configuring system startup..."
      echo -n "Manually configuring system startup..." >> install.log

      # Remove existing startup entries
      find /etc -iname "S98DGC133" -exec rm -vf {} \; >> install.log
      find /etc -iname "K98DGC133" -exec rm -vf {} \; >> install.log

      # Add new startup entries
      cd /etc/rc0.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 K98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc1.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 K98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc2.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 S98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc2.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 K98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc3.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 S98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc4.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 S98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc5.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 S98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1
      cd /etc/rc6.d > /dev/null 2>&1
      ln -sf ../init.d/DGC133 K98DGC133 > /dev/null 2>&1
      cd - > /dev/null 2>&1

   else
      if [ -e /etc/init.d/DGC133 ] ; then
         # Remove existing startup entries
         $UPDATERCD -f DGC133 remove >> install.log
      fi

      # Add new startup entries
      $UPDATERCD DGC133 start 98 2 3 4 5 . stop 98 0 1 6 . >> install.log

   fi

   echo "DONE"
   echo "DONE" >> install.log
}

install_binaries ( )
{
   mkdir -p /usr/local/Vision > /dev/null 2>&1

   echo -n "Installing ForceDetect into /usr/local/Vision..."
   echo -n "Installing ForceDetect into /usr/local/Vision..." >> install.log
   cp -f ./bin/ForceDetect /usr/local/Vision/ForceDetect > /dev/null 2>&1
   echo "DONE"
   echo "DONE" >> install.log

   echo -n "Installing Edid into /usr/local/Vision..."
   echo -n "Installing Edid into /usr/local/Vision..." >> install.log
   cp -f ./bin/Edid /usr/local/Vision/Edid > /dev/null 2>&1
   echo "DONE"
   echo "DONE" >> install.log

   echo -n "Installing RGBHelper into /usr/local/Vision..."
   echo -n "Installing RGBHelper into /usr/local/Vision..." >> install.log
   cp -f ./bin/RGBHelper /usr/local/Vision/RGBHelper > /dev/null 2>&1
   echo "DONE"
   echo "DONE" >> install.log
}

start_links( )
{
   TRIES=0
   echo -n "Start Links Service control channel $CONTROLDEV..."
   echo -n "Start Links Service control channel $CONTROLDEV..." >> install.log

   start_progress ctrl
   while [ ! -c $CONTROLDEV -a $TRIES -ne 10 ] ; do
      sleep 1
      TRIES=$(($TRIES + 1))
   done
   stop_progress ctrl

   if [ ! -c $CONTROLDEV ] ; then
      echo "FAILED"
      echo "FAILED" >> install.log
      echo "${DRIVER_NAME} control device $CONTROLDEV failed to initialise..."
      echo "${DRIVER_NAME} control device $CONTROLDEV failed to initialise..." >> install.log
   else
      if [ -e /etc/systemd/system/RGB133Links.service ] ; then 
         start_progress links 
         
         # enable RGB133Links.service to start at boot time and start for this session
         echo -en "\n" >> install.log
         systemctl enable RGB133Links.service >> install.log 2>&1 
         systemctl start RGB133Links.service >> install.log 2>&1
         
         # Give the app init time
         sleep 1
         stop_progress links
         echo "DONE"
         echo "DONE" >> install.log
      elif [ -e /etc/init.d/RGB133Links ] ; then
         start_progress links
         
         # Ubuntu 15.04 uses systemd by default but because it still can use System V
         # On Ubuntu 15.04 systemd creates RGB133Links.service config files
         # (does not use RGB133Links.service which we provide in <install_dir>/configurations)
         # In such case, to avoid warning messages that the config files have changed on driver reinstall,
         # issue this command to reload systemd configuration before it will actually try to start the service
         if [ -d /etc/systemd/ ] ; then
            # We have /etc/systemd/ but do we have the command to control the service manager?
            type systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
               systemctl daemon-reload >> install.log 2>&1
            fi
         fi
         
         /etc/init.d/RGB133Links start >> install.log

         # Give the app init time
         sleep 1
         stop_progress links
         echo "DONE"
         echo "DONE" >> install.log
      else
         echo "FAILED"
         echo "FAILED" >> install.log
         echo "Links service script missing."
         echo "Links service script missing." >> install.log
      fi
   fi
}

install_links ( )
{
   echo -n "Installing RGB133Links into /usr/local/DGC133..."
   echo -n "Installing RGB133Links into /usr/local/DGC133..." >> install.log

   DIR_OUT="/etc/init.d/"
   DIR_LIB_SYSTEMD="/usr/lib/systemd/"
   SYSTEMCTLCMD=`which systemctl 2>/dev/null`

   # Copy exe to usr local
   mkdir -p /usr/local/DGC133 > /dev/null 2>&1
   cp -f ./bin/RGB133Links /usr/local/DGC133/RGB133Links > /dev/null 2>&1

   if [ -d "$DIR_LIB_SYSTEMD" ] && [ -n "$SYSTEMCTLCMD" ]; then
      # Use systemd service management
      DIR_SYSTEM="/etc/systemd/system"

      if [ -d "$DIR_SYSTEM" ]; then
         # Remove existing startup .service link and install new one
         find $DIR_SYSTEM -iname "RGB133Links.service" -exec rm -vf {} \; >> install.log
         cp -vf ./configurations/RGB133Links.service $DIR_SYSTEM/ >> install.log
         echo "DONE"
         echo "DONE" >> install.log
         echo "Using systemd service management"
         echo "Using systemd service management" >> install.log
      else
         echo "FAILED"
         echo "FAILED" >> install.log
         echo "$DIR_SYSTEM/ missing."
         echo "$DIR_SYSTEM/ missing." >> install.log
      fi
   elif [ -d "$DIR_OUT" ]; then
      UPDATERCD=`which update-rc.d 2>/dev/null`
      
      cp -vf ./scripts/RGB133Links /etc/init.d/ >> install.log
      chmod uga+x /etc/init.d/RGB133Links

      MANUAL="NO"
      if [ -z "$UPDATERCD" ] ; then
         MANUAL="YES"
      elif [ ! -e "$UPDATERCD" ] ; then
         MANUAL="YES"
      fi

      if [ "$MANUAL" = "YES" ] ; then
         echo -n "Manually configuring system startup..."
         echo -n "Manually configuring system startup..." >> install.log

         # Remove existing startup entries
         find /etc -iname "S99RGB133Links" -exec rm -vf {} \; >> install.log

         # Add new startup entries
         cd /etc/rc2.d > /dev/null 2>&1
         ln -sf ../init.d/RGB133Links S99RGB133Links > /dev/null 2>&1
         cd - > /dev/null 2>&1
         cd /etc/rc3.d > /dev/null 2>&1
         ln -sf ../init.d/RGB133Links S99RGB133Links > /dev/null 2>&1
         cd - > /dev/null 2>&1
         cd /etc/rc4.d > /dev/null 2>&1
         ln -sf ../init.d/RGB133Links S99RGB133Links > /dev/null 2>&1
         cd - > /dev/null 2>&1
         cd /etc/rc5.d > /dev/null 2>&1
         ln -sf ../init.d/RGB133Links S99RGB133Links > /dev/null 2>&1
         cd - > /dev/null 2>&1
      else
         if [ -e /etc/init.d/DGC133 ] ; then
            # Remove existing startup entries
            $UPDATERCD -f RGB133Links remove >> install.log
         fi

         # Add new startup entries
         $UPDATERCD RGB133Links start 98 2 3 4 5 . >> install.log
      fi

      echo "DONE"
      echo "DONE" >> install.log   
   else
      echo "FAILED"
      echo "FAILED" >> install.log
      echo "Unsupported service management or missing components."
      echo "Unsupported service management or missing components." >> install.log
   fi
}

install_conf ( )
{
   if [ -d /etc/modprobe.d -a -e ./scripts/rgb${TYPE}.sample.conf ] ; then
     if [ ! -e /etc/modprobe.d/rgb${TYPE}.sample.conf ] ; then
       INSTALL_RGB133_CONF="YES"
     else
       CMP=`cmp ./scripts/rgb${TYPE}.sample.conf /etc/modprobe.d/rgb${TYPE}.sample.conf`
       RET=$?
       if [ $RET -ne 0 ] ; then
         if [ "$SILENT" != "YES" ] ; then
           echo -n "Overwrite existing rgb${TYPE}.sample.conf file [y/N]: "
           echo -n "Overwrite existing rgb${TYPE}.sample.conf file [y/N]: " >> install.log
           read ow
           if [ "$ow" = "y" -o "$ow" = "Y" ] ; then
             INSTALL_RGB133_CONF="YES"
           fi
         fi
       fi
     fi
     
     if [ "$INSTALL_RGB133_CONF" = "YES" ] ; then
       echo -n "Installing rgb${TYPE}.sample.conf into /etc/modprobe.d/..."
       echo -n "Installing rgb${TYPE}.sample.conf into /etc/modprobe.d/..." >> install.log

       # Copy file
       cp -vf ./scripts/rgb${TYPE}.sample.conf /etc/modprobe.d/ >> install.log

       echo "DONE"
       echo "DONE" >> install.log
     else
       echo -n "Skipping installation of rgb${TYPE}.sample.conf [file exists]..."
       echo -n "Skipping installation of rgb${TYPE}.sample.conf [file exists]..." >> install.log
     fi
   else
     echo -n "Skipping installation of rgb${TYPE}.sample.conf..."
     echo -n "Skipping installation of rgb${TYPE}.sample.conf..." >> install.log
   fi
}

upgrade_flash ( )
{
   DEVICES="NO"
   MODIFY="NO"
   FLASHIMG=`find ./firmware -name "FLASHIMG*"`
   if [ -z "$FLASHIMG" ] ; then
      echo "Failed to find suitable flash image.  Firmware may not be usable..."
      echo "Failed to find suitable flash image.  Firmware may not be usable..." >> install.log
      echo "ls -al ./firmware" >> install.log
      ls -al ./firmware >> install.log
      print_help
      return
   fi
   
   echo -n "Checking for flashable devices, please wait..."
   echo -n "Checking for flashable devices, please wait..." >> install.log
   start_progress list

   ./bin/Flash${TYPE} -l >> install.log
   RET=$?
   stop_progress list
   if [ $RET -gt 0 ] ; then
      echo "DONE"
      echo "Flashable devices found..."
      echo "Flashable devices found..." >> install.log
      DEVICES="YES"
   elif [ $RET -eq 0 ] ; then
      echo "DONE"
      echo "No flashable devices found..."
      echo "No flashable devices found..." >> install.log
      DEVICES="NO"
   else
      echo "FAILED($RET)"
      DEVICES="NO"
   fi

   if [ "$DEVICES" = "YES" ] ; then
      echo -n "Checking current flash version(s), please wait..."
      echo -n "Checking current flash version(s), please wait..." >> install.log
      start_progress comp

      ################### TEMP ###################
      # Delay until IRQ handshake has finished.  #
      sleep 5
      ############################################

      ./bin/Flash${TYPE} -c -f $FLASHIMG >> install.log
      RET=$?
      stop_progress comp
      if [ $RET -eq 254 ] ; then
         echo "DONE"
         echo -n "Flash requires downgrading, please wait..."
         echo -n "Flash requires downgrading, please wait..." >> install.log
         MODIFY="YES"
      elif [ $RET -eq 255 ] ; then
         echo "DONE"
         echo -n "Flash requires upgrading, please wait..."
         echo -n "Flash requires upgrading, please wait..." >> install.log
         MODIFY="YES"
      elif [ $RET -eq 0 ] ; then
         echo "DONE"
         echo "Flash is up to date..."
         echo "Flash is up to date..." >> install.log
      else
         echo "FAILED($RET)"
      fi

      if [ "$MODIFY" = "YES" ] ; then
         start_progress write
         ./bin/Flash${TYPE} -w -f $FLASHIMG >> install.log
         stop_progress write
         echo "DONE"
         echo "DONE" >> install.log
         print_shutdown
      fi
   fi
}

cleanup_arch_files ( )
{
   FILES=`find ./bin -name "*.$NOT_ARCH_BITS*"`
   rm -vf $FILES >> install.log
   if [ -e ./bin/rgb${TYPE}.$RT$ARCH_BITS.a ] ; then
      # Extensions of the objects need resolving to .o or the kernel builds will not link them (as of kernel 4.20.0).
      mv ./bin/rgb${TYPE}.$RT$ARCH_BITS.a ./bin/rgb${TYPE}.o
      # Build optimisation switched on.
      # Although, the object is not being compiled, the overall compilation fails
      # on non-existing .cmd files in kernels >= v5.8.
      touch ./bin/.rgb${TYPE}.o.cmd >> install.log
      if [ -e ./bin/rgb${TYPE}.$ARCH_BITS.a ]  ; then
         rm -vf ./bin/rgb${TYPE}.$ARCH_BITS.a >> install.log
      elif [ -e ./bin/rgb${TYPE}.RT.$ARCH_BITS.a ] ; then
         rm -vf ./bin/rgb${TYPE}.RT.$ARCH_BITS.a >> install.log
      fi
   fi
   if [ -e ./bin/dgc${TYPE}sys.$RT$ARCH_BITS.a ] ; then
      # Extensions of the objects need resolving to .o or the kernel builds will not link them (as of kernel 4.20.0).
      mv ./bin/dgc${TYPE}sys.$RT$ARCH_BITS.a ./bin/dgc${TYPE}sys.o
      # Build optimisation switched on.
      # Although, the object is not being compiled, the overall compilation fails
      # on non-existing .cmd files in kernels >= v5.8.
      touch ./bin/.dgc${TYPE}sys.o.cmd >> install.log
      if [ -e ./bin/dgc${TYPE}sys.$ARCH_BITS.a ]  ; then
         rm -vf ./bin/dgc${TYPE}sys.$ARCH_BITS.a >> install.log
      elif [ -e ./bin/dgc${TYPE}sys.RT.$ARCH_BITS.a ] ; then
         rm -vf ./bin/dgc${TYPE}sys.RT.$ARCH_BITS.a >> install.log
      fi
   fi
   if [ -e ./bin/DGC${TYPE}.$ARCH_BITS.static ] ; then
      mv ./bin/DGC${TYPE}.$ARCH_BITS.static ./bin/DGC${TYPE}
   fi
   if [ -e ./bin/Flash${TYPE}.$ARCH_BITS ] ; then
      mv ./bin/Flash${TYPE}.$ARCH_BITS ./bin/Flash${TYPE}
   fi
   if [ -e ./bin/RGB133Links.$ARCH_BITS ] ; then
      mv ./bin/RGB133Links.$ARCH_BITS ./bin/RGB133Links
   fi
   if [ -e ./bin/ForceDetect.$ARCH_BITS ] ; then
      mv ./bin/ForceDetect.$ARCH_BITS ./bin/ForceDetect
   fi
   if [ -e ./bin/Edid.$ARCH_BITS ] ; then
      mv ./bin/Edid.$ARCH_BITS ./bin/Edid
   fi
   if [ -e ./bin/RGBHelper.$ARCH_BITS ] ; then
      mv ./bin/RGBHelper.$ARCH_BITS ./bin/RGBHelper
   fi
   if [ -e ./bin/RGB133Debug.$ARCH_BITS ] ; then
      mv ./bin/RGB133Debug.$ARCH_BITS ./bin/RGB133Debug
   fi
}

## Script start
if [ "$VERSION" = "YES" ] ; then
   print_long_version
   exit
fi

echo "${DRIVER_NAME} Linux Driver Installer..."
echo "${DRIVER_NAME} Linux Driver Installer..." > install.log

echo -n "Checking for previous installation..."
echo -n "Checking for previous installation..." >> install.log

KERNEL_STR=`uname -r`
THIS_MODULE=`find /lib/modules/$KERNEL_STR -iname "rgb${TYPE}.ko*"`
Vision_DIR=`find /usr/local/ -iname "Vision"`

# If we are installing rgb133, we need to check if rgb200 is installed... and reversed
if [ "$TYPE" = "133" ]; then
   # Other module to look for
   OTHER_MODULE=`find /lib/modules/$KERNEL_STR -iname "rgb200.ko*"`
else
   OTHER_MODULE=`find /lib/modules/$KERNEL_STR -iname "rgb133.ko*"`
fi

FOUND="NO"

# We have a candidate to uninstall if this module is already installed or
# if Vision_DIR exists on its own (no other module installed)
if [ -n "$THIS_MODULE" ] ; then
   FOUND="YES"
fi
if [ -z "$OTHER_MODULE" -a -n "$Vision_DIR" ] ; then
   FOUND="YES"   
fi

if [ "$FOUND" = "YES" ]; then
   if [ "$SILENT" != "YES" ] ; then
      echo -n "Do you wish to remove the previous installation (Y/N) [N]: "
      read cont

      if [ "$cont" != "YES" -a "$cont" != "yes" -a \
         "$cont" != "Y" -a "$cont" != "y" ] ; then
         echo
         echo "Cancelling installation, previous installation unmodified."
         exit
      fi
   fi

   # Remove previous installation
   ./scripts/uninstall.sh -s
else
   echo "NONE"
   echo "NONE" >> install.log
fi

print_header

if [ `id -u` -ne 0 ] ; then
   echo "This installer needs to be run as the superuser"
   echo "This installer needs to be run as the superuser" >> install.log
   exit
fi

LIC_TEXT=`find ./docs -iname "LICENCE"`
USE_LICENCE="NO"
if [ -n "$LIC_TEXT" ] ; then
   USE_LICENCE="YES"
   print_license
fi

echo "Beginning install, please wait..."
echo "Beginning install, please wait..." >> install.log

# Check for important files
if [ "$TYPE" = "133" ]; then
   IMPORTANT="DGC133FW.BIN"
else
   IMPORTANT="DGC200FW.BIN"
fi

echo -n "Check for important files..."
echo -n "Check for important files..." >> install.log
for file in $IMPORTANT ; do
  FOUND=`find . -iname "$file"`
  if [ -z "$FOUND" ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "$0: $file is missing, please re-unpack the installation"
     echo "$0: $file is missing, please re-unpack the installation" >> install.log
     echo "    package and try again."
     echo "    package and try again." >> install.log
     print_help
     exit
  fi
done

echo "DONE"
echo "DONE" >> install.log

echo -n "Check for important tools..."
echo -n "Check for important tools..." >> install.log
test_tools

echo "DONE"
echo "DONE" >> install.log

# Look for depmod, start with the most common locations...
SEARCH_DIRS="/sbin/ /bin/ /usr/bin/ /usr/sbin/"
DEPMOD=""
for dir in $SEARCH_DIRS ; do
   DEPMOD=`find $dir -name "depmod"`
   if [ -n "$DEPMOD" ] ; then
      # we found depmod; do not search further
      break
   fi
done

if [ -z "$DEPMOD" ] ; then
   echo "FAILED"
   echo "FAILED" >> install.log
   echo "$0: Failed to find command: depmod"
   echo "$0: Failed to find command: depmod"  >> install.log
   print_help
fi

# Cleanup unnecessary files
echo -n "Performing pre-build cleanup..."
echo "Performing pre-build cleanup..." >> install.log
cleanup_arch_files

echo "DONE"
echo "DONE" >> install.log

# Build
MAKEOPTS=
if [ $KVERSION -gt 1 ] ; then
   if [ $KVERSION -eq 2 ] ; then
      if [ $KPATCHLEVEL -ne 6 ] ; then
         echo "Unsupported kernel patch level: $KERNELVERSION"
         echo "Unsupported kernel patch level: $KERNELVERSION" >> install.log
         print_help
         exit
      fi
   fi

   copy_firmware
   if [ "$USER_MODE" = "YES" ] ; then
      build_user_module_and_install
   else
      build_module_and_install
   fi
   load_module $DEPMOD
   if [ "$USER_MODE" = "YES" ] ; then
      install_user_mode_service
      start_user_mode_service
   fi

   install_binaries
   install_sample_app
   install_links
   install_conf
   start_links
   upgrade_flash
else
   echo "Unsupported kernel version: $KERNELVERSION"
   echo "Unsupported kernel version: $KERNELVERSION" >> install.log
   print_help
   exit
fi

echo
echo >> install.log
echo "========================================================================="
echo "=========================================================================" >> install.log
echo "== ${DRIVER_NAME} Linux Driver Installer Complete!"
echo "== ${DRIVER_NAME} Linux Driver Installer Complete!" >> install.log
echo "========================================================================="
echo "=========================================================================" >> install.log

