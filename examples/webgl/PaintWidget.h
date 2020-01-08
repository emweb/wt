/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef PAINTWIDGET_H_
#define PAINTWIDGET_H_

#include <Wt/WGLWidget.h>

using namespace Wt;

// You must inherit Wt::WGLWidget to draw a 3D scene
class PaintWidget: public WGLWidget
{
public:
  PaintWidget(const bool & useBinaryBuffers);
  
  // Specialization of WGLWidgeT::intializeGL()
  void initializeGL();

  // Specialization of WGLWidgeT::paintGL()
  void paintGL();

  // Specialization of WGLWidgeT::resizeGL()
  void resizeGL(int width, int height);

  // Sets the shader source. Must be set before the widget is first rendered.
  void setShaders(const std::string &vertexShader,
      const std::string &fragmentShader);

private:
  bool initialized_;

  // The shaders, in plain text format
  std::string vertexShader_;
  std::string fragmentShader_;

  // Program and related variables
  Program         shaderProgram_;
  AttribLocation  vertexPositionAttribute_;
  AttribLocation  vertexNormalAttribute_;
  UniformLocation pMatrixUniform_;
  UniformLocation cMatrixUniform_;
  UniformLocation mvMatrixUniform_;
  UniformLocation nMatrixUniform_;

  // A client-side JavaScript matrix variable
  JavaScriptMatrix4x4 jsMatrix_;

  // The so-called VBOs, Vertex Buffer Objects
  // This one contains both vertex (xyz) and normal (xyz) data
  Buffer objBuffer_;

  bool useBinaryBuffers_;
};

#endif

