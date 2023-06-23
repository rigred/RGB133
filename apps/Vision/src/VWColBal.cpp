
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWColBal.h"

VisionWinColBal::VisionWinColBal(const QString &title, eVWColBal col, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
   QString str;
   eVWColBalType brightnessType = VW_COL_BAL_UNKNOWN;
   eVWColBalType contrastType = VW_COL_BAL_UNKNOWN;

   switch(col)
   {
      case VW_COL_BAL_RED:
         brightnessType = VW_COL_BAL_RED_BRIGHTNESS;
         contrastType = VW_COL_BAL_RED_CONTRAST;
         break;
      case VW_COL_BAL_GREEN:
         brightnessType = VW_COL_BAL_GREEN_BRIGHTNESS;
         contrastType = VW_COL_BAL_GREEN_CONTRAST;
         break;
      case VW_COL_BAL_BLUE:
         brightnessType = VW_COL_BAL_BLUE_BRIGHTNESS;
         contrastType = VW_COL_BAL_BLUE_CONTRAST;
         break;
      default:
         break;
   }
    
   m_pBrightnessScrollBar = new QScrollBar(Qt::Horizontal);
   m_pBrightnessScrollBar->setMinimum(m_pData->getColBalBrightnessMin(brightnessType));
   m_pBrightnessScrollBar->setMaximum(m_pData->getColBalBrightnessMax(brightnessType));
   m_pBrightnessScrollBar->setValue(m_pData->getColBalBrightness(brightnessType));
   m_pBrightnessValue = new QLabel(str.setNum(m_pData->getColBalBrightness(brightnessType)));
   m_pBrightnessValue->setMaximumWidth(40);
    
   connect(m_pBrightnessScrollBar, SIGNAL(valueChanged(int)),
         m_pBrightnessValue, SLOT(setNum(int)));
    
   m_pContrastScrollBar = new QScrollBar(Qt::Horizontal);
   m_pContrastScrollBar->setMinimum(m_pData->getColBalContrastMin(contrastType));
   m_pContrastScrollBar->setMaximum(m_pData->getColBalContrastMax(contrastType));
   m_pContrastScrollBar->setValue(m_pData->getColBalContrast(contrastType));
   m_pContrastValue = new QLabel(str.setNum(m_pData->getColBalContrast(contrastType)));
   m_pContrastValue->setMaximumWidth(40);
    
   connect(m_pContrastScrollBar, SIGNAL(valueChanged(int)),
         m_pContrastValue, SLOT(setNum(int)));
    
   QFormLayout* formLayout = new QFormLayout;
   formLayout->addRow(tr("Brightness"), m_pBrightnessScrollBar);
   formLayout->addRow(tr("Contrast"), m_pContrastScrollBar);

   QVBoxLayout* valueLayout = new QVBoxLayout;
   valueLayout->addWidget(m_pBrightnessValue);
   valueLayout->addWidget(m_pContrastValue);
 
   QHBoxLayout* combLayout = new QHBoxLayout;
   combLayout->addLayout(formLayout);
   combLayout->addLayout(valueLayout);
   setLayout(combLayout);
}

void VisionWinColBal::rejected(eVWColBalType brType, eVWColBalType ctType)
{
   m_pBrightnessScrollBar->setValue(m_pData->getColBalBrightness(brType));
   m_pBrightnessValue->setNum(m_pData->getColBalBrightness(brType));
   m_pContrastScrollBar->setValue(m_pData->getColBalContrast(ctType));
   m_pContrastValue->setNum(m_pData->getColBalContrast(ctType));
   m_pData->setColBalBrightness(m_pData->getColBalBrightness(brType), brType, true);
   m_pData->setColBalContrast(m_pData->getColBalContrast(ctType), ctType, true);
}

void VisionWinColBal::reset(eVWColBalType brType, eVWColBalType ctType)
{
   m_pBrightnessScrollBar->setValue(m_pData->getColBalBrightnessDef(brType));
   m_pBrightnessValue->setNum(m_pData->getColBalBrightnessDef(brType));
   m_pContrastScrollBar->setValue(m_pData->getColBalContrastDef(ctType));
   m_pContrastValue->setNum(m_pData->getColBalContrastDef(ctType));
}
