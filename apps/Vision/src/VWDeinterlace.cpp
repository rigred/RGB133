
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWDeinterlace.h"

VisionWinDeinterlace::VisionWinDeinterlace(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setDisabled(true);
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    bobRadioButton = new QRadioButton(tr("Bob"));
    bobRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    weaveRadioButton = new QRadioButton(tr("Weave"));
    weaveRadioButton->setChecked(true);
    weaveRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    noneRadioButton = new QRadioButton(tr("None (single field)"));
    noneRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;
    QBoxLayout *deinterlaceLayout = new QBoxLayout(direction);
    deinterlaceLayout->setSpacing(5);
    deinterlaceLayout->setAlignment(Qt::AlignTop);
    deinterlaceLayout->setContentsMargins(10, 0, 10, 1);
    deinterlaceLayout->addWidget(bobRadioButton);
    deinterlaceLayout->addWidget(weaveRadioButton);
    deinterlaceLayout->addWidget(noneRadioButton);
    setLayout(deinterlaceLayout);
}

