/*
 * Enumerate.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver device
 * enumeration application sample source code.
 *
 * Purpose: Declares device enumeration app functions.
 *
 */

#include "rgb133control.h"

/*! 
 * Function to read Vision / VisionLC system information from the control device.
 *    - This function will read the number of Vision / VisionLC devices present
 *      in the system along with the total number of inputs.
 * @param  fd is the file descriptor of an open control device handle
 * @param  pSysInfo is a pointer to a sVWSystemInfo structure into 
 *         which the system information will be read.
 *         Note: The structure members magic and device must be set
 *               prior to calling this function.
 *         @see _eVWMagic
 *         @see _sVWSystemInfo
 * @return 0 on success otherwise negative value. 
 */
int readCtrlSystemInfo(int fd, sVWSystemInfo* pSysInfo);

/*! 
 * Function to read Vision / VisionLC device information from the control device.
 *    - This function will read the number of channels present on the
 *      desired device along with the Vision / VisionLC hardware name and
 *      videodev device node name (ie. VisionRGB-E1 and /dev/video0).
 * @param  fd is the file descriptor of an open control device handle
 * @param  pDeviceInfo is a pointer to a sVWDeviceInfo structure into 
 *         which the device information will be read.
 *         Note: The structure members magic and device must be set
 *               prior to calling this function.
 *         @see _eVWMagic
 *         @see _sVWDeviceInfo
 * @return 0 on success otherwise negative value. 
 */
int readDeviceInfo(int fd, sVWDeviceInfo* pDeviceInfo);

/*! 
 * Function to read Vision / VisionLC device parameter information from the
 * control device.
 *    - This function will read the input signal state, input source
 *      characteristics (width, height and refresh) and input
 *      parameters such as brightness and contrast.
 * @param  fd is the file descriptor of an open control device handle
 * @param  pDevice is a pointer to a sVWDevice structure into which
 *         the device parameters will be read.
 *         Note: The structure members magic, device and input must
 *         be set prior to calling this function.
 *         @see _eVWMagic
 *         @see _sVWDevice
 * @return 0 on success otherwise negative value. 
 */
int readDeviceParms(int fd, sVWDevice* pDevice);

/*! 
 * Function to check the signal state of a given set of input flags.
 * @param  flags are the current input flags
 * @return String representation os the current input flags. 
 */
const char* GetSignalState(unsigned long flags);
