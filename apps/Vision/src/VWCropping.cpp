
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWCropping.h"

#include "rgb133control.h"

VisionWinCropping::VisionWinCropping(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
   sVWAOI aoi;
   eVWCroppingType type;

   m_bIgnore = false;

   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pOffRadioButton = new QRadioButton(tr("Off"));
   m_pOffRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pOnRadioButton = new QRadioButton(tr("On"));
   m_pOnRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pOverscanRadioButton = new QRadioButton(tr("Overscan"));
   m_pOverscanRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pTopLabel = new QLabel(tr("Top"));
   m_pTopLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pWidthLabel = new QLabel(tr("Width"));
   m_pWidthLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pLeftLabel = new QLabel(tr("Left"));
   m_pLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pHeightLabel = new QLabel(tr("Height"));
   m_pHeightLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   // Get current values
   m_pData->getCropping(&aoi);
   m_pTopSpinBox = new QSpinBox;
   m_pTopSpinBox->setRange(0, m_pData->getVidTimingHeightMax()-1);
   m_pTopSpinBox->setValue(aoi.top);
   m_pTopSpinBox->setSingleStep(1);
   m_pTopSpinBox->setMinimumWidth(70);
   m_pTopSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pTopSpinBox->setAccelerated(true);

   m_pWidthSpinBox = new QSpinBox;
   m_pWidthSpinBox->setMaximum(m_pData->getVidTimingWidthMax()-1);
   m_pWidthSpinBox->setValue(aoi.right);
   m_pWidthSpinBox->setMinimum(4);
   m_pWidthSpinBox->setSingleStep(1);
   m_pWidthSpinBox->setMinimumWidth(70);
   m_pWidthSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pWidthSpinBox->setAccelerated(true);

   m_pLeftSpinBox = new QSpinBox;
   m_pLeftSpinBox->setRange(0, m_pData->getVidTimingWidthMax());
   m_pLeftSpinBox->setValue(aoi.left);
   m_pLeftSpinBox->setSingleStep(1);
   m_pLeftSpinBox->setMinimumWidth(70);
   m_pLeftSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pLeftSpinBox->setAccelerated(true);

   m_pHeightSpinBox = new QSpinBox;
   m_pHeightSpinBox->setRange(1, m_pData->getVidTimingHeightMax());
   m_pHeightSpinBox->setValue(aoi.bottom);
   m_pHeightSpinBox->setSingleStep(1);
   m_pHeightSpinBox->setMinimumWidth(70);
   m_pHeightSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pHeightSpinBox->setAccelerated(true);

   QGridLayout *croppingLayout = new QGridLayout;
   croppingLayout->addWidget(m_pOffRadioButton, 0, 0);
   croppingLayout->addWidget(m_pOnRadioButton, 1, 0);
   croppingLayout->addWidget(m_pOverscanRadioButton, 2, 0);
   croppingLayout->addWidget(m_pTopLabel, 0, 2);
   croppingLayout->addWidget(m_pTopSpinBox, 0, 3);
   croppingLayout->addWidget(m_pWidthLabel, 0, 4);
   croppingLayout->addWidget(m_pWidthSpinBox, 0, 5);
   croppingLayout->addWidget(m_pLeftLabel, 1, 2);
   croppingLayout->addWidget(m_pLeftSpinBox, 1, 3);
   croppingLayout->addWidget(m_pHeightLabel, 1, 4);
   croppingLayout->addWidget(m_pHeightSpinBox, 1, 5);

   m_pTopLabel->setDisabled(true);
   m_pWidthLabel->setDisabled(true);
   m_pLeftLabel->setDisabled(true);
   m_pHeightLabel->setDisabled(true);
   m_pTopSpinBox->setDisabled(true);
   m_pWidthSpinBox->setDisabled(true);
   m_pLeftSpinBox->setDisabled(true);
   m_pHeightSpinBox->setDisabled(true);

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         if(m_pData->getCroppingType() == VW_CROPPING_OVERSCAN ||
            m_pData->getCroppingType() == VW_CROPPING_UNSET)
         {
            m_pOverscanRadioButton->setVisible(true);
            m_pOverscanRadioButton->setChecked(true);
            type = VW_CROPPING_OVERSCAN;
         }
         else if(m_pData->getCroppingType() == VW_CROPPING_ON)
         {
            m_pTopLabel->setDisabled(false);
            m_pWidthLabel->setDisabled(false);
            m_pLeftLabel->setDisabled(false);
            m_pHeightLabel->setDisabled(false);
            m_pTopSpinBox->setDisabled(false);
            m_pWidthSpinBox->setDisabled(false);
            m_pLeftSpinBox->setDisabled(false);
            m_pHeightSpinBox->setDisabled(false);
            m_pOnRadioButton->setChecked(true);
            type = VW_CROPPING_ON;
         }
         else
         {
            m_pOffRadioButton->setChecked(true);
            type = VW_CROPPING_OFF;
         }
         break;
      case VW_TYPE_VIDEO:
      case VW_TYPE_DVI:
         m_pOverscanRadioButton->setVisible(false);
         m_pOverscanRadioButton->setDisabled(true);
         if(m_pData->getCroppingType() == VW_CROPPING_OFF ||
            m_pData->getCroppingType() == VW_CROPPING_UNSET)
         {
            m_pOffRadioButton->setChecked(true);
            m_pOnRadioButton->setChecked(false);
            type = VW_CROPPING_OFF;
         }
         else
         {
            m_pTopLabel->setDisabled(false);
            m_pWidthLabel->setDisabled(false);
            m_pLeftLabel->setDisabled(false);
            m_pHeightLabel->setDisabled(false);
            m_pTopSpinBox->setDisabled(false);
            m_pWidthSpinBox->setDisabled(false);
            m_pLeftSpinBox->setDisabled(false);
            m_pHeightSpinBox->setDisabled(false);
            m_pOnRadioButton->setChecked(true);
            type = VW_CROPPING_ON;
         }
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         setDisabled(true);
         m_pTopSpinBox->setRange(0, 0);
         m_pLeftSpinBox->setRange(0, 0);
         m_pWidthSpinBox->setRange(0, 0);
         m_pHeightSpinBox->setRange(0, 0);
         m_pTopSpinBox->setValue(0);
         m_pLeftSpinBox->setValue(0);
         m_pWidthSpinBox->setValue(0);
         m_pHeightSpinBox->setValue(0);
         m_pOverscanRadioButton->setVisible(true);
         m_pOverscanRadioButton->setDisabled(true);
         m_pOffRadioButton->setChecked(true);
         type = VW_CROPPING_OFF;
         break;
   }

   if(m_pOverscanRadioButton->isChecked())
   {
      switch(m_pData->getVideoStd())
      {
         case VW_VIDSTD_NTSC_M:
         case VW_VIDSTD_NTSC_4_43_60:
         case VW_VIDSTD_PAL_M:
         case VW_VIDSTD_PAL_4_43_60:
         case VW_VIDSTD_NTSC_J:
            m_pTopSpinBox->setValue(VW_OVERSCAN_S1_TOP);
            m_pLeftSpinBox->setValue(VW_OVERSCAN_S1_LEFT);
            m_pWidthSpinBox->setValue(VW_OVERSCAN_S1_WIDTH);
            m_pHeightSpinBox->setValue(VW_OVERSCAN_S1_HEIGHT);
            break;
         case VW_VIDSTD_PAL_I:
         case VW_VIDSTD_SECAM_L:
         case VW_VIDSTD_PAL_NC:
         case VW_VIDSTD_NTSC_4_43_50:
            m_pTopSpinBox->setValue(VW_OVERSCAN_S2_TOP);
            m_pLeftSpinBox->setValue(VW_OVERSCAN_S2_LEFT);
            m_pWidthSpinBox->setValue(VW_OVERSCAN_S2_WIDTH);
            m_pHeightSpinBox->setValue(VW_OVERSCAN_S2_HEIGHT);
            break;
         default:
            m_pTopSpinBox->setRange(0, 0);
            m_pLeftSpinBox->setRange(0, 0);
            m_pWidthSpinBox->setRange(0, 0);
            m_pHeightSpinBox->setRange(0, 0);
            m_pTopSpinBox->setValue(0);
            m_pLeftSpinBox->setValue(0);
            m_pWidthSpinBox->setValue(0);
            m_pHeightSpinBox->setValue(0);
            break;
      }
   }

   m_pData->setCropping(aoi, type);

   connect(m_pTopSpinBox, SIGNAL(valueChanged(int)), this, SLOT(croppingValuesChanged()));
   connect(m_pWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(croppingValuesChanged()));
   connect(m_pLeftSpinBox, SIGNAL(valueChanged(int)), this, SLOT(croppingValuesChanged()));
   connect(m_pHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(croppingValuesChanged()));

   connect(m_pOffRadioButton, SIGNAL(toggled(bool)), this, SLOT(checkCroppingStatus(bool)));
   connect(m_pOnRadioButton, SIGNAL(toggled(bool)), this, SLOT(checkCroppingStatus(bool)));
   connect(m_pOverscanRadioButton, SIGNAL(toggled(bool)), this, SLOT(checkCroppingStatus(bool)));

   connect(m_pData, SIGNAL(inputPropertiesReset()), this, SLOT(inputPropertiesReset()));

   setLayout(croppingLayout);
}

