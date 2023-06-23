
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWInputTab.h"
#include "VWSource.h"
#include "VWCapSettings.h"
#include "VWVidTiming.h"
#include "VWCropping.h"
#include "VWVideoAdj.h"
#include "VWColourAdj.h"

InputTab::InputTab(VWData* data, QWidget* parent)
   : p(parent), m_pData(data)
{
   connect(m_pData, SIGNAL(inputValuesChanged()), this, SLOT(inputValuesChanged()));

   /* Source Box */
   source = new VisionWinSource(tr("Source"), m_pData, this);
   /* End Source */

   /* Caption Box */
   capSet = new VisionWinCapSettings(tr("CaptureSettings"), m_pData, this);
   connect(this, SIGNAL(capSettingsTypeChanged()), capSet, SLOT(capSettingsTypeChanged()));
   /* End Caption */

   /* Video Timings Box */
   vidTiming = new VisionWinVidTiming(tr("Resolution && Refresh"), m_pData, this);
   connect(this, SIGNAL(vidTimingTypeChanged()), vidTiming, SLOT(vidTimingTypeChanged()));
   /* End Video Timings */

   /* Cropping Box */
   cropping = new VisionWinCropping(tr("Cropping"), m_pData, this);
   connect(this, SIGNAL(croppingTypeChanged()), cropping, SLOT(croppingTypeChanged()));
   /* End Cropping */

   /* Video Adj Box */
   vidAdj = new VisionWinVideoAdj(tr("Video Adjustments"), m_pData, this);
   connect(this, SIGNAL(vidAdjTypeChanged()), vidAdj, SLOT(vidAdjTypeChanged()));
   /* End Video Adj */

   /* Colour Adj Box */
   colAdj = new VisionWinColourAdj(tr("Colour Adjustments"), m_pData, this);
   connect(this, SIGNAL(colAdjTypeChanged()), colAdj, SLOT(colAdjTypeChanged()));
   /* End Colour Adj */

   QGridLayout *inputLayout = new QGridLayout;
   inputLayout->setAlignment(Qt::AlignTop);
   inputLayout->setSpacing(1);
   inputLayout->addWidget(source, 0, 0, 1, 4);
   inputLayout->addWidget(capSet, 0, 4, 1, 6);
   inputLayout->addWidget(vidTiming, 1, 0, 1, 5);
   inputLayout->addWidget(cropping, 1, 5, 1, 5);
   inputLayout->addWidget(vidAdj, 2, 0, 1, 5);
   inputLayout->addWidget(colAdj, 2, 5, 1, 5);
   setLayout(inputLayout);
}

// Public SLOTs
void InputTab::inputValuesChanged()
{
   emit vidTimingTypeChanged();
   emit vidAdjTypeChanged();
   emit colAdjTypeChanged();
   emit croppingTypeChanged();
   emit capSettingsTypeChanged();
}
