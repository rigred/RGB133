
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWWindowTab.h"
#include "VWData.h"
#include "VWPosition.h"

VisionWinPosition::VisionWinPosition(const QString &title,
      QPair<int, int>& position, QPair<int, int>& positionMax,
      VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
   setMinimumHeight(50);
   setMaximumHeight(50);
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pTopLabel = new QLabel(tr("Top"));
   m_pTopLabel->setMinimumWidth(30);
   m_pTopLabel->setMaximumWidth(30);
   m_pLeftLabel = new QLabel(tr("Left"));
   m_pLeftLabel->setMinimumWidth(30);
   m_pLeftLabel->setMaximumWidth(30);

   m_pTopSpinBox = new QSpinBox;
   m_pTopSpinBox->setRange(0, positionMax.first);
   m_pTopSpinBox->setSingleStep(1);
   m_pTopSpinBox->setMinimumWidth(70);
   m_pTopSpinBox->setValue(position.first);
   m_pTopSpinBox->setAccelerated(true);

   m_pLeftSpinBox = new QSpinBox;
   m_pLeftSpinBox->setRange(0, positionMax.second);
   m_pLeftSpinBox->setSingleStep(1);
   m_pLeftSpinBox->setMinimumWidth(70);
   m_pLeftSpinBox->setValue(position.second);
   m_pLeftSpinBox->setAccelerated(true);

   connect(m_pTopSpinBox, SIGNAL(valueChanged(int)),
         this, SLOT(positionChanged()));
   connect(m_pLeftSpinBox, SIGNAL(valueChanged(int)),
         this, SLOT(positionChanged()));

   QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
   QBoxLayout *positionLayout = new QBoxLayout(direction);
   positionLayout->setSpacing(5);
   positionLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   positionLayout->setContentsMargins(10, 0, 10, 1);
   positionLayout->addWidget(m_pTopLabel);
   positionLayout->addSpacing(8);
   positionLayout->addWidget(m_pTopSpinBox);
   positionLayout->addWidget(m_pLeftLabel);
   positionLayout->addSpacing(13);
   positionLayout->addWidget(m_pLeftSpinBox);
   setLayout(positionLayout);
}

void VisionWinPosition::topSetValue(int value)
{
    m_pTopSpinBox->setValue(value);
}

void VisionWinPosition::leftSetValue(int value)
{
    m_pLeftSpinBox->setValue(value);
}

QPoint VisionWinPosition::getPosition( ) const
{
   return QPoint(m_pLeftSpinBox->value(), m_pTopSpinBox->value());
};

// Private slots
void VisionWinPosition::positionChanged( )
{
   m_pData->setPosition(QPair<int, int>(m_pTopSpinBox->value(), m_pLeftSpinBox->value()));
   m_pData->setModified(VW_MOD_POSITION);
}
