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

#include "geometryengine.h"

#include <QVector2D>
#include <QVector3D>

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

//! [0]
GeometryEngine::GeometryEngine()
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    generatePlaneGeometry(10, 10, 200, 200);
}

GeometryEngine::~GeometryEngine()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}
//! [0]

void GeometryEngine::generatePlaneGeometry(int width, int length, int widthSegments, int lengthSegments, bool repeatUVs, bool reverseWinding)
{
    int xCount = widthSegments + 1;
    int zCount = lengthSegments + 1;

    int vertexCount = xCount * zCount;
    int indexCount = 2 * vertexCount - 2 * xCount + zCount - 1;

    float uFactor = static_cast<float>(xCount) / (xCount - 1);
    float vFactor = static_cast<float>(zCount) / (zCount - 1);
    // Create vertices by lines
    // 0 -- 1 -- ... -- W
    // |    |     |     |
    // W+1  . --  . --  2W
    // |    |     |     |
    // . -- . --  . --  .
    // |    |     |     |
    // . -- . --  . --  LW

    std::vector<VertexData> vertices;
    std::vector<GLushort> indices;

    vertices.reserve(vertexCount);
    indices.reserve(indexCount);

    for (int z = 0; z < zCount; z++)
    {
        for (int x = 0; x < xCount; x++)
        {
            float u = static_cast<float>(x) / xCount;
            float v = static_cast<float>(z) / zCount;
            QVector3D position(u * width, 0.0F, v * length);
            if (repeatUVs)
            {
                QVector2D texCoords(x, z);
                vertices.push_back({position, texCoords});
            }
            else
            {
                QVector2D texCoords(u * uFactor, v * vFactor);
                vertices.push_back({position, texCoords});
            }
        }
    }

    // Indices (Strip)
    // 0  w+1   1  w+2   3 ....  w-1 2w-1 2w-1
    // w 2w+1 W+1 2w+2 w+3 .... 2w-1 3w-1 3w-1
    // ...
    // pair => left to right
    // impair => right to left


    for (int line = 0; line < zCount - 1; line++)
    {
        if ((line & 1) == 0) // Even
        {
            for (int segment = 0; segment < xCount; segment ++)
            {
                indices.push_back(line * xCount + segment);
                indices.push_back((line + 1) * xCount + segment);
            }
            indices.push_back((line + 1) * xCount + xCount - 1);
        }
        else
        {
            for (int segment = xCount - 1; segment >= 0; segment--)
            {
                indices.push_back(line * xCount + segment);
                indices.push_back((line + 1) * xCount + segment);
            }
            indices.push_back((line + 1) * xCount);
        }
    }


    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), vertexCount * sizeof(VertexData));

    indexBuf.bind();
    if (reverseWinding == true)
    {
        std::vector<GLushort> correctIndices;
        correctIndices.reserve(indexCount);
        for (int i = indexCount - 2; i >= 0; i--)
        {
            correctIndices.push_back(indices[i]);
        }
        correctIndices.push_back(indices[0]);
        indexBuf.allocate(correctIndices.data(), indexCount * sizeof(GLushort));
    }
    else
    {
        indexBuf.allocate(indices.data(), indexCount * sizeof(GLushort));
    }

}

//! [2]
void GeometryEngine::draw(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("texCoord0");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size(), GL_UNSIGNED_SHORT, 0);
}
//! [2]
