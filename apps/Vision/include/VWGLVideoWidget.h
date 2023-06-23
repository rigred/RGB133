
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWGLWIDGET_H
#define VWGLWIDGET_H

#include <QtOpenGL/QGLWidget>

class VWGLVideoWidget : public QGLWidget {

    Q_OBJECT // must be included if Qt signals/slots are used

public:
    VWGLVideoWidget(QWidget *parent = NULL);
    ~VWGLVideoWidget( ) { };

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
};

#endif  /* VWGLWIDGET_H */
