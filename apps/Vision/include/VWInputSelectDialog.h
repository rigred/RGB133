
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWINPUTSELECTDIALOG_H_
#define VWINPUTSELECTDIALOG_H_

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QDialogButtonBox;
class QMainWindow;
QT_END_NAMESPACE

class VWData;

class VWInputSelectDialog : public QDialog
{
    Q_OBJECT

public:
    VWInputSelectDialog(QMainWindow *parent, VWData* data);
    ~VWInputSelectDialog();

    // Getter methods
    int getSourceNumber() const { return m_source; };
    int getInputIndex() const { return m_index; };

protected:

signals:

public slots:
   void setInput();
   void invalidateInput();

private slots:

private:
    QMainWindow       *p;

    VWData            *m_pData;

    int                m_source;
    int                m_index;

    QLabel            *m_pLabel;
    QComboBox         *m_pComboBox;
    QDialogButtonBox  *m_pButtonBox;

};

#endif /*VWINPUTSELECTDIALOG_H_*/
