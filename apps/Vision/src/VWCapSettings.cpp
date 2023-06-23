
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWCapSettings.h"

VisionWinCapSettings::VisionWinCapSettings(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_pSharedRadioButton = new QRadioButton(tr("Share these capture settings across all inputs"));
    m_pSharedRadioButton->setChecked(true);
    m_pSharedRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pSharedRadioButton->setDisabled(true);

    m_pUniqueRadioButton = new QRadioButton(tr("Use these capture settings for this input only"));
    m_pUniqueRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pUniqueRadioButton->setDisabled(true);

    m_pPushButton = new QPushButton(tr("Reset"));
    m_pPushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_pPushButton, SIGNAL(clicked()), m_pData, SLOT(reset()));
    
    QVBoxLayout *radioButtonLayout = new QVBoxLayout;
    radioButtonLayout->setSpacing(1);
    radioButtonLayout->addWidget(m_pSharedRadioButton);
    radioButtonLayout->addWidget(m_pUniqueRadioButton);

    QHBoxLayout *capSetLayout = new QHBoxLayout;
    capSetLayout->setSpacing(30);
    capSetLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    capSetLayout->setContentsMargins(10, 0, 10, 1);
    capSetLayout->addLayout(radioButtonLayout);
    capSetLayout->addWidget(m_pPushButton);
    setLayout(capSetLayout);
}

// Public Slots
void VisionWinCapSettings::capSettingsTypeChanged( )
{
   switch(m_pData->getSignalType())
   {
      case VW_TYPE_NOSIGNAL:
         m_pPushButton->setDisabled(true);
         break;
      default:
         m_pPushButton->setDisabled(false);
         break;
   }
}
