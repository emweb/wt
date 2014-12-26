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

#include <Wt/WMatrix4x4>
#include <Wt/WGenericMatrix>

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
"uniform mat4 uCMatrix;\n"
"uniform mat4 uPMatrix;\n"
"\n"
"varying vec4 vColor;\n"
"\n"
"void main(void) {\n"
"  gl_Position = uPMatrix * uCMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
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
"uniform mat4 uCMatrix;\n"
"uniform mat4 uPMatrix;\n"
""
"varying vec2 vTextureCoord;\n"
""
"void main(void) {\n"
"  gl_Position = uPMatrix * uCMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"  vTextureCoord = aTextureCoord;\n"
"}\n";

class PaintWidget: public WGLWidget
{
public:
  PaintWidget(WContainerWidget *root):
    WGLWidget(root)
  {
    jsMatrix_ = JavaScriptMatrix4x4();
    addJavaScriptMatrix4(jsMatrix_);
  }

  void initializeGL()
  {
    initJavaScriptMatrix4(jsMatrix_);
    WMatrix4x4 worldTransform;
    worldTransform.lookAt(0, 0, 5, 0, 0, -1, 0, 1, 0);
    setJavaScriptMatrix4(jsMatrix_, worldTransform);

    //setClientSideWalkHandler(jsMatrix_, 1./20, 1./100);
    setClientSideLookAtHandler(jsMatrix_, 0, 0, 0, 0, 1, 0, 0.005, 0.005);
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
    cMatrixUniform_ = getUniformLocation(shaderProgram_, "uCMatrix");
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
    texCMatrixUniform_ = getUniformLocation(shaderProgram_, "uCMatrix");
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
    WMatrix4x4 proj;
    proj.perspective(90, 1, 1, 40);
    useProgram(shaderProgram_);
    // Set projection matrix
    uniformMatrix4(pMatrixUniform_, proj);
    uniformMatrix4(cMatrixUniform_, jsMatrix_);

    // Draw the scene
#if 0
    bindBuffer(ARRAY_BUFFER, triangleVertexPositionBuffer_);
    vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 0, 0);
    bindBuffer(ARRAY_BUFFER, triangleVertexColorBuffer_);
    vertexAttribPointer(vertexColorAttribute_, 4, FLOAT, false, 0, 0);
    WMatrix4x4 modelMatrix1(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      -2.0, 0, 0, 1
    );
    modelMatrix1.rotate(45, 0, 1, 0);
    uniformMatrix4fv(mvMatrixUniform_, false, modelMatrix1);
    drawArrays(TRIANGLES, 0, 3);
#endif
    bindBuffer(ARRAY_BUFFER, squareVertexPositionBuffer_);
    vertexAttribPointer(vertexPositionAttribute_, 3, FLOAT, false, 0, 0);
    bindBuffer(ARRAY_BUFFER, squareVertexColorBuffer_);
    vertexAttribPointer(vertexColorAttribute_, 4, FLOAT, false, 0, 0);
    WMatrix4x4 modelMatrix2(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    );
    //modelMatrix2.rotate(45, 1, 0, 0);
    //modelMatrix2.translate(0, 0, -2);
    uniformMatrix4(mvMatrixUniform_, modelMatrix2);
    //drawArrays(TRIANGLE_STRIP, 0, 4);
    bindBuffer(ELEMENT_ARRAY_BUFFER, squareElementBuffer_);
    drawElements(TRIANGLE_STRIP, 4, DT_UNSIGNED_BYTE, 0);

#if 0
    // now draw something textured
    useProgram(texShaderProgram_);
    uniformMatrix4fv(texPMatrixUniform_, false, projectionMatrix);
    //WMatrix4x4 mm3(
    //  2, 0, 0, 0,
     // 0, 1, 0, -2.5,
    //  0, 0, 1, 0,
    //  0, 0, 0, 1
    //  );
    WMatrix4x4 mm3;
    //mm3.scale(.5, 2, 1);
    mm3.rotate(45, 0, 0, 1);

    uniformMatrix4fv(texMvMatrixUniform_, false, mm3.data());
    bindBuffer(ARRAY_BUFFER, textureCoordBuffer_);
    vertexAttribPointer(texCoordAttribute_, 2, FLOAT, false, 0, 0);
    activeTexture(TEXTURE0);
    bindTexture(TEXTURE_2D, texture_);
    uniform1i(texSamplerUniform_, 0);

    bindBuffer(ARRAY_BUFFER, squareVertexPositionBuffer_);
    vertexAttribPointer(texVertexPositionAttribute_, 3, FLOAT, false, 0, 0);
    drawArrays(TRIANGLE_STRIP, 0, 4);
#endif
  }

  void resizeGL(int width, int height)
  {
  }
