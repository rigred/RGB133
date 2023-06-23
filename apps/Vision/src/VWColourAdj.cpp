
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWColourAdj.h"
#include "VWColourBalDialog.h"

VisionWinColourAdjVid::VisionWinColourAdjVid(VWData* data, QWidget *parent)
    : p(parent), m_pData(data)
{
   QString str;

   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pBrightnessScrollBar = new QScrollBar(Qt::Horizontal);
   m_pBrightnessScrollBar->setMinimum(m_pData->getColAdjBrightnessMin());
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightness());
   m_pBrightnessScrollBar->setMaximum(m_pData->getColAdjBrightnessMax());
   m_pBrightnessValue = new QLabel(str.setNum(m_pData->getColAdjBrightness()));
   m_pBrightnessValue->setMaximumWidth(40);

   connect(m_pBrightnessScrollBar, SIGNAL(valueChanged(int)),
         m_pBrightnessValue, SLOT(setNum(int)));
   connect(m_pBrightnessScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(brightnessChanged(int)));

   m_pContrastScrollBar = new QScrollBar(Qt::Horizontal);
   m_pContrastScrollBar->setMinimum(m_pData->getColAdjContrastMin());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrast());
   m_pContrastScrollBar->setMaximum(m_pData->getColAdjContrastMax());
   m_pContrastValue = new QLabel(str.setNum(m_pData->getColAdjContrast()));
   m_pContrastValue->setMaximumWidth(40);

   connect(m_pContrastScrollBar, SIGNAL(valueChanged(int)),
         m_pContrastValue, SLOT(setNum(int)));
   connect(m_pContrastScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(contrastChanged(int)));

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_VIDEO:
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         m_pBrightnessScrollBar->setDisabled(false);
         m_pContrastScrollBar->setDisabled(false);
         break;
      case VW_TYPE_NOSIGNAL:
         setDisabled(true);
         setVisible(false);
         break;
      case VW_TYPE_DVI:
      default:
         setDisabled(false);
         break;
   }

   QFormLayout* formLayout = new QFormLayout;
   formLayout->addRow(tr("Brightness"), m_pBrightnessScrollBar);
   formLayout->addRow(tr("Contrast"), m_pContrastScrollBar);

   QVBoxLayout* valueLayout = new QVBoxLayout;
   valueLayout->addWidget(m_pBrightnessValue);
   valueLayout->addWidget(m_pContrastValue);

   QHBoxLayout* combLayout = new QHBoxLayout;
   combLayout->addLayout(formLayout);
   combLayout->addLayout(valueLayout);

   QVBoxLayout* layout = new QVBoxLayout;
   layout->addLayout(combLayout);
   layout->addSpacing(64);
   setLayout(layout);
}

void VisionWinColourAdjVid::vidTypeChanged(int type)
{
   QString str;

   m_pBrightnessScrollBar->setMinimum(m_pData->getColAdjBrightnessMin());
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightness());
   m_pBrightnessScrollBar->setMaximum(m_pData->getColAdjBrightnessMax());
   m_pBrightnessValue->setNum(m_pData->getColAdjBrightness());

   m_pContrastScrollBar->setMinimum(m_pData->getColAdjContrastMin());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrast());
   m_pContrastScrollBar->setMaximum(m_pData->getColAdjContrastMax());
   m_pContrastValue->setNum(m_pData->getColAdjContrast());

   switch(type)
   {
      case VW_TYPE_VIDEO:
      case VW_TYPE_DVI:
         setDisabled(false);
         m_pBrightnessScrollBar->setDisabled(false);
         m_pContrastScrollBar->setDisabled(false);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         break;
         break;
   }
}

void VisionWinColourAdjVid::setDefaults( )
{
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightnessDef());
   m_pBrightnessValue->setNum(m_pData->getColAdjBrightnessDef());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrastDef());
   m_pContrastValue->setNum(m_pData->getColAdjContrastDef());
}

