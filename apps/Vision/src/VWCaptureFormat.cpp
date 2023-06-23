
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWCaptureFormat.h"

VisionWinCaptureFormat::VisionWinCaptureFormat(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
    setAlignment(Qt::AlignLeft);
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    radioButtonAuto = new QRadioButton(tr("Automatic"));
    radioButtonAuto->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    radioButton555 = new QRadioButton(tr("5-5-5"));
    radioButton555->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    radioButton565 = new QRadioButton(tr("5-6-5"));
    radioButton565->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    radioButton888 = new QRadioButton(tr("8-8-8"));
    radioButton888->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    switch(m_pData->getPixelFormat())
    {
       case VW_PIX_FMT_AUTO:
          radioButtonAuto->setChecked(true);
          break;
       case VW_PIX_FMT_RGB555:
          radioButton555->setChecked(true);
          break;
       case VW_PIX_FMT_RGB565:
          radioButton565->setChecked(true);
          break;
       case VW_PIX_FMT_RGB888:
          radioButton888->setChecked(true);
          break;
       case VW_PIX_FMT_YUY2:
       default:
          break;
    }

    connect(radioButtonAuto, SIGNAL(toggled(bool)),
          this, SLOT(setPixFmt()));
    connect(radioButton555, SIGNAL(toggled(bool)),
          this, SLOT(setPixFmt()));
    connect(radioButton565, SIGNAL(toggled(bool)),
          this, SLOT(setPixFmt()));
    connect(radioButton888, SIGNAL(toggled(bool)),
          this, SLOT(setPixFmt()));

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *capFormatLayout = new QBoxLayout(direction);
    capFormatLayout->setSpacing(5);
    capFormatLayout->setAlignment(Qt::AlignTop);
    capFormatLayout->setContentsMargins(10, 0, 10, 1);
    capFormatLayout->addWidget(radioButtonAuto);
    capFormatLayout->addSpacing(25);
    capFormatLayout->addWidget(radioButton555);
    capFormatLayout->addWidget(radioButton565);
    capFormatLayout->addWidget(radioButton888);
    setLayout(capFormatLayout);
}

void VisionWinCaptureFormat::setPixFmt( )
{
   if(radioButtonAuto->isChecked())
   {
      m_pData->setPixelFormat(VW_PIX_FMT_AUTO);
   }
   else if(radioButton555->isChecked())
   {
      m_pData->setPixelFormat(VW_PIX_FMT_RGB555);
      m_pData->setModified(VW_MOD_PIXEL_FORMAT);
   }
   else if(radioButton565->isChecked())
   {
      m_pData->setPixelFormat(VW_PIX_FMT_RGB565);
      m_pData->setModified(VW_MOD_PIXEL_FORMAT);
   }
   else if(radioButton888->isChecked())
   {
      m_pData->setPixelFormat(VW_PIX_FMT_RGB888);
      m_pData->setModified(VW_MOD_PIXEL_FORMAT);
   }
   else
      return;
   m_pData->setModified(VW_MOD_PIXEL_FORMAT);
}

