/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WGLWidget>

using namespace Wt;

const char *fragmentShaderSrc =
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"varying vec4 vColor;\n"
"\n"
"void main(void) {\n"
"  gl_FragColor = vColor;\n"
"}\n";

const char *vertexShaderSrc =
"attribute vec3 aVertexPosition;\n"
"attribute vec4 aVertexColor;\n"
"\n"
"uniform mat4 uMVMatrix;\n"
"uniform mat4 uPMatrix;\n"
"\n"
"varying vec4 vColor;\n"
"\n"
"void main(void) {\n"
"  gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"  vColor = aVertexColor;\n"
"}\n";

class PaintWidget: public WGLWidget
{
public:
  PaintWidget(WContainerWidget *root):
    WGLWidget(root)
  {
  }
  
  void initializeGL()
  {
    Shader fragmentShader = createShader(FRAGMENT_SHADER);
    shaderSource(fragmentShader, fragmentShaderSrc);
    compileShader(fragmentShader);
    Shader vertexShader = createShader(VERTEX_SHADER);
    shaderSource(vertexShader, vertexShaderSrc);
    compileShader(vertexShader);
    Program shaderProgram = createProgram();
    attachShader(shaderProgram, vertexShader);
    attachShader(shaderProgram, fragmentShader);
    linkProgram(shaderProgram);
    useProgram(shaderProgram);

    vertexPositionAttribute_ = getAttribLocation(shaderProgram, "aVertexPosition");
    enableVertexAttribArray(vertexPositionAttribute_);

    vertexColorAttribute_ = getAttribLocation(shaderProgram, "aVertexColor");
    enableVertexAttribArray(vertexColorAttribute_);

    pMatrixUniform_ = getUniformLocation(shaderProgram, "uPMatrix");
    mvMatrixUniform_ = getUniformLocation(shaderProgram, "uMVMatrix");

    triangleVertexPositionBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, triangleVertexPositionBuffer_);
    double trianglePosition[] = {
      0.0, 1.0, 0.0,
      -1.0,-1.0, 0.0,
      1.0,-1.0, 0.0
    };
    bufferDatafv(ARRAY_BUFFER, trianglePosition, 9, STATIC_DRAW);

    triangleVertexColorBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, triangleVertexColorBuffer_);
    double triangleColor[] = {
      1.0,0.0,0.0,1.0,
      0.0,1.0,0.0,1.0,
      0.0,0.0,1.0,1.0
    };
    bufferDatafv(ARRAY_BUFFER, triangleColor, 12, STATIC_DRAW);

    squareVertexPositionBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, squareVertexPositionBuffer_);
    double squarePosition[] = {
       1.0, 1.0,0.0,
      -1.0, 1.0,0.0,
       1.0,-1.0,0.0,
      -1.0,-1.0,0.0
    };
    bufferDatafv(ARRAY_BUFFER, squarePosition, 12, STATIC_DRAW);

    squareVertexColorBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, squareVertexColorBuffer_);
    double squareColor[] = {
      1.0,0.0,0.0,1.0,
      0.0,1.0,0.0,1.0,
      0.0,0.0,1.0,1.0,
      0.0,1.0,1.0,1.0
    };
    bufferDatafv(ARRAY_BUFFER, squareColor, 16, STATIC_DRAW);
  }

  void paintGL()
  {
  // Drawing starts here!
  //clearColor(0.8, 0.8, 0.8, 1);
  clearColor(0, 0, 0, 1);
  clearDepth(1);
  disable(DEPTH_TEST);
  disable(CULL_FACE);
  depthFunc(LEQUAL);
  viewport(0, 0, 640, 360);
  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

  // Set projection matrix
  float projectionMatrix[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 4};
  uniformMatrix4fv(pMatrixUniform_, false, projectionMatrix);

  // Draw the scene
  bindBuffer(ARRAY_BUFFER, triangleVertexPositionBuffer_);
  vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 0, 0);
  bindBuffer(ARRAY_BUFFER, triangleVertexColorBuffer_);
  vertexAttribPointer(vertexColorAttribute_, 4, FLOAT, false, 0, 0);
  double modelMatrix1[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    -2.0, 0, 0, 1};
  uniformMatrix4fv(mvMatrixUniform_, false, modelMatrix1);
  drawArrays(TRIANGLES, 0, 3);

  bindBuffer(ARRAY_BUFFER, squareVertexPositionBuffer_);
  vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 0, 0);
  bindBuffer(ARRAY_BUFFER, squareVertexColorBuffer_);
  vertexAttribPointer(vertexColorAttribute_, 4, FLOAT, false, 0, 0);
  float modelMatrix2[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    2.0, 0, 0, 1};
  uniformMatrix4fv(mvMatrixUniform_, false, modelMatrix2);
  drawArrays(TRIANGLE_STRIP, 0, 4);
  }

  void resizeGL(int width, int height)
  {
  }
private:
  AttribLocation vertexPositionAttribute_;
  AttribLocation vertexColorAttribute_;
  UniformLocation pMatrixUniform_;
  UniformLocation mvMatrixUniform_;
  Buffer triangleVertexPositionBuffer_;
  Buffer triangleVertexColorBuffer_;
  Buffer squareVertexPositionBuffer_;
  Buffer squareVertexColorBuffer_;
};

/*
 * A simple WebGL demo application
 */
class WebGLDemo : public WApplication
{
public:
  WebGLDemo(const WEnvironment& env);

};

WebGLDemo::WebGLDemo(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("WebGL Demo");

  root()->addWidget(new WText("This is a preliminary WebGL demo. It will "
    "become more spectacular in time. This technology preview has only "
    "been tested on Chrome Canary builds. No 3D because I did "
    "not yet invest in 3D projection matrices."));
  root()->addWidget(new WBreak());

  PaintWidget *gl = new PaintWidget(root());
  gl->resize(640, 360);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new WebGLDemo(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

