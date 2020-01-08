/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PaintWidget.h"

#include <Wt/WGLWidget.h>
#include <Wt/WMatrix4x4.h>
#include <Wt/WMemoryResource.h>

// To avoid copying large constant data around, the data points are stored
// in a global variable.
extern std::vector<float> data;

// Calculates the centerpoint of the data. This is where the camera looks at.
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

PaintWidget::PaintWidget(const bool & useBinaryBuffers):
  WGLWidget(), initialized_(false), useBinaryBuffers_(useBinaryBuffers)
{
  jsMatrix_ = JavaScriptMatrix4x4();
  addJavaScriptMatrix4(jsMatrix_);
}

// The initializeGL() captures all JS commands that are to be executed
// before the scene is rendered for the first time. It is executed only
// once. It is re-executed when the WebGL context is restored after it
// was lost.
// In general, it should be used to set up shaders, create VBOs, initialize
// matrices, ...
void PaintWidget::initializeGL()
{
  // In order to know where to look at, calculate the centerpoint of the
  // scene
  double cx, cy, cz;
  centerpoint(cx, cy, cz);

  // Transform the world so that we look at the centerpoint of the scene
  WMatrix4x4 worldTransform;
  worldTransform.lookAt(
      cx, cy, cz + 10, // camera position
      cx, cy, cz,      // looking at
      0, 1, 0);        // 'up' vector

  // We want to be able to change the camera position client-side. In
  // order to do so, the world transformation matrix must be stored in
  // a matrix that can be manipulated from JavaScript.
  if (!initialized_) {
    initJavaScriptMatrix4(jsMatrix_);
    setJavaScriptMatrix4(jsMatrix_, worldTransform);

    // This installs a client-side mouse handler that modifies the
    // world transformation matrix. Like WMatrix4x4::lookAt, this works
    // by specifying a center point and an up direction; mouse movements
    // will allow the camera to be moved around the center point.
    setClientSideLookAtHandler(jsMatrix_, // the name of the JS matrix
	cx, cy, cz,                       // the center point
	0, 1, 0,                          // the up direction
	0.005, 0.005);                    // 'speed' factors
    // Alternative: this installs a client-side mouse handler that allows
    // to 'walk' around: go forward, backward, turn left, turn right, ...
    //setClientSideWalkHandler(jsMatrix_, 0.05, 0.005);
    initialized_ = true;
  }

  // First, load a simple shader
  Shader fragmentShader = createShader(FRAGMENT_SHADER);
  shaderSource(fragmentShader, fragmentShader_);
  compileShader(fragmentShader);
  Shader vertexShader = createShader(VERTEX_SHADER);
  shaderSource(vertexShader, vertexShader_);
  compileShader(vertexShader);
  shaderProgram_ = createProgram();
  attachShader(shaderProgram_, vertexShader);
  attachShader(shaderProgram_, fragmentShader);
  linkProgram(shaderProgram_);
  useProgram(shaderProgram_);

  // Extract the references to the attributes from the shader.
  vertexNormalAttribute_   =
    getAttribLocation(shaderProgram_, "aVertexNormal");
  vertexPositionAttribute_ =
    getAttribLocation(shaderProgram_, "aVertexPosition");
  enableVertexAttribArray(vertexPositionAttribute_);
  enableVertexAttribArray(vertexNormalAttribute_);

  // Extract the references the uniforms from the shader
  pMatrixUniform_  = getUniformLocation(shaderProgram_, "uPMatrix");
  cMatrixUniform_  = getUniformLocation(shaderProgram_, "uCMatrix");
  mvMatrixUniform_ = getUniformLocation(shaderProgram_, "uMVMatrix");
  nMatrixUniform_  = getUniformLocation(shaderProgram_, "uNMatrix");

  // Create a Vertex Buffer Object (VBO) and load all polygon's data
  // (points, normals) into it. In this case we use one VBO that contains
  // all data (6 per point: vx, vy, vz, nx, ny, nz); alternatively you
  // can use multiple VBO's (e.g. one VBO for normals, one for points,
  // one for texture coordinates).
  // Note that if you use indexed buffers, you cannot have indexes
  // larger than 65K, due to the limitations of WebGL.
  objBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, objBuffer_);

  // Embed the buffer directly in the JavaScript stream if useBinaryBuffers_ is false.
  // Alternatively transfer the array directly as a binary data resource if useBinaryBuffers_
  // is true.
  bufferDatafv(ARRAY_BUFFER, data.begin(), data.end(), STATIC_DRAW, useBinaryBuffers_);

  // Set the clear color to a transparant background
  clearColor(0, 0, 0, 0);

  // Reset Z-buffer, enable Z-buffering
  clearDepth(1);
  enable(DEPTH_TEST);
  depthFunc(LEQUAL);
}