// Public Slots
void VisionWinCropping::inputPropertiesReset()
{
   m_pTopSpinBox->setValue(0);
   m_pWidthSpinBox->setValue(m_pData->getVidTimingWidth());
   m_pLeftSpinBox->setValue(0);
   m_pHeightSpinBox->setValue(m_pData->getVidTimingHeight());

   m_pTopLabel->setDisabled(true);
   m_pWidthLabel->setDisabled(true);
   m_pLeftLabel->setDisabled(true);
   m_pHeightLabel->setDisabled(true);
   m_pTopSpinBox->setDisabled(true);
   m_pWidthSpinBox->setDisabled(true);
   m_pLeftSpinBox->setDisabled(true);
   m_pHeightSpinBox->setDisabled(true);

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         switch(m_pData->getVideoStd())
         {
            case VW_VIDSTD_NTSC_M:
            case VW_VIDSTD_NTSC_4_43_60:
            case VW_VIDSTD_PAL_M:
            case VW_VIDSTD_PAL_4_43_60:
            case VW_VIDSTD_NTSC_J:
               m_pTopSpinBox->setValue(6);
               m_pLeftSpinBox->setValue(14);
               m_pWidthSpinBox->setValue(708);
               m_pHeightSpinBox->setValue(480);
               break;
            case VW_VIDSTD_PAL_I:
            case VW_VIDSTD_SECAM_L:
            case VW_VIDSTD_PAL_NC:
            case VW_VIDSTD_NTSC_4_43_50:
               m_pTopSpinBox->setValue(4);
               m_pLeftSpinBox->setValue(14);
               m_pWidthSpinBox->setValue(690);
               m_pHeightSpinBox->setValue(566);
               break;
            default:
               m_pTopSpinBox->setRange(0, 0);
               m_pLeftSpinBox->setRange(0, 0);
               m_pWidthSpinBox->setRange(0, 0);
               m_pHeightSpinBox->setRange(0, 0);
               m_pTopSpinBox->setValue(0);
               m_pLeftSpinBox->setValue(0);
               m_pWidthSpinBox->setValue(0);
               m_pHeightSpinBox->setValue(0);
               break;
         }
         m_pOverscanRadioButton->setVisible(true);
         m_pOverscanRadioButton->setDisabled(false);
         m_pOverscanRadioButton->setChecked(true);
         break;
      case VW_TYPE_VIDEO:
      case VW_TYPE_DVI:
         setDisabled(false);
         m_pOffRadioButton->setChecked(true);
         m_pOverscanRadioButton->setVisible(false);
         m_pOverscanRadioButton->setDisabled(true);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         m_bIgnore = true;
         setDisabled(true);
         m_pTopSpinBox->setRange(0, 0);
         m_pLeftSpinBox->setRange(0, 0);
         m_pWidthSpinBox->setRange(0, 0);
         m_pHeightSpinBox->setRange(0, 0);
         m_pTopSpinBox->setValue(0);
         m_pLeftSpinBox->setValue(0);
         m_pWidthSpinBox->setValue(0);
         m_pHeightSpinBox->setValue(0);
         m_pOffRadioButton->setChecked(true);
         m_pOverscanRadioButton->setVisible(false);
         m_pOverscanRadioButton->setDisabled(true);
         m_bIgnore = false;
         break;
   }
}

