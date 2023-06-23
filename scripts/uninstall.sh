#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# uninstall.sh
# Date: 03/10/2013 
# support@datapath.co.uk
################################################################

# TYPE is set to appropriate driver in release build
TYPE=133
if [ "$TYPE" != "133" -a "$TYPE" != "200" ] ; then
   echo "Invalid TYPE: $TYPE"
   echo "Invalid TYPE: $TYPE" >> install.log
   exit
else
   if [ "$TYPE" = "133" ]; then
      DRIVER_NAME=Vision
      SAMPLE_APP="Vision"
   else
      DRIVER_NAME=VisionLC
      SAMPLE_APP="VisionLC"
   fi
fi

SILENT="NO"
KVER=`uname -r`

if [ "$TYPE" = "133" ]; then
   OTHER_MODULE=`find /lib/modules/$KVER -iname "rgb200.ko*"`
else
   OTHER_MODULE=`find /lib/modules/$KVER -iname "rgb133.ko*"`
fi

while getopts "s" flag ; do
   case "$flag" in
      s)SILENT="YES";;
   esac
done
                                    
if [ `id -u` -ne 0 ] ; then
   echo "This uninstaller needs to be run as the superuser"
   echo "This uninstaller needs to be run as the superuser" >> install.log
   exit
fi

usage ( )
{
   echo "Usage: $0 [-s]"
   echo
   echo "-s: Silently uninstall."
   exit
}

print_help ( )
{
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
}

start_progress ( )
{
   touch .$1_lock 2> /dev/null
   ./scripts/progress.sh .$1_lock &
}

stop_progress ( )
{
   rm -f .$1_lock 2> /dev/null
}

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

if [ -z $DEPMOD ] ; then
   echo "FAILED"
   echo "FAILED" >> install.log
   echo "$0: Failed to find command: depmod"
   echo "$0: Failed to find command: depmod"  >> install.log
   print_help
fi

if [ "$TYPE" = "133" ]; then
   # Vision firmwares
   FW="DGC133FW25.BIN dgc133fw.bin DGC133FW.BIN dgc153fw.bin DGC153FW.BIN dgc154fw.bin DGC154FW.BIN dgc159fw.bin DGC159FW.BIN DGC179FW.BIN DGC182FW.BIN DGC184FW.BIN DGC214FW.BIN DGC224FW.BIN"
else
   # VisionLC firmwares
   FW="DGC186FW.BIN dgc199fw.bin DGC199FW.BIN dgc200fw.bin DGC200FW.BIN DGC204FW.BIN DGC205FW.BIN"
fi

remove_firmware ( )
{
   echo -n "Removing firmware..."
   echo -n "Removing firmware..." >> install.log
   
   for file in $FW ; do
      if [ -f /lib/firmware/$file ]; then
         rm -vf /lib/firmware/$file >> install.log 2>&1
         RET=$?
         if [ $RET -ne 0 ] ; then
            echo "FAILED"
            echo "FAILED" >> install.log
            echo "Failed to remove $file from /lib/firmware : $RET"
            echo "Failed to remove $file from /lib/firmware : $RET" >> install.log
            print_help
            exit
         fi
      fi
   done
   
   echo "DONE"
   echo "DONE" >> install.log
}

stop_user_mode_service ( )
{
   # User Mode no longer supported
   if [ "$TYPE" = "133" ]; then
      if [ -e /etc/init.d/DGC133 ] ; then
         echo -n "Stopping User Mode service..."
         echo -n "Stopping User Mode service..." >> install.log
         start_progress stop_ums
         /etc/init.d/DGC133 stop >> install.log
         stop_progress stop_ums
         echo "DONE"
         echo "DONE" >> install.log
      fi
   fi
}

