/*
 * rgb_windows_types.h
 *
 * Copyright (c) 2009 Datapath Limited All rights reserved.
 *
 * All information contained herein is proprietary and
 * confidential to Datapath Limited and is licensed under
 * the terms of the Datapath Limited Software License.
 * Please read the LICENCE file for full license terms
 * and conditions.
 *
 * http://www.datapath.co.uk/
 * support@datapath.co.uk
 *
 */

#ifndef RGB_WINDOWS_TYPES_H_
#define RGB_WINDOWS_TYPES_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/workqueue.h>
#include "rgb133config.h"
#ifdef RGB133_CONFIG_HAVE_ASM_SEMA4
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#else
#include <pthread.h>
#include <wchar.h>
#endif /* __KERNEL__ */

#ifdef __KERNEL__
#  ifndef RGB133_USER_MODE
#    include "rgb_base_types.h"
#  else
#    include "rgb_linux_usermode_types.h"
#  endif /* RGB133_USER_MODE */
#else
#  include "rgb_base_types.h"
#  include "rgb_linux_user_types.h"
#endif /* __KERNEL__ */

#include "rgb_linux_types.h"

/* Empty Windows Definitions */
#define IN 
#define OUT

#define STREAMAPI

#define HKEY_LOCAL_MACHINE 0
#define KEY_ALL_ACCESS     0

#define __stdcall
#define __in
#define _In_

//#define ERROR_SUCCESS      1

/* Useful Windows definitions */
#define HKEY                        ULONG

#ifdef __KERNEL__
#ifdef RGB133_CONFIG_HAVE_WCHAR_INT
#define WCHAR                       int
#else
#  ifdef RGB133_CONFIG_HAVE_WCHAR_LONG_INT
#  define WCHAR                     long int
#  else
#  define WCHAR
#  endif /* RGB133_CONFIG_HAVE_WCHAR_LONG_INT */ 
#endif /* RGB133_CONFIG_HAVE_WCHAR_INT */
#else
#define WCHAR                       wchar_t
#endif /* __KERNEL__ */

#define PWCHAR                      WCHAR*
#define PWSTR                       WCHAR*

#define CHAR                        char
#define CCHAR                       char

#ifndef TRUE
#define TRUE                        1
#endif
#ifndef FALSE
#define FALSE                       0
#endif


#define PHW_STREAM_OBJECT           void*
#define PKSTREAM_HEADER             void*
#define PHW_STREAM_DESCRIPTOR       void*
#define KS_VIDEOINFOHEADER          void
#define KS_FRAME_INFO               ULONG
#define KSSTATE                     DWORD
#define PSTREAM_PROPERTY_DESCRIPTOR void*
#define HANDLE                      ULONG
#define PHANDLE                     HANDLE*
#define PSTREAM_DATA_INTERSECT_INFO void*
#define PkSCATTER_GATHER            void*

#define PDRIVER_DISPATCH            void*
#define PPCI_COMMON_HEADER          void*

#define HW_STREAM_HEADER            int
#define HW_STREAM_INFORMATION       int

#define MAX_PATH 255

typedef struct _KSPIN_LOCK {
   spinlock_t    lock;
   unsigned long flags;
} KSPIN_LOCK, *PKSPIN_LOCK;

#define SLIST_ENTRY                 struct list_head
#define PSLIST_ENTRY                struct list_head*
#define SLIST_HEADER                struct list_head
#define PSLIST_HEADER               struct list_head*

#define KDPC                        struct work_struct
#define PKDPC                       struct work_struct*

#ifdef __KERNEL__
#define IO_WORKITEM                 struct work_struct
#define PIO_WORKITEM                struct work_struct*
#else
#define IO_WORKITEM                 pthread_t
#define PIO_WORKITEM                pthread_t*
#endif

#define WORK_QUEUE_TYPE             const char*

#define KIRQL                       ULONG
#define KINTERRUPT_MODE             int
#define KAFFINITY                   int

#define OBJECT_ATTRIBUTES           int

#define REG_DWORD                   0x0
#define REG_SZ                      0x0
#define KEY_SET_VALUE               0x0

#define KFLOATING_SAVE              ULONG

#define KSERVICE_ROUTINE            BOOLEAN

#define DEFINE_GUID(a, b, c, d, e, f, g, h, i, j, k, l)

#define CONTAINING_RECORD  container_of

#define DEBUG_ASSERT(x)  (x)

#define CALLBACK

#define ERROR_SUCCESS 0

#define NTKERNELAPI

#define DelayedWorkQueue "DelayedWork"

