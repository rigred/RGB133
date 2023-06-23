
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWInputSelectDialog.h"
#include "VWData.h"

VWInputSelectDialog::VWInputSelectDialog(QMainWindow *parent, VWData* data)
    : p(parent), m_pData(data)
{
   m_pLabel = new QLabel(tr("Input"));
   m_pLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   m_pComboBox = new QComboBox;
   for(int i=1; i<=m_pData->getNumInputs(); i++)
   {
      m_pComboBox->addItem(QString("%1").arg(i, 0));
   }
   m_pComboBox->setMaxVisibleItems(4);
   m_pComboBox->setMinimumWidth(100);
   m_pComboBox->setMaximumWidth(100);
   m_pComboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pComboBox->setCurrentIndex(0);
   m_source = 1;
   m_index = 0;

   m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                  | QDialogButtonBox::Cancel);
   connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(setInput()));
   connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(invalidateInput()));


   QHBoxLayout* rowLayout = new QHBoxLayout;
   rowLayout->setSpacing(5);
   rowLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   rowLayout->setContentsMargins(10, 0, 10, 1);
   rowLayout->addWidget(m_pLabel);
   rowLayout->addWidget(m_pComboBox);

   QVBoxLayout* colLayout = new QVBoxLayout;
   colLayout->addLayout(rowLayout);
   colLayout->addWidget(m_pButtonBox);

   setLayout(colLayout);
}

VWInputSelectDialog::~VWInputSelectDialog()
{
}

void VWInputSelectDialog::setInput()
{
   m_source = m_pComboBox->currentText().toInt();
   m_index = m_source-1;
   close();
}

void VWInputSelectDialog::invalidateInput()
{
   m_source = 0;
   m_index = m_source-1;
   close();
}
