
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWWindowTab.h"
#include "VisionWindowDialog.h"
#include "VWData.h"
#include "VWPosition.h"
#include "VWCaption.h"
#include "VWSize.h"
#include "VWInvalidInput.h"
#include "VWAspectRatio.h"
#include "VWCaptureFormat.h"
#include "VWDeinterlace.h"
#include "VWStyle.h"
#include "VWTransferData.h"
#include "VWScaling.h"
#include "VWWindowID.h"
#include "VWCaptureRate.h"
#include "VWLiveStream.h"

WindowTab::WindowTab(VWData* data, QWidget* parent)
   : p(parent), m_pData(data)
{
   /* Position Box */
   position = new VisionWinPosition(tr("Position"), m_pData->getPosition(), m_pData->getPositionMax(),
         m_pData, this);
   /* End Position */

   /* Caption Box */
   caption = new VisionWinCaption(tr("Caption"), m_pData);
   /* End Caption */

   /* Size Box */
   size = new VisionWinSize(tr("Size"), m_pData->getClientPosition(), m_pData->getPositionMax(),
         m_pData, this);
   /* End Size */

   /* Size Box */
   invInput = new VisionWinInvalidInput(tr("Invalid Input"));
   /* End Size */

   /* Aspect Ratio Box */
   aspRatio = new VisionWinAspectRatio(tr("Aspect Ratio"), NULL, m_pData);
   /* End Aspect Ratio */

   /* Capture Format Box */
   capFmt = new VisionWinCaptureFormat(tr("Capture Format"), m_pData);
   /* End Capture Format */

   /* Deinterlace Box */
   deint = new VisionWinDeinterlace(tr("Deinterlace"));
   /* End Deinterlace */

   /* Style Box */
   style = new VisionWinStyle(tr("Style"), m_pData);
   /* End Style */

   /* Style Box */
   xfer = new VisionWinTransferData(tr("Transfer Data"));
   /* End Style */

   /* LiveStream Box */
   liveStr = new VisionWinLiveStream(tr("LiveStream"), m_pData);
   /* End LiveStream */

   /* Style Box */
   scale = new VisionWinScaling(tr("Scaling"));
   /* End Style */

   /* ID Box */
   id = new VisionWinWindowID(tr("Window ID"));
   /* End ID */

   /* ID Box */
   capRate = new VisionWinCaptureRate(tr("Capture Rate"), m_pData);
   /* End ID */

   QVBoxLayout* column1Layout = new QVBoxLayout;
   column1Layout->setAlignment(Qt::AlignTop);
   column1Layout->setSpacing(1);
   column1Layout->addWidget(position);
   column1Layout->addWidget(size);
   column1Layout->addWidget(aspRatio);
   column1Layout->addWidget(style);
   column1Layout->addWidget(id);
   column1Layout->addWidget(invInput);
   column1Layout->addSpacing(30);

   QHBoxLayout* column2Rox5Layout = new QHBoxLayout;
   column2Rox5Layout->setSpacing(1);
   column2Rox5Layout->addWidget(xfer);
   column2Rox5Layout->addWidget(scale);

   QVBoxLayout* column2Layout = new QVBoxLayout;
   column2Layout->setAlignment(Qt::AlignTop);
   column2Layout->setSpacing(1);
   column2Layout->addWidget(caption);
   column2Layout->addWidget(capFmt);
   column2Layout->addWidget(deint);
   column2Layout->addLayout(column2Rox5Layout);
   column2Layout->addWidget(liveStr);
   column2Layout->addWidget(capRate);

   QHBoxLayout* layout = new QHBoxLayout;
   layout->setSpacing(1);
   layout->addLayout(column1Layout);
   layout->addLayout(column2Layout);
   setLayout(layout);
}