remove_driver ( )
{
   echo -n "Unloading driver ${DRIVER_NAME}..."
   echo -n "Unloading driver ${DRIVER_NAME}..." >> install.log
   ./scripts/load_rgb133.sh -u >> install.log
   echo "DONE"

   DRIVER=`find /lib/modules/$KVER -iname "rgb${TYPE}.ko*"`
   echo -n "Removing driver ${DRIVER_NAME}..."
   echo -n "Removing driver ${DRIVER_NAME}..." >> install.log
   rm -vf $DRIVER >> install.log 2>&1
   RET=$?

   if [ $RET -ne 0 ] ; then
     echo "FAILED"
     echo "FAILED" >> install.log
     echo "Failed to remove rgb${TYPE}.ko : $RET"
     echo "Failed to remove rgb${TYPE}.ko : $RET" >> install.log
     print_help
     exit
   fi
   echo "DONE"
   echo "DONE" >> install.log

   if [ -n $DEPMOD ] ; then
      echo -n "Re-run $DEPMOD..."
      echo -n "Re-run $DEPMOD..." >> install.log
      start_progress depmod
      $DEPMOD -a >> install.log 2>&1
      stop_progress depmod
      echo "DONE"
      echo "DONE" >> install.log
   else
      echo "Please run the depmod command manually after this script exits..."
      echo "Please run the depmod command manually after this script exits..." >> install.log
   fi
}

remove_user_mode_service ( )
{
   # User Mode no longer supported
   if [ "$TYPE" = "133" ]; then
      if [ -e /usr/local/DGC133/DGC133 ] ; then
         echo -n "Removing User Mode service /usr/local/DGC133/DGC133..."
         echo -n "Removing User Mode service /usr/local/DGC133/DGC133..." >> install.log

         rm -vf /usr/local/DGC133/DGC133 >> install.log

         UPDATERCD=`which update-rc.d 2> /dev/null`
         if [ -z "$UPDATERCD" -o ! -e "$UPDATERCD" ] ; then
            # Remove existing startup entries
            find /etc -iname "S98DGC133" -exec rm -vf {} \; >> install.log
            find /etc -iname "K98DGC133" -exec rm -vf {} \; >> install.log
            rm -vf /etc/init.d/DGC133 >> install.log
         else
            if [ -e /etc/init.d/DGC133 ] ; then
               # Remove existing startup entries
               $UPDATERCD -f DGC133 remove >> install.log
               rm -vf /etc/init.d/DGC133 >> install.log
            fi
         fi

         echo "DONE"
         echo "DONE" >> install.log
      fi
   fi
}

remove_links ( )
{
   # Remove only if no other modules are installed, which are using Links
   if [ -z "$OTHER_MODULE" ] ; then
      if [ -e /usr/local/DGC133/RGB133Links ] ; then
         echo -n "Removing Links service /usr/local/DGC133/RGB133Links..."
         echo -n "Removing Links service /usr/local/DGC133/RGB133Links..." >> install.log

         rm -vf /usr/local/DGC133/RGB133Links >> install.log
         echo "DONE"
         echo "DONE" >> install.log

         if [ -d "/usr/lib/systemd/" ]; then   
            if [ -e /etc/systemd/system/RGB133Links.service ] ; then
               # Stop RGB133Links.service, disable starting it at boot time and remove existing startup entries for systemd
               echo -n "Removing Links service unit and symbolic links..."
               echo -n "Removing Links service unit and symbolic links..." >> install.log      
               systemctl disable RGB133Links.service >> install.log 2>&1
               systemctl stop RGB133Links.service >> install.log 2>&1
               rm -vf /etc/systemd/system/RGB133Links.service >> install.log
               echo "DONE"
               echo "DONE" >> install.log
            fi
         fi 
    
         if [ -d "/etc/init.d/" ]; then
            echo -n "Removing existing startup entries from /etc/init.d/..."
            echo -n "Removing existing startup entries from /etc/init.d/..." >> install.log

            UPDATERCD=`which update-rc.d 2> /dev/null`
            if [ -z "$UPDATERCD" -o ! -e "$UPDATERCD" ] ; then
               # Remove existing startup entries
               find /etc -iname "S99DGC133Links" -exec rm -vf {} \; >> install.log
               rm -vf /etc/init.d/RGB133Links >> install.log
            else
               if [ -e /etc/init.d/RGB133Links ] ; then
                  # Remove existing startup entries
                  $UPDATERCD -f RGB133Links remove >> install.log
                  rm -vf /etc/init.d/RGB133Links >> install.log
               fi
            fi
            echo "DONE"
            echo "DONE" >> install.log
         fi
      fi
      if [ -h /dev/dada0 ] ; then
         echo -n "Removing ordered symbolic links /dev/dada*..."
         echo -n "Removing ordered symbolic links /dev/dada*..." >> install.log

         rm -vf /dev/dada* >> install.log
         echo "DONE"
         echo "DONE" >> install.log
      fi
   fi
}

