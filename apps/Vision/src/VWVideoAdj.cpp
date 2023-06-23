
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWVideoAdj.h"

const char* ccVWVidStdStr[VW_VIDSTD_NUM_ITEMS] =
   { "UNKNOWN",
     "NTSC-M", "NTSC-J", "NTSC 4:3 50Hz", "NTSC 4:3 60Hz",
     "PAL-I", "PAL-M", "PAL-NC", "PAL 4:3 60Hz",
     "SECAM-L"
   };

void VisionWinVideoAdjBase::setScrollBarValues(QScrollBar *pScrollBar,
      int min, int val, int max)
{
   int cmin = pScrollBar->minimum();
   int cval = pScrollBar->value();
   int cmax = pScrollBar->maximum();

   if(max >= cmax)
   {
      // Set max, val, min
      pScrollBar->setMaximum(max);
      pScrollBar->setValue(val);
      pScrollBar->setMinimum(min);
   }
   else
   {
      if(min <= cmin)
      {
         // Set min, val, max
         pScrollBar->setMinimum(min);
         pScrollBar->setValue(val);
         pScrollBar->setMaximum(max);
      }
      else
      {
         if( (min >= cval && val <= max) ||
             (max <= cval && val >= min) )
         {
            // Set val, min, max
            pScrollBar->setValue(val);
            pScrollBar->setMinimum(min);
            pScrollBar->setMaximum(max);
         }
         else
         {
            printf("VisionWinVideoAdjBase::setScrollBarValues: failed new(%d,%d,%d) - old(%d,%d,%d)\n",
                  min, val, max, cmin, cval, cmax);
         }
      }
   }
}

VisionWinVideoAdj::VisionWinVideoAdj(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

   m_index = 0;

   m_pDvi = new VisionWinVideoAdjDvi;
   m_pVga = new VisionWinVideoAdjVGA(m_pData, this);
   m_pVid = new VisionWinVideoAdjVideo(m_pData, this);
   m_pNosignal = new VisionWinVideoAdjNoSignal;

   m_pStack = new QStackedWidget;
   m_pStack->addWidget(m_pDvi);
   m_pStack->addWidget(m_pVga);
   m_pStack->addWidget(m_pVid);
   m_pStack->addWidget(m_pNosignal);

   connect(m_pData, SIGNAL(inputPropertiesReset()), this, SLOT(inputPropertiesReset()));
   connect(this, SIGNAL(stackChanged(int)), m_pStack, SLOT(setCurrentIndex(int)));
   connect(this, SIGNAL(vidTypeChanged(int)), m_pVga, SLOT(vidTypeChanged(int)));
   connect(this, SIGNAL(vidTypeChanged(int)), m_pVid, SLOT(vidTypeChanged(int)));

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_DVI:
         setDisabled(false);
         m_pStack->setCurrentIndex(0);
         break;
      case VW_TYPE_VIDEO:
         setDisabled(false);
         m_pStack->setCurrentIndex(1);
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         m_pStack->setCurrentIndex(2);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         setVisible(false);
         m_pStack->setCurrentIndex(3);
         break;
   }

   QVBoxLayout* layout = new QVBoxLayout;
   layout->addWidget(m_pStack);
   setLayout(layout);
}

VisionWinVideoAdjDvi::VisionWinVideoAdjDvi(void)
{
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pDviMessage = new QLabel(tr("There are no Video Adjustments available for DVI\\SDI modes"));
   m_pDviMessage->setWordWrap(true);

   QHBoxLayout* layout = new QHBoxLayout;
   layout->addWidget(m_pDviMessage);
   setLayout(layout);
}

VisionWinVideoAdjNoSignal::VisionWinVideoAdjNoSignal(void)
{
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pNoSignalMessage = new QLabel(tr("There are no Video Adjustments available: No Signal"));
   m_pNoSignalMessage->setWordWrap(true);

   QHBoxLayout* layout = new QHBoxLayout;
   layout->addWidget(m_pNoSignalMessage);
   setLayout(layout);
}

