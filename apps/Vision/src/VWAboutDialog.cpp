
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWAboutDialog.h"
#include "VWData.h"

VWAboutDialog::VWAboutDialog(QMainWindow *parent, VWData* data)
    : p(parent), m_pData(data)
{
   m_pVisionLabel = new QLabel(DRIVER_TAG);
   m_pVisionLabel->setAlignment(Qt::AlignHCenter);

   QString ver("Version: ");
   ver += m_pData->getVersion();
   m_pVersionLabel = new QLabel(ver);
   m_pVersionLabel->setAlignment(Qt::AlignHCenter);

   m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
   connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(close()));

   m_pLayout = new QVBoxLayout;
   m_pLayout->addWidget(m_pVisionLabel);
   m_pLayout->addWidget(m_pVersionLabel);
   m_pLayout->addWidget(m_pButtonBox);
   m_pLayout->setAlignment(Qt::AlignCenter);

   setLayout(m_pLayout);
}

VWAboutDialog::~VWAboutDialog()
{
}
