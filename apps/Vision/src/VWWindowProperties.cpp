
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "VWConfig.h"
#include "VWWindowProperties.h"
#include "VWData.h"

/**********************************************************************/
// Window Properties
VWWindowProperties::VWWindowProperties( )
{
   m_modified = VW_MOD_NONE;

   m_position.first    = 0; m_position.second    = 0;
   m_positionMin.first = 0; m_positionMin.second = 0;
   m_positionMax.first = 0; m_positionMax.second = 0;

   m_ePixFmt = VW_PIX_FMT_RGB565;
   m_eAspectRatio = VW_DO_NOT_MAINTAIN_AR;
   m_eBorderTitle = VW_BORDER_AND_TITLE;
   m_bShowMenu = true;
   m_bAlwaysOnTop = false;
   m_eLiveStream = VW_LIVESTREAM_OFF;

   m_caption = QString(DRIVER_TAG) + " - %input%";

   m_bInactiveRateEnabled = false;
}

VWWindowProperties::~VWWindowProperties( )
{
}

// Getter methods

// Setter methods
void VWWindowProperties::setPosition(QPair<int, int> position)
{
   m_position = position;
}

void VWWindowProperties::setPositionMin(QPair<int, int> position)
{
   m_positionMin.first = position.first;
   m_positionMin.second = position.second;
}

void VWWindowProperties::setPositionMax(QPair<int, int> position)
{
   m_positionMax.first = position.first;
   m_positionMax.second = position.second;
}

void VWWindowProperties::setClientPosition(QRect position)
{
   m_clientPosition = position;
}

void VWWindowProperties::setModified(unsigned int modified)
{
   if(modified)
   {
      m_modified |= modified;
   }
   else
   {
      m_modified = 0;
   }
}

void VWWindowProperties::setLiveStream(eVWLiveStream ls)
{
   m_eLiveStream = ls;
};

void VWWindowProperties::clearModified(unsigned int modified)
{
   if(modified)
   {
      m_modified ^= modified;
   }
}

// Overloaded operators
VWWindowProperties& VWWindowProperties::operator=(VWWindowProperties& properties)
{
   this->setPosition(properties.getPosition());
   this->setPositionMin(properties.getPositionMin());
   this->setPositionMax(properties.getPositionMax());

   this->setClientPosition(properties.getClientPosition());

   this->setPixelFormat(properties.getPixelFormat());
   this->setAspectRatioType(properties.getAspectRatioType());
   this->setBorderTitleType(properties.getBorderTitleType());
   this->setShowMenu(properties.getShowMenu());
   this->setAlwaysOnTop(properties.getAlwaysOnTop());

   this->setCaption(properties.getCaption());

   this->setActiveCaptureRate(properties.getActiveCaptureRate());
   this->setInactiveCaptureRate(properties.getInactiveCaptureRate());
   this->setInactiveRateEnabled(properties.getInactiveRateEnabled());

   return *this;
}
