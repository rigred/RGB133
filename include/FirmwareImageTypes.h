// To use this file, define "DEVICE" to be a macro taking ten parameters
// Parm 1 is the DGC Number of the device.
// Parm 2 is the compatibility index (1 per device entry, must be unique; is not a bitmask)
// Parm 3 is the FlashMethod used for this device.  Used inside the Flash application to instantiate
//        the correct driver class object.
// Parm 4 is the Family of the device - used as a subtype for FlashMethod.
// Parm 5 is the text of the DGC Number, less the literal DGC (need "DGC133", use #DGCID)
// Parm 6 is the length of the text of the DGC Number.
// Parm 7 is the coprocessor image used for this device.
// Parm 8 is the Device ID which is used to distinguish this device from all others in hardware.
//        This parameter needs to be unique within the column; it is used in a switch statement
//        inside at least the Flash application, fake numbers used for non FAMILY_DGC133 members.
// Parm 9 is the common image type; all common images which describe the same version number of
//        image in the same file should all have the same common image type.
// Parm 10 is the string ID in the flash application which is associated with this image type.
//
// You should also define a set of symbols which control the rows of the table emitted by the
// inclusion of this file.  Four basic families are defined as of now:
// INCLUDE_VISION
// INCLUDE_X4
// INCLUDE_EMBEDDED
// INCLUDE_FX4
//
// Additionally, there is a cover-all include; INCLUDE_ALL_IMAGE_TYPES which is the equivalent
// of defining all four symbols above.
// There are also some exclusion defines, which take precedence over the INCLUDE_ symbols:
// EXCLUDE_VISION
// EXCLUDE_X4
// EXCLUDE_EMBEDDED
// EXCLUDE_FX4
//
// Additionally, for ease of use, define DEVICE_ENUM which includes the correct DEVICE() macro
// definition to emit the enumerated type definition statements, which is the most common use
// of this file.
//
// If neither DEVICE nor DEVICE_ENUM are defined, the file will emit a message which looks to
// devstudio like an error message, but compilation won't stop.
//
// For example, to establish the enumerated type defining the device types for Vision cards:
//
// clang-format off
// #define INCLUDE_VISION
// typedef enum _tagDEVICETYPES {
// #define DEVICE(DGCID, COMPAT, FLASHMETHOD, FAMILY, NIOS, IDTXT, LEN, COPROIMAGE, DEVID, VERCOMMON, STRINGID) DGCID=COMPAT,
// #include "FirmwareImageTypes.h"
// } DEVICETYPES;
// clang-format on**
// Or, for Embedded device/image enumeration
//
// #define INCLUDE_EMBEDDED
// #define DEVICE_ENUM
// typedef enum _tagDEVICETYPES {
// #include "FirmwareImageTypes.h"
// } DEVICETYPES;
//
// By default, with no INCLUDE_* symbols defined, INCLUDE_VISION will be defined on your behalf.
// This is for the most common usage of this file before it was expanded.
// Also note that the first line of the table is always emitted; it is a placeholder for an unknown
// device, and cannot be omitted.


// DGCRFS looks like a generic name - it's specific to DGC165.  Can't be used for future kit.
// DGCKRN looks like a generic name - it's specific to DGC165.  Can't be used for future kit.
// DGC183 doesn't exist as a flash target, but needs to be present for the flash program to work.

// There are a lot of long-line tables in this file; it's inappropriate for clang-format to do it's thing.
// clang-format off
#ifndef __LOC__
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)"): error DPERROR: "
#endif
#if !defined(DEVICE) && !defined(DEVICE_ENUM)
#pragma message(__LOC__"no DEVICE macros defined.  Emitting no entries from header file.")
#else
#if defined(DEVICE_ENUM)
#undef DEVICE
#define DEVICE(DGCID, COMPAT, FLASHMETHOD, FAMILY, NIOS, IDTXT, LEN, COPROIMAGE, DEVID, VERCOMMON, STRINGID) DGCID=COMPAT,
#endif

