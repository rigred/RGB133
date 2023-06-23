
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INPUTTAB_H
#define INPUTTAB_H

#include <QtWidgets>

#include "rgb133control.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class VWData;
class VisionWinSource;
class VisionWinCapSettings;
class VisionWinVidTiming;
class VisionWinCropping;
class VisionWinVideoAdj;
class VisionWinColourAdj;

class InputTab : public QWidget
{
    Q_OBJECT

public:
    InputTab(VWData* data, QWidget* parent = 0);

    QWidget* getParent() const { return p; };

signals:
   void vidTimingTypeChanged();
   void vidAdjTypeChanged();
   void colAdjTypeChanged();
   void croppingTypeChanged();
   void capSettingsTypeChanged();

public slots:
   void inputValuesChanged();

private:
   QWidget*              p;
    
   VWData                *m_pData;

   VisionWinSource*      source;
   VisionWinCapSettings* capSet;
   VisionWinVidTiming*   vidTiming;
   VisionWinCropping*    cropping;
   VisionWinVideoAdj*    vidAdj;
   VisionWinColourAdj*   colAdj;
};

#endif /* INPUTTAB_H */
