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

const char *texFragmentShaderSrc =
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
""
"varying vec2 vTextureCoord;\n"
""
"uniform sampler2D uSampler;\n"
""
"void main(void) {\n"
"  gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));\n"
"}\n";

const char *texVertexShaderSrc =
"attribute vec3 aVertexPosition;\n"
"attribute vec2 aTextureCoord;\n"
""
"uniform mat4 uMVMatrix;\n"
"uniform mat4 uPMatrix;\n"
""
"varying vec2 vTextureCoord;\n"
""
"void main(void) {\n"
"  gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"  vTextureCoord = aTextureCoord;\n"
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

    vertexPositionAttribute_ = getAttribLocation(shaderProgram_, "aVertexPosition");
    enableVertexAttribArray(vertexPositionAttribute_);

    vertexColorAttribute_ = getAttribLocation(shaderProgram_, "aVertexColor");
    enableVertexAttribArray(vertexColorAttribute_);

    pMatrixUniform_ = getUniformLocation(shaderProgram_, "uPMatrix");
    mvMatrixUniform_ = getUniformLocation(shaderProgram_, "uMVMatrix");

    // Next, load a texture shader
    Shader texFragmentShader = createShader(FRAGMENT_SHADER);
    shaderSource(texFragmentShader, texFragmentShaderSrc);
    compileShader(texFragmentShader);
    Shader texVertexShader = createShader(VERTEX_SHADER);
    shaderSource(texVertexShader, texVertexShaderSrc);
    compileShader(texVertexShader);
    texShaderProgram_ = createProgram();
    attachShader(texShaderProgram_, texVertexShader);
    attachShader(texShaderProgram_, texFragmentShader);
    linkProgram(texShaderProgram_);
    useProgram(texShaderProgram_);

    texVertexPositionAttribute_ = getAttribLocation(texShaderProgram_, "aVertexPosition");
    enableVertexAttribArray(texVertexPositionAttribute_);

    texCoordAttribute_ = getAttribLocation(texShaderProgram_, "aTextureCoord");
    enableVertexAttribArray(texCoordAttribute_);

    texPMatrixUniform_ = getUniformLocation(texShaderProgram_, "uPMatrix");
    texMvMatrixUniform_ = getUniformLocation(texShaderProgram_, "uMVMatrix");
    texSamplerUniform_ = getUniformLocation(texShaderProgram_, "uSampler");

    // Now, preload buffers
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

    squareElementBuffer_ = createBuffer();
    bindBuffer(ELEMENT_ARRAY_BUFFER, squareElementBuffer_);
    int elements[] = {
      0, 1, 2, 3
    };
    bufferDataiv(ELEMENT_ARRAY_BUFFER, elements, 4, STATIC_DRAW, DT_UNSIGNED_BYTE);

    texture_ = createTextureAndLoad("texture.jpg");
    bindTexture(TEXTURE_2D, texture_);
    pixelStorei(UNPACK_FLIP_Y_WEBGL, true);
    texImage2D(TEXTURE_2D, 0, RGB, RGB, PT_UNSIGNED_BYTE, texture_);
    texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
    texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
    textureCoordBuffer_ = createBuffer();
    bindBuffer(ARRAY_BUFFER, textureCoordBuffer_);
    float textureCoords[] = {
      1, 1,
      0, 1,
      1, 0,
      0, 0
    };
    bufferDatafv(ARRAY_BUFFER, textureCoords, 8, STATIC_DRAW);

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
    float projectionMatrix[] = {
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 4
    };

    useProgram(shaderProgram_);
    // Set projection matrix
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
      -2.0, 0, 0, 1
    };
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
      2.0, 0, 0, 1
    };
    uniformMatrix4fv(mvMatrixUniform_, false, modelMatrix2);
    //drawArrays(TRIANGLE_STRIP, 0, 4);
    bindBuffer(ELEMENT_ARRAY_BUFFER, squareElementBuffer_);
    drawElements(TRIANGLE_STRIP, 4, DT_UNSIGNED_BYTE, 0);

    // now draw something textured
    useProgram(texShaderProgram_);
    uniformMatrix4fv(texPMatrixUniform_, false, projectionMatrix);
    float modelMatrix3[] = {
      2, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, -2.5, 0, 1
    };
    uniformMatrix4fv(texMvMatrixUniform_, false, modelMatrix3);
    bindBuffer(ARRAY_BUFFER, textureCoordBuffer_);
    vertexAttribPointer(texCoordAttribute_, 2, FLOAT, false, 0, 0);
    activeTexture(TEXTURE0);
    bindTexture(TEXTURE_2D, texture_);
    uniform1i(texSamplerUniform_, 0);

    bindBuffer(ARRAY_BUFFER, squareVertexPositionBuffer_);
    vertexAttribPointer(texVertexPositionAttribute_, 3, FLOAT, false, 0, 0);
    drawArrays(TRIANGLE_STRIP, 0, 4);
  }

  void resizeGL(int width, int height)
  {
  }
private:
  Program shaderProgram_;
  AttribLocation vertexPositionAttribute_;
  AttribLocation vertexColorAttribute_;
  UniformLocation pMatrixUniform_;
  UniformLocation mvMatrixUniform_;

  Program texShaderProgram_;
  AttribLocation texVertexPositionAttribute_;
  AttribLocation texCoordAttribute_;
  UniformLocation texPMatrixUniform_;
  UniformLocation texMvMatrixUniform_;
  UniformLocation texSamplerUniform_;

  Buffer triangleVertexPositionBuffer_;
  Buffer triangleVertexColorBuffer_;
  Buffer squareVertexPositionBuffer_;
  Buffer squareVertexColorBuffer_;
  Buffer squareElementBuffer_;

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
  gl->resize(640, 320);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new WebGLDemo(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

