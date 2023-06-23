
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWDATA_H_
#define VWDATA_H_

#include <QtWidgets>

#include "VWWindowProperties.h"
#include "VWInputProperties.h"

#ifdef INCLUDE_VISION
# define DRIVER_TAG  "Vision"
#else
#ifdef INCLUDE_VISIONLC
# define DRIVER_TAG  "VisionLC"
#endif
#endif

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class VWData : public QThread
{
    Q_OBJECT

public:
    VWData(QObject *parent = 0);
    VWData(VWData* data);
    ~VWData( );

    void openVideoDevice( );
    void closeVideoDevice(int fd);

    // Monitor methods
    void setExit(bool exit) { m_bExit = exit; };

    // Update value methods
    void getLatestValues( );
    int  setRequestedIndexDeviceInput(int index) { return m_inputProperties.setRequestedIndexDeviceInput(index); };

    // Monitor methods
    void startMonitor( );
    void stopMonitor( );

    // Device getter methods
    int     getVideoDeviceHandle( ) { return m_video_handle; }
    QString getVideoDeviceNode( ) { return m_node; };
    QString getVersion( ) { return m_inputProperties.getVersion(); };
    int     getNumDevices( ) const { return m_inputProperties.getNumDevices(); };
    int     getNumInputs( ) const { return m_inputProperties.getNumInputs(); };
    int     getCurrentDevice( ) const { return m_inputProperties.getCurrentDevice(); };
    int     getCurrentInput( ) const { return m_inputProperties.getCurrentInput(); };
    int     getCurrentIndex( ) const { return m_inputProperties.getCurrentIndex(); };
    int     getCurrentClient( ) const { return m_inputProperties.getCurrentClient(); };

    // Window Properties Getter methods
    QPair<int, int>& getPosition( ) { return m_windowProperties.getPosition(); };
    QPair<int, int>& getPositionMin( ) { return m_windowProperties.getPositionMin(); };
    QPair<int, int>& getPositionMax( ) { return m_windowProperties.getPositionMax(); };

    QRect& getClientPosition( ) { return m_windowProperties.getClientPosition(); };

    eVWPixelFormat getPixelFormat( ) { return m_windowProperties.getPixelFormat(); };
    eVWAspectRatio getAspectRatioType( ) { return m_windowProperties.getAspectRatioType(); };
    eVWBorderTitle getBorderTitleType( ) { return m_windowProperties.getBorderTitleType(); };
    eVWLiveStream  getLiveStream( ) { return m_windowProperties.getLiveStream(); };
    bool getShowMenu( ) { return m_windowProperties.getShowMenu(); };
    bool getAlwaysOnTop( ) { return m_windowProperties.getAlwaysOnTop(); };

    float getSourceAR( ) { return m_inputProperties.getSourceAR(); };

    QString getCaption( ) { return m_windowProperties.getCaption(); };
    QString getRealCaption(QString caption);
    QString getRealCaption( );

    int getActiveCaptureRate( ) { return m_windowProperties.getActiveCaptureRate(); };
    int getInactiveCaptureRate( ) { return m_windowProperties.getInactiveCaptureRate(); };
    bool getInactiveRateEnabled( ) { return m_windowProperties.getInactiveRateEnabled(); };

    // Input Properties Getter methods
    enum _eVWSignalType getSignalType() const { return m_inputProperties.getSignalType(); };
    enum _eVWVidStd getVideoStd() const { return m_inputProperties.getVideoStd(); };

    int getVidTimingWidth() const { return m_inputProperties.getVidTimingWidth(); };
    int getVidTimingWidthMin() const { return m_inputProperties.getVidTimingWidthMin(); };
    int getVidTimingWidthMax() const { return m_inputProperties.getVidTimingWidthMax(); };
    int getVidTimingWidthDef() const { return m_inputProperties.getVidTimingWidthDef(); };
    int getVidTimingHeight() const { return m_inputProperties.getVidTimingHeight(); };
    int getVidTimingHeightMin() const { return m_inputProperties.getVidTimingHeightMin(); };
    int getVidTimingHeightMax() const { return m_inputProperties.getVidTimingHeightMax(); };
    int getVidTimingHeightDef() const { return m_inputProperties.getVidTimingHeightDef(); };
    double getVidTimingRefresh() const { return m_inputProperties.getVidTimingRefresh(); };

    int getColAdjBrightness() const { return m_inputProperties.getColAdjBrightness(); };
    int getColAdjBrightnessMin() const { return m_inputProperties.getColAdjBrightnessMin(); };
    int getColAdjBrightnessMax() const { return m_inputProperties.getColAdjBrightnessMax(); };
    int getColAdjBrightnessDef() const { return m_inputProperties.getColAdjBrightnessDef(); };
    int getColAdjContrast() const { return m_inputProperties.getColAdjContrast(); };
    int getColAdjContrastMin() const { return m_inputProperties.getColAdjContrastMin(); };
    int getColAdjContrastMax() const { return m_inputProperties.getColAdjContrastMax(); };
    int getColAdjContrastDef() const { return m_inputProperties.getColAdjContrastDef(); };
    int getColAdjSaturation() const { return m_inputProperties.getColAdjSaturation(); };
    int getColAdjSaturationMin() const { return m_inputProperties.getColAdjSaturationMin(); };
    int getColAdjSaturationMax() const { return m_inputProperties.getColAdjSaturationMax(); };
    int getColAdjSaturationDef() const { return m_inputProperties.getColAdjSaturationDef(); };
    int getColAdjHue() const { return m_inputProperties.getColAdjHue(); };
    int getColAdjHueMin() const { return m_inputProperties.getColAdjHueMin(); };
    int getColAdjHueMax() const { return m_inputProperties.getColAdjHueMax(); };
    int getColAdjHueDef() const { return m_inputProperties.getColAdjHueDef(); };

    int getVidAdjHorPos() const { return m_inputProperties.getVidAdjHorPos(); };
    int getVidAdjHorPosMin() const { return m_inputProperties.getVidAdjHorPosMin(); };
    int getVidAdjHorPosMax() const { return m_inputProperties.getVidAdjHorPosMax(); };
    int getVidAdjHorPosDef() const { return m_inputProperties.getVidAdjHorPosDef(); };
    int getVidAdjHorSize() const { return m_inputProperties.getVidAdjHorSize(); };
    int getVidAdjHorSizeMin() const { return m_inputProperties.getVidAdjHorSizeMin(); };
    int getVidAdjHorSizeMax() const { return m_inputProperties.getVidAdjHorSizeMax(); };
    int getVidAdjHorSizeDef() const { return m_inputProperties.getVidAdjHorSizeDef(); };
    int getVidAdjPhase() const { return m_inputProperties.getVidAdjPhase(); };
    int getVidAdjPhaseMin() const { return m_inputProperties.getVidAdjPhaseMin(); };
    int getVidAdjPhaseMax() const { return m_inputProperties.getVidAdjPhaseMax(); };
    int getVidAdjPhaseDef() const { return m_inputProperties.getVidAdjPhaseDef(); };
    int getVidAdjVertPos() const { return m_inputProperties.getVidAdjVertPos(); };
    int getVidAdjVertPosMin() const { return m_inputProperties.getVidAdjVertPosMin(); };
    int getVidAdjVertPosMax() const { return m_inputProperties.getVidAdjVertPosMax(); };
    int getVidAdjVertPosDef() const { return m_inputProperties.getVidAdjVertPosDef(); };
    int getVidAdjBlackLevel() const { return m_inputProperties.getVidAdjBlackLevel(); };
    int getVidAdjBlackLevelMin() const { return m_inputProperties.getVidAdjBlackLevelMin(); };
    int getVidAdjBlackLevelMax() const { return m_inputProperties.getVidAdjBlackLevelMax(); };
    int getVidAdjBlackLevelDef() const { return m_inputProperties.getVidAdjBlackLevelDef(); };

    int getColBalBrightness(eVWColBalType type) const { return m_inputProperties.getColBalBrightness(type); };
    int getColBalBrightnessMin(eVWColBalType type) const { return m_inputProperties.getColBalBrightnessMin(type); };
    int getColBalBrightnessMax(eVWColBalType type) const { return m_inputProperties.getColBalBrightnessMax(type); };
    int getColBalBrightnessDef(eVWColBalType type) const { return m_inputProperties.getColBalBrightnessDef(type); };
    int getColBalContrast(eVWColBalType type) const { return m_inputProperties.getColBalContrast(type); };
    int getColBalContrastMin(eVWColBalType type) const { return m_inputProperties.getColBalContrastMin(type); };
    int getColBalContrastMax(eVWColBalType type) const { return m_inputProperties.getColBalContrastMax(type); };
    int getColBalContrastDef(eVWColBalType type) const { return m_inputProperties.getColBalContrastDef(type); };

    void getCropping(sVWAOI *pAOI) const;
    eVWCroppingType getCroppingType( ) const \
       { return m_inputProperties.getCroppingType( ); };

    // Setter methods
    void setCurrentIndex(int index) { m_inputProperties.setCurrentIndex(index); };
    void setCurrentClient(int id) { m_inputProperties.setCurrentClient(id); };

    // Window Properties Setter methods
    void setPosition(QPair<int, int> pos) { m_windowProperties.setPosition(pos); };
    void setClientPosition(QRect pos ) { m_windowProperties.setClientPosition(pos); };
    void setCaption(const QString& caption) { m_windowProperties.setCaption(caption); };
    void setActiveCaptureRate(int rate) { m_windowProperties.setActiveCaptureRate(rate); };
    void setInactiveCaptureRate(int rate) { m_windowProperties.setInactiveCaptureRate(rate); };
    void setInactiveRateEnabled(bool enabled) { m_windowProperties.setInactiveRateEnabled(enabled); };

    // Window Properties Setter methods
    void setPixelFormat(eVWPixelFormat fmt);
    void setAspectRatioType(eVWAspectRatio ar);
    void setBorderTitleType(eVWBorderTitle bt);
    void setLiveStream(eVWLiveStream ls) { m_windowProperties.setLiveStream(ls); };

    void setShowMenu(bool show);
    void setAlwaysOnTop(bool onTop);

    // Input Properties Setter methods
    void setVidTimingWidth(unsigned long value, bool force) { m_inputProperties.setVidTimingWidth(value, force); };
    void setVidTimingHeight(unsigned long value, bool force) { m_inputProperties.setVidTimingHeight(value, force); };

    void setBrightness(unsigned long value, bool force) { m_inputProperties.setBrightness(value, force); };
    void setContrast(unsigned long value, bool force) { m_inputProperties.setContrast(value, force); };
    void setSaturation(unsigned long value, bool force) { m_inputProperties.setSaturation(value, force); };
    void setHue(unsigned long value, bool force) { m_inputProperties.setHue(value, force); };

    void setHorPos(unsigned long value, bool force) { m_inputProperties.setHorPos(value, force); };
    void setHorSize(unsigned long value, bool force) { m_inputProperties.setHorSize(value, force); };
    void setPhase(unsigned long value, bool force) { m_inputProperties.setPhase(value, force); };
    void setVertPos(unsigned long value, bool force) { m_inputProperties.setVertPos(value, force); };
    void setBlackLevel(unsigned long value, bool force) { m_inputProperties.setBlackLevel(value, force); };

    void setColBalBrightness(unsigned long value, eVWColBalType type, bool force) { m_inputProperties.setColBalBrightness(value, type, force); };
    void setColBalContrast(unsigned long value, eVWColBalType type, bool force) { m_inputProperties.setColBalContrast(value, type, force); };

    void setCropping(sVWAOI AOI, eVWCroppingType type);

    void updateLiveStream(eVWLiveStream ls) { emit updateLS(ls); };
    void updateVideoNode( ) { m_node = m_inputProperties.getCurrentIndexNode(); };

    // Modification operators
    unsigned int getModified( );
    unsigned int getModified(unsigned int mask);
    void setModified(unsigned int modified);
    void clearModified(unsigned int modified);

    // Overloaded operators
    VWData& operator=(VWData& data);

    //
    VWWindowProperties& getWindowProperties( ) { return m_windowProperties; };
    VWInputProperties& getInputProperties( ) { return m_inputProperties; };

signals:
   void windowPropertiesRestore();
   void inputPropertiesRestore();
   void inputPropertiesReset();
   void inputValuesChanged();
   void positionValuesChanged();
   void clientParmsChanged();
   void aspectRatioTypeChanged();
   void borderTitleTypeChanged();

   void newSourceIndex();
   void restartVideo();

   void enableApply(bool);

   void updateLS(eVWLiveStream ls);

public slots:
   void sourceIndexChanged(int index);
   void sourceInputChanged( );

   void reset( );

protected:
   void run( );

private slots:
   void inputEvent( ) { emit newSourceIndex(); };

private:
   // Parent
   QObject            *p;
   // Private window properties helpers
   QRect& getCurrentFramePosition() const;
   QRect& getMaxWindowPosition() const;
   QRect& getCurrentClientSize() const;

   // Input data
   int                 m_video_handle;   // Currently opened video device file descriptor
   QString             m_node;           // Currently opened video device node

   VWWindowProperties  m_windowProperties;
   VWInputProperties   m_inputProperties;

   bool                m_bExit;
};

#endif /* VWDATA_H_ */
