
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWCaptureRate.h"

#define NUM_RATES 9
const int rates[NUM_RATES] = { 100, 50, 33, 25, 20, 15, 10, 5, 1 };

VisionWinCaptureRate::VisionWinCaptureRate(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_bDisabled = !m_pData->getInactiveRateEnabled();

   m_pSameRadioButton = new QRadioButton(tr("Use same capture rate when window is inactive"));
   m_pSameRadioButton->setChecked(m_bDisabled);
   m_pSameRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pSpecificRadioButton = new QRadioButton(tr("Capture at"));
   m_pSpecificRadioButton->setChecked(!m_bDisabled);
   m_pSpecificRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   connect(m_pSameRadioButton, SIGNAL(toggled(bool)),
         this, SLOT(enableInactive()));

   m_pRateLabel = new QLabel(tr("Capture at"));
   m_pRateLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pInactiveLabel = new QLabel(tr("when window is inactive"));
   m_pInactiveLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pActiveCapRate = new QComboBox;
   for(int i=0; i<NUM_RATES; i++)
   {
      m_pActiveCapRate->addItem(QString("%1\%").arg(rates[i], 0));
      if(m_pData->getActiveCaptureRate() == rates[i])
         m_pActiveCapRate->setCurrentIndex(i);
   }
   m_pActiveCapRate->setMaxVisibleItems(NUM_RATES);
   m_pActiveCapRate->setMinimumWidth(100);
   m_pActiveCapRate->setMaximumWidth(100);
   m_pActiveCapRate->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   connect(m_pActiveCapRate, SIGNAL(currentIndexChanged(int)),
         this, SLOT(activeRateChanged(int)));

   m_pInactiveCapRate = new QComboBox;
   for(int i=0; i<NUM_RATES; i++)
   {
      m_pInactiveCapRate->addItem(QString("%1\%").arg(rates[i], 0));
      if(m_pData->getInactiveCaptureRate() == rates[i])
         m_pInactiveCapRate->setCurrentIndex(i);
   }
   m_pInactiveCapRate->setMaxVisibleItems(NUM_RATES);
   m_pInactiveCapRate->setMinimumWidth(100);
   m_pInactiveCapRate->setMaximumWidth(100);
   m_pActiveCapRate->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   connect(m_pInactiveCapRate, SIGNAL(currentIndexChanged(int)),
         this, SLOT(inactiveRateChanged(int)));

   if(m_bDisabled)
   {
      m_pInactiveCapRate->setDisabled(true);
   }

   QHBoxLayout *activeLayout = new QHBoxLayout;
   activeLayout->setSpacing(5);
   activeLayout->setAlignment(Qt::AlignHCenter);
   activeLayout->addWidget(m_pRateLabel);
   activeLayout->addWidget(m_pActiveCapRate);

   QHBoxLayout *inactiveLayout = new QHBoxLayout;
   inactiveLayout->setAlignment(Qt::AlignLeft);
   inactiveLayout->addWidget(m_pSpecificRadioButton);
   inactiveLayout->addWidget(m_pInactiveCapRate);
   inactiveLayout->addWidget(m_pInactiveLabel);

   QVBoxLayout *capRateLayout = new QVBoxLayout;
   capRateLayout->setSpacing(1);
   capRateLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   capRateLayout->setContentsMargins(10, 0, 10, 1);
   capRateLayout->addLayout(activeLayout);
   capRateLayout->addWidget(m_pSameRadioButton);
   capRateLayout->addLayout(inactiveLayout);
   setLayout(capRateLayout);
}

int VisionWinCaptureRate::getActiveCaptureRate( )
{
   return rates[m_pActiveCapRate->currentIndex()];
}

int VisionWinCaptureRate::getInactiveCaptureRate( )
{
   return rates[m_pInactiveCapRate->currentIndex()];
}

void VisionWinCaptureRate::enableInactive()
{
   m_bDisabled = !m_bDisabled;
   m_pInactiveCapRate->setDisabled(m_bDisabled);
   m_pData->setInactiveRateEnabled(!m_bDisabled);
   m_pData->setModified(VW_MOD_ENABLE_INACTIVE_RATE);
}

void VisionWinCaptureRate::activeRateChanged(int index)
{
   m_pData->setActiveCaptureRate(rates[index]);
   m_pData->setModified(VW_MOD_CAPTURE_RATE);
}

void VisionWinCaptureRate::inactiveRateChanged(int index)
{
   m_pData->setInactiveCaptureRate(rates[index]);
   m_pData->setModified(VW_MOD_CAPTURE_RATE);
}
