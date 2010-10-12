// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGLWidget"
#include "Wt/WWebWidget"
#include "DomElement.h"
#include "Utils.h"

using namespace Wt;

// TODO: for uniform*v, attribute8v, generate the non-v version in js. We
//       will probably end up with less bw and there's no use in allocating
//       an extra array.


const char *WGLWidget::toString(WGLWidget::BufferEnum target)
{
  switch (target) {
    case WGLWidget::ARRAY_BUFFER: return "ctx.ARRAY_BUFFER";
    case WGLWidget::ELEMENT_ARRAY_BUFFER: return "ctx.ELEMENT_ARRAY_BUFFER";
  }
  return "BAD_BUFFER_ENUM";
}
const char *WGLWidget::toString(WGLWidget::BufferUsageEnum usage)
{
  switch(usage) {
    case WGLWidget::STREAM_DRAW: return "ctx.STREAM_DRAW";
    case WGLWidget::STATIC_DRAW: return "ctx.STATIC_DRAW";
    case WGLWidget::DYNAMIC_DRAW: return "ctx.DYNAMIC_DRAW";
  }
  return "BAD_BUFFER_USAGE_ENUM";
}
const char *WGLWidget::toString(WGLWidget::DataTypeEnum type)
{
  switch(type) {
  case WGLWidget::BYTE: return "ctx.BYTE";
  case WGLWidget::UNSIGNED_BYTE: return "ctx.UNSIGNED_BYTE";
  case WGLWidget::SHORT: return "ctx.SHORT";
  case WGLWidget::UNSIGNED_SHORT: return "ctx.UNSIGNED_SHORT";
  case WGLWidget::INT: return "ctx.INT";
  case WGLWidget::UNSIGNED_INT: return "ctx.UNSIGNED_INT";
  case WGLWidget::FLOAT: return "ctx.FLOAT";
  }
  return "BAD_DATA_TYPE_ENUM";
}
const char *WGLWidget::toString(WGLWidget::BeginModeEnum mode)
{
  switch(mode) {
    case WGLWidget::POINTS: return "ctx.POINTS";
    case WGLWidget::LINES: return "ctx.LINES";
    case WGLWidget::LINE_LOOP: return "ctx.LINE_LOOP";
    case WGLWidget::LINE_STRIP: return "ctx.LINE_STRIP";
    case WGLWidget::TRIANGLES: return "ctx.TRIANGLES";
    case WGLWidget::TRIANGLE_STRIP: return "ctx.TRIANGLE_STRIP";
    case WGLWidget::TRIANGLE_FAN: return "ctx.TRIANGLE_FAN";
  }
  return "BAD_BEGIN_MODE_ENUM";
}
const char *WGLWidget::toString(WGLWidget::EnableCapEnum cap)
{
  switch(cap) {
    case WGLWidget::CULL_FACE: return "ctx.CULL_FACE";
    case WGLWidget::BLEND: return "ctx.BLEND";
    case WGLWidget::DITHER: return "ctx.DITHER";
    case WGLWidget::STENCIL_TEST: return "ctx.STENCIL_TEST";
    case WGLWidget::DEPTH_TEST: return "ctx.DEPTH_TEST";
    case WGLWidget::SCISSOR_TEST: return "ctx.SCISSOR_TEST";
    case WGLWidget::POLYGON_OFFSET_FILL: return "ctx.POLYGON_OFFSET_FILL";
    case WGLWidget::SAMPLE_ALPHA_TO_COVERAGE: return "ctx.SAMPLE_ALPHA_TO_COVERAGE";
    case WGLWidget::SAMPLE_COVERAGE: return "ctx.SAMPLE_COVERAGE";
  }
  return "BAD_ENABLE_CAP_ENUM";
}
const char *WGLWidget::toString(WGLWidget::StencilFunctionEnum f)
{
  switch(f) {
    case WGLWidget::NEVER: return "ctx.NEVER";
    case WGLWidget::LESS: return "ctx.LESS";
    case WGLWidget::EQUAL: return "ctx.EQUAL";
    case WGLWidget::LEQUAL: return "ctx.LEQUAL";
    case WGLWidget::GREATER: return "ctx.GREATER";
    case WGLWidget::NOTEQUAL: return "ctx.NOTEQUAL";
    case WGLWidget::GEQUAL: return "ctx.GEQUAL";
    case WGLWidget::ALWAYS: return "ctx.ALWAYS";
  }
  return "BAD_STENCIL_FN_ENUM";
}