// Some applications (such as the firmware image assembler) need to know about all the types.
// So let's make it easy on them to get them all...
#if defined (INCLUDE_ALL_IMAGE_TYPES)
#define INCLUDE_X4
#define INCLUDE_EMBEDDED
#define INCLUDE_FX4
#define INCLUDE_VISION
#define INCLUDE_VISIONLC
#define BACKPLANES_NO_REALLY_ITS_AN_EXPRESS_11
#define INCLUDE_DISPLAY
#define INCLUDE_ALIGO
#endif
// Excludes take precedence over includes.
#if defined(EXCLUDE_X4)
#undef INCLUDE_X4
#endif
#if defined(EXCLUDE_FX4)
#undef INCLUDE_FX4
#endif
#if defined(EXCLUDE_VISION)
#undef INCLUDE_VISION
#endif
#if defined(EXCLUDE_VISIONLC)
#undef INCLUDE_VISIONLC
#endif
#if defined(EXCLUDE_EMBEDDED)
#undef INCLUDE_EMBEDDED
#endif
#if defined(EXCLUDE_BACKPLANES)
#undef BACKPLANES_NO_REALLY_ITS_AN_EXPRESS_11
#endif
#if defined(EXCLUDE_DISPLAY)
#undef INCLUDE_DISPLAY
#endif
#if defined(EXCLUDE_ALIGO)
#undef INCLUDE_ALIGO
#endif

// Sort out legacy includes - Vision is probably all they expected to see.  To get all features,
// all INCLUDE_* macros need defining separately.
#if !defined(INCLUDE_X4) && !defined(INCLUDE_EMBEDDED) && !defined(INCLUDE_FX4) && !defined(INCLUDE_VISION) && !defined(INCLUDE_VISIONLC) && !defined(INCLUDE_DISPLAY)
#error No INCLUDE_ macro specified
#endif

#if defined(REQUIRE_VARIANTS)
// VARIANTs are the new variant firmware types. If an application defines this, it must be prepared to handle the 11 parameter DEVICE macro for *all* boards.
#define DEVICE_COMPAT(a,b,c,d,e,f,g,h,i,j,k,l) DEVICE(a,b,c,d,e,f,g,h,i,j,k,l)
#else
// Most products have no idea about variant firmwares.
#define DEVICE_COMPAT(a,b,c,d,e,f,g,h,i,j,k,l) DEVICE(a,b,c,d,e,f,g,h,i,j,k)
#endif

