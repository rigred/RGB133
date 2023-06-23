
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QApplication>

#include "VisionWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    VisionWindow window;

    QFont font = QApplication::font();
    font.setPointSize(8);
    QApplication::setFont(font);
    
    if(window.isOk())
       window.show();
    else
       return 0;

    return app.exec();
}