WGLWidget::WGLWidget(WContainerWidget *parent):
  WWebWidget(parent),
  shaders_(0),
  programs_(0),
  attributes_(0),
  uniforms_(0),
  buffers_(0)
{
  setInline(false);
}

WGLWidget::~WGLWidget()
{
}

char *WGLWidget::makeFloat(double d, char *buf)
{
  return Utils::round_str(d, 3, buf);
}

char *WGLWidget::makeInt(int i, char *buf)
{
  return Utils::itoa(i, buf);
}

void WGLWidget::updateMediaDom(DomElement& element, bool all)
{
#if 0
  if (all || flagsChanged_) {
    if ((!all) || flags_ & Controls)
      element.setAttribute("controls", flags_ & Controls ? "controls" : "");
    if ((!all) || flags_ & Autoplay)
      element.setAttribute("autoplay", flags_ & Autoplay ? "autoplay" : "");
    if ((!all) || flags_ & Loop)
      element.setAttribute("loop", flags_ & Loop ? "loop" : "");
  }
#endif
}

DomElement *WGLWidget::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(DomElement_CANVAS);;

  setId(result, app);
  updateDom(*result, true);

  std::stringstream tmp;

  tmp <<
    "if(" << jsRef() << ".getContext){";
  js_.str("");
  initializeGL();
  std::string initJs = js_.str();
  js_.str("");
  paintGL();

  tmp << "var ctx=" << jsRef() << ".getContext('experimental-webgl');"
    << initJs
    << js_.str();
  tmp << "}";

  js_.str();

  result->callJavaScript(tmp.str());

  return result;
}

void WGLWidget::getDomChanges(std::vector<DomElement *>& result,
                                WApplication *app)
{
  WWebWidget::getDomChanges(result, app);
}

DomElementType WGLWidget::domElementType() const
{
  return Wt::DomElement_CANVAS;
}


void WGLWidget::paintGL()
{
}

void WGLWidget::resizeGL(int width, int height)
{
}

void WGLWidget::initializeGL()
{
}


void WGLWidget::clearColor(float r, float g, float b, float a)
{
  js_ << "ctx.clearColor(" << r << "," << g << "," << b << ");";
  GLDEBUG;
}
void WGLWidget::clearDepth(float depth)
{
  js_ << "ctx.clearDepth(" << depth << ");";
  GLDEBUG;
}

void WGLWidget::viewport(int x, int y, unsigned width, unsigned height)
{
  js_ << "ctx.viewport(" << x << "," << y << "," << width << "," << height << ");";
  GLDEBUG;
}

void WGLWidget::clear(WFlags<ClearBufferEnum> mask)
{
  js_ << "ctx.clear(";
  if (mask & COLOR_BUFFER_BIT) js_ << "ctx.COLOR_BUFFER_BIT|";
  if (mask & DEPTH_BUFFER_BIT) js_ << "ctx.DEPTH_BUFFER_BIT|";
  if (mask & STENCIL_BUFFER_BIT) js_ << "ctx.STENCIL_BUFFER_BIT|";

  js_ << "0);";
  GLDEBUG;
}


Shader WGLWidget::createShader(ShaderEnum shader)
{
  Shader retval = "WtShader" + boost::lexical_cast<std::string>(shaders_++);
  js_ << retval << "=ctx.createShader(";
  switch (shader) {
    case FRAGMENT_SHADER:
      js_ << "ctx.FRAGMENT_SHADER";
      break;
    case VERTEX_SHADER:
      js_ << "ctx.VERTEX_SHADER";
      break;
  }
  js_ << ");";
  return retval;
  GLDEBUG;
}

void WGLWidget::shaderSource(Shader shader, const std::string &src)
{
  js_ << "ctx.shaderSource(" << shader << "," << jsStringLiteral(src) << ");";
  GLDEBUG;
}

