
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWTransferData.h"

VisionWinTransferData::VisionWinTransferData(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setDisabled(true);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    radioButtonDirect = new QRadioButton(tr("Direct to graphics card"));
    radioButtonDirect->setChecked(true);
    radioButtonDirect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    radioButtonSystem = new QRadioButton(tr("Via system memory"));
    radioButtonSystem->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    QBoxLayout::Direction direction = QBoxLayout::TopToBottom;
    QBoxLayout *xferLayout = new QBoxLayout(direction);
    xferLayout->setSpacing(1);
    xferLayout->setAlignment(Qt::AlignLeft);
    xferLayout->setContentsMargins(10, 0, 10, 1);
    xferLayout->addWidget(radioButtonDirect);
    xferLayout->addWidget(radioButtonSystem);
    setLayout(xferLayout);
}