VisionWinColourAdjVidExt::VisionWinColourAdjVidExt(VWData* data, QWidget *parent)
    : p(parent), m_pData(data)
{
   QString str;

   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pBrightnessScrollBar = new QScrollBar(Qt::Horizontal);
   m_pBrightnessScrollBar->setMinimum(m_pData->getColAdjBrightnessMin());
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightness());
   m_pBrightnessScrollBar->setMaximum(m_pData->getColAdjBrightnessMax());
   m_pBrightnessValue = new QLabel(str.setNum(m_pData->getColAdjBrightness()));
   m_pBrightnessValue->setMaximumWidth(40);

   connect(m_pBrightnessScrollBar, SIGNAL(valueChanged(int)),
         m_pBrightnessValue, SLOT(setNum(int)));
   connect(m_pBrightnessScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(brightnessChanged(int)));

   m_pContrastScrollBar = new QScrollBar(Qt::Horizontal);
   m_pContrastScrollBar->setMinimum(m_pData->getColAdjContrastMin());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrast());
   m_pContrastScrollBar->setMaximum(m_pData->getColAdjContrastMax());
   m_pContrastValue = new QLabel(str.setNum(m_pData->getColAdjContrast()));
   m_pContrastValue->setMaximumWidth(40);

   connect(m_pContrastScrollBar, SIGNAL(valueChanged(int)),
         m_pContrastValue, SLOT(setNum(int)));
   connect(m_pContrastScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(contrastChanged(int)));

   m_pSaturationScrollBar = new QScrollBar(Qt::Horizontal);
   m_pSaturationScrollBar->setMinimum(m_pData->getColAdjSaturationMin());
   m_pSaturationScrollBar->setValue(m_pData->getColAdjSaturation());
   m_pSaturationScrollBar->setMaximum(m_pData->getColAdjSaturationMax());
   m_pSaturationValue = new QLabel(str.setNum(m_pData->getColAdjSaturation()));
   m_pSaturationValue->setMaximumWidth(40);

   connect(m_pSaturationScrollBar, SIGNAL(valueChanged(int)),
         m_pSaturationValue, SLOT(setNum(int)));
   connect(m_pSaturationScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(saturationChanged(int)));

   m_pHueScrollBar = new QScrollBar(Qt::Horizontal);
   m_pHueScrollBar->setMinimum(m_pData->getColAdjHueMin());
   m_pHueScrollBar->setValue(m_pData->getColAdjHue());
   m_pHueScrollBar->setMaximum(m_pData->getColAdjHueMax());
   m_pHueValue = new QLabel(str.setNum(m_pData->getColAdjHue()));
   m_pHueValue->setMaximumWidth(40);

   connect(m_pHueScrollBar, SIGNAL(valueChanged(int)),
         m_pHueValue, SLOT(setNum(int)));
   connect(m_pHueScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(hueChanged(int)));

   QFormLayout* formLayout = new QFormLayout;
   formLayout->addRow(tr("Brightness"), m_pBrightnessScrollBar);
   formLayout->addRow(tr("Contrast"), m_pContrastScrollBar);
   formLayout->addRow(tr("Saturation"), m_pSaturationScrollBar);
   formLayout->addRow(tr("Hue"), m_pHueScrollBar);

   QVBoxLayout* valueLayout = new QVBoxLayout;
   valueLayout->setAlignment(Qt::AlignTop);
   valueLayout->addWidget(m_pBrightnessValue);
   valueLayout->addWidget(m_pContrastValue);
   valueLayout->addWidget(m_pSaturationValue);
   valueLayout->addWidget(m_pHueValue);

   QHBoxLayout* combLayout = new QHBoxLayout;
   combLayout->addLayout(formLayout);
   combLayout->addLayout(valueLayout);

   QVBoxLayout* layout = new QVBoxLayout;
   layout->addLayout(combLayout);
   setLayout(layout);
}

void VisionWinColourAdjVidExt::vidTypeChanged(int type)
{
   QString str;

   m_pBrightnessScrollBar->setMinimum(m_pData->getColAdjBrightnessMin());
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightness());
   m_pBrightnessScrollBar->setMaximum(m_pData->getColAdjBrightnessMax());
   m_pBrightnessValue->setNum(m_pData->getColAdjBrightness());

   m_pContrastScrollBar->setMinimum(m_pData->getColAdjContrastMin());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrast());
   m_pContrastScrollBar->setMaximum(m_pData->getColAdjContrastMax());
   m_pContrastValue->setNum(m_pData->getColAdjContrast());

   switch(type)
   {
      case VW_TYPE_VIDEO:
      case VW_TYPE_DVI:
         setDisabled(false);
         m_pBrightnessScrollBar->setDisabled(false);
         m_pContrastScrollBar->setDisabled(false);
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         m_pSaturationScrollBar->setMinimum(m_pData->getColAdjSaturationMin());
         m_pSaturationScrollBar->setValue(m_pData->getColAdjSaturation());
         m_pSaturationScrollBar->setMaximum(m_pData->getColAdjSaturationMax());
         m_pSaturationValue->setNum(m_pData->getColAdjSaturation());

         m_pHueScrollBar->setMinimum(m_pData->getColAdjHueMin());
         m_pHueScrollBar->setValue(m_pData->getColAdjHue());
         m_pHueScrollBar->setMaximum(m_pData->getColAdjHueMax());
         m_pHueValue->setNum(m_pData->getColAdjHue());

         m_pBrightnessScrollBar->setDisabled(false);
         m_pContrastScrollBar->setDisabled(false);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         break;
   }
}

