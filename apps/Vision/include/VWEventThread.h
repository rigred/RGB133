
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/*
 * VWEventThread.h
 *
 *  Created on: 7 Apr 2010
 *      Author: danny
 */

#ifndef VWEVENTTHREAD_H_
#define VWEVENTTHREAD_H_

#include <QThread>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class VWEventThread : public QThread
{
   Q_OBJECT

public:
   VWEventThread(int _fd);

   int pollCtrlDevice();
   void setPoll(int value);

   void run();

signals:
   void pollRefreshValues();

private:
   int fd;
   int poll;
};

#endif /* VWEVENTTHREAD_H_ */