void VisionWinCropping::croppingTypeChanged()
{
   sVWAOI aoi;

   m_bIgnore = true;

   // Get current values
   m_pData->getCropping(&aoi);

   m_pTopSpinBox->setRange(0, m_pData->getVidTimingHeightMax()-1);
   m_pWidthSpinBox->setRange(4, m_pData->getVidTimingWidthMax());
   m_pLeftSpinBox->setRange(0, m_pData->getVidTimingWidthMax()-1);
   m_pHeightSpinBox->setRange(1, m_pData->getVidTimingHeightMax());

   m_pTopSpinBox->setValue(aoi.top);
   m_pWidthSpinBox->setValue(aoi.right);
   m_pLeftSpinBox->setValue(aoi.left);
   m_pHeightSpinBox->setValue(aoi.bottom);

   m_pTopLabel->setDisabled(true);
   m_pWidthLabel->setDisabled(true);
   m_pLeftLabel->setDisabled(true);
   m_pHeightLabel->setDisabled(true);
   m_pTopSpinBox->setDisabled(true);
   m_pWidthSpinBox->setDisabled(true);
   m_pLeftSpinBox->setDisabled(true);
   m_pHeightSpinBox->setDisabled(true);

   switch(m_pData->getSignalType())
   {
      case VW_TYPE_3WIRE_SOG:
      case VW_TYPE_4WIRE_COMPOSITE_SYNC:
      case VW_TYPE_5WIRE_SEPARATE_SYNCS:
      case VW_TYPE_YPRPB:
      case VW_TYPE_CVBS:
      case VW_TYPE_YC:
         setDisabled(false);
         switch(m_pData->getVideoStd())
         {
            case VW_VIDSTD_NTSC_M:
            case VW_VIDSTD_NTSC_4_43_60:
            case VW_VIDSTD_PAL_M:
            case VW_VIDSTD_PAL_4_43_60:
            case VW_VIDSTD_NTSC_J:
               m_pTopSpinBox->setValue(6);
               m_pLeftSpinBox->setValue(14);
               m_pWidthSpinBox->setValue(708);
               m_pHeightSpinBox->setValue(480);
               break;
            case VW_VIDSTD_PAL_I:
            case VW_VIDSTD_SECAM_L:
            case VW_VIDSTD_PAL_NC:
            case VW_VIDSTD_NTSC_4_43_50:
               m_pTopSpinBox->setValue(4);
               m_pLeftSpinBox->setValue(14);
               m_pWidthSpinBox->setValue(690);
               m_pHeightSpinBox->setValue(566);
               break;
            default:
               m_pTopSpinBox->setRange(0, 0);
               m_pLeftSpinBox->setRange(0, 0);
               m_pWidthSpinBox->setRange(0, 0);
               m_pHeightSpinBox->setRange(0, 0);
               m_pTopSpinBox->setValue(0);
               m_pLeftSpinBox->setValue(0);
               m_pWidthSpinBox->setValue(0);
               m_pHeightSpinBox->setValue(0);
               break;
         }
         m_pOverscanRadioButton->setVisible(true);
         m_pOverscanRadioButton->setDisabled(false);
         m_pOverscanRadioButton->setChecked(true);
         break;
      case VW_TYPE_VIDEO:
      case VW_TYPE_DVI:
         setDisabled(false);
         m_pOverscanRadioButton->setVisible(false);
         m_pOverscanRadioButton->setDisabled(true);
         break;
      case VW_TYPE_NOSIGNAL:
      default:
         m_bIgnore = true;
         setDisabled(true);
         m_pTopSpinBox->setRange(0, 0);
         m_pLeftSpinBox->setRange(0, 0);
         m_pWidthSpinBox->setRange(0, 0);
         m_pHeightSpinBox->setRange(0, 0);
         m_pTopSpinBox->setValue(0);
         m_pLeftSpinBox->setValue(0);
         m_pWidthSpinBox->setValue(0);
         m_pHeightSpinBox->setValue(0);
         m_pOffRadioButton->setChecked(true);
         m_pOverscanRadioButton->setVisible(false);
         m_pOverscanRadioButton->setDisabled(true);
         m_bIgnore = false;
         break;
   }
   m_bIgnore = false;
}

