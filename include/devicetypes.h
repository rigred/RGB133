#ifndef DATAPATH_DEVICETYPES_H_INCLUDED
#define DATAPATH_DEVICETYPES_H_INCLUDED

#if !defined( __KERNEL__ ) && !defined( KERNEL_MODE )
// If you need this include file, you're more likely than not to need the
// structures and so on defined below.  Unless you're in kernel mode.
// Windows and Linux both define different kernel mode tokens, so we need
// to check that both of them are undefined to indicate user mode code.

#ifdef _INTSAFE_H_INCLUDED_
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <stdint.h> // needed for uint32_t type.

#ifdef _INTSAFE_H_INCLUDED_
#pragma warning( pop )
#endif

#ifndef _DATAPATH_IMAGE_FAMILY_TYPES
#define _DATAPATH_IMAGE_FAMILY_TYPES
typedef enum _tagProductFamily
{
   FAMILY_DGC133,  // An honest to goodness Vision device, no frills, VHDL+firmware file.
   FAMILY_X4,      // Standalone wall controllers.
   FAMILY_DGC165,  // SQX+Vision.
   FAMILY_DGC183,  // SQX standalone.
   FAMILY_DISPLAY, // Iolite family.
   FAMILY_ALIGO,   // Aligo family
   FAMILY_NONE,    // No discernable family.
} PRODUCT_FAMILY;
#endif

#ifndef _DATAPATH_BOARDVARIANT_TYPES
#define _DATAPATH_BOARDVARIANT_TYPES
typedef enum _BoardVariants
{
   VariantUnknown = -1,
   VariantDevelopment = 0,
   VariantVirtual,
   VariantProduction,
   VariantReleased,
} BoardVariants;
#endif

// A structure defining aan array of compatibility items.
#ifndef _DATAPATH_IMAGE_COMPAT_LIST
#define _DATAPATH_IMAGE_COMPAT_LIST
typedef struct _tagCompatList
{
   uint32_t dwEntries;
   uint32_t *pEntries;
} COMPAT_LIST, *PCOMPAT_LIST;
#endif

// Methods of flashing devices.  Used internally by the flash application.
#ifndef _DATAPATH_FLASH_METHODS
#define _DATAPATH_FLASH_METHODS
typedef enum _tagFlashMethods
{
   FLASH133_METHOD,
   FLASH165_METHOD,
   FLASH147_METHOD,
   FLASHDISPLAY_METHOD,
   FLASH000_METHOD,
   FLASHIQS4_METHOD,
   FLASHALIGO_METHOD
} FlashMethods;
#endif

#ifndef _DATAPATH_FLASH_IMAGE_COMMONS
#define _DATAPATH_FLASH_IMAGE_COMMONS
// This enum defines the version number commonality of the VHDL images.
// All DGC133 device types (150, 150S, 167, etc) all have a common version
// number, so the flash app needs to be able to know the commonality of
// version numbers to device types and suppress related version numbers in
// the list of images contained in the file, so that it doesn't report
// each device type as a newly encountered version number.
typedef enum _tagFlashImageCommons
{
   TYPE_DGCUNK,  // Unknown/invalid
   TYPE_DGC133,  // all 133 type images
   TYPE_DGC147,  // 147 images
   TYPE_DGCRFS,  // DGC165 Root file system
   TYPE_DGCKRN,  // DGC165 Kernel
   TYPE_DGC183K, // DGC183 Kernel
   TYPE_DGC183R, // DGC183 RFS
   TYPE_DGC199,  // Unknown new device in vision range.
   TYPE_DGC177,  // FX4.
   TYPE_ALIGO,   // Aligo
   TYPE_IQS4,    // IQS4
   TYPE_DGC210K,  // DGC210 Kernel
   TYPE_DGC210R,  // DGC210 RFS
} FlashImageCommons;
#endif

#endif

typedef enum _tagDGCDEVICETYPE
{
#define DEVICE_ENUM
#include "FirmwareImageTypes.h"
} DGCDEVICETYPE;

#endif
