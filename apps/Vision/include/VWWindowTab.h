
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef WINDOWTAB_H
#define WINDOWTAB_H

#include <QtWidgets>

#include "VWPosition.h"
#include "VWSize.h"
#include "VWAspectRatio.h"
#include "VWStyle.h"
#include "VWCaption.h"
#include "VWCaptureRate.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class VWData;
class VisionWinPosition;
class VisionWinSize;
class VisionWinInvalidInput;
class VisionWinCaptureFormat;
class VisionWinLiveStream;
class VisionWinDeinterlace;
class VisionWinTransferData;
class VisionWinScaling;
class VisionWinWindowID;

class WindowTab : public QWidget
{
    Q_OBJECT

public:
    WindowTab(VWData* data, QWidget* parent = 0);

    QWidget* getParent() const { return p; } ;

    QPoint   getPosition() const { return position->getPosition(); };
    QRect    getSize() const { return size->getSize(); };
    float    getSpecificAR() { return aspRatio->getSpecificAR(); };
    bool     getShowMenu() { return style->getShowMenu(); };
    bool     getAlwaysOnTop() { return style->getAlwaysOnTop(); };
    QString  getCaption() { return caption->getCaption(); };
    int      getActiveCaptureRate() { return capRate->getActiveCaptureRate(); };
    int      getInactiveCaptureRate() { return capRate->getInactiveCaptureRate(); };

    void updateSize(QRect& sz) { size->updateSize(sz); };

private:
    QWidget*                p;

    VWData                 *m_pData;
        
    VisionWinPosition*      position;
    VisionWinCaption*       caption;
    VisionWinSize*          size;
    VisionWinInvalidInput*  invInput;
    VisionWinAspectRatio*   aspRatio;
    VisionWinCaptureFormat* capFmt;
    VisionWinLiveStream*    liveStr;
    VisionWinDeinterlace*   deint;
    VisionWinStyle*         style;
    VisionWinTransferData*  xfer;
    VisionWinScaling*       scale;
    VisionWinWindowID*      id;
    VisionWinCaptureRate*   capRate;
};

#endif /* WINDOWTAB_H */
