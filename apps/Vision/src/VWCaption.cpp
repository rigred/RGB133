
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWCaption.h"

VisionWinCaption::VisionWinCaption(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), m_pData(data)
{
    setMinimumHeight(50);
    setMaximumHeight(50);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_pLineEdit = new QLineEdit(m_pData->getCaption());
    m_pLineEdit->setAlignment(Qt::AlignLeft);
    m_pLineEdit->setMaxLength(256);
    connect(m_pLineEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(captionChanged(const QString&)));
    
    m_pButton = new QPushButton(tr("Variables..."));
    m_pButton->setDisabled(true);
    
    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;

    QBoxLayout *captionLayout = new QBoxLayout(direction);
    captionLayout->setSpacing(10);
    captionLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    captionLayout->setContentsMargins(10, 1, 10, 1);
    captionLayout->addWidget(m_pLineEdit);
    captionLayout->addWidget(m_pButton);
    setLayout(captionLayout);
}

QString VisionWinCaption::getCaption( )
{
   return m_pLineEdit->text();
}

void VisionWinCaption::captionChanged(const QString& caption)
{
   m_pData->setCaption(caption);
   m_pData->setModified(VW_MOD_CAPTION);
}