#define ASSERT

#define try
#define except(_X_) if(0)
#define _try
#define _except(_X_) if(0)
#define GetExceptionCode( ) (0)
#define __try try
#define __except except

/* Useful Windows Enumerations */
typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE Data4_1;
  BYTE Data4_2;
  BYTE Data4_3;
  BYTE Data4_4;
  BYTE Data4_5;
  BYTE Data4_6;
  BYTE Data4_7;
  BYTE Data4_8;
} GUID;

typedef enum _POOL_TYPE {  
   NonPagedPool,  
   PagedPool,  
   NonPagedPoolMustSucceed,  
   DontUseThisType,  
   NonPagedPoolCacheAligned,  
   PagedPoolCacheAligned,  
   NonPagedPoolCacheAlignedMustS
} POOL_TYPE;

typedef enum _INTERFACE_TYPE {
   InterfaceTypeUndefined = -1,
   PCIBus = 5
} INTERFACE_TYPE, *PINTERFACE_TYPE;

typedef enum _DMA_WIDTH {
   Width8Bits,
   Width16Bits,
   Width32Bits,
   Width64Bits
} DMA_WIDTH, *PDMA_WIDTH;

typedef enum _DMA_SPEED {
   Compatible,
   TypeA,
   TypeB,
   TypeC,
   TypeF,
   MaximumDmaSpeed
} DMA_SPEED, *PDMA_SPEED;

