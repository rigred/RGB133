#!/bin/sh

################################################################
# Datapath Limited Vision / VisionLC Linux Driver
# progress.sh
# Date: 19/04/2013 
# support@datapath.co.uk
################################################################

LOCK=$1

if [ -z "$LOCK" ] ; then
  exit
fi

if [ -e $LOCK ] ; then
  while [ -e $LOCK ] ; do
    echo -n "."
    sleep 1
  done
fi

