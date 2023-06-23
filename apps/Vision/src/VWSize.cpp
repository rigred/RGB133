
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWWindowTab.h"
#include "VWData.h"
#include "VWSize.h"

VisionWinSize::VisionWinSize(const QString &title, QRect &pos, QPair<int, int>& positionMax,
      VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    m_pWidthLabel = new QLabel(tr("Width"));
    m_pWidthLabel->setMinimumWidth(36);
    m_pHeightLabel = new QLabel(tr("Height"));
    m_pHeightLabel->setMinimumWidth(36);
    m_pExBorderLabel = new QLabel(tr("Exclude Borders"));
    m_pExBorderLabel->setWordWrap(true);

    m_pWidthSpinBox = new QSpinBox;
    m_pWidthSpinBox->setRange(0, positionMax.first);
    m_pWidthSpinBox->setSingleStep(1);
    m_pWidthSpinBox->setValue(pos.width());
    m_pWidthSpinBox->setMinimumWidth(70);
    m_pWidthSpinBox->setMaximumWidth(70);
    m_pWidthSpinBox->setAccelerated(true);

    m_pHeightSpinBox = new QSpinBox;
    m_pHeightSpinBox->setRange(0, positionMax.second);
    m_pHeightSpinBox->setSingleStep(1);
    m_pHeightSpinBox->setValue(pos.height());
    m_pHeightSpinBox->setMinimumWidth(70);
    m_pHeightSpinBox->setMaximumWidth(70);
    m_pHeightSpinBox->setAccelerated(true);

    m_pExBorderCheckBox = new QCheckBox("");
    m_pExBorderCheckBox->setChecked(true);
    m_pExBorderCheckBox->setDisabled(true);

    connect(m_pWidthSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(sizeChanged()));
    connect(m_pHeightSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(sizeChanged()));

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *sizeLayout = new QBoxLayout(direction);
    sizeLayout->setSpacing(5);
    sizeLayout->setAlignment(Qt::AlignTop);
    sizeLayout->setContentsMargins(10, 0, 10, 1);
    sizeLayout->addWidget(m_pWidthLabel);
    sizeLayout->addWidget(m_pWidthSpinBox);
    sizeLayout->addWidget(m_pHeightLabel);
    sizeLayout->addWidget(m_pHeightSpinBox);
    sizeLayout->addWidget(m_pExBorderCheckBox);
    sizeLayout->addWidget(m_pExBorderLabel);
    setLayout(sizeLayout);
}

void VisionWinSize::widthSetValue(int value)
{
   m_pWidthSpinBox->setValue(value);
}

void VisionWinSize::heightSetValue(int value)
{
   m_pHeightSpinBox->setValue(value);
}

QRect VisionWinSize::getSize( ) const
{
   return QRect(0, 0, m_pWidthSpinBox->value(), m_pHeightSpinBox->value());
};

// Private slots
void VisionWinSize::sizeChanged( )
{
   m_pData->setModified(VW_MOD_SIZE);
}

void VisionWinSize::updateSize(QRect& sz)
{
   m_pWidthSpinBox->setValue(sz.width());
   m_pHeightSpinBox->setValue(sz.height());
}
