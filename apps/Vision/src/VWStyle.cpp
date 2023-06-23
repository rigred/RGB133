
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWStyle.h"

VisionWinStyle::VisionWinStyle(const QString &title, VWData* data, QWidget *parent)
    : QGroupBox(title, parent), p(parent), m_pData(data)
{
   setAlignment(Qt::AlignLeft);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   m_pRadioButtonBorTitle = new QRadioButton(tr("Border and title bar"));
   m_pRadioButtonBorTitle->setChecked(true);
   m_pRadioButtonBorTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pRadioButtonBorTitle->setDisabled(true);

   m_pRadioButtonBor = new QRadioButton(tr("Border only"));
   m_pRadioButtonBor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pRadioButtonBor->setDisabled(true);

   m_pRadioButtonNoBorNoTitle = new QRadioButton(tr("No border or title bar"));
   m_pRadioButtonNoBorNoTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   m_pRadioButtonNoBorNoTitle->setDisabled(true);

   m_pShowMenuCheckBox = new QCheckBox("Show menu bar");
   m_pShowMenuCheckBox->setChecked(m_pData->getShowMenu());
   m_pShowMenuCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   connect(m_pShowMenuCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showMenu(int)));

   m_pOnTopCheckBox = new QCheckBox("Always on top");
   m_pOnTopCheckBox->setChecked(m_pData->getAlwaysOnTop());
   m_pOnTopCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   connect(m_pOnTopCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onTop(int)));

   QGridLayout *styleLayout = new QGridLayout;
   styleLayout->setVerticalSpacing(1);
   styleLayout->setHorizontalSpacing(50);
   styleLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   styleLayout->setContentsMargins(10, 0, 10, 1);
   styleLayout->addWidget(m_pRadioButtonBorTitle, 0, 0);
   styleLayout->addWidget(m_pRadioButtonBor, 1, 0);
   styleLayout->addWidget(m_pRadioButtonNoBorNoTitle, 2, 0);
   styleLayout->addWidget(m_pShowMenuCheckBox, 0, 1);
   styleLayout->addWidget(m_pOnTopCheckBox, 1, 1);
   setLayout(styleLayout);
}

bool VisionWinStyle::getShowMenu( )
{
   if(m_pShowMenuCheckBox->checkState() == Qt::Unchecked)
      return false;
   return true;
}

bool VisionWinStyle::getAlwaysOnTop( )
{
   if(m_pOnTopCheckBox->checkState() == Qt::Unchecked)
      return false;
   return true;
}

// Private slots
void VisionWinStyle::showMenu(int show)
{
   if(show == Qt::Unchecked)
      m_pData->setShowMenu(false);
   else
      m_pData->setShowMenu(true);
   m_pData->setModified(VW_MOD_STYLE);
}

void VisionWinStyle::onTop(int top)
{
   if(top == Qt::Unchecked)
      m_pData->setAlwaysOnTop(false);
   else
      m_pData->setAlwaysOnTop(true);
   m_pData->setModified(VW_MOD_STYLE);
}
