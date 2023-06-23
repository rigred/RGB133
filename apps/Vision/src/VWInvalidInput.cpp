
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWInvalidInput.h"

VisionWinInvalidInput::VisionWinInvalidInput(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setDisabled(true);
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    label = new QLabel(tr("Display messages after"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    lineEdit = new QLineEdit(tr("0"));
    lineEdit->setAlignment(Qt::AlignLeft);
    lineEdit->setMaxLength(4);
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    resolution = new QLabel(tr("milliseconds"));
    resolution->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *invalidInputLayout = new QBoxLayout(direction);
    invalidInputLayout->setSpacing(5);
    invalidInputLayout->setAlignment(Qt::AlignTop);
    invalidInputLayout->setContentsMargins(10, 0, 10, 1);
    invalidInputLayout->addWidget(label);
    invalidInputLayout->addWidget(lineEdit);
    invalidInputLayout->addWidget(resolution);
    setLayout(invalidInputLayout);
}