//            DGCID,   COMPAT,       FLASHMETHOD,      FAMILY,        NIOS_BOARD_TYPE,     IDTXT, LEN, COPROIMAGE,     DEVID,    VERCOMMON,   STRINGID, VARIANT
DEVICE_COMPAT(DGCXXX,  0,            FLASH000_METHOD,  FAMILY_NONE,   BOARD_TYPE_UNKNOWN,  "XXX",   3, "",             0x000000, TYPE_DGCUNK, 119, VariantReleased)
#if defined(INCLUDE_VISION)
DEVICE_COMPAT(DGC133,  0x00000001,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_RGBX2,    "133",   3, "dgc133fw.bin", 0xda0133, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC139,  0x00000002,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_RGBE1,    "139",   3, "dgc133fw.bin", 0xda0139, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC144,  0x00000004,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SD8,      "144",   3, "dgc133fw.bin", 0xda0144, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC150,  0x00000008,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_RGBE2,    "150",   3, "dgc133fw.bin", 0xda0150, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC151,  0x00000010,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SD4,      "151",   3, "dgc133fw.bin", 0xda0151, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC139S, 0x00000020,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_RGBE1S,   "139S",  4, "dgc133fw.bin", 0xda1139, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC150S, 0x00000040,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_RGBE2S,   "150S",  4, "dgc133fw.bin", 0xda1150, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC151S, 0x00000080,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SD4S,     "151S",  4, "dgc133fw.bin", 0xda1151, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC153,  0x00000100,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_DVI,      "153",   3, "dgc153fw.bin", 0xda0153, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC154,  0x00000200,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SDI2,     "154",   3, "dgc154fw.bin", 0xda0154, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC159,  0x00000400,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_AV,       "159",   3, "dgc159fw.bin", 0xda0159, TYPE_DGC133, 119, VariantReleased)
#endif
#if defined(INCLUDE_X4)
DEVICE_COMPAT(DGC147,  0x00000800,   FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "147",   3, "",             0x000001, TYPE_DGC147, 119, VariantReleased)
#endif
#if defined(INCLUDE_VISION)
DEVICE_COMPAT(DGC161,  0x00001000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_HD4,      "161",   3, "dgc159fw.bin", 0xda0161, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC165,  0x00002000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_AVHDSQX,  "165",   3, "dgc159fw.bin", 0xda0165, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC167,  0x00004000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_AVSDI,    "167",   3, "dgc159fw.bin", 0xda0167, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC168,  0x00008000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_AVHD,     "168",   3, "dgc159fw.bin", 0xda0168, TYPE_DGC133, 119, VariantReleased)
#endif
#if defined(BACKPLANES_NO_REALLY_ITS_AN_EXPRESS_11)
DEVICE_COMPAT(DGC169,  0x00010000,   FLASH000_METHOD,  FAMILY_NONE,   BOARD_TYPE_UNKNOWN,  "169",   3, "",             0x000004, TYPE_DGCUNK, 119, VariantReleased)
#endif
#if defined(INCLUDE_EMBEDDED)
DEVICE_COMPAT(DGCRFS,  0x00020000,   FLASH165_METHOD,  FAMILY_DGC165, BOARD_TYPE_UNKNOWN,  "RFS",   3, "",             0x100001, TYPE_DGCRFS, 121, VariantReleased)
DEVICE_COMPAT(DGCKRN,  0x00040000,   FLASH165_METHOD,  FAMILY_DGC165, BOARD_TYPE_UNKNOWN,  "KRN",   3, "",             0x100002, TYPE_DGCKRN, 120, VariantReleased)
DEVICE_COMPAT(DGCUNK,  0x00080000,   FLASH000_METHOD,  FAMILY_NONE,   BOARD_TYPE_UNKNOWN,  "UNK",   3, "",             0x100003, TYPE_DGCUNK, 119, VariantReleased)
#endif
#if defined(INCLUDE_VISION)
DEVICE_COMPAT(DGC179,  0x00100000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SCDP2,    "179",   3, "dgc179fw.bin", 0xda0179, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC182,  0x00200000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SCHDMI4,  "182",   3, "dgc182fw.bin", 0xda0182, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC184,  0x00400000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_SCSDI4,   "184",   3, "dgc184fw.bin", 0xda0184, TYPE_DGC133, 119, VariantReleased)
#endif
#if defined(INCLUDE_EMBEDDED)
DEVICE_COMPAT(DGC183K, 0x01000000,   FLASH165_METHOD,  FAMILY_DGC183, BOARD_TYPE_UNKNOWN,  "183K",  4, "",             0x100004, TYPE_DGC183K,120, VariantReleased)
DEVICE_COMPAT(DGC183R, 0x02000000,   FLASH165_METHOD,  FAMILY_DGC183, BOARD_TYPE_UNKNOWN,  "183R",  4, "",             0x100005, TYPE_DGC183R,121, VariantReleased)
DEVICE_COMPAT(DGC183,  0x04000000,   FLASH000_METHOD,  FAMILY_NONE,   BOARD_TYPE_UNKNOWN,  "183",   3, "",             0x100006, TYPE_DGCUNK, 119, VariantReleased)

DEVICE_COMPAT(DGC210K, 0x01000001,   FLASH165_METHOD,  FAMILY_DGC183, BOARD_TYPE_UNKNOWN,  "210K",  4, "",             0xda1210, TYPE_DGC210K, 120, VariantReleased)
DEVICE_COMPAT(DGC210R, 0x02000001,   FLASH165_METHOD,  FAMILY_DGC183, BOARD_TYPE_UNKNOWN,  "210R",  4, "",             0xda2210, TYPE_DGC210R, 121, VariantReleased)
DEVICE_COMPAT(DGC210,  0x04000001,   FLASH000_METHOD,  FAMILY_NONE,   BOARD_TYPE_UNKNOWN,  "210",   3, "",             0xda3210, TYPE_DGCUNK,  119, VariantReleased)
#endif

