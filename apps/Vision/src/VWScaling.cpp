
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWScaling.h"

VisionWinScaling::VisionWinScaling(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setDisabled(true);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    radioButtonFast = new QRadioButton(tr("Fast (default)"));
    radioButtonFast->setChecked(true);
    radioButtonFast->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    radioButtonSlow = new QRadioButton(tr("Slow (high quality)"));
    radioButtonSlow->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    QBoxLayout::Direction direction = QBoxLayout::TopToBottom;
    QBoxLayout *scalingLayout = new QBoxLayout(direction);
    scalingLayout->setSpacing(1);
    scalingLayout->setAlignment(Qt::AlignLeft);
    scalingLayout->setContentsMargins(10, 0, 10, 1);
    scalingLayout->addWidget(radioButtonFast);
    scalingLayout->addWidget(radioButtonSlow);
    setLayout(scalingLayout);
}

