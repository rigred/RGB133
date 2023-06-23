
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWWINDOWPROPERTIES_H_
#define VWWINDOWPROPERTIES_H_

enum eVWModified {
   VW_MOD_NONE                 = 0x00000000,
   VW_MOD_PIXEL_FORMAT         = 0x00000001,
   VW_MOD_ASPECT_RATIO         = 0x00000002,
   VW_MOD_BORDER_TITLE         = 0x00000004,
   VW_MOD_CAPTION              = 0x00000008,
   VW_MOD_ENABLE_INACTIVE_RATE = 0x00000010,
   VW_MOD_CAPTURE_RATE         = 0x00000020,
   VW_MOD_POSITION             = 0x00000040,
   VW_MOD_SIZE                 = 0x00000080,
   VW_MOD_STYLE                = 0x00000100,
   VW_MOD_VID_TIMINGS          = 0x00000200,
   VW_MOD_VID_ADJ              = 0x00000400,
   VW_MOD_ROTATION             = 0x00000800,
   VW_MOD_CAPTURE_SETTINGS     = 0x00001000,
   VW_MOD_CROPPING             = 0x00002000,
   VW_MOD_COLOUR_ADJ           = 0x00004000,
   VW_MOD_COLOUR_BAL           = 0x00008000,
   VW_MOD_LIVE_STREAM          = 0x00010000,
};

#define MOD_WINDOW_PROPERTIES_FLAGS ( VW_MOD_PIXEL_FORMAT           | VW_MOD_ASPECT_RATIO     | \
                                      VW_MOD_BORDER_TITLE           | VW_MOD_CAPTION          | \
                                      VW_MOD_ENABLE_INACTIVE_RATE   | VW_MOD_CAPTURE_RATE     | \
                                      VW_MOD_POSITION               | VW_MOD_SIZE             | \
                                      VW_MOD_STYLE                  | VW_MOD_LIVE_STREAM )

#define MOD_INPUT_PROPERTIES_FLAGS  ( VW_MOD_VID_TIMINGS            | VW_MOD_VID_ADJ          | \
                                      VW_MOD_ROTATION               | VW_MOD_CAPTURE_SETTINGS | \
                                      VW_MOD_CROPPING               | VW_MOD_COLOUR_ADJ       | \
                                      VW_MOD_COLOUR_BAL )

#endif /* VWWINDOWPROPERTIES_H_ */