#if defined(INCLUDE_FX4)
DEVICE_COMPAT(DGC177F, 0x10000000,   FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "177F",  4, "",             0x000003, TYPE_DGC177, 119, VariantReleased)
DEVICE_COMPAT(DGC177,  0x20000000,   FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "177",   3, "",             0x000002, TYPE_DGC177, 119, VariantReleased)
#endif
#if defined(INCLUDE_VISION)
DEVICE_COMPAT(DGC201,  0x40000000,   FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_IOSDI4,   "201",   3, "dgc201fw.bin", 0xda0201, TYPE_DGC133, 119, VariantReleased)
#endif

/*
   WARNING FOR NEW ENTRIES

   Do not try and use just any number above 0x80000000, this may end up with the card being bricked
   in the field by a customer using an old Flash133/flashimg133.
   The old Flash133 applications will just be looking at the dwCompatibility as a mask. If any
   bits are set, it will think it can flash that card. The DGC211 was originally 0x80000007,
   the dwCompatibility in an old flash file was 0x0000000B. This meant Flash133 thought it could
   flash that card when, in fact, it couldn't and ended up bricking it.

   The compatibility mask for Vision is 0x4070F7FF. This gives us 12 bits (4095) in total to use.
   Use the VISION_COMPAT macro to use a valid number for Vision. ALL Vision cards packaged within#]
   DriverInstall and VisionStandalone MUST use this macro.

   For all other new entries (including VisionLC), use the NEW_COMPAT macro.
*/
#define VISION_COMPAT(x)         (0x80000000 | \
                                  ((x & 0x00000001) << 11) | \
                                  ((x & 0x0000001E) << 15) | \
                                  ((x & 0x00000FE0) << 18))

#define NEW_COMPAT(x)            (0x80000000 | \
                                  ((x & 0x000007FF) <<  0) | \
                                  ((x & 0x00007800) <<  1) | \
                                  ((x & 0x00038000) <<  5) | \
                                  ((x & 0x00040000) << 12))

//
// Prototype boards that never made it to production.
//
//DEVICE(DGC196, NEW_COMPAT(1), FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_IOHD2,    "196",   3, "dgc196fw.bin", 0xda0196, TYPE_DGC133, 119)
//DEVICE(DGC197, NEW_COMPAT(2), FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_IOSDI2,   "197",   3, "dgc197fw.bin", 0xda0197, TYPE_DGC133, 119)

#if defined(INCLUDE_VISIONLC)
DEVICE_COMPAT(DGC186, 0x00800000,    FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_EC,       "186",   3, "dgc186fw.bin", 0xda0186, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC199, 0x08000000,    FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_ECHD,     "199",   3, "dgc199fw.bin", 0xda0199, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC200, NEW_COMPAT(3), FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_LCHD,     "200",   3, "dgc200fw.bin", 0xda0200, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC204, NEW_COMPAT(4), FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_LCSDI,    "204",   3, "dgc204fw.bin", 0xda0204, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC205, NEW_COMPAT(5), FLASH133_METHOD,  FAMILY_DGC133, BOARD_TYPE_LCHD2,    "205",   3, "dgc205fw.bin", 0xda0205, TYPE_DGC133, 119, VariantReleased)
#endif

#if defined(INCLUDE_FX4)
DEVICE_COMPAT(DGC208, NEW_COMPAT(6), FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "208",   3, "",             0xda0208, TYPE_DGC177, 119, VariantReleased)
DEVICE_COMPAT(DGC216, NEW_COMPAT(7), FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "216",   3, "",             0xda0216, TYPE_DGC177, 119, VariantReleased)
DEVICE_COMPAT(DGC218, NEW_COMPAT(8), FLASH147_METHOD,  FAMILY_X4,     BOARD_TYPE_UNKNOWN,  "218",   3, "",             0xda0218, TYPE_DGC177, 119, VariantReleased)
#endif

#if defined(INCLUDE_VISION)
DEVICE_COMPAT(DGC211, VISION_COMPAT(1), FLASH133_METHOD, FAMILY_DGC133, BOARD_TYPE_IOXD2,  "211", 3, "dgc211fw.bin", 0xda0211, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC214, VISION_COMPAT(2), FLASH133_METHOD, FAMILY_DGC133, BOARD_TYPE_SCUHD2, "214", 3, "dgc214fw.bin", 0xda0214, TYPE_DGC133, 119, VariantReleased)
DEVICE_COMPAT(DGC224, VISION_COMPAT(3), FLASH133_METHOD, FAMILY_DGC133, BOARD_TYPE_SCSDVOE, "224", 3, "dgc224fw.bin", 0xda0224, TYPE_DGC133, 119, VariantReleased)
#endif

