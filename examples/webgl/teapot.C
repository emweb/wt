/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WPushButton.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WServer.h>
#include <Wt/WEnvironment.h>

#include "readObj.h"
#include "PaintWidget.h"

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
"uniform mat4 uMVMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 uCMatrix;  // Client-side manipulated [C]amera matrix\n"
"uniform mat4 uPMatrix;  // Perspective [P]rojection matrix\n"
"uniform mat4 uNMatrix;  // [N]ormal transformation\n"
"// uNMatrix is the transpose of the inverse of uCMatrix * uMVMatrix\n"
"\n"
"varying vec3 vLightWeighting;\n"
"\n"
"void main(void) {\n"
"  // Calculate the position of this vertex\n"
"  gl_Position = uPMatrix * uCMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"\n"
"  // Phong shading\n"
"  vec3 transformedNormal = normalize((uNMatrix * vec4(normalize(aVertexNormal), 0)).xyz);\n"
"  vec3 lightingDirection = normalize(vec3(1, 1, 1));\n"
"  float directionalLightWeighting = max(dot(transformedNormal, lightingDirection), 0.0);\n"
"  vec3 uAmbientLightColor = vec3(0.2, 0.2, 0.2);\n"
"  vec3 uDirectionalColor = vec3(0.8, 0.8, 0.8);\n"
"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n"
"}\n";

/*
 * A pretty basic WebGL demo application
 */
std::vector<float> data;

class WebGLDemo : public WApplication
{
public:
  WebGLDemo(const WEnvironment& env);

private:
  void updateShaders();
  void resetShaders();

  WContainerWidget *glContainer_;
  PaintWidget      *paintWidget_;
  WTextArea        *fragmentShaderText_;
  WTextArea        *vertexShaderText_;
};

WebGLDemo::WebGLDemo(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("WebGL Demo");

  root()->addWidget(std::make_unique<WText>("If your browser supports WebGL, you'll "
    "see a teapot below.<br/>Use your mouse to move around the teapot.<br/>"
    "Edit the shaders below the teapot to change how the teapot is rendered."));
  root()->addWidget(std::make_unique<WBreak>());

  paintWidget_ = 0;

  glContainer_ = root()->addWidget(std::make_unique<WContainerWidget>());
  glContainer_->resize(500, 500);
  glContainer_->setInline(false);

  WPushButton *updateButton = root()->addWidget(std::make_unique<WPushButton>("Update shaders"));
  updateButton->clicked().connect(this, &WebGLDemo::updateShaders);
  WPushButton *resetButton = root()->addWidget(std::make_unique<WPushButton>("Reset shaders"));
  resetButton->clicked().connect(this, &WebGLDemo::resetShaders);

  WTabWidget *tabs = root()->addWidget(std::make_unique<WTabWidget>());

  auto fragmentShaderText = std::make_unique<WTextArea>();
  fragmentShaderText_ = fragmentShaderText.get();
  fragmentShaderText_->resize(750, 250);
  tabs->addTab(std::move(fragmentShaderText), "Fragment Shader");

  auto vertexShaderText = std::make_unique<WTextArea>();
  vertexShaderText_ = vertexShaderText.get();
  vertexShaderText_->resize(750, 250);
  tabs->addTab(std::move(vertexShaderText), "Vertex Shader");

  resetShaders();
}

void WebGLDemo::updateShaders()
{
  // check if binary buffers are enabled
  // i.e. if your application url is "webgl" on localhost:8080, use this to enable binary buffers:
  // localhost:8080/webgl?binaryBuffers
  // query given URL arguments...
  Http::ParameterValues pv = wApp->environment().getParameterValues("binaryBuffers");
  bool useBinaryBuffers = false;
  if (!pv.empty())
  {
      useBinaryBuffers = true;
  }

  if(paintWidget_)
    paintWidget_->removeFromParent(); //FIXME: reset the current paintWidget to replace it with a new one

  paintWidget_ = glContainer_->addWidget(std::make_unique<PaintWidget>(useBinaryBuffers));
  paintWidget_->resize(500, 500);
  paintWidget_->setShaders(vertexShaderText_->text().toUTF8(),
    fragmentShaderText_->text().toUTF8());
  paintWidget_->setAlternativeContent(std::make_unique<WImage>("nowebgl.png"));
}

void WebGLDemo::resetShaders()
{
  fragmentShaderText_->setText(fragmentShaderSrc);
  vertexShaderText_->setText(vertexShaderSrc);
  updateShaders();
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<WebGLDemo>(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);
    readObj(WApplication::appRoot() + "teapot.obj", data);

    server.addEntryPoint(EntryPointType::Application, &createApplication);
    server.run();
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}