VisionWinVideoAdjVGA::VisionWinVideoAdjVGA(VWData* data, QWidget* parent)
   : p(parent), m_pData(data)
{
   QString str;
    
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

   m_pHorPosScrollBar = new QScrollBar(Qt::Horizontal);
   m_pHorPosScrollBar->setMinimum(m_pData->getVidAdjHorPosMin());
   m_pHorPosScrollBar->setMaximum(m_pData->getVidAdjHorPosMax());
   m_pHorPosScrollBar->setValue(m_pData->getVidAdjHorPos());
   m_pHorPosValue = new QLabel(str.setNum(m_pData->getVidAdjHorPos()));
   m_pHorPosValue->setMaximumWidth(40);

   connect(m_pHorPosScrollBar, SIGNAL(valueChanged(int)),
         m_pHorPosValue, SLOT(setNum(int)));
   connect(m_pHorPosScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(horPosChanged(int)));

   m_pHorSizeScrollBar = new QScrollBar(Qt::Horizontal);
   m_pHorSizeScrollBar->setMinimum(m_pData->getVidAdjHorSizeMin());
   m_pHorSizeScrollBar->setMaximum(m_pData->getVidAdjHorSizeMax());
   m_pHorSizeScrollBar->setValue(m_pData->getVidAdjHorSize());
   m_pHorSizeValue = new QLabel(str.setNum(m_pData->getVidAdjHorSize()));
   m_pHorSizeValue->setMaximumWidth(40);

   connect(m_pHorSizeScrollBar, SIGNAL(valueChanged(int)),
         m_pHorSizeValue, SLOT(setNum(int)));
   connect(m_pHorSizeScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(horSizeChanged(int)));

   m_pPhaseScrollBar = new QScrollBar(Qt::Horizontal);
   m_pPhaseScrollBar->setMinimum(m_pData->getVidAdjPhaseMin());
   m_pPhaseScrollBar->setMaximum(m_pData->getVidAdjPhaseMax());
   m_pPhaseScrollBar->setValue(m_pData->getVidAdjPhase());
   m_pPhaseValue = new QLabel(str.setNum(m_pData->getVidAdjPhase()));
   m_pPhaseValue->setMaximumWidth(40);

   connect(m_pPhaseScrollBar, SIGNAL(valueChanged(int)),
         m_pPhaseValue, SLOT(setNum(int)));
   connect(m_pPhaseScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(phaseChanged(int)));

   m_pVertPosScrollBar = new QScrollBar(Qt::Horizontal);
   m_pVertPosScrollBar->setMinimum(m_pData->getVidAdjVertPosMin());
   m_pVertPosScrollBar->setMaximum(m_pData->getVidAdjVertPosMax());
   m_pVertPosScrollBar->setValue(m_pData->getVidAdjVertPos());
   m_pVertPosValue = new QLabel(str.setNum(m_pData->getVidAdjVertPos()));
   m_pVertPosValue->setMaximumWidth(40);

   connect(m_pVertPosScrollBar, SIGNAL(valueChanged(int)),
         m_pVertPosValue, SLOT(setNum(int)));
   connect(m_pVertPosScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(vertPosChanged(int)));

   m_pBlackLevelScrollBar = new QScrollBar(Qt::Horizontal);
   m_pBlackLevelScrollBar->setMinimum(m_pData->getVidAdjBlackLevelMin());
   m_pBlackLevelScrollBar->setMaximum(m_pData->getVidAdjBlackLevelMax());
   m_pBlackLevelScrollBar->setValue(m_pData->getVidAdjBlackLevel());
   m_pBlackLevelValue = new QLabel(str.setNum(m_pData->getVidAdjBlackLevel()));
   m_pBlackLevelValue->setMaximumWidth(40);

   connect(m_pBlackLevelScrollBar, SIGNAL(valueChanged(int)),
         m_pBlackLevelValue, SLOT(setNum(int)));
   connect(m_pBlackLevelScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(blacklevelChanged(int)));

   QFormLayout* formLayout = new QFormLayout;
   formLayout->addRow(tr("Horizontal Position"), m_pHorPosScrollBar);
   formLayout->addRow(tr("Horizontal Size"), m_pHorSizeScrollBar);
   formLayout->addRow(tr("Phase"), m_pPhaseScrollBar);
   formLayout->addRow(tr("Vertical Position"), m_pVertPosScrollBar);
   formLayout->addRow(tr("Black Level"), m_pBlackLevelScrollBar);

   QVBoxLayout* valueLayout = new QVBoxLayout;
   valueLayout->setAlignment(Qt::AlignTop);
   valueLayout->addWidget(m_pHorPosValue);
   valueLayout->addWidget(m_pHorSizeValue);
   valueLayout->addWidget(m_pPhaseValue);
   valueLayout->addWidget(m_pVertPosValue);
   valueLayout->addWidget(m_pBlackLevelValue);

   QHBoxLayout* layout = new QHBoxLayout;
   layout->addLayout(formLayout);
   layout->addLayout(valueLayout);
   setLayout(layout);
}

