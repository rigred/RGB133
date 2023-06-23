
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWAspectRatio.h"

VisionWinAspectRatio::VisionWinAspectRatio(const QString &title, QWidget *parent, VWData* data)
    : QGroupBox(title, parent), m_pData(data)
{
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    radioButtonNoMaintain = new QRadioButton(tr("Do not maintain aspect ratio"));
    radioButtonNoMaintain->setChecked(true);
    radioButtonNoMaintain->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    radioButtonMaintainSrc = new QRadioButton(tr("Maintain aspect ratio of source"));
    radioButtonMaintainSrc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    radioButtonMaintainFixed = new QRadioButton(tr("Maintain aspect ratio"));
    radioButtonMaintainFixed->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    connect(radioButtonNoMaintain, SIGNAL(toggled(bool)),
          this, SLOT(aspectRatioTypeChanged()));
    connect(radioButtonMaintainSrc, SIGNAL(toggled(bool)),
          this, SLOT(aspectRatioTypeChanged()));
    connect(radioButtonMaintainFixed, SIGNAL(toggled(bool)),
          this, SLOT(aspectRatioTypeChanged()));

    widthRatioSpinBox = new QSpinBox;
    widthRatioSpinBox->setRange(0, 1920);
    widthRatioSpinBox->setSingleStep(1);
    widthRatioSpinBox->setValue(4);
    widthRatioSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    widthRatioSpinBox->setAccelerated(true);

    heightRatioSpinBox = new QSpinBox;
    heightRatioSpinBox->setRange(0, 1200);
    heightRatioSpinBox->setSingleStep(1);
    heightRatioSpinBox->setValue(3);
    heightRatioSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    heightRatioSpinBox->setAccelerated(true);
   
    ratioDividerLabel = new QLabel(tr(":"));
    ratioDividerLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *maintainFixedLayout = new QBoxLayout(direction);
    maintainFixedLayout->setAlignment(Qt::AlignLeft);
    maintainFixedLayout->setSpacing(5);
    maintainFixedLayout->addWidget(radioButtonMaintainFixed);
    maintainFixedLayout->addWidget(widthRatioSpinBox);
    maintainFixedLayout->addWidget(ratioDividerLabel);
    maintainFixedLayout->addWidget(heightRatioSpinBox);
    maintainFixedLayout->setAlignment(Qt::AlignLeft);

    direction = QBoxLayout::TopToBottom;
    QBoxLayout *aspectRatioLayout = new QBoxLayout(direction);
    aspectRatioLayout->setSpacing(1);
    aspectRatioLayout->setAlignment(Qt::AlignVCenter);
    aspectRatioLayout->setContentsMargins(10, 0, 10, 1);
    aspectRatioLayout->addWidget(radioButtonNoMaintain);
    aspectRatioLayout->addWidget(radioButtonMaintainSrc);
    aspectRatioLayout->addLayout(maintainFixedLayout);
    setLayout(aspectRatioLayout);
}

void VisionWinAspectRatio::aspectRatioTypeChanged( )
{
   if(radioButtonNoMaintain->isChecked())
      m_pData->setAspectRatioType(VW_DO_NOT_MAINTAIN_AR);
   else if(radioButtonMaintainSrc->isChecked())
      m_pData->setAspectRatioType(VW_MAINTAIN_SRC_AR);
   else if(radioButtonMaintainFixed->isChecked())
      m_pData->setAspectRatioType(VW_MAINTAIN_SPECIFIC_AR);
   else
      return;
   m_pData->setModified(VW_MOD_ASPECT_RATIO);
}

float VisionWinAspectRatio::getSpecificAR( )
{
   float ret = (float)((float)widthRatioSpinBox->value() / (float)heightRatioSpinBox->value());
   return ret;
}
