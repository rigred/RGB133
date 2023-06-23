
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWLiveStream.h"

VisionWinLiveStream::VisionWinLiveStream(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
    setAlignment(Qt::AlignLeft);
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    radioButtonOff = new QRadioButton(tr("Off"));
    radioButtonOff->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    radioButtonOn = new QRadioButton(tr("On"));
    radioButtonOn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    switch(m_pData->getLiveStream())
    {
       case VW_LIVESTREAM_OFF:
          radioButtonOff->setChecked(true);
          break;
       case VW_LIVESTREAM_ON:
          radioButtonOn->setChecked(true);
          break;
       default:
          break;
    }

    connect(radioButtonOff, SIGNAL(toggled(bool)),
          this, SLOT(setLiveStream()));
    connect(radioButtonOn, SIGNAL(toggled(bool)),
          this, SLOT(setLiveStream()));
    connect(m_pData, SIGNAL(updateLS(eVWLiveStream)), this, SLOT(updateLiveStream(eVWLiveStream)));

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *livestreamLayout = new QBoxLayout(direction);
    livestreamLayout->setSpacing(5);
    livestreamLayout->setAlignment(Qt::AlignTop);
    livestreamLayout->setContentsMargins(10, 0, 10, 1);
    livestreamLayout->addWidget(radioButtonOff);
    livestreamLayout->addWidget(radioButtonOn);
    setLayout(livestreamLayout);
}

/**********************************************************************/

// Public slots
void VisionWinLiveStream::setLiveStream( )
{
   if(radioButtonOff->isChecked())
   {
      m_pData->setLiveStream(VW_LIVESTREAM_OFF);
      m_pData->setModified(VW_MOD_LIVE_STREAM);
   }
   else if(radioButtonOn->isChecked())
   {
      m_pData->setLiveStream(VW_LIVESTREAM_ON);
      m_pData->setModified(VW_MOD_LIVE_STREAM);
   }
   else
      return;
}

void VisionWinLiveStream::updateLiveStream(eVWLiveStream liveStream)
{
   if((radioButtonOn->isChecked()) && (liveStream == VW_LIVESTREAM_OFF))
   {
      radioButtonOn->setChecked(false);
      radioButtonOff->setChecked(true);
   }
   else if((radioButtonOff->isChecked()) && (liveStream == VW_LIVESTREAM_ON))
   {
      radioButtonOff->setChecked(false);
      radioButtonOn->setChecked(true);
   }
   else
      return;
}

