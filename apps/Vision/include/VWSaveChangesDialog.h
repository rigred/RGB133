
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWSAVECHANGESDIALOG_H_
#define VWSAVECHANGESDIALOG_H_

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QDialogButtonBox;
class QMainWindow;
QT_END_NAMESPACE

class VWSaveChangesDialog : public QDialog
{
    Q_OBJECT

public:
    VWSaveChangesDialog( );
    ~VWSaveChangesDialog( );

protected:

signals:

public slots:

private slots:

private:
    QLabel            *m_pLabel;
    QDialogButtonBox  *m_pButtonBox;

};

#endif /*VWSAVECHANGESDIALOG_H_*/
