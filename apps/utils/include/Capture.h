/*
 * Capture.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Declares capture app main defines, structures and
 * functions.
 *
 */

#include <linux/videodev2.h>

#include <pthread.h>

#include "CaptureTypes.h"

#define NUM_STREAMING_BUFFERS 4

typedef int BOOL;

typedef struct _sBuffer
{
   /* Capture Data */
   char         *pData;            /*!< Dynamically allocated io buffer.*/
   unsigned int  DataLength;       /*!< io buffer length in bytes.*/
} sBuffer, *psBuffer;

/*!
 * Parent capture structure.
 *    - Contains all video4linux data as well as output
 *      buffer pointer and information.
 */
typedef struct _sCapture
{
   /* File Descriptors */
   int                        fd;               /*!< File descriptor for the video4linux device.*/
   int                        ctrlFd;           /*!< File descriptor for the control device.*/

   /* Internals */
   int                        inputs;           /*!< the number of enumerated inputs on the device.*/
   int                        standards;        /*!< the number of enumerated video standards on the device.*/
   int                        formats;          /*!< the number of enumerated formats on the device.*/

   unsigned long              source_width;     /*!< The input source width.*/
   unsigned long              source_height;    /*!< The input source height.*/

   unsigned long              output_width;     /*!< The output buffer width.*/
   unsigned long              output_height;    /*!< The output buffer height.*/
   unsigned int               output_pixfmt;    /*!< The output buffer pixel format (chroma).*/

   eIOType                    ioType;           /*!< The IO capture method, read or mmap.*/

   float                      fFrameRate;       /*!< The desired capture frame rate (in Hz, 2 dp.). */

   BOOL                       CroppingActive;   /*!< The cropping rectangle is active */
   int                        CroppingLeft;     /*!< The x origin for the capture (cropping) */
   int                        CroppingTop;      /*!< The y origin for the capture (cropping)*/
   int                        CroppingWidth;    /*!< The cropping width for the capture */
   int                        CroppingHeight;   /*!< The cropping height for the capture */

   BOOL                       ComposingActive;  /*!< The image composition rectangle is active */
   int                        ComposeLeft;      /*!< The x origin of the composed image */
   int                        ComposeTop;       /*!< The y origin of the composed image */
   int                        ComposeWidth;     /*!< The width of the composed image */
   int                        ComposeHeight;    /*!< The height of the composed image */

   int                        capture_count;    /*!< The number of frames to capture. <0 = infinite */

   int                        timestamp;        /*!< Output of timestamp information is enabled(1) or disabled(0) */

   int                        livestream;       /*!< liveStream is enabled(1) or disabled(0) */

   int                        ganging_type;     /*!< ganging_type for chaining inputs together */

   int                        delay;            /*!< Delay in ms between dequeued frames */

   /* video4linux structures */
   struct v4l2_capability     caps;             /*!< The video4linux capabilities structure.*/

   struct v4l2_input         *pInputs;          /*!< Dynamically allocated video4linux input structures.*/

   v4l2_std_id                standard;         /*!< Currently selected video4linux standard.*/
   struct v4l2_standard      *pStandards;       /*!< Dynamically allocated video4linux standard structures.*/

   struct v4l2_fmtdesc       *pFormats;         /*!< Dynamically allocated video4linux format description structures.*/

   struct v4l2_format         src_fmt;          /*!< Input source video4linux format structure.*/
   struct v4l2_format         fmt;              /*!< Output buffer video4linux format structure.*/

   struct v4l2_streamparm     StrmParm;         /*!< Streaming parameters, includes time per frame interval in seconds */

   /* mmap internals */
   struct v4l2_requestbuffers req;
   unsigned int               buffers;

   /* Capture Buffers */
   sBuffer                    pBuffers[NUM_STREAMING_BUFFERS];

   /* Capture Thread */
   pthread_t               CaptureThread;    /*!< Capture thread handle.*/
} sCapture, *psCapture;

/*!
 * Function to open a Vision / VisionLC video4linux capture device.
 *    - Performs basic enumeration operations on inputs and
 *      supported video standards and formats.
 * @param  device is the video4linux name of the device to be opened
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the device and video4linux data will be stored.
 * @param  bBlock flag to setup blocking/non-blocking driver IO.
 * @return 0 on success otherwise negative value.
 */
int OpenCaptureDevice(char* device, psCapture pCapture, BOOL bBlock);

/*!
 * Function to open the Vision / VisionLC control device.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the control device file descriptor will be stored.
 * @param  ctrlDevice is a name of control device to open
 * @return 0 on success otherwise negative value.
 */
int OpenCaptureDeviceControl(psCapture pCapture, const char* ctrlDevice);

/*!
 * Function to close a Vision / VisionLC video4linux capture device.
 *    - Frees any allocated memory used to store video4linux data.
 * @param  fd is the video4linux device file descriptor to be closed
 * @param  pCapture is a pointer to a sCapture structure from which
 *         allocated memory can be freed.
 * @return 0 on success otherwise negative value.
 */
void CloseCaptureDevice(int fd, psCapture pCapture);

/*!
 * Function to close the Vision / VisionLC control device.
 * @param  fd is the control device file descriptor to be closed
 * @return 0 on success otherwise negative value.
 */
void CloseCaptureDeviceControl(int fd);

/*!
 * Function to initialise and allocate memory for the read io buffer.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int InitialiseReadCapture(psCapture pCapture, unsigned int buffer_size);

/*!
 * Function to uninitialise and deallocate memory for the read io buffer.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int UninitialiseReadCapture(psCapture pCapture);

/*!
 * Function to initialise and allocate memory for the mmap io buffers.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int InitialiseMmapCapture(psCapture pCapture, unsigned int buffer_size);

/*!
 * Function to initialise and allocate memory for the userptr io buffers.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int InitialiseUserPtrCapture(psCapture pCapture, unsigned int buffer_size);

/*!
 * Function to uninitialise and deallocate memory for the mmap io buffers.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int UninitialiseMmapCapture(psCapture pCapture);

/*!
 * Function to uninitialise and deallocate memory for the userptr io buffers.
 * @param  pCapture is a pointer to a sCapture structure into which
 *         the read buffer can be initialised.
 * @return 0 on success otherwise negative value.
 */
int UninitialiseUserptrCapture(psCapture pCapture);

/*!
 * Function to initialise a video capture.
 *    - Only initialises capture frame rate for the time being.
 * @param  pCapture is a pointer to a sCapture structure that contains
 *         the desired capture frame rate.
 * @return 0 on success otherwise negative value.
 */
int InitialiseCapture(psCapture pCapture);

/*!
 * Function to start a video capture.
 *    - Starts a capture thread so the main thread can perform alternative
 *      work.
 * @param  pCapture is a pointer to a sCapture structure which is passed
 *         into the capture thread.
 * @return 0 on success otherwise negative value.
 */
int StartCapture(psCapture pCapture);

/*!
 * Function to verify if pixel format is supported by the application.
 * It also converts a fourcc pixel format to its v4l2 representation.
 * @param  fourcc pixel format as string
 * @return v4l2 code of the corresponding fourcc pixel format or 0 if format is not supported.
 */
int IsSupportedPixelFormat(char* pixfmt);

