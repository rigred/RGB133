
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWWindowID.h"

VisionWinWindowID::VisionWinWindowID(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setDisabled(true);
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    idLabel = new QLabel(tr("Identifier"));
    idLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    idSpinBox = new QSpinBox;
    idSpinBox->setRange(0, 4);
    idSpinBox->setSingleStep(1);
    idSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(idSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *winIDLayout = new QBoxLayout(direction);
    winIDLayout->setSpacing(5);
    winIDLayout->setAlignment(Qt::AlignLeft);
    winIDLayout->setContentsMargins(10, 0, 10, 1);
    winIDLayout->addWidget(idLabel);
    winIDLayout->addWidget(idSpinBox);
    setLayout(winIDLayout);
}

void VisionWinWindowID::idSetValue(int value)
{
    idSpinBox->setValue(value);
}