void VisionWinVideoAdjVGA::vidTypeChanged(int type)
{
   QString str;

   switch(type)
   {
      case VW_TYPE_VIDEO:
         setScrollBarValues(m_pHorPosScrollBar,
               m_pData->getVidAdjHorPosMin(),
               m_pData->getVidAdjHorPos(),
               m_pData->getVidAdjHorPosMax());
         m_pHorPosValue->setNum(m_pData->getVidAdjHorPos());

         setScrollBarValues(m_pHorSizeScrollBar,
               m_pData->getVidAdjHorSizeMin(),
               m_pData->getVidAdjHorSize(),
               m_pData->getVidAdjHorSizeMax());
         m_pHorSizeValue->setNum(m_pData->getVidAdjHorSize());

         setScrollBarValues(m_pPhaseScrollBar,
               m_pData->getVidAdjPhaseMin(),
               m_pData->getVidAdjPhase(),
               m_pData->getVidAdjPhaseMax());
         m_pPhaseValue->setNum(m_pData->getVidAdjPhase());

         setScrollBarValues(m_pVertPosScrollBar,
               m_pData->getVidAdjVertPosMin(),
               m_pData->getVidAdjVertPos(),
               m_pData->getVidAdjVertPosMax());
         m_pVertPosValue->setNum(m_pData->getVidAdjVertPos());

         setScrollBarValues(m_pBlackLevelScrollBar,
               m_pData->getVidAdjBlackLevelMin(),
               m_pData->getVidAdjBlackLevel(),
               m_pData->getVidAdjBlackLevelMax());
         m_pBlackLevelValue->setNum(m_pData->getVidAdjBlackLevel());
         break;
      default:
         break;
   }
}

void VisionWinVideoAdjVGA::setDefaults( )
{
   setScrollBarValues(m_pHorPosScrollBar,
         m_pData->getVidAdjHorPosMin(),
         m_pData->getVidAdjHorPosDef(),
         m_pData->getVidAdjHorPosMax());
   m_pHorPosValue->setNum(m_pData->getVidAdjHorPosDef());

   setScrollBarValues(m_pHorSizeScrollBar,
         m_pData->getVidAdjHorSizeMin(),
         m_pData->getVidAdjHorSizeDef(),
         m_pData->getVidAdjHorSizeMax());
   m_pHorSizeValue->setNum(m_pData->getVidAdjHorSizeDef());

   setScrollBarValues(m_pPhaseScrollBar,
         m_pData->getVidAdjPhaseMin(),
         m_pData->getVidAdjPhaseDef(),
         m_pData->getVidAdjPhaseMax());
   m_pPhaseValue->setNum(m_pData->getVidAdjPhaseDef());

   setScrollBarValues(m_pVertPosScrollBar,
         m_pData->getVidAdjVertPosMin(),
         m_pData->getVidAdjVertPosDef(),
         m_pData->getVidAdjVertPosMax());
   m_pVertPosValue->setNum(m_pData->getVidAdjVertPosDef());

   setScrollBarValues(m_pBlackLevelScrollBar,
         m_pData->getVidAdjBlackLevelMin(),
         m_pData->getVidAdjBlackLevelDef(),
         m_pData->getVidAdjBlackLevelMax());
   m_pBlackLevelValue->setNum(m_pData->getVidAdjBlackLevelDef());
}


VisionWinVideoAdjVideo::VisionWinVideoAdjVideo(VWData* data, QWidget* parent)
   : p(parent), m_pData(data)
{
   QString str;

   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

   m_pCbLabel = new QLabel(tr("Video Standard"));
   m_pCbLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pComboBox = new QComboBox;
   m_pComboBox->addItem(QString(ccVWVidStdStr[m_pData->getVideoStd()]));

   m_pComboBox->setMaxVisibleItems(1);
   m_pComboBox->setMinimumWidth(100);
   m_pComboBox->setMaximumWidth(100);
   m_pComboBox->setDisabled(true);
   m_pComboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pHorPosScrollBar = new QScrollBar(Qt::Horizontal);
   m_pHorPosScrollBar->setMinimum(m_pData->getVidAdjHorPosMin());
   m_pHorPosScrollBar->setMaximum(m_pData->getVidAdjHorPosMax());
   m_pHorPosScrollBar->setValue(m_pData->getVidAdjHorPos());
   m_pHorPosValue = new QLabel(str.setNum(m_pData->getVidAdjHorPos()));
   m_pHorPosValue->setMaximumWidth(40);

   connect(m_pHorPosScrollBar, SIGNAL(valueChanged(int)),
         m_pHorPosValue, SLOT(setNum(int)));
   connect(m_pHorPosScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(horPosChanged(int)));

   m_pVertPosScrollBar = new QScrollBar(Qt::Horizontal);
   m_pVertPosScrollBar->setMinimum(m_pData->getVidAdjVertPosMin());
   m_pVertPosScrollBar->setMaximum(m_pData->getVidAdjVertPosMax());
   m_pVertPosScrollBar->setValue(m_pData->getVidAdjVertPos());
   m_pVertPosValue = new QLabel(str.setNum(m_pData->getVidAdjVertPos()));
   m_pVertPosValue->setMaximumWidth(40);

   connect(m_pVertPosScrollBar, SIGNAL(valueChanged(int)),
         m_pVertPosValue, SLOT(setNum(int)));
   connect(m_pVertPosScrollBar, SIGNAL(valueChanged(int)),
         p, SLOT(vertPosChanged(int)));

   QFormLayout* formLayout = new QFormLayout;
   formLayout->addRow(tr("Video Standard"), m_pComboBox);
   formLayout->addRow(tr("Horizontal Position"), m_pHorPosScrollBar);
   formLayout->addRow(tr("Vertical Position"), m_pVertPosScrollBar);

   QVBoxLayout* valueLayout = new QVBoxLayout;
   valueLayout->addSpacing(30);
   valueLayout->setAlignment(Qt::AlignTop);
   valueLayout->addWidget(m_pHorPosValue);
   valueLayout->addWidget(m_pVertPosValue);

   QHBoxLayout* layout = new QHBoxLayout;
   layout->addLayout(formLayout);
   layout->addLayout(valueLayout);
   setLayout(layout);
}

