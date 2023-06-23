#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# load_rgb133.sh
# Date: 19/04/2013 
# support@datapath.co.uk
################################################################

FORCE="NO"
UNLOAD="YES"

# TYPE is set to appropriate driver in release build
TYPE=133
if [ "$TYPE" != "133" -a "$TYPE" != "200" ] ; then
   echo "Invalid TYPE: $TYPE"
   echo "Invalid TYPE: $TYPE" >> install.log
   exit
fi

PULSE_WAS_RUNNING=0
PULSE_BACKUP_CONFIG=0
PULSE_CONFIG_DIR=0

# Look for modprobe, start with the most common locations...
SEARCH_DIRS="/sbin/ /bin/ /usr/sbin/ /usr/bin/"
EXTDIR=""
INSMOD=""
for dir in $SEARCH_DIRS ; do
   INSMOD=`find $dir -name "insmod"`
   if [ -n $INSMOD ] ; then
      export PATH=$PATH:$dir
      EXTDIR="$dir"
      break
   fi
done
MODPROBE=""
for dir in $SEARCH_DIRS ; do
   MODPROBE=`find $dir -name "modprobe"`
   if [ -n $MODPROBE ] ; then
      if [ "$dir" != "$EXTDIR" ] ; then
        export PATH=$PATH:$dir
      fi
      break
   fi
done

if [ -z $INSMOD ] ; then
   echo "Failed to find command: insmod"
   exit
fi
if [ -z $MODPROBE ] ; then
   echo "Failed to find command: modprobe"
   exit
fi

usage ( )
{
   echo "Usage: $0 < -l | -u | -r >"
   echo "  -l: Load module"
   echo "  -u: Unload module"
   echo "  -r: Re-load module"
   exit 1
}

# this function checks if there are running pulseaudio services; if there are, it kills them;
# function modifies variables: PULSE_WAS_RUNNING, PULSE_BACKUP_CONFIG, and PULSE_CONFIG_DIR
# ...which are then used by mirror function check_and_start_pulseaudio()
check_and_stop_pulseaudio ( )
{
   # check if pulseaudio is running
   if ps aux | grep "pulseaudio" | grep -v grep > /dev/null; then
      PULSE_WAS_RUNNING=1
      # check if local pulseaudio config directory exists
      if [ ! -d "$HOME/.pulse" -a ! -h "$HOME/.pulse" ]; then
         PULSE_CONFIG_DIR="/etc/pulse"
      else
         PULSE_CONFIG_DIR="$HOME/.pulse"
      fi  
      # check if local client.config exists in $PULSE_CONFIG_DIR; if yes, make backup
      if [ -f "$PULSE_CONFIG_DIR/client.conf" -o -h "$PULSE_CONFIG_DIR/client.conf" ]; then    
         cp $PULSE_CONFIG_DIR/client.conf $PULSE_CONFIG_DIR/client_backup.conf
         PULSE_BACKUP_CONFIG=1
         rm $PULSE_CONFIG_DIR/client.conf
      fi
      echo "autospawn = no" > $PULSE_CONFIG_DIR/client.conf
      # kill running pulseaudio processes; ask politely by sending SIGTERM
      killall pulseaudio
      sleep 2
   fi
}

# this is a mirror function to check_and_stop_pulseaudio()
# it cleans up after check_and_stop_pulseaudio and relaunches pulseaudio service if it was killed by that function
check_and_start_pulseaudio ( )
{
   if [ $PULSE_WAS_RUNNING = 1 ] ; then
      # remove our custom config client.conf
      rm $PULSE_CONFIG_DIR/client.conf
      if [ $PULSE_BACKUP_CONFIG = 1 ] ; then
         # config file had existed before and we made backup, so now let us restore it
         cp $PULSE_CONFIG_DIR/client_backup.conf $PULSE_CONFIG_DIR/client.conf
         rm $PULSE_CONFIG_DIR/client_backup.conf   
      fi
      # last, we closed PulseAudio so now let us start it back 
      # while script is always run as root, following command is supposed to be run as normal user 
      su -c "pulseaudio --daemonize" $SUDO_USER    
   fi
}

load ( )
{
   echo -n "Checking to see if rgb${TYPE} module is loaded..."
   LOADED=`lsmod | grep rgb${TYPE}`
   if [ -n "$LOADED" ] ; then
      echo "LOADED"
      echo "Failed to load rgb${TYPE} module - module already loaded."
      exit 2
   else
      echo "NOT LOADED"
   fi
   echo -n "Loading rgb${TYPE} module..."
   echo "${SUDO} modprobe rgb${TYPE}"
   ${SUDO} $MODPROBE rgb${TYPE}
   RET=$?
   if [ $RET -ne 0 ] ; then
      echo "FAILED"
      echo -n "Loading unsupported rgb${TYPE} module..."
      echo "${SUDO} modprobe --allow-unsupported rgb${TYPE}"
      ${SUDO} $MODPROBE --allow-unsupported rgb${TYPE}
      RET=$?
      if [ $RET -ne 0 ] ; then
         echo "FAILED"
         echo "Failed to load rgb${TYPE} module - $RET"
         exit 3
      else
         echo "LOADED"
         # Add unsupported option to modprobe.d for this rgb${TYPE} module
         ADD_UNSUPP="NO"
         if [ -e /etc/modprobe.d/rgb${TYPE}.conf ] ; then
            UNSUPPORTED=`cat /etc/modprobe.d/rgb${TYPE}.conf | grep "\-\-allow\-unsupported\-modules rgb${TYPE}"`
            if [ -z $UNSUPPORTED ] ; then
              ADD_UNSUPP="YES"
            fi
         else
            ADD_UNSUPP="YES"
         fi
         if [ "$ADD_UNSUPP" = "YES" ] ; then
            echo -n "Adding unsupported modules load option to /etc/modprobe.d/rgb${TYPE}.conf"
            ${SUDO} echo "install rgb${TYPE} /sbin/modprobe --ignore-install --allow-unsupported-modules rgb${TYPE}" >> /etc/modprobe.d/rgb${TYPE}.conf
            echo "DONE"
         fi
      fi
   else
      echo "LOADED"
   fi
}

unload ( )
{
   echo -n "Checking to see if rgb${TYPE} module is loaded..."
   LOADED=`lsmod | grep rgb${TYPE}`
   if [ -z "$LOADED" ] ; then
      echo -n "NOT LOADED"
      if [ "$FORCE" != "YES" ] ; then
         echo
         echo "Failed to unload rgb${TYPE} module - module is not loaded."
         exit 4
      else
         echo " - FORCE"
         UNLOAD="NO"
      fi
   else
      echo "LOADED"
   fi
   if [ "$UNLOAD" = "YES" ] ; then
      echo -n "Unloading rgb${TYPE} module..."
      check_and_stop_pulseaudio
      ${SUDO} modprobe -r rgb${TYPE}
      RET=$?
      if [ $RET -ne 0 ] ; then
         check_and_start_pulseaudio
         echo "FAILED"
         echo "Failed to unload rgb${TYPE} module - $RET"
         exit 5
      else
         check_and_start_pulseaudio
         echo "UNLOADED"
      fi
   fi   
}

reload ( )
{
   echo "Reloading rgb${TYPE} module..."
   unload
   load
}

if [ $# -lt 1 ] ; then
   echo "Bad argument to $0"
   usage
fi

while getopts "flru" opt ; do
   case $opt in
      f) FORCE="YES";;
      l) load;;
      r) reload;;
      u) unload;;
      *) usage;;
    esac
done
