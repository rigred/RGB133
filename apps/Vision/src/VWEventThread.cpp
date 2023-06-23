
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sys/select.h>
#include <stdio.h>

#include "VWConfig.h"
#include "VWEventThread.h"

VWEventThread::VWEventThread(int _fd)
   : fd(_fd)
{
   this->poll = 1;
}

int VWEventThread::pollCtrlDevice()
{
   fd_set reads;
   int n;

   FD_ZERO(&reads);

   FD_SET(0, &reads);
   FD_SET(fd, &reads);

   while(this->poll)
   {
      if((n = ::select(fd+1, &reads, NULL, NULL, NULL)) == -1)
      {
         perror("VWEventThread::pollCtrlDevice\n");
         return -1;
      }
      else if(n == 0) { /* timeout */ }

      if(this->poll)
      {
         if(FD_ISSET(fd, &reads))
            emit pollRefreshValues();
      }
   }

   return 0;
}

void VWEventThread::setPoll(int value)
{
   this->poll = value;
}

void VWEventThread::run()
{
   pollCtrlDevice();
}