void VisionWinColourAdjVidExt::setDefaults( )
{
   m_pBrightnessScrollBar->setValue(m_pData->getColAdjBrightnessDef());
   m_pBrightnessValue->setNum(m_pData->getColAdjBrightnessDef());
   m_pContrastScrollBar->setValue(m_pData->getColAdjContrastDef());
   m_pContrastValue->setNum(m_pData->getColAdjContrastDef());
   m_pSaturationScrollBar->setValue(m_pData->getColAdjSaturationDef());
   m_pSaturationValue->setNum(m_pData->getColAdjSaturationDef());
   m_pHueScrollBar->setValue(m_pData->getColAdjHueDef());
   m_pHueValue->setNum(m_pData->getColAdjHueDef());
}

VisionWinColourAdj::VisionWinColourAdj(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
    QString str;
    
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_pVidAdj = new VisionWinColourAdjVid(m_pData, this);
    m_pVidAdjExt = new VisionWinColourAdjVidExt(m_pData, this);

    connect(m_pData, SIGNAL(inputPropertiesReset()), this, SLOT(inputPropertiesReset()));
    connect(this, SIGNAL(vidTypeChanged(int)), m_pVidAdj, SLOT(vidTypeChanged(int)));
    connect(this, SIGNAL(vidTypeChanged(int)), m_pVidAdjExt, SLOT(vidTypeChanged(int)));

    m_pStack = new QStackedWidget;
    m_pStack->addWidget(m_pVidAdj);
    m_pStack->addWidget(m_pVidAdjExt);
    m_pStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(this, SIGNAL(stackChanged(int)),
          m_pStack, SLOT(setCurrentIndex(int)));

    m_pButton = new QPushButton(tr("Colour Balance..."));
    m_pButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(m_pButton, SIGNAL(pressed(void)),
          this, SLOT(pressed()));

    switch(m_pData->getSignalType())
    {
       case VW_TYPE_DVI:
          setDisabled(false);
          m_pStack->setCurrentIndex(VW_COL_ADJ_STACK_DVI_VID);
          m_pButton->setDisabled(true);
          emit vidTypeChanged(m_pData->getSignalType());
          break;
       case VW_TYPE_VIDEO:
          setDisabled(false);
          m_pStack->setCurrentIndex(VW_COL_ADJ_STACK_DVI_VID);
          m_pButton->setDisabled(false);
          emit vidTypeChanged(m_pData->getSignalType());
          break;
       case VW_TYPE_3WIRE_SOG:
       case VW_TYPE_4WIRE_COMPOSITE_SYNC:
       case VW_TYPE_5WIRE_SEPARATE_SYNCS:
       case VW_TYPE_YPRPB:
       case VW_TYPE_CVBS:
       case VW_TYPE_YC:
          setDisabled(false);
          m_pStack->setCurrentIndex(VW_COL_ADJ_STACK_ANALOG);
          m_pButton->setDisabled(false);
          emit vidTypeChanged(m_pData->getSignalType());
          break;
       case VW_TYPE_NOSIGNAL:
       default:
          setDisabled(true);
          setVisible(false);
          m_pStack->setCurrentIndex(VW_COL_ADJ_STACK_NO_SIG);
          emit vidTypeChanged(m_pData->getSignalType());
          break;
    }

    m_pDialog = new VisionWinColourBalDialog("Colour Balance", m_pData);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_pButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_pStack);
    layout->addLayout(buttonLayout);
    setLayout(layout);
}

void VisionWinColourAdj::pressed(void)
{
   m_pDialog->exec();
}

// Public Slots
void VisionWinColourAdj::inputPropertiesReset( )
{
   if(m_pStack->currentIndex() == VW_COL_ADJ_STACK_DVI_VID ||
      m_pStack->currentIndex() == VW_COL_ADJ_STACK_NO_SIG)
      m_pVidAdj->setDefaults();
   else
      m_pVidAdjExt->setDefaults();
}

void VisionWinColourAdj::colAdjTypeChanged(void)
{
   switch(m_pData->getSignalType())
   {
      case VW_TYPE_DVI:
         setDisabled(false);
         setVisible(true);
         m_pButton->setDisabled(true);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(0);
         break;
      case VW_TYPE_VIDEO:
         setDisabled(false);
         setVisible(true);
         m_pButton->setDisabled(false);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(0);
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         setVisible(true);
         m_pButton->setDisabled(false);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(1);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         setVisible(false);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(0);
         break;
   }
}

void VisionWinColourAdj::brightnessChanged(int value)
{
   m_pData->setBrightness(value, false);
   m_pData->setModified(VW_MOD_COLOUR_ADJ);
}

void VisionWinColourAdj::contrastChanged(int value)
{
   m_pData->setContrast(value, false);
   m_pData->setModified(VW_MOD_COLOUR_ADJ);
}

void VisionWinColourAdj::saturationChanged(int value)
{
   m_pData->setSaturation(value, false);
   m_pData->setModified(VW_MOD_COLOUR_ADJ);
}

void VisionWinColourAdj::hueChanged(int value)
{
   m_pData->setHue(value, false);
   m_pData->setModified(VW_MOD_COLOUR_ADJ);
}



