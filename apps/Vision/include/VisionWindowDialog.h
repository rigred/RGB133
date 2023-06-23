
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VISIONWINDOWDIALOG_H_
#define VISIONWINDOWDIALOG_H_

#include <QDialog>

#include <stdio.h>

#include "rgb133control.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QTabWidget;
class QScrollArea;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
QT_END_NAMESPACE

class WindowTab;
class InputTab;
class VWData;

typedef enum _eVWDialogTab {
   DEF_PROP_TAB = 0,
   WIN_PROP_TAB = 0,
   INP_PROP_TAB,
} eVWDialogTab;

class VisionWindowDialog : public QDialog
{
    Q_OBJECT

public:
    VisionWindowDialog(QMainWindow *parent, VWData* data, eVWDialogTab tab = DEF_PROP_TAB);
    ~VisionWindowDialog();

    // Window getter methods
    QRect& getCurrentFramePosition() const;
    QRect& getMaxWindowPosition() const;
    QRect& getCurrentClientSize() const;

protected:
    void applyChanges(bool save);
    bool hasWidthChanged(QRect to, QRect from);
    bool hasHeightChanged(QRect to, QRect from);

    void positionRestore( );
    void sizeRestore( );
    void windowRestore( );
    void titleRestore( );
    void rateRestore(bool active);

    void vidTimingRestore( );
    void vidAdjRestore( );
    void rotationRestore( );
    void captureSettingsRestore( );
    void croppingRestore( );
    void colAdjRestore( );

signals:

public slots:
   void enableApply(bool enable);
   void applyCaptureRate(bool active);
   void rereadControls( );

private slots:
   void resaveData( );

   void windowPropertiesRestore( );
   void inputPropertiesRestore( );
   void apply();
   void accepted( );
   void cancel( );

   void positionChanged( );
   void sizeChanged( );
   void windowChanged( );
   void titleChanged( );
   void rateChanged( );
   void pixelFormatChanged( );
   void liveStreamChanged( );

private:
    QMainWindow       *p;

    VWData            *m_pData;
    VWData            *m_pSavedData;

    QLabel            *m_pInfoLabel;

    QDialogButtonBox  *m_pButtonBox;
    QPushButton       *m_pApplyButton;

    // Tab Data
    QTabWidget        *m_pTabWidget;

    WindowTab         *m_pWindowTab;
    InputTab          *m_pInputTab;
};

#endif /*VISIONWINDOWDIALOG_H_*/
