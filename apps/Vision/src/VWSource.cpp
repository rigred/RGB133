
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWInputTab.h"
#include "VWData.h"
#include "VWSource.h"

VisionWinSource::VisionWinSource(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

   label = new QLabel(tr("Input"));
   label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   comboBox = new QComboBox;
   for(int i=1; i<=m_pData->getNumInputs(); i++)
   {
      comboBox->addItem(QString("%1").arg(i, 0));
   }
   comboBox->setMaxVisibleItems(2);
   comboBox->setMinimumWidth(100);
   comboBox->setMaximumWidth(100);
   comboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   comboBox->setCurrentIndex(m_pData->getCurrentIndex());

   connect(comboBox, SIGNAL(currentIndexChanged(int)),
         m_pData, SLOT(sourceIndexChanged(int)));

   QGridLayout *slidersLayout = new QGridLayout;
   slidersLayout->setSpacing(5);
   slidersLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   slidersLayout->setContentsMargins(10, 0, 10, 1);
   slidersLayout->addWidget(label, 0, 0);
   slidersLayout->addWidget(comboBox, 0, 1);
   setLayout(slidersLayout);
}