remove_local_dgc133dir ( )
{
   # Remove only if no other modules are installed
   if [ -z "$OTHER_MODULE" ] ; then
      if [ -d /usr/local/DGC133 ] ; then
         echo -n "Removing Local DGC133 directory /usr/local/DGC133..."
         echo -n "Removing Local DGC133 directory /usr/local/DGC133..." >> install.log
         rm -vrf /usr/local/DGC133 >> install.log
         echo "DONE"
         echo "DONE" >> install.log
      fi
   fi
}

remove_Vision ( )
{
   if [ -e /usr/local/Vision/${SAMPLE_APP} ] ; then
      echo -n "Removing ${DRIVER_NAME} /usr/local/Vision/${SAMPLE_APP}..."
      echo -n "Removing ${DRIVER_NAME} /usr/local/Vision/${SAMPLE_APP}..." >> install.log

      rm -vf /usr/local/Vision/${SAMPLE_APP} >> install.log

      echo "DONE"
      echo "DONE" >> install.log
   fi

   # Remove only if no other modules are installed
   if [ -z "$OTHER_MODULE" ] ; then
      if [ -e /usr/local/Vision/ ] ; then
         echo -n "Removing ${DRIVER_NAME} /usr/local/Vision/..."
         echo -n "Removing ${DRIVER_NAME} /usr/local/Vision/..." >> install.log

         rm -vrf /usr/local/Vision >> install.log

         echo "DONE"
         echo "DONE" >> install.log
      fi
   fi
}

remove_rgb133_conf ( )
{
   if [ -e /etc/modprobe.d/rgb${TYPE}.sample.conf ] ; then
      echo -n "Removing rgb${TYPE}.sample.conf..."
      echo -n "Removing rgb${TYPE}.sample.conf..." >> install.log
      rm -vrf /etc/modprobe.d/rgb${TYPE}.sample.conf >> install.log
      echo "DONE"
      echo "DONE" >> install.log
   fi
   
   REMOVE_RGB133_CONF="NO"
   if [ -e /etc/modprobe.d/rgb${TYPE}.conf ] ; then
     if [ "$SILENT" != "YES" ] ; then
       echo -n "File rgb${TYPE}.conf exists, remove? [y/N]: "
       read option
       if [ "$option" = "y" -o "$option" = "Y" ] ; then
         REMOVE_RGB133_CONF="YES"
       fi
     else
       echo "File rgb${TYPE}.conf exists, please manually remove..."
     fi

     if [ "$REMOVE_RGB133_CONF" = "YES" ] ; then
       echo -n "Removing rgb${TYPE}.conf..."
       echo -n "Removing rgb${TYPE}.conf..." >> install.log
       rm -vrf /etc/modprobe.d/rgb${TYPE}.conf >> install.log
       echo "DONE"
       echo "DONE" >> install.log
     fi
   fi
}

stop_user_mode_service
remove_driver
remove_firmware
remove_user_mode_service
remove_links
remove_local_dgc133dir
remove_Vision
remove_rgb133_conf

echo "Successfully removed ${DRIVER_NAME} Linux driver"
echo "Successfully removed ${DRIVER_NAME} Linux driver" >> install.log