#if defined(INCLUDE_DISPLAY)
DEVICE_COMPAT(DGC190, 0x000dada0,     FLASHDISPLAY_METHOD, FAMILY_DISPLAY, BOARD_TYPE_UNKNOWN, "190", 3, "",             0xda0190, TYPE_DGCUNK, 119, VariantReleased)
DEVICE_COMPAT(DGC222, NEW_COMPAT(16), FLASHIQS4_METHOD,    FAMILY_DISPLAY, BOARD_TYPE_UNKNOWN, "222", 3, "dgc222fw.bin", 0xda0222, TYPE_IQS4,   119, VariantReleased)
#endif

#if defined(INCLUDE_ALIGO)
// DGCVER - Release version of an Aligo package containing Bootloader, RootFS and System
DEVICE_COMPAT(DGCVER, NEW_COMPAT(15), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "VER", 3, "", 0x400000, TYPE_ALIGO, 000, VariantReleased)

/* Split Aligo into 3 components for each board: B = Bootloader, R = RootFS, S = System */
DEVICE_COMPAT(DGC219B, NEW_COMPAT(9),  FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "219B", 4, "", 0x200001, TYPE_ALIGO, 219, VariantReleased)
DEVICE_COMPAT(DGC219R, NEW_COMPAT(10), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "219R", 4, "", 0x200002, TYPE_ALIGO, 219, VariantReleased)
DEVICE_COMPAT(DGC219S, NEW_COMPAT(11), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "219S", 4, "", 0x200003, TYPE_ALIGO, 219, VariantReleased)
DEVICE_COMPAT(DGC220B, NEW_COMPAT(12), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "220B", 4, "", 0x300001, TYPE_ALIGO, 220, VariantReleased)
DEVICE_COMPAT(DGC220R, NEW_COMPAT(13), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "220R", 4, "", 0x300002, TYPE_ALIGO, 220, VariantReleased)
DEVICE_COMPAT(DGC220S, NEW_COMPAT(14), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "220S", 4, "", 0x300003, TYPE_ALIGO, 220, VariantReleased)
DEVICE_COMPAT(DGC221B, NEW_COMPAT(17), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "221B", 4, "", 0x500001, TYPE_ALIGO, 221, VariantReleased)
DEVICE_COMPAT(DGC221R, NEW_COMPAT(18), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "221R", 4, "", 0x500002, TYPE_ALIGO, 221, VariantReleased)
DEVICE_COMPAT(DGC221S, NEW_COMPAT(19), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "221S", 4, "", 0x500003, TYPE_ALIGO, 221, VariantReleased)
DEVICE_COMPAT(DGC225B, NEW_COMPAT(20), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "225B", 4, "", 0x600001, TYPE_ALIGO, 225, VariantReleased)
DEVICE_COMPAT(DGC225R, NEW_COMPAT(21), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "225R", 4, "", 0x600002, TYPE_ALIGO, 225, VariantReleased)
DEVICE_COMPAT(DGC225S, NEW_COMPAT(22), FLASHALIGO_METHOD, FAMILY_ALIGO, BOARD_TYPE_UNKNOWN, "225S", 4, "", 0x600003, TYPE_ALIGO, 225, VariantReleased)
#endif

// clang-format on

#undef DEVICE
#undef DEVICE_COMPAT
#if defined( INCLUDE_ALL_IMAGE_TYPES )
#undef INCLUDE_X4
#undef INCLUDE_EMBEDDED
#undef INCLUDE_FX4
#undef INCLUDE_VISION
#undef INCLUDE_VISIONLC
#undef BACKPLANES_NO_REALLY_ITS_AN_EXPRESS_11
#undef INCLUDE_DISPLAY
#undef INCLUDE_ALIGO
#endif
#if defined( DEVICE_ENUM )
#undef DEVICE_ENUM
#endif
#endif
