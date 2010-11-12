/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef PAINTWIDGET_H_
#define PAINTWIDGET_H_

#include <Wt/WGLWidget>

class PaintWidget: public Wt::WGLWidget
{
public:
  PaintWidget(Wt::WContainerWidget *root);
  
  void initializeGL();

  void paintGL();

  void resizeGL(int width, int height);

  void setShaders(const std::string &vertexShader,
      const std::string &fragmentShader);

private:
  std::string vertexShader_;
  std::string fragmentShader_;

  Program shaderProgram_;
  AttribLocation vertexPositionAttribute_;
  AttribLocation vertexNormalAttribute_;
  UniformLocation pMatrixUniform_;
  UniformLocation cMatrixUniform_;
  UniformLocation mvMatrixUniform_;
  UniformLocation nMatrixUniform_;

  JavaScriptMatrix4x4 jsMatrix_;

  Buffer objVertexBuffer_;
  Buffer objNormalBuffer_;
  Buffer objElementBuffer_;

  Buffer textureCoordBuffer_;
  Texture texture_;
};

#endif

