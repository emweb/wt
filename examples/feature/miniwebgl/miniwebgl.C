/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WText.h>
#include <Wt/WGLWidget.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

// This fragment shader simply paints white.
const char *fragmentShaderSrc =
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"void main(void) {\n"
"  gl_FragColor = vec4(1, 1, 1, 1);\n"
"}\n";

// This vertex shader does not transform at all
const char *vertexShaderSrc =
"attribute vec3 aVertexPosition;\n"
"\n"
"void main(void) {\n"
"  gl_Position = vec4(aVertexPosition, 1.0);\n"
"}\n";

class PaintWidget: public WGLWidget
{
public:
  PaintWidget():
    WGLWidget()
  {
  }

  void initializeGL()
  {
    // Create a shader
    Shader fragmentShader = createShader(FRAGMENT_SHADER);
    shaderSource(fragmentShader, fragmentShaderSrc);
    compileShader(fragmentShader);
    Shader vertexShader = createShader(VERTEX_SHADER);
    shaderSource(vertexShader, vertexShaderSrc);
    compileShader(vertexShader);
    shaderProgram_ = createProgram();
    attachShader(shaderProgram_, vertexShader);
    attachShader(shaderProgram_, fragmentShader);
    linkProgram(shaderProgram_);
    useProgram(shaderProgram_);

    // Extract the attribute location
    vertexPositionAttribute_ = getAttribLocation(shaderProgram_, "aVertexPosition");
    enableVertexAttribArray(vertexPositionAttribute_);

    // Now, preload the vertex buffer
    triangleVertexPositionBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, triangleVertexPositionBuffer_);
    double trianglePosition[] = {
       0.0, 0.5, 0.0,
      -0.5,-0.5, 0.0,
       0.5,-0.5, 0.0
    };
    bufferDatafv(ARRAY_BUFFER, trianglePosition, trianglePosition + 9, STATIC_DRAW);
  }

  void resizeGL(int width, int height)
  {
    viewport(0, 0, width, height);
  }

  void paintGL()
  {
    // Drawing starts here!
    clearColor(0, 0, 0, 1);
    disable(DEPTH_TEST);
    disable(CULL_FACE);
    clear(COLOR_BUFFER_BIT);

    useProgram(shaderProgram_);

    // Draw the scene
    bindBuffer(ARRAY_BUFFER, triangleVertexPositionBuffer_);
    vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 0, 0);
    drawArrays(TRIANGLES, 0, 3);
  }

private:
  Program shaderProgram_;
  AttribLocation vertexPositionAttribute_;

  Buffer triangleVertexPositionBuffer_;
};

/*
 * A simple WebGL demo application
 */
class MiniWebGL: public WApplication
{
public:
  MiniWebGL(const WEnvironment& env);
};

MiniWebGL::MiniWebGL(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Minimalistic WebGL Demo");

  root()->addWidget(cpp14::make_unique<WText>("This is a minimalistic demonstration "
    "application for WebGL. If your browser supports WebGL, or if Wt is built "
    "with OpenGL support, you will "
    "see a black square with a triangle inside."));

  root()->addWidget(cpp14::make_unique<WBreak>());

  PaintWidget *gl = root()->addWidget(cpp14::make_unique<PaintWidget>());
  gl->resize(640, 640);
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<MiniWebGL>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