private:
  Program shaderProgram_;
  AttribLocation vertexPositionAttribute_;
  AttribLocation vertexColorAttribute_;
  UniformLocation pMatrixUniform_;
  UniformLocation cMatrixUniform_;
  UniformLocation mvMatrixUniform_;

  Program texShaderProgram_;
  AttribLocation texVertexPositionAttribute_;
  AttribLocation texCoordAttribute_;
  UniformLocation texPMatrixUniform_;
  UniformLocation texCMatrixUniform_;
  UniformLocation texMvMatrixUniform_;
  UniformLocation texSamplerUniform_;

  JavaScriptMatrix4x4 jsMatrix_;

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
  gl->resize(640, 640);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new WebGLDemo(env);
}

int main(int argc, char **argv)
{

  WGenericMatrix<double, 3, 6> gm1;
  WGenericMatrix<double, 3, 6> gm2(gm1);
  WGenericMatrix<double, 3, 6> gm3;
  WGenericMatrix<double, 6, 3> gm4;
  double foo[] = {1, 2, 3, 4};
  WGenericMatrix<double, 2, 2> gm5(foo);
  std::cout << gm5.constData()[0] << std::endl;
  std::cout << gm5(0, 0) << std::endl;
  std::cout << gm5 << std::endl;
  gm5.copyDataTo(foo);
  WGenericMatrix<double, 2, 2> const gm6(foo);
  std::cout << "equality test: " << (gm5 == gm6 ? "OK" : "NOK") << std::endl;
  std::cout << "inequality test: " << (gm5 != gm6 ? "NOK" : "OK") << std::endl;
  std::cout << gm5.data()[0] << std::endl;
  std::cout << gm6.constData()[0] << std::endl;

  gm3.fill(42);
  std::cout << gm3 << std::endl;
  std::cout << gm3.transposed() << std::endl;
  std::cout << "isidentity non-square: " << (gm2.isIdentity() ? "OK" : "NOK") << std::endl;
  std::cout << "isidentity non-square: " << (!gm3.isIdentity() ? "OK" : "NOK") << std::endl;
  gm3.setToIdentity();
  std::cout << gm3 << std::endl;
  std::cout << "isidentity non-square: " << (gm3.isIdentity() ? "OK" : "NOK") << std::endl;
#if 1
  std::cout << gm1 << std::endl;
  gm3 = gm1 + gm2;
  std::cout << gm3 << std::endl;
  gm3 = gm1 - gm2;
  std::cout << gm3 << std::endl;
  gm3 = -gm2;
  std::cout << gm3 << std::endl;
  gm3 = gm2 * 2.0;
  std::cout << gm3 << std::endl;
  gm3 = gm2 / 2.0;
  std::cout << gm3 << std::endl;
  gm3 = 2.0 * gm2;
  std::cout << gm3 << std::endl;
  std::cout << gm3 * gm4 << std::endl;
  WGenericMatrix<double, 3, 3> gm7= gm3 * gm4;
#endif
  gm3 *= 2.0;
  std::cout << gm3 << std::endl;
  gm3 /= 2.0;
  std::cout << gm3 << std::endl;
  gm3 += gm1;
  std::cout << gm3 << std::endl;
  gm3 -= gm1;
  std::cout << gm3 << std::endl;



  WMatrix4x4 m1;
  std::cout << m1 << std::endl;
  std::cout << "Determinant: " << m1.determinant() << std::endl;
  std::cout << "Inverted: ";
  std::cout << m1.inverted() << std::endl;
  std::cout << "Identity? " << m1.isIdentity() << std::endl;


  WMatrix4x4 m2;
  m2.rotate(0, 0, 0, 1);
  std::cout << m2 << std::endl;
  m2.rotate(0, 0, 0, 2);
  std::cout << m2 << std::endl;
  m2.rotate(0, 0, 2, 0);
  std::cout << m2 << std::endl;
  m2.rotate(0, 2, 0, 0);
  std::cout << m2 << std::endl;
  m2.rotate(0, 1, 1, 1);
  std::cout << m2 << std::endl;
  m2.rotate(90, 1, 0, 0);
  std::cout << m2 << std::endl;
  m2.rotate(-90, 1, 0, 0);
  std::cout << m2 << std::endl;
  m2.rotate(45, 1, 0, 0);
  std::cout << m2 << std::endl;
  m2.rotate(-45, 1, 0, 0);
  std::cout << m2 << std::endl;
  WMatrix4x4 m3;
  m3 = m1 * m2;
  WMatrix4x4 m4(m1*m2);
  m3.setToIdentity();
  m3.lookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
  std::cout << m3 << std::endl;
  m3.setToIdentity();
  m3.lookAt(0, 0, 0, 0, 0, -2, 0, 8, 0);
  std::cout << m3 << std::endl;
  m3.setToIdentity();
  m3.lookAt(0, 0, -1, 0, 0, -2, 0, 1, 0);
  std::cout << m3 << std::endl;
  m3.setToIdentity();
  m3.lookAt(0, 0, 0, 0, 0, 0, 0, 1, 0);
  std::cout << m3 << std::endl;
  m3.setToIdentity();
  m3.lookAt(0, 0, 0,1, 1, 1, 0, 1, 0);
  std::cout << m3 << std::endl;
  return WRun(argc, argv, &createApplication);
}

