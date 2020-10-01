/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>

#include <math.h>

#define TAU 6.28318530718

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    _cameraSpeed(1),
    _cameraDistance(3 * 0.70710678118 /* sqrt(2) / 2 */),
    _cameraAngle(0)
{
    view.setToIdentity();
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *)
{
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    // https://doc.qt.io/qt-5/qwidget.html#keyPressEvent
    // Calling base implementation is needed to have a correct handling
    QWidget::keyPressEvent(event);

    switch(event->key())
    {
    case Qt::Key_Left:
        _movementInput.setX(_movementInput.x() + 1.0F);
        break;
    case Qt::Key_Right:
        _movementInput.setX(_movementInput.x() - 1.0F);
        break;
    case Qt::Key_Up:
        _movementInput.setY(_movementInput.y() + 1.0F);
        break;
    case Qt::Key_Down:
        _movementInput.setY(_movementInput.y() - 1.0F);
        break;
    }
}

void MainWidget::keyReleaseEvent(QKeyEvent *event)
{
    // https://doc.qt.io/qt-5/qwidget.html#keyReleaseEvent
    // Calling base implementation is needed to have a correct handling
    QWidget::keyReleaseEvent(event);

    switch(event->key())
    {
    case Qt::Key_Left:
        _movementInput.setX(_movementInput.x() - 1.0F);
        break;
    case Qt::Key_Right:
        _movementInput.setX(_movementInput.x() + 1.0F);
        break;
    case Qt::Key_Up:
        _movementInput.setY(_movementInput.y() - 1.0F);
        break;
    case Qt::Key_Down:
        _movementInput.setY(_movementInput.y() + 1.0F);
        break;
    }
}

void MainWidget::timerEvent(QTimerEvent *)
{
//    if (_movementInput.lengthSquared() > 0.1F)
//    {
//        auto dX = _movementInput.normalized() * -0.01;
//        view.translate(dX.x(), 0, dX.y());
//        update();
//    }

    _cameraAngle += _cameraSpeed * 0.016 / _cameraDistance;
    if (_cameraAngle > TAU) _cameraAngle -= TAU;
    QVector3D camPos(
        _cameraDistance * cos(_cameraAngle),
        _cameraDistance,
        _cameraDistance * sin(_cameraAngle)
    );

    // Compute the up vector.
    QVector3D camUp(camPos);
    camUp.setX(-camUp.x());
    camUp.setZ(-camUp.z());

    // LookAt method directly gives inverted matrix
    // This is not optimal here because we work with the camera
    // model, but we don't care about perfs for now.
    QMatrix4x4 lookAtMat;
    lookAtMat.lookAt(camPos, QVector3D(0.0F, 0.0F, 0.0F), camUp);
    view = lookAtMat.inverted();

    update();
}

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    geometries = std::make_unique<GeometryEngine>();

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    std::array<QImage, 3> images {
        QImage(":/grass.png"),
        QImage(":/rock.png"),
        QImage(":/snowrocks.png")
    };

    for (int i = 0; i < 3; i++)
    {
        terrainTextures[i] = std::make_unique<QOpenGLTexture>(images[i]);

        // Set nearest filtering mode for texture minification
        terrainTextures[i]->setMinificationFilter(QOpenGLTexture::Nearest);

        // Set bilinear filtering mode for texture magnification
        terrainTextures[i]->setMagnificationFilter(QOpenGLTexture::Linear);

        // Wrap texture coordinates by repeating
        // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
        terrainTextures[i]->setWrapMode(QOpenGLTexture::Repeat);

    }

    heightMap = std::make_unique<QOpenGLTexture>(QImage(":/Heightmap_Rocky.png"));
}

void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.3, zFar = 100.0, fov = 60.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    heightMap->bind(0);
    for (int i = 0; i < 3; i++)
    {
        terrainTextures[i]->bind(i + 1);
    }

    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(-5.0, -1, -5.0);

    // Set modelview-projection matrix
    program.setUniformValue("mat_ObjectToWorld", matrix);
    program.setUniformValue("mat_WorldToView", view.inverted());
    program.setUniformValue("mat_ViewToClip", projection);

    program.setUniformValue("cam_Position", view.column(3));

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("_HeightMap", 0);

    program.setUniformValue("_GrassTexMap", 1);
    program.setUniformValue("_RockTexMap", 2);
    program.setUniformValue("_SnowrockTexMap", 3);

    // Draw cube geometry
    geometries->draw(&program);
}