void PaintWidget::resizeGL(int width, int height)
{
  // Set the viewport size.
  viewport(0, 0, width, height);

  // Set projection matrix to some fixed values
  WMatrix4x4 proj;
  proj.perspective(45, ((double)width)/height, 1, 40);
  uniformMatrix4(pMatrixUniform_, proj);
}

// The paintGL function is executed every time the canvas is to be
// repainted. For example: when the camera location is modified,
// an animated object is changed, ...
void PaintWidget::paintGL()
{
  // Clear color an depth buffers
  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);


  // Configure the shader: set the uniforms
  // Uniforms are 'configurable constants' in a shader: they are
  // identical for every point that has to be drawn.
  // Set the camera transformation to the value of a client-side JS matrix
  uniformMatrix4(cMatrixUniform_, jsMatrix_);
  // Often, a model matrix is used to move the model around. We're happy
  // with the location of the model, so we leave it as the unit matrix
  WMatrix4x4 modelMatrix;
  uniformMatrix4(mvMatrixUniform_, modelMatrix);
  // The next one is a bit complicated. In desktop OpenGL, a shader
  // has the gl_NormalMatrix matrix available in the shader language,
  // a matrix that is used to transform normals to e.g. implement proper
  // Phong shading (google will help you to find a detailed explanation
  // of why you need it). It is the transposed inverse of the model view
  // matrix. Unfortunately, this matrix is not available in WebGL, so if
  // you want to do phong shading, you must calculate it yourself.
  // Wt provides methods to calculate the transposed inverse of a matrix,
  // when client-side JS matrices are involved. Here, we inverse-transpose
  // the product of the client-side camera matrix and the model matrix.
  uniformMatrix4(nMatrixUniform_, (jsMatrix_ * modelMatrix).inverted().transposed());

  // Configure the shaders: set the attributes.
  // Attributes are 'variables' within a shader: they vary for every point
  // that has to be drawn. All are stored in one VBO.
  bindBuffer(ARRAY_BUFFER, objBuffer_);
  // Configure the vertex attributes:
  vertexAttribPointer(vertexPositionAttribute_,
      3,     // size: Every vertex has an X, Y anc Z component
      FLOAT, // type: They are floats
      false, // normalized: Please, do NOT normalize the vertices
      2*3*4, // stride: The first byte of the next vertex is located this
      //         amount of bytes further. The format of the VBO is
      //         vx, vy, vz, nx, ny, nz and every element is a
      //         Float32, hence 4 bytes large
      0);    // offset: The byte position of the first vertex in the buffer
  //         is 0.
  vertexAttribPointer(vertexNormalAttribute_,
      3,
      FLOAT,
      false,
      2*3*4, // stride: see above. We jump from normal to normal now
      3*4);  // offset: the first normal is located after the first vertex
             //         position, consisting of three four-byte floats

  // Now draw all the triangles.
  drawArrays(TRIANGLES, 0, data.size()/6);
}

void PaintWidget::setShaders(const std::string &vertexShader,
    const std::string &fragmentShader)
{
  vertexShader_ = vertexShader;
  fragmentShader_ = fragmentShader;
}


