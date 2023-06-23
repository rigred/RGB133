
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VWData.h"
#include "VWAboutDialog.h"
#include "VWSaveChangesDialog.h"

#include "VWColBal.h"
#include "VWColourBalDialog.h"

VisionWinColourBalDialog::VisionWinColourBalDialog(const QString &input, VWData* data, QWidget *parent)
    : QDialog(parent), m_pData(data)
{
   m_pSavedData = new VWData(m_pData);

   /* All */
   m_pAll = new VisionWinColBal("All Colours", VW_COL_BAL_ALL, m_pData);
   m_pAll->setDisabled(true);

   m_pRed = new VisionWinColBal("Red", VW_COL_BAL_RED, m_pData);
   connect(m_pRed->getBrightnessScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(redColBalBrightnessChanged(int)));
   connect(m_pRed->getContrastScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(redColBalContrastChanged(int)));

   m_pGreen = new VisionWinColBal("Green", VW_COL_BAL_GREEN, m_pData);
   connect(m_pGreen->getBrightnessScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(greenColBalBrightnessChanged(int)));
   connect(m_pGreen->getContrastScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(greenColBalContrastChanged(int)));

   m_pBlue = new VisionWinColBal("Blue", VW_COL_BAL_BLUE, m_pData);
   connect(m_pBlue->getBrightnessScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(blueColBalBrightnessChanged(int)));
   connect(m_pBlue->getContrastScrollBar(), SIGNAL(valueChanged(int)),
         this, SLOT(blueColBalContrastChanged(int)));
    
   m_pActionButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                    | QDialogButtonBox::Cancel
                                    | QDialogButtonBox::Help
                                    | QDialogButtonBox::Reset);
   m_pResetButton = m_pActionButtonBox->button(QDialogButtonBox::Reset);

   connect(m_pActionButtonBox, SIGNAL(accepted()), this, SLOT(accepted()));
   connect(m_pActionButtonBox, SIGNAL(rejected()), this, SLOT(rejected()));
   connect(m_pActionButtonBox, SIGNAL(helpRequested()), this, SLOT(showAbout()));
   connect(m_pResetButton, SIGNAL(clicked()), this, SLOT(reset()));
    
   QVBoxLayout *mainLayout = new QVBoxLayout;
   mainLayout->addWidget(m_pAll);
   mainLayout->addWidget(m_pRed);
   mainLayout->addWidget(m_pGreen);
   mainLayout->addWidget(m_pBlue);
   mainLayout->addWidget(m_pActionButtonBox);
   setLayout(mainLayout);

   QString title = "Colour Balance - " + input;
   setWindowTitle(title);
}

// Private slots
void VisionWinColourBalDialog::showAbout( )
{
   m_pAboutDialog = new VWAboutDialog(NULL, m_pData);
   m_pAboutDialog->exec();
}

void VisionWinColourBalDialog::accepted( )
{
   m_pData->clearModified(VW_MOD_COLOUR_BAL);

   if(m_pSavedData)
   {
      delete m_pSavedData;
      m_pSavedData = new VWData(m_pData);
   }
   accept();
}

void VisionWinColourBalDialog::rejected( )
{
   VWSaveChangesDialog save;
   if(m_pData->getModified(VW_MOD_COLOUR_BAL))
   {
      if(save.exec() != QDialog::Accepted)
      {
         m_pData->setColBalBrightness(m_pSavedData->getColBalBrightness(VW_COL_BAL_RED_BRIGHTNESS), VW_COL_BAL_RED_BRIGHTNESS, true);
         m_pData->setColBalContrast(m_pSavedData->getColBalContrast(VW_COL_BAL_RED_CONTRAST), VW_COL_BAL_RED_CONTRAST, true);
         m_pData->setColBalBrightness(m_pSavedData->getColBalBrightness(VW_COL_BAL_GREEN_BRIGHTNESS), VW_COL_BAL_GREEN_BRIGHTNESS, true);
         m_pData->setColBalContrast(m_pSavedData->getColBalContrast(VW_COL_BAL_GREEN_CONTRAST), VW_COL_BAL_GREEN_CONTRAST, true);
         m_pData->setColBalBrightness(m_pSavedData->getColBalBrightness(VW_COL_BAL_BLUE_BRIGHTNESS), VW_COL_BAL_BLUE_BRIGHTNESS, true);
         m_pData->setColBalContrast(m_pSavedData->getColBalContrast(VW_COL_BAL_BLUE_CONTRAST), VW_COL_BAL_BLUE_CONTRAST, true);

         *m_pData = *m_pSavedData;

         m_pRed->rejected(VW_COL_BAL_RED_BRIGHTNESS, VW_COL_BAL_RED_CONTRAST);
         m_pGreen->rejected(VW_COL_BAL_GREEN_BRIGHTNESS, VW_COL_BAL_GREEN_CONTRAST);
         m_pBlue->rejected(VW_COL_BAL_BLUE_BRIGHTNESS, VW_COL_BAL_BLUE_CONTRAST);

         m_pData->clearModified(VW_MOD_COLOUR_BAL);
      }
   }
   close();
}

void VisionWinColourBalDialog::reset( )
{
   m_pRed->reset(VW_COL_BAL_RED_BRIGHTNESS, VW_COL_BAL_RED_CONTRAST);
   m_pGreen->reset(VW_COL_BAL_GREEN_BRIGHTNESS, VW_COL_BAL_GREEN_CONTRAST);
   m_pBlue->reset(VW_COL_BAL_BLUE_BRIGHTNESS, VW_COL_BAL_BLUE_CONTRAST);
   m_pData->clearModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::redColBalBrightnessChanged(int)
{
   m_pData->setColBalBrightness(m_pRed->getBrightnessScrollBar()->value(), VW_COL_BAL_RED_BRIGHTNESS, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::redColBalContrastChanged(int)
{
   m_pData->setColBalContrast(m_pRed->getContrastScrollBar()->value(), VW_COL_BAL_RED_CONTRAST, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::greenColBalBrightnessChanged(int)
{
   m_pData->setColBalBrightness(m_pGreen->getBrightnessScrollBar()->value(), VW_COL_BAL_GREEN_BRIGHTNESS, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::greenColBalContrastChanged(int)
{
   m_pData->setColBalContrast(m_pGreen->getContrastScrollBar()->value(), VW_COL_BAL_GREEN_CONTRAST, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::blueColBalBrightnessChanged(int)
{
   m_pData->setColBalBrightness(m_pBlue->getBrightnessScrollBar()->value(), VW_COL_BAL_BLUE_BRIGHTNESS, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}

void VisionWinColourBalDialog::blueColBalContrastChanged(int)
{
   m_pData->setColBalContrast(m_pBlue->getContrastScrollBar()->value(), VW_COL_BAL_BLUE_CONTRAST, false);
   m_pData->setModified(VW_MOD_COLOUR_BAL);
}