void VisionWinVideoAdjVideo::setDefaults( )
{
   setScrollBarValues(m_pHorPosScrollBar,
         m_pData->getVidAdjHorPosMin(),
         m_pData->getVidAdjHorPos(),
         m_pData->getVidAdjHorPosMax());
   m_pHorPosValue->setNum(m_pData->getVidAdjHorPos());

   setScrollBarValues(m_pVertPosScrollBar,
         m_pData->getVidAdjVertPosMin(),
         m_pData->getVidAdjVertPos(),
         m_pData->getVidAdjVertPosMax());
   m_pVertPosValue->setNum(m_pData->getVidAdjVertPos());
}

void VisionWinVideoAdjVideo::vidTypeChanged(int type)
{
   QString str;

   switch(type)
   {
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         m_pComboBox->setItemText(0, QString(ccVWVidStdStr[m_pData->getVideoStd()]));

         setScrollBarValues(m_pHorPosScrollBar,
               m_pData->getVidAdjHorPosMin(),
               m_pData->getVidAdjHorPos(),
               m_pData->getVidAdjHorPosMax());
         m_pHorPosValue->setNum(m_pData->getVidAdjHorPos());

         setScrollBarValues(m_pVertPosScrollBar,
               m_pData->getVidAdjVertPosMin(),
               m_pData->getVidAdjVertPos(),
               m_pData->getVidAdjVertPosMax());
         m_pVertPosValue->setNum(m_pData->getVidAdjVertPos());
         break;
      default:
         break;
   }
}

// Public Slots
void VisionWinVideoAdj::inputPropertiesReset( )
{
   if(m_pStack->currentIndex() == VW_VID_ADJ_VIDEO)
   {
      m_pVga->setDefaults();
   }
   else if(m_pStack->currentIndex() == VW_VID_ADJ_ANALOG)
   {
      m_pVid->setDefaults();
   }
}
void VisionWinVideoAdj::vidAdjTypeChanged()
{
   switch(m_pData->getSignalType())
   {
      case VW_TYPE_DVI:
         setDisabled(false);
         setVisible(true);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(VW_VID_ADJ_DVI);
         break;
      case VW_TYPE_VIDEO:
         setDisabled(false);
         setVisible(true);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(VW_VID_ADJ_VIDEO);
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         setVisible(true);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(VW_VID_ADJ_ANALOG);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         setVisible(false);
         emit vidTypeChanged(m_pData->getSignalType());
         emit stackChanged(VW_VID_ADJ_NO_SIG);
         break;
   }
}

void VisionWinVideoAdj::horPosChanged(int value)
{
   m_pData->setHorPos(value, false);
   m_pData->setModified(VW_MOD_VID_ADJ);
}

void VisionWinVideoAdj::horSizeChanged(int value)
{
   m_pData->setHorSize(value, false);
   m_pData->setModified(VW_MOD_VID_ADJ);
}

void VisionWinVideoAdj::phaseChanged(int value)
{
   m_pData->setPhase(value, false);
   m_pData->setModified(VW_MOD_VID_ADJ);
}

void VisionWinVideoAdj::vertPosChanged(int value)
{
   m_pData->setVertPos(value, false);
   m_pData->setModified(VW_MOD_VID_ADJ);
}

void VisionWinVideoAdj::blacklevelChanged(int value)
{
   m_pData->setBlackLevel(value, false);
   m_pData->setModified(VW_MOD_VID_ADJ);
}
