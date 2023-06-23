
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWABOUTDIALOG_H_
#define VWABOUTDIALOG_H_

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
class QDialogButtonBox;
class QMainWindow;
class QVBoxLayout;
QT_END_NAMESPACE

class VWData;

class VWAboutDialog : public QDialog
{
    Q_OBJECT

public:
    VWAboutDialog(QMainWindow *parent, VWData* data);
    ~VWAboutDialog();

protected:

signals:

public slots:

private slots:

private:
    QMainWindow       *p;

    VWData            *m_pData;

    QVBoxLayout       *m_pLayout;
    QLabel            *m_pVisionLabel;
    QLabel            *m_pVersionLabel;
    QDialogButtonBox  *m_pButtonBox;
};

#endif /*VWABOUTDIALOG_H_*/