void WGLWidget::compileShader(Shader shader)
{
  js_ << "ctx.compileShader(" << shader << ");";
  js_ << "if (!ctx.getShaderParameter(" << shader << ", ctx.COMPILE_STATUS)) {"
    << "alert(ctx.getShaderInfoLog(" << shader << "));}";
  GLDEBUG;
}

Program WGLWidget::createProgram()
{
  Program retval = "WtProgram" + boost::lexical_cast<std::string>(programs_++);
  js_ << retval << "=ctx.createProgram();";
  GLDEBUG;
  return retval;
}

void WGLWidget::attachShader(Program program, Shader shader)
{
  js_ << "ctx.attachShader(" << program << ", " << shader << ");";
  GLDEBUG;
}

void WGLWidget::linkProgram(Program program)
{
  js_ << "ctx.linkProgram(" << program << ");";
  js_ << "if(!ctx.getProgramParameter(" << program << ",ctx.LINK_STATUS)){"
    << "alert('Could not initialize shaders');}";
  GLDEBUG;
}

void WGLWidget::useProgram(Program program)
{
  js_ << "ctx.useProgram(" << program << ");";
  GLDEBUG;
}

AttribLocation WGLWidget::getAttribLocation(Program program, const std::string &attrib)
{
  AttribLocation retval = "WtAttrib" + boost::lexical_cast<std::string>(attributes_++);
  js_ << retval << "=ctx.getAttribLocation(" << program << "," << jsStringLiteral(attrib) << ");";
  GLDEBUG;
  return retval;
}

void WGLWidget::enableVertexAttribArray(AttribLocation index)
{
  js_ << "ctx.enableVertexAttribArray(" << index << ");";
  GLDEBUG;
}

UniformLocation WGLWidget::getUniformLocation(Program program, const std::string location)
{
  UniformLocation retval = "WtUniform" + boost::lexical_cast<std::string>(uniforms_++);
  js_ << retval << "=ctx.getUniformLocation(" << program << "," << jsStringLiteral(location) << ");";
  GLDEBUG;
  return retval;
}

//void WGLWidget::uniformMatrix4fv(UniformLocation location, bool transpose, const std::string &matrix)
//{
//  js_ << "ctx.uniformMatrix4fv(" << location << "," << (transpose?"true":"false") << "," << matrix << ");";
//  GLDEBUG;
//}

Buffer WGLWidget::createBuffer()
{
  Buffer retval = "WtBuffer" + boost::lexical_cast<std::string>(buffers_++);
  js_ << retval << "=ctx.createBuffer();";
  GLDEBUG;
  return retval;
}

void WGLWidget::bindBuffer(BufferEnum target, Buffer buffer)
{
  js_ << "ctx.bindBuffer(" << toString(target) << "," << buffer << ");";
  GLDEBUG;
}

//void WGLWidget::bufferData(BufferEnum target, const std::string &data, BufferUsageEnum usage)
//{
//  js_ << "ctx.bufferData(" << toString(target) << "," << data << ","<< toString(usage) << ");";
//  GLDEBUG;
//}

void WGLWidget::vertexAttribPointer(AttribLocation location, int size,
                                 DataTypeEnum type, bool normalized,
                                 unsigned stride, unsigned offset)
{
  js_ << "ctx.vertexAttribPointer(" << location << "," << size << "," << toString(type)
    << "," << (normalized?"true":"false") << "," << stride << "," << offset << ");";
  GLDEBUG;
}

void WGLWidget::drawArrays(BeginModeEnum mode, int first, unsigned count)
{
  js_ << "ctx.drawArrays(" << toString(mode) << "," << first << "," << count << ");";
  GLDEBUG;
}

void WGLWidget::enable(EnableCapEnum cap)
{
  js_ << "ctx.enable(" << toString(cap) << ");";
  GLDEBUG;
}

void WGLWidget::disable(EnableCapEnum cap)
{
  js_ << "ctx.disable(" << toString(cap) << ");";
  GLDEBUG;
}

void WGLWidget::depthFunc(StencilFunctionEnum func)
{
  js_ << "ctx.depthFunc(" << toString(func) << ");";
  GLDEBUG;
}
