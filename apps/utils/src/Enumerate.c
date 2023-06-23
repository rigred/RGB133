/*
 * Enumerate.c
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver device
 * enumeration application sample source code.
 *
 * Purpose: Implements the enumeration application functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <fcntl.h>

#include "Enumerate.h"

#include "rgb133vdif.h"


const char* controlDevices[][2] = {
      { "/dev/video64", "VisionLC" },
      { "/dev/video63", "Vision" }
};

/**
 * Implementation of readCtrlSystemInfo
 **/
int readCtrlSystemInfo(int fd, sVWSystemInfo* pSysInfo)
{
   int bytes_read = 0;

   memset(pSysInfo, 0, sizeof(sVWSystemInfo));
   pSysInfo->magic = VW_MAGIC_SYSTEM_INFO;

   bytes_read = read(fd, pSysInfo, sizeof(sVWSystemInfo));
   if (bytes_read <= 0)
   {
      printf("readCtrlSystemInfo: Failed to read %d bytes of control info into buffer(%p)\n",
            sizeof(sVWSystemInfo), pSysInfo);
      return -1;
   }

   return 0;
}

/**
 * Implementation of readDeviceInfo
 **/
int readDeviceInfo(int fd, sVWDeviceInfo* pDeviceInfo)
{
   int bytes_read = 0;

   pDeviceInfo->magic = VW_MAGIC_DEVICE_INFO;

   bytes_read = read(fd, pDeviceInfo, sizeof(sVWDeviceInfo));
   if (bytes_read <= 0)
   {
      printf("readDeviceInfo: Failed to read %d bytes of device[%d] info into buffer(%p)\n",
            sizeof(sVWDeviceInfo), pDeviceInfo->device, pDeviceInfo);
      return -1;
   }

   return 0;
}

/**
 * Implementation of readDeviceParms
 **/
int readDeviceParms(int fd, sVWDevice* pDevice)
{
   int bytes_read = 0;

   pDevice->magic = VW_MAGIC_DEVICE;

   bytes_read = read(fd, pDevice, sizeof(sVWDevice));
   if (bytes_read <= 0)
   {
      printf("readDeviceParms: Failed to read %d bytes of device[%d] info into buffer(%p)\n",
            sizeof(sVWDevice), pDevice->device, pDevice);
      return -1;
   }

   return 0;
}

/**
 * Implementation of GetSignalState
 **/
const char* GetSignalState(unsigned long flags)
{

   if(flags & RGB133_VDIF_FLAG_DVI)
   {
      if(flags & RGB133_VDIF_FLAG_INTERLACED)
         return "Interlaced DVI";
      else
         return "DVI";

   }
   else if(flags & RGB133_VDIF_FLAG_VIDEO)
   {
      if(flags & RGB133_VDIF_FLAG_INTERLACED)
         return "Interlaced VideoInput";
      else
         return "VideoInput";
   }
   else if(flags & RGB133_VDIF_FLAG_SDI)
   {
      if(flags & RGB133_VDIF_FLAG_INTERLACED)
         return "Interlaced SDI";
      else
         return "SDI";
   }
   else if(flags & RGB133_VDIF_FLAG_DVI_DUAL_LINK)
   {
      return "DVI-DL";
   }
   else if(flags != RGB133_VDIF_FLAG_NOSIGNAL &&
           flags != RGB133_VDIF_FLAG_OUTOFRANGE &&
           flags != RGB133_VDIF_FLAG_UNRECOGNISABLE)
   {
      if(flags & RGB133_VDIF_FLAG_INTERLACED)
         return "Interlaced Analogue";
      else
         return "Analogue";
   }

   return "Unknown Video Input";
}

int fileExists(const char *filename)
{
   int result = access(filename, F_OK);

   return (result == 0);
}

int main(int argc, char** argv)
{
   int fd = 0;
   int i = 0, j = 0, k = 0;
   int numCtrlDevices = (sizeof(controlDevices) / (sizeof(const char*) * 2));;
   int ctrlDevicesExist = 0;

   sVWSystemInfo  sysInfo;
   sVWDeviceInfo  devInfo;
   sVWDevice      device;

   /* This covers both drivers: rgb133 and rgb200 */
   for(k=0; k<numCtrlDevices; k++)
   {
      /* Initialise structures */
      memset(&sysInfo, 0, sizeof(sysInfo));
      memset(&devInfo, 0, sizeof(devInfo));
      memset(&device, 0, sizeof(device));

      if(!fileExists(controlDevices[k][0]))
         continue;
      ctrlDevicesExist = 1;

      /* Open Vision / VisionLC Control device */
      fd = open(controlDevices[k][0], O_RDWR);
      if(fd == 0)
      {
         printf("Failed to open \"%s\"...\n", controlDevices[k][0]);
         return -1;
      }

      /* Open Vision / VisionLC Control device */
      if(readCtrlSystemInfo(fd, &sysInfo))
      {
         printf("Failed to read %s system info...\n", controlDevices[k][1]);
         close(fd);
         return -2;
      }

      printf("\n\nSystem has %d %s devices with a total of %d inputs...\n\n",
            sysInfo.devices, controlDevices[k][1], sysInfo.inputs);

      /* Open Vision / VisionLC Control device */
      for(i=0; i<sysInfo.devices; i++)
      {
         /* Set the device to be queried */
         devInfo.device = i;
         device.device = i;
         if(readDeviceInfo(fd, &devInfo))
            printf("Failed to read %s device[%d] info...\n", controlDevices[k][1], i);
         else
         {
            /* Get inputs for device[i] */
            for(j=0; j<devInfo.channels; j++)
            {
               /* Set the device's input to be queried */
               device.input = j;

               printf("%s Device[%.2d] Input[%.2d]: Name(%s)\n", controlDevices[k][1], i, j, devInfo.name);
               printf("                             Node(%s)\n", devInfo.node);

               /* Read data from selected device and input */
               if(readDeviceParms(fd, &device))
                  printf("Failed to read %s device[%d] parms...\n", controlDevices[k][1], i);
               else
               {
                  printf("                             Input(%s)\n", GetSignalState(device.inputs[j].curDeviceParms.flags));
                  printf("                             Width(%lu)\n", device.inputs[j].curDeviceParms.VideoTimings.HorAddrTime);
                  printf("                             Height(%lu)\n", device.inputs[j].curDeviceParms.VideoTimings.VerAddrTime);
                  printf("                             Refresh(%lu)\n", device.inputs[j].curDeviceParms.VideoTimings.VerFrequency);
                  printf("                             Brightness(%ld)\n", device.inputs[j].curDeviceParms.Brightness);
                  printf("                             Contrast(%ld)\n", device.inputs[j].curDeviceParms.Contrast);
               }
            }
         }
      }

      close(fd);
   }

   if(!ctrlDevicesExist)
      return -3;

   return 0;
}
