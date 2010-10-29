/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// THIS EXAMPLE IS NOT YET EXPECTED TO WORK

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WGLWidget>

#include <Wt/WMatrix4x4>
#include <Wt/WGenericMatrix>
#include <Wt/WStringUtil>

#include <boost/tuple/tuple.hpp>

#include "readObj.h"

using namespace Wt;

const char *fragmentShaderSrc =
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"varying vec3 vLightWeighting;\n"
"\n"
"void main(void) {\n"
"  vec4 matColor = vec4(0.278, 0.768, 0.353, 1.0);\n"
"  gl_FragColor = vec4(matColor.rgb * vLightWeighting, matColor.a);\n"
"}\n";

const char *vertexShaderSrc =
"attribute vec3 aVertexPosition;\n"
"attribute vec3 aVertexNormal;\n"
"\n"
"uniform mat4 uMVMatrix;\n"
"uniform mat4 uCMatrix;\n"
"uniform mat4 uPMatrix;\n"
"uniform mat4 uNMatrix;\n"
"\n"
"varying vec3 vLightWeighting;\n"
"\n"
"void main(void) {\n"
"  gl_Position = uPMatrix * uCMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"  vec3 transformedNormal = normalize((uNMatrix * vec4(normalize(aVertexNormal), 0)).xyz);\n"
"  vec3 uLightingDirection = normalize(vec4(1, 1, 1, 0)).xyz;\n"
"  float directionalLightWeighting = max(dot(transformedNormal, uLightingDirection), 0.0);\n"
"  vec3 uAmbientLightColor = vec3(0.2, 0.2, 0.2);\n"
"  vec3 uDirectionalColor = vec3(0.8, 0.8, 0.8);\n"
"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;"
"}\n";

std::vector<double> data;

void centerpoint(double &x, double &y, double &z)
{
  double minx, miny, minz;
  double maxx, maxy, maxz;
  minx = maxx = data[0];
  miny = maxy = data[1];
  minz = maxz = data[2];
  for (unsigned int i = 0; i < data.size()/6; ++i) {
    if (data[i*6] < minx) minx = data[i*6];
    if (data[i*6] > maxx) maxx = data[i*6];
    if (data[i*6 + 1] < miny) miny = data[i*6 + 1];
    if (data[i*6 + 1] > maxy) maxy = data[i*6 + 1];
    if (data[i*6 + 2] < minz) minz = data[i*6 + 2];
    if (data[i*6 + 2] > maxz) maxz = data[i*6 + 2];
  }
  x = (minx + maxx)/2.;
  y = (miny + maxy)/2.;
  z = (minz + maxz)/2.;
}

class PaintWidget: public WGLWidget
{
public:
  PaintWidget(WContainerWidget *root):
    WGLWidget(root)
  {
  }
  
  void initializeGL()
  {
    jsMatrix_ = createJavaScriptMatrix4();
    WMatrix4x4 worldTransform;
    double cx, cy, cz;
    centerpoint(cx, cy, cz);
    worldTransform.lookAt(cx, cy, cz + 10, cx, cy, cz, 0, 1, 0);
    setJavaScriptMatrix4(jsMatrix_, worldTransform);

    setClientSideLookAtHandler(jsMatrix_, cx, cy, cz, 0, 1, 0, 0.005, 0.005);
    //setClientSideWalkHandler(jsMatrix_, 0.05, 0.005);
    // First, load a simple shader
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

    vertexNormalAttribute_   = getAttribLocation(shaderProgram_, "aVertexNormal");
    vertexPositionAttribute_ = getAttribLocation(shaderProgram_, "aVertexPosition");
    enableVertexAttribArray(vertexPositionAttribute_);
    enableVertexAttribArray(vertexNormalAttribute_);

    pMatrixUniform_  = getUniformLocation(shaderProgram_, "uPMatrix");
    cMatrixUniform_  = getUniformLocation(shaderProgram_, "uCMatrix");
    mvMatrixUniform_ = getUniformLocation(shaderProgram_, "uMVMatrix");
    nMatrixUniform_  = getUniformLocation(shaderProgram_, "uNMatrix");

    // Now, preload buffers
    objVertexBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, objVertexBuffer_);
    bufferDatafv(ARRAY_BUFFER, data.begin(), data.end(), STATIC_DRAW);

  }

  void paintGL()
  {
    // Drawing starts here!
    //clearColor(0.8, 0.8, 0.8, 1);
    clearColor(0, 0, 0, 0);
    clearDepth(1);
    enable(DEPTH_TEST);
    //disable(CULL_FACE);
    depthFunc(LEQUAL);
    viewport(0, 0, 500, 500);
    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    WMatrix4x4 proj;
    proj.perspective(45, 1, 1, 40);
    useProgram(shaderProgram_);
    // Set projection matrix
    uniformMatrix4(pMatrixUniform_, proj);
    uniformMatrix4(cMatrixUniform_, jsMatrix_);

    bindBuffer(ARRAY_BUFFER, objVertexBuffer_);
    vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 2*3*4, 0);
    vertexAttribPointer(vertexNormalAttribute_, 3, FLOAT, false, 2*3*4, 3*4);
    WMatrix4x4 modelMatrix;
    uniformMatrix4(mvMatrixUniform_, modelMatrix);
    uniformNormalMatrix4(nMatrixUniform_, jsMatrix_, modelMatrix);
    drawArrays(TRIANGLES, 0, data.size()/6);
  }

  void resizeGL(int width, int height)
  {
  }
private:
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
  gl->resize(500, 500);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new WebGLDemo(env);
}

int main(int argc, char **argv)
{
  //readObj("plane.obj", data);
  readObj("teapot.obj", data);
  //readObj("treeleaves.obj", data);
  //readObj("treebranches.obj", data);

  return WRun(argc, argv, &createApplication);
}

