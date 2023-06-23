
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWVidTiming.h"

void VisionWinVidTimingBase::setSpinBoxValues(QSpinBox *pSpinBox,
      int min, int val, int max)
{
   int cmin = pSpinBox->minimum();
   int cval = pSpinBox->value();
   int cmax = pSpinBox->maximum();

   if(max >= cmax)
   {
      // Set max, val, min
      pSpinBox->setMaximum(max);
      pSpinBox->setValue(val);
      pSpinBox->setMinimum(min);
   }
   else
   {
      if(min <= cmin)
      {
         // Set min, val, max
         pSpinBox->setMinimum(min);
         pSpinBox->setValue(val);
         pSpinBox->setMaximum(max);
      }
      else
      {
         if( (min > cval && val < max) ||
             (max < cval && val > min) )
         {
            // Set val, min, max
            pSpinBox->setValue(val);
            pSpinBox->setMinimum(min);
            pSpinBox->setMaximum(max);
         }
         else
         {
            printf("VisionWinVidTimingBase::setSpinBoxValues: failed (%d,%d,%d)\n",
                  min, val, max);
         }
      }
   }
}

VisionWinVidTiming::VisionWinVidTiming(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pWidthLabel = new QLabel(tr("Width"));
   m_pWidthLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pHeightLabel = new QLabel(tr("Height"));
   m_pHeightLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pRefreshLabel = new QLabel(tr("Vertical Refresh"));
   m_pRefreshLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pHzLabel = new QLabel(tr("Hz"));
   m_pHzLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pWidthSpinBox = new QSpinBox;
   m_pWidthSpinBox->setMaximum(m_pData->getVidTimingWidthMax());
   m_pWidthSpinBox->setValue(m_pData->getVidTimingWidth());
   {
      int min = m_pData->getVidTimingWidthMin();

      /* must be word-divisible to set to driver */
      min = (min & 1) != 0 ? min + 1 : min;
      m_pWidthSpinBox->setMinimum(min);
   }
   m_pWidthSpinBox->setSingleStep(2);
   m_pWidthSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pWidthSpinBox->setAccelerated(true);

   m_pHeightSpinBox = new QSpinBox;
   m_pHeightSpinBox->setMaximum(m_pData->getVidTimingHeightMax());
   m_pHeightSpinBox->setValue(m_pData->getVidTimingHeight());
   m_pHeightSpinBox->setMinimum(m_pData->getVidTimingHeightMin());
   m_pHeightSpinBox->setSingleStep(2);
   m_pHeightSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pHeightSpinBox->setAccelerated(true);

   connect(m_pData, SIGNAL(inputPropertiesReset()), this, SLOT(inputPropertiesReset()));
   connect(m_pWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(vidTimingWidthChanged()));
   connect(m_pHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(vidTimingHeightChanged()));

   m_pRefreshLineEdit = new QLineEdit(QString("%1").arg(m_pData->getVidTimingRefresh(), 0, 'f', 2));
   m_pRefreshLineEdit->setReadOnly(true);
   m_pRefreshLineEdit->setAlignment(Qt::AlignLeft);
   m_pRefreshLineEdit->setMaxLength(6);
   m_pRefreshLineEdit->setReadOnly(true);
   m_pRefreshLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_VIDEO:
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         break;
      case VW_TYPE_DVI:
         m_pWidthLabel->setDisabled(true);
         m_pHeightLabel->setDisabled(true);
         m_pWidthSpinBox->setDisabled(true);
         m_pHeightSpinBox->setDisabled(true);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         m_pWidthSpinBox->setValue(0);
         m_pHeightSpinBox->setValue(0);
         m_pRefreshLineEdit->setText(QString("0.00"));
         break;
   }

   QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
   QBoxLayout *row1Layout = new QBoxLayout(direction);
   row1Layout->addWidget(m_pWidthLabel);
   row1Layout->addWidget(m_pWidthSpinBox);
   row1Layout->addWidget(m_pHeightLabel);
   row1Layout->addWidget(m_pHeightSpinBox);

   QBoxLayout *row2Layout = new QBoxLayout(direction);
   row2Layout->addWidget(m_pRefreshLabel);
   row2Layout->addWidget(m_pRefreshLineEdit);
   row2Layout->addWidget(m_pHzLabel);

   direction = QBoxLayout::TopToBottom;
   QBoxLayout *vidTimingLayout = new QBoxLayout(direction);
   vidTimingLayout->addLayout(row1Layout);
   vidTimingLayout->addLayout(row2Layout);
   setLayout(vidTimingLayout);
}

// Private Methods
void VisionWinVidTiming::setDefaults( )
{
   m_pWidthSpinBox->setValue(m_pData->getVidTimingWidthDef());
   m_pHeightSpinBox->setValue(m_pData->getVidTimingHeightDef());
}

// Public Slots
void VisionWinVidTiming::inputPropertiesReset()
{
   this->setDefaults();
}

void VisionWinVidTiming::vidTimingTypeChanged()
{
   switch(m_pData->getSignalType())
   {
      case VW_TYPE_VIDEO:
         m_pWidthSpinBox->setValue(m_pData->getVidTimingWidth());
         m_pHeightSpinBox->setValue(m_pData->getVidTimingHeight());
         m_pWidthLabel->setDisabled(false);
         m_pHeightLabel->setDisabled(false);
         m_pWidthSpinBox->setDisabled(false);
         m_pHeightSpinBox->setDisabled(false);
         m_pRefreshLineEdit->setText(QString("%1").arg(m_pData->getVidTimingRefresh(), 0, 'f', 2));
         break;
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         m_pWidthSpinBox->setValue(m_pData->getVidTimingWidth());
         m_pHeightSpinBox->setValue(m_pData->getVidTimingHeight());
         m_pWidthLabel->setDisabled(true);
         m_pHeightLabel->setDisabled(true);
         m_pWidthSpinBox->setDisabled(true);
         m_pHeightSpinBox->setDisabled(true);
         m_pRefreshLineEdit->setText(QString("%1").arg(m_pData->getVidTimingRefresh(), 0, 'f', 2));
         break;
      case VW_TYPE_DVI:
         m_pWidthSpinBox->setValue(m_pData->getVidTimingWidth());
         m_pHeightSpinBox->setValue(m_pData->getVidTimingHeight());
         m_pWidthLabel->setDisabled(true);
         m_pHeightLabel->setDisabled(true);
         m_pWidthSpinBox->setDisabled(true);
         m_pHeightSpinBox->setDisabled(true);
         m_pRefreshLineEdit->setText(QString("%1").arg(m_pData->getVidTimingRefresh(), 0, 'f', 2));
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         m_pWidthSpinBox->setValue(0);
         m_pHeightSpinBox->setValue(0);
         m_pRefreshLineEdit->setText(QString("0.00"));
         break;
   }

}

// Private slots
void VisionWinVidTiming::vidTimingWidthChanged( )
{
   m_pData->setVidTimingWidth(m_pWidthSpinBox->value(), false);
   m_pData->setModified(VW_MOD_VID_TIMINGS);
}

void VisionWinVidTiming::vidTimingHeightChanged( )
{
   m_pData->setVidTimingHeight(m_pHeightSpinBox->value(), false);
   m_pData->setModified(VW_MOD_VID_TIMINGS);
}

