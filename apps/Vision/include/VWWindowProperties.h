
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWPROPERTIES_H_
#define VWPROPERTIES_H_

#include <QtWidgets>

#include "VWProperties.h"

#include "rgb133control.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

enum eVWPixelFormat {
   VW_PIX_FMT_AUTO,
   VW_PIX_FMT_RGB555,
   VW_PIX_FMT_RGB565,
   VW_PIX_FMT_RGB888,
   VW_PIX_FMT_YUY2,
};

enum eVWAspectRatio {
   VW_DO_NOT_MAINTAIN_AR,
   VW_MAINTAIN_SRC_AR,
   VW_MAINTAIN_SPECIFIC_AR,
};

enum eVWBorderTitle {
   VW_BORDER_AND_TITLE,
   VW_BORDER_ONLY,
   VW_NO_BORDER_OR_TITLE,
};

enum eVWLiveStream {
   VW_LIVESTREAM_OFF = 0,
   VW_LIVESTREAM_ON,
};

class VWWindowProperties
{
public:
   VWWindowProperties( );
   ~VWWindowProperties( );

   QPair<int, int>& getPosition( ) { return m_position; };
   QPair<int, int>& getPositionMin( ) { return m_positionMin; };
   QPair<int, int>& getPositionMax( ) { return m_positionMax; };

   QRect& getClientPosition( ) { return m_clientPosition; };

   eVWPixelFormat getPixelFormat( ) { return m_ePixFmt; };
   eVWAspectRatio getAspectRatioType( ) { return m_eAspectRatio; };
   eVWBorderTitle getBorderTitleType( ) { return m_eBorderTitle; };
   eVWLiveStream  getLiveStream( ) { return m_eLiveStream; };
   bool getShowMenu( ) { return m_bShowMenu; };
   bool getAlwaysOnTop( ) { return m_bAlwaysOnTop; };

   QString getCaption( ) { return m_caption; };

   int getActiveCaptureRate( ) { return m_activeRate; };
   int getInactiveCaptureRate( ) { return m_inactiveRate; };
   bool getInactiveRateEnabled( ) { return m_bInactiveRateEnabled; };

   // Setter methods
   void setPosition(QPair<int, int> position);
   void setPositionMin(QPair<int, int> position);
   void setPositionMax(QPair<int, int> position);

   void setClientPosition(QRect position);

   void setPixelFormat(eVWPixelFormat fmt) { m_ePixFmt = fmt; };
   void setAspectRatioType(eVWAspectRatio ar) { m_eAspectRatio = ar; };
   void setBorderTitleType(eVWBorderTitle bt) { m_eBorderTitle = bt; };
   void setLiveStream(eVWLiveStream ls);
   void setShowMenu(bool show) { m_bShowMenu = show; };
   void setAlwaysOnTop(bool onTop) { m_bAlwaysOnTop = onTop; };
   void setCaption(const QString& caption) { m_caption = caption; };
   void setActiveCaptureRate(int rate) { m_activeRate = rate; };
   void setInactiveCaptureRate(int rate) { m_inactiveRate = rate; };
   void setInactiveRateEnabled(bool enabled) { m_bInactiveRateEnabled = enabled; };

   // Modification operators
   unsigned int getModified( ) { return m_modified; };
   void setModified(unsigned int modified);
   void clearModified(unsigned int modified);

   // Overloaded operators
   VWWindowProperties& operator=(VWWindowProperties& properties);

signals:

private:
   QPair<int, int> m_position;
   QPair<int, int> m_positionMin;
   QPair<int, int> m_positionMax;

   QRect           m_clientPosition;

   eVWPixelFormat  m_ePixFmt;
   eVWAspectRatio  m_eAspectRatio;
   eVWBorderTitle  m_eBorderTitle;
   eVWLiveStream   m_eLiveStream;
   bool            m_bShowMenu;
   bool            m_bAlwaysOnTop;

   QString         m_caption;

   int             m_activeRate;
   int             m_inactiveRate;
   bool            m_bInactiveRateEnabled;

   unsigned int    m_modified;

};

#endif /* VWWINDOWPROPERTIES_H_ */