void VisionWinCropping::checkCroppingStatus(bool value)
{
   sVWAOI aoi;

   m_bIgnore = true;

   if(value == true)
   {
      if(m_pOffRadioButton->isChecked() ||
            m_pOverscanRadioButton->isChecked())
      {
         m_pTopLabel->setDisabled(true);
         m_pWidthLabel->setDisabled(true);
         m_pLeftLabel->setDisabled(true);
         m_pHeightLabel->setDisabled(true);
         m_pTopSpinBox->setDisabled(true);
         m_pWidthSpinBox->setDisabled(true);
         m_pLeftSpinBox->setDisabled(true);
         m_pHeightSpinBox->setDisabled(true);

         if(m_pOverscanRadioButton->isChecked())
         {
            switch(m_pData->getVideoStd())
            {
               case VW_VIDSTD_NTSC_M:
               case VW_VIDSTD_NTSC_4_43_60:
               case VW_VIDSTD_PAL_M:
               case VW_VIDSTD_PAL_4_43_60:
               case VW_VIDSTD_NTSC_J:
                  m_pTopSpinBox->setValue(VW_OVERSCAN_S1_TOP);
                  m_pLeftSpinBox->setValue(VW_OVERSCAN_S1_LEFT);
                  m_pWidthSpinBox->setValue(VW_OVERSCAN_S1_WIDTH);
                  m_pHeightSpinBox->setValue(VW_OVERSCAN_S1_HEIGHT);
                  break;
               case VW_VIDSTD_PAL_I:
               case VW_VIDSTD_SECAM_L:
               case VW_VIDSTD_PAL_NC:
               case VW_VIDSTD_NTSC_4_43_50:
                  m_pTopSpinBox->setValue(VW_OVERSCAN_S2_TOP);
                  m_pLeftSpinBox->setValue(VW_OVERSCAN_S2_LEFT);
                  m_pWidthSpinBox->setValue(VW_OVERSCAN_S2_WIDTH);
                  m_pHeightSpinBox->setValue(VW_OVERSCAN_S2_HEIGHT);
                  break;
               default:
                  m_pTopSpinBox->setRange(0, 0);
                  m_pLeftSpinBox->setRange(0, 0);
                  m_pWidthSpinBox->setRange(0, 0);
                  m_pHeightSpinBox->setRange(0, 0);
                  m_pTopSpinBox->setValue(0);
                  m_pLeftSpinBox->setValue(0);
                  m_pWidthSpinBox->setValue(0);
                  m_pHeightSpinBox->setValue(0);
                  break;
            }
            aoi.top = m_pTopSpinBox->value();
            aoi.left = m_pLeftSpinBox->value();
            aoi.right = m_pWidthSpinBox->value();
            aoi.bottom = m_pHeightSpinBox->value();

            m_pData->setCropping(aoi, VW_CROPPING_OVERSCAN);
            m_pData->setModified(VW_MOD_CROPPING);
         }
         else
         {
            sVWAOI aoi;
            aoi.top = 0;
            aoi.left = 0;
            aoi.right = m_pData->getVidTimingWidth();
            aoi.bottom = m_pData->getVidTimingHeight();

            m_pData->setCropping(aoi, VW_CROPPING_OFF);
            m_pData->setModified(VW_MOD_CROPPING);
         }
      }
      else if(m_pOnRadioButton->isChecked())
      {
         m_pTopLabel->setDisabled(false);
         m_pWidthLabel->setDisabled(false);
         m_pLeftLabel->setDisabled(false);
         m_pHeightLabel->setDisabled(false);
         m_pTopSpinBox->setDisabled(false);
         m_pWidthSpinBox->setDisabled(false);
         m_pLeftSpinBox->setDisabled(false);
         m_pHeightSpinBox->setDisabled(false);

         aoi.top = m_pTopSpinBox->value();
         aoi.left = m_pLeftSpinBox->value();
         aoi.right = m_pWidthSpinBox->value();
         aoi.bottom = m_pHeightSpinBox->value();

         m_pData->setCropping(aoi, VW_CROPPING_ON);
         m_pData->setModified(VW_MOD_CROPPING);
}
   }
   m_bIgnore = false;
}

// Private Slots
void VisionWinCropping::croppingValuesChanged( )
{
   sVWAOI aoi;

   if(!m_bIgnore)
   {
      aoi.top = m_pTopSpinBox->value();
      aoi.left = m_pLeftSpinBox->value();
      aoi.right = m_pWidthSpinBox->value();
      aoi.bottom = m_pHeightSpinBox->value();

      m_pData->setCropping(aoi, VW_CROPPING_ON);
      m_pData->setModified(VW_MOD_CROPPING);
   }
}
