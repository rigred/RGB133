
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWSaveChangesDialog.h"

VWSaveChangesDialog::VWSaveChangesDialog( )
{
   m_pLabel = new QLabel(tr("Save changes?"));
   m_pLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Yes
                                  | QDialogButtonBox::No);

   connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
   connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));

   QVBoxLayout* colLayout = new QVBoxLayout;
   colLayout->addWidget(m_pLabel);
   colLayout->addWidget(m_pButtonBox);

   setLayout(colLayout);
}

VWSaveChangesDialog::~VWSaveChangesDialog()
{
}