typedef enum _DEVICE_POWER_STATE {
   PowerDeviceUnspecified
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

typedef enum {
  DevicePropertyDeviceDescription             = 0x0,
  DevicePropertyHardwareID                    = 0x1,
  DevicePropertyCompatibleIDs                 = 0x2,
  DevicePropertyBootConfiguration             = 0x3,
  DevicePropertyBootConfigurationTranslated   = 0x4,
  DevicePropertyClassName                     = 0x5,
  DevicePropertyClassGuid                     = 0x6,
  DevicePropertyDriverKeyName                 = 0x7,
  DevicePropertyManufacturer                  = 0x8,
  DevicePropertyFriendlyName                  = 0x9,
  DevicePropertyLocationInformation           = 0xa,
  DevicePropertyPhysicalDeviceObjectName      = 0xb,
  DevicePropertyBusTypeGuid                   = 0xc,
  DevicePropertyLegacyBusType                 = 0xd,
  DevicePropertyBusNumber                     = 0xe,
  DevicePropertyEnumeratorName                = 0xf,
  DevicePropertyAddress                       = 0x10,
  DevicePropertyUINumber                      = 0x11,
  DevicePropertyInstallState                  = 0x12,
  DevicePropertyRemovalPolicy                 = 0x13,
  DevicePropertyResourceRequirements          = 0x14,
  DevicePropertyAllocatedResources            = 0x15,
  DevicePropertyContainerID                   = 0x16
} DEVICE_REGISTRY_PROPERTY;

typedef enum _KIRQL_E {
   DISPATCH_LEVEL = 0,
   PASSIVE_LEVEL = 0
} KIRQL_E, *PKIRQL_E;

/* Useful Windows typedef's */
typedef struct _KTHREAD {
} KTHREAD, *PKTHREAD;

typedef struct _RECT {
   LONG left;
   LONG top;
   LONG right;
   LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct _SIZE {
   LONG cx;
   LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef void (*PKDEFERRED_ROUTINE)(void* arg);
typedef void (IO_WORKITEM_ROUTINE)(void* arg);
typedef void (*PIO_WORKITEM_ROUTINE)(void* arg);

typedef struct _UNICODE_STRING {
   USHORT Length;
   USHORT MaximumLength;
   PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _DRIVER_OBJECT {
} DRIVER_OBJECT, *PDRIVER_OBJECT;

typedef struct _HW_INITIALIZATION_DATA {
} HW_INITIALIZATION_DATA, *PHW_INITIALIZATION_DATA;

typedef struct _DEVICE_OBJECT {
   ULONG Flags;
   PVOID pContext;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

#define DEVICE_DESCRIPTION_VERSION 0
typedef struct _DEVICE_DESCRIPTION {
   ULONG          Version;
   BOOLEAN        Master;
   BOOLEAN        ScatterGather;
   BOOLEAN        DemandMode;
   BOOLEAN        AutoInitialise;
   BOOLEAN        Dma32BitAddresses;
   BOOLEAN        IgnoreCount;
   BOOLEAN        Reserved1;
   BOOLEAN        Dma64BitAddresses;
   ULONG          BusNumber;
   ULONG          DmaChannel;
   INTERFACE_TYPE InterfaceType;
   DMA_WIDTH      DmaWidth;
   DMA_SPEED      DmaSpeed;
   ULONG          MaximumLength;
   ULONG          DmaPort;
} DEVICE_DESCRIPTION, *PDEVICE_DESCRIPTION;

typedef LARGE_INTEGER PHYSICAL_ADDRESS;

typedef struct _ACCESS_RANGE {
   LARGE_INTEGER  RangeStart;
   ULONG          RangeLength;
   BOOLEAN        RangeInMemory;
} ACCESS_RANGE, *PACCESS_RANGE;

#ifdef linux
typedef VOID (*PKSTART_ROUTINE)(PVOID arg);
typedef VOID (KSTART_ROUTINE)(PVOID arg);
#endif

enum {
   STREAM_Capture,
   STREAM_Preview,
   STREAM_AnalogVideoInput,
   STREAM_Count,
};

typedef struct _PORT_CONFIGURATION_INFORMATION {
   PVOID             HwDeviceExtension;
   ULONG             NumberOfAccessRanges;
   ACCESS_RANGE      AccessRanges[4];

   ULONG             StreamDescriptorSize;

   PDEVICE_OBJECT    RealPhysicalDeviceObject;
} PORT_CONFIGURATION_INFORMATION, *PPORT_CONFIGURATION_INFORMATION;

typedef struct _SCATTER_GATHER_ELEMENT {
   PHYSICAL_ADDRESS  Address;
   ULONG Length;
   UINT* Reserved;
} SCATTER_GATHER_ELEMENT, *PSCATTER_GATHER_ELEMENT;

typedef struct _SCATTER_GATHER_LIST {
   ULONG NumberOfElements;
   UINT* Reserved;
   SCATTER_GATHER_ELEMENT Elements[];
} SCATTER_GATHER_LIST, *PSCATTER_GATHER_LIST;

typedef struct _FILE_OBJECT {
   PVOID             FsContext2;
} FILE_OBJECT, *PFILE_OBJECT;

typedef struct _IO_STACK_LOCATION {
   PVOID             DeviceObject;
   PFILE_OBJECT      FileObject;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct _MDL {
   struct _MDL*         Next;
   struct page**        map;
   LONG                 Size;
   LONG                 MdlFlags;
   PVOID                MappedSystemVa;
   PVOID                StartVa;
   PVOID                UserVa;
   ULONG                ByteCount;
   ULONG                ByteOffset;
   OSPROCESSOR_MODE     AccessMode;
} MDL, *PMDL;

typedef struct _IO_STATUS_BLOCK {
   union {
      NTSTATUS Status;
      PVOID    Pointer;
   };
   ULONG   Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _IRP {
   PMDL               MdlAddress;
   PMDL               MdlPlanarAddress[3];
   
   union {
     PVOID  SystemBuffer;
   } AssociatedIrp;

   IO_STATUS_BLOCK    IoStatus;
   
   PIO_STACK_LOCATION irpStack;

   int                DmaBufIndex;
} IRP, *PIRP;

typedef enum _SRB_COMMAND {  
   SRB_READ_DATA,
   SRB_WRITE_DATA, 
   SRB_GET_STREAM_STATE,
   SRB_SET_STREAM_STATE,
   SRB_SET_STREAM_PROPERTY,
   SRB_GET_STREAM_PROPERTY,
   SRB_OPEN_MASTER_CLOCK,

   SRB_INDICATE_MASTER_CLOCK,
   SRB_UNKNOWN_STREAM_COMMAND,
   SRB_SET_STREAM_RATE,
   SRB_PROPOSE_DATA_FORMAT,
   SRB_CLOSE_MASTER_CLOCK,
   SRB_PROPOSE_STREAM_RATE,
   SRB_SET_DATA_FORMAT,
   SRB_GET_DATA_FORMAT,
   SRB_BEGIN_FLUSH,
   SRB_END_FLUSH,

   SRB_GET_STREAM_INFO = 0x100,
   SRB_OPEN_STREAM,
   SRB_CLOSE_STREAM,
   SRB_OPEN_DEVICE_INSTANCE,
   SRB_CLOSE_DEVICE_INSTANCE,
   SRB_GET_DEVICE_PROPERTY,
   SRB_SET_DEVICE_PROPERTY,
   SRB_INITIALIZE_DEVICE,
   SRB_CHANGE_POWER_STATE,
   SRB_UNINITIALIZE_DEVICE,
   SRB_UNKNOWN_DEVICE_COMMAND,
   SRB_PAGING_OUT_DRIVER,
   SRB_GET_DATA_INTERSECTION,
   SRB_INITIALIZATION_COMPLETE,
   SRB_SURPRISE_REMOVAL
} SRB_COMMAND;

typedef struct _HW_STREAM_REQUEST_BLOCK {
   NTSTATUS          Status;
   SRB_COMMAND       Command;

   union _CommandData {
      struct _PORT_CONFIGURATION_INFORMATION*   ConfigInfo;
   } CommandData;

   PVOID             HwDeviceExtension;
   PIRP              Irp;

} HW_STREAM_REQUEST_BLOCK, *PHW_STREAM_REQUEST_BLOCK;

typedef struct _TIME_FIELDS {
   short Year;
   short Month;
   short Day;
   short Hour;
   short Minute;
   short Second;
   short Milliseconds;
   short Weekday;
} TIME_FIELDS, *PTIME_FIELDS;

typedef unsigned int LOCK_OPERATION;

#define IoWriteAccess   0
#define IoReadAccess    1

#define UserRequest 0

#ifndef __KERNEL__
typedef pthread_cond_t EVENT_COND;
typedef pthread_mutex_t EVENT_MUTEX;
#endif /* __KERNEL__ */

typedef struct _KINTERRUPT {
   KSPIN_LOCK SpinLock;
   ULONG      Flags;
} KINTERRUPT, *PKINTERRUPT;

typedef void (*PDRIVER_LIST_CONTROL)(PDEVICE_OBJECT fdo,
                                     PIRP junk,
                                     PSCATTER_GATHER_LIST pWinSGL,
                                     PVOID pContext);

struct _DMA_OPERATIONS;

typedef struct _DMA_ADAPTER {
   USHORT                  Version;
   USHORT                  Size;
   PVOID                   Context;
   struct _DMA_OPERATIONS* DmaOperations;
} DMA_ADAPTER, *PDMA_ADAPTER;

/*typedef NTSTATUS (*PGET_SCATTER_GATHER_LIST)(PDMA_ADAPTER DmaAdapter, PDEVICE_OBJECT DeviceObject,
                         PMDL Mdl, PVOID CurrentVa, ULONG Length,
                         PDRIVER_LIST_CONTROL ExecutionRoutine,
                         PVOID Context, BOOLEAN WriteToDevice);*/

typedef struct _DMA_OPERATIONS {
   ULONG    Size;
   NTSTATUS (*GetScatterGatherList)(PDMA_ADAPTER DmaAdapter, PDEVICE_OBJECT DeviceObject,
                                    PMDL Mdl, PVOID CurrentVa, ULONG Length,
                                    PDRIVER_LIST_CONTROL ExecutionRoutine,
                                    PVOID Context, BOOLEAN WriteToDevice);
   NTSTATUS (*PutScatterGatherList)(PDMA_ADAPTER DmaAdapter, PSCATTER_GATHER_LIST pWinSGList,
                                    int BufferIndex, BOOLEAN WriteToDevice);
   VOID     (*PutDmaAdapter)(PDMA_ADAPTER pDmaAdapter);
} DMA_OPERATIONS, *PDMA_OPERATIONS;

typedef struct KSIDENTIFIER {
   GUID   Set;
   ULONG  Id;
   ULONG  Flags;
} KSPIN_MEDIUM, *PKSPIN_MEDIUM;

typedef struct tagKS_BITMAPINFOHEADER {
   DWORD    biSize;
   LONG     biWidth;
   LONG     biHeight;
   WORD     biPlanes;
   WORD     biBitCount;
   DWORD    biCompression;
   DWORD    biSizeImage;
   LONG     biXPelsPerMeter;
   LONG     biYPelsPerMeter;
   DWORD    biClrUsed;
   DWORD    biClrImportant;
} KS_BITMAPINFOHEADER, *PKS_BITMAPINFOHEADER;

typedef ULONG REFERENCE_TIME;

typedef void* PKS_VIDEOINFOHEADER;
typedef void* PKS_VIDEOINFOHEADER2;

typedef struct _KSDATAFORMAT {
   ULONG     FormatSize;
   ULONG     Flags;
   ULONG     SampleSize;
   ULONG     Reserved;
   GUID      MajorFormat;
   GUID      SubFormat;
   GUID      Specifier;
} KSDATAFORMAT, *PKSDATAFORMAT;

#endif /*RGB_WINDOWS_TYPES_H_*/
