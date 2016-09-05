// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGLWidget"

#include "Wt/WApplication"
#include "Wt/WAbstractGLImplementation"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WClientGLWidget"
#include "Wt/WLogger"

#ifndef WT_TARGET_JAVA
#ifdef WT_USE_OPENGL
#include "Wt/WServerGLWidget"
#endif
#else
#include <java/WServerGLWidget>
#endif

#include "Wt/WBoostAny"
#include "Wt/WEnvironment"
#include "Wt/WVideo"
#include "Wt/WImage"
#include "Wt/WPaintedWidget"
#include "Wt/WText"
#include "Wt/WWebWidget"
#include "DomElement.h"
#include "WebUtils.h"

#include "Wt/Json/Array"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"

#ifndef WT_DEBUG_JS
#include "js/WGLWidget.min.js"
#include "js/WPaintedWidget.min.js"
#include "js/WtGlMatrix.min.js"
#endif

namespace Wt {

LOGGER("WGLWidget");

// TODO: for uniform*v, attribute8v, generate the non-v version in js. We
//       will probably end up with less bw and there's no use in allocating
//       an extra array.
// TODO: allow VBO's to be served from a file

WGLWidget::WGLWidget(WContainerWidget *parent)
: WInteractWidget(parent),
  renderOptions_(ClientSideRendering | ServerSideRendering | AntiAliasing),
  pImpl_(0),
  jsValues_(0),
  repaintSignal_(this, "repaintSignal"),
  alternative_(0),
  webglNotAvailable_(this, "webglNotAvailable"),
  webGlNotAvailable_(false),
  contextRestored_(this, "contextRestored"),
  restoringContext_(false),
  valueChanged_(false),
  mouseWentDownSlot_("function(o, e){" + this->glObjJsRef() + ".mouseDown(o, e);}", this),
  mouseWentUpSlot_("function(o, e){" + this->glObjJsRef() + ".mouseUp(o, e);}", this),
  mouseDraggedSlot_("function(o, e){" + this->glObjJsRef() + ".mouseDrag(o, e);}", this),
  mouseMovedSlot_("function(o, e){" + this->glObjJsRef() + ".mouseMove(o, e);}", this),
  mouseWheelSlot_("function(o, e){" + this->glObjJsRef() + ".mouseWheel(o, e);}", this),
  touchStarted_("function(o, e){" + this->glObjJsRef() + ".touchStart(o, e);}", this),
  touchEnded_("function(o, e){" + this->glObjJsRef() + ".touchEnd(o, e);}", this),
  touchMoved_("function(o, e){" + this->glObjJsRef() + ".touchMoved(o, e);}", this),
  repaintSlot_("function() {"
    "var o = " + this->glObjJsRef() + ";"
    "if(o.ctx) o.paintGL();"
    "}", this)
{
  setInline(false);
  setLayoutSizeAware(true);
  webglNotAvailable_.connect(this, &WGLWidget::webglNotAvailable);
  repaintSignal_.connect(boost::bind(&WGLWidget::repaintGL, this, PAINT_GL));
  contextRestored_.connect(boost::bind(&WGLWidget::contextRestored, this));
  mouseWentDown().connect(mouseWentDownSlot_);
  mouseWentUp().connect(mouseWentUpSlot_);
  mouseDragged().connect(mouseDraggedSlot_);
  mouseMoved().connect(mouseMovedSlot_);
  mouseWheel().connect(mouseWheelSlot_);
  touchStarted().connect(touchStarted_);
  touchEnded().connect(touchEnded_);
  touchMoved().connect(touchMoved_);
  setAlternativeContent(new WText("Your browser does not support WebGL"));

  setFormObject(true);
}

WGLWidget::~WGLWidget()
{
  delete pImpl_;
}

void WGLWidget::setRenderOptions(WFlags<RenderOption> options)
{
  renderOptions_ = options;
}

void WGLWidget::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

void WGLWidget::enableClientErrorChecks(bool enable) {
  pImpl_->enableClientErrorChecks(enable);
}

void WGLWidget::injectJS(const std::string & jsString)
{
  pImpl_->injectJS(jsString);
}

void WGLWidget::webglNotAvailable()
{
  std::cout << "WebGL Not available in client!\n";
  webGlNotAvailable_ = true;
}

std::string WGLWidget::renderRemoveJs(bool recursive)
{
  if (webGlNotAvailable_) {
    // The canvas was already deleted client-side
    return alternative_->webWidget()->renderRemoveJs(recursive);
  } else {
    // Nothing special, behave as usual
    return WInteractWidget::renderRemoveJs(recursive);
  }
}

std::string WGLWidget::glObjJsRef() const
{
  return "(function(){"
    "var r = " + jsRef() + ";"
    "var o = r ? jQuery.data(r,'obj') : null;"
    "return o ? o : {ctx: null};"
    "})()";
}

DomElement *WGLWidget::createDomElement(WApplication *app)
{
  DomElement *result = 0;

  if (!pImpl_) { // no GL support whatsoever
    result = DomElement::createNew(DomElement_DIV);
    result->addChild(alternative_->createSDomElement(app));
    webGlNotAvailable_ = true;
  } else {
    result = DomElement::createNew(domElementType());
    repaintGL(PAINT_GL | RESIZE_GL);
  }
  setId(result, app);

  updateDom(*result, true);

  return result;
}

void WGLWidget::repaintGL(WFlags<ClientSideRenderer> which)
{
  if (!pImpl_)
    return;

  pImpl_->repaintGL(which);

  if (which != 0)
    repaint();
}

void WGLWidget::updateDom(DomElement &element, bool all)
{
  if (all || valueChanged_) {
    valueChanged_ = false;
  }
  if (webGlNotAvailable_)
    return;

  DomElement *el = &element;
  pImpl_->updateDom(el, all);

  WInteractWidget::updateDom(element, all);
}

void WGLWidget::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  WWebWidget::getDomChanges(result, app);
}

DomElementType WGLWidget::domElementType() const
{
  if (dynamic_cast<WClientGLWidget*>(pImpl_) != 0) {
    return Wt::DomElement_CANVAS;
  } else {
    return Wt::DomElement_IMG;
  }
}

void WGLWidget::resize(const WLength &width, const WLength &height)
{
  WInteractWidget::resize(width, height);
  if (pImpl_)
    layoutSizeChanged(static_cast<int>(width.value()),
		      static_cast<int>(height.value()));
}

void WGLWidget::layoutSizeChanged(int width, int height)
{
  pImpl_->layoutSizeChanged(width, height);

  repaintGL(RESIZE_GL);
}

void WGLWidget::contextRestored()
{
  restoringContext_ = true;
  pImpl_->restoreContext(jsRef());

  repaintGL(UPDATE_GL | RESIZE_GL | PAINT_GL);
  restoringContext_ = false;
}

void WGLWidget::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  if (renderOptions_ & ClientSideRendering &&
      WApplication::instance()->environment().webGL()) {
    LOAD_JAVASCRIPT(app, "js/WPaintedWidget.js", "gfxUtils", wtjs11);
  }
  LOAD_JAVASCRIPT(app, "js/WtGlMatrix.js", "glMatrix", wtjs2);
  LOAD_JAVASCRIPT(app, "js/WGLWidget.js", "WGLWidget", wtjs1);
}

void WGLWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    if (!pImpl_) {
      if (renderOptions_ & ClientSideRendering &&
	  WApplication::instance()->environment().webGL()) {
	pImpl_ = new WClientGLWidget(this);
      } else {
#ifndef WT_TARGET_JAVA
#ifdef WT_USE_OPENGL
	if (renderOptions_ & ServerSideRendering) {
	  try {
	    pImpl_ = new WServerGLWidget(this);
	  } catch (WException& e) {
	    LOG_WARN("Failed to initialize server rendering fallback: " << e.what());
	    pImpl_ = 0;
	  }
	} else {
	  pImpl_ = 0;
	}
#else
	pImpl_ = 0;
#endif
#else
	if (renderOptions_ & ServerSideRendering) {
	  pImpl_ = new WServerGLWidget(this);
	} else {
	  pImpl_ = 0;
	}
#endif
      }
    }

    if (pImpl_ && !width().isAuto() && !height().isAuto()) {
      layoutSizeChanged(static_cast<int>(width().toPixels()),
			static_cast<int>(height().toPixels()));
    }

    defineJavaScript();
  }

  if (pImpl_)
    pImpl_->render(jsRef(), flags);

  WInteractWidget::render(flags);
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

void WGLWidget::updateGL()
{
}

void WGLWidget::debugger()
{
  pImpl_->debugger();
}

void WGLWidget::activeTexture(GLenum texture)
{
  pImpl_->activeTexture(texture);
}

void WGLWidget::attachShader(Program program, Shader shader)
{
  pImpl_->attachShader(program, shader);
}

void WGLWidget::bindAttribLocation(Program program, unsigned index, const std::string &name)
{
  pImpl_->bindAttribLocation(program, index, name);
}

void WGLWidget::bindBuffer(GLenum target, Buffer buffer)
{
  pImpl_->bindBuffer(target, buffer);
}

void WGLWidget::bindFramebuffer(GLenum target, Framebuffer buffer)
{
  pImpl_->bindFramebuffer(target, buffer);
}

void WGLWidget::bindRenderbuffer(GLenum target, Renderbuffer buffer)
{
  pImpl_->bindRenderbuffer(target, buffer);
}

void WGLWidget::bindTexture(GLenum target, Texture texture)
{
  pImpl_->bindTexture(target, texture);
}

void WGLWidget::blendColor(double red, double green, double blue, double alpha)
{
  pImpl_->blendColor(red, green, blue, alpha);
}

void WGLWidget::blendEquation(GLenum mode)
{
  pImpl_->blendEquation(mode);
}

void WGLWidget::blendEquationSeparate(GLenum modeRGB,
                                      GLenum modeAlpha)
{
  pImpl_->blendEquationSeparate(modeRGB, modeAlpha);
}

void WGLWidget::blendFunc(GLenum sfactor, GLenum dfactor)
{
  pImpl_->blendFunc(sfactor, dfactor);
}

void WGLWidget::blendFuncSeparate(GLenum srcRGB,
                                  GLenum dstRGB,
                                  GLenum srcAlpha,
                                  GLenum dstAlpha)
{
  pImpl_->blendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void WGLWidget::bufferData(Wt::WGLWidget::GLenum target, ArrayBuffer res,
			   unsigned bufferResourceOffset,
			   unsigned bufferResourceSize,
			   Wt::WGLWidget::GLenum usage)
{
  pImpl_->bufferData(target, res, bufferResourceOffset, bufferResourceSize, usage);
}

void WGLWidget::bufferData(Wt::WGLWidget::GLenum target, ArrayBuffer res,
			   Wt::WGLWidget::GLenum usage)
{
  pImpl_->bufferData(target, res, usage);
}

void WGLWidget::bufferSubData(Wt::WGLWidget::GLenum target, unsigned offset,
			      ArrayBuffer res)
{
  pImpl_->bufferSubData(target, offset, res);
}

void WGLWidget::bufferSubData(Wt::WGLWidget::GLenum target, unsigned offset,
			      ArrayBuffer res, unsigned bufferResourceOffset,
			      unsigned bufferResourceSize)
{
  pImpl_->bufferSubData(target, offset, res, bufferResourceOffset, bufferResourceSize);
}

void WGLWidget::bufferData(GLenum target, int size, GLenum usage)
{
  pImpl_->bufferData(target, size, usage);
}

void WGLWidget::bufferDatafv(GLenum target, const FloatBuffer &buffer, GLenum usage, bool binary)
{
  pImpl_->bufferDatafv(target, buffer, usage, binary);
}

#ifdef WT_TARGET_JAVA
void WGLWidget::bufferDatafv(GLenum target, const FloatNotByteBuffer &buffer, GLenum usage)
{
  pImpl_->bufferDatafv(target, buffer, usage);
}
#endif

void WGLWidget::bufferDataiv(GLenum target, IntBuffer &buffer, GLenum usage,
			     GLenum type)
{
  pImpl_->bufferDataiv(target, buffer, usage, type);
}

void WGLWidget::bufferSubDatafv(GLenum target, unsigned offset,
				const FloatBuffer &buffer, bool binary)
{
  pImpl_->bufferSubDatafv(target, offset, buffer, binary);
}

#ifdef WT_TARGET_JAVA
void WGLWidget::bufferSubDatafv(GLenum target, unsigned offset,
				const FloatNotByteBuffer &buffer)
{
  pImpl_->bufferSubDatafv(target, offset, buffer);
}
#endif

void WGLWidget::bufferSubDataiv(GLenum target,
				unsigned offset, IntBuffer &buffer, 
				GLenum type)
{
  pImpl_->bufferSubDataiv(target, offset, buffer, type);
}

void WGLWidget::clear(WFlags<GLenum> mask)
{
  pImpl_->clear(mask);
}

void WGLWidget::clearColor(double r, double g, double b, double a)
{
  pImpl_->clearColor(r, g, b, a);
}

void WGLWidget::clearDepth(double depth)
{
  pImpl_->clearDepth(depth);
}

void WGLWidget::clearStencil(int s)
{
  pImpl_->clearStencil(s);
}

void WGLWidget::colorMask(bool red, bool green, bool blue, bool alpha)
{
  pImpl_->colorMask(red, green, blue, alpha);
}

void WGLWidget::compileShader(Shader shader)
{
  pImpl_->compileShader(shader);
}

void WGLWidget::copyTexImage2D(GLenum target, int level,
                               GLenum internalFormat,
                               int x, int y,
                               unsigned width, unsigned height, 
                               int border)
{
  pImpl_->copyTexImage2D(target, level, internalFormat, x, y, width, height,
			 border);
}

void WGLWidget::copyTexSubImage2D(GLenum target, int level,
                                  int xoffset, int yoffset,
                                  int x, int y,
                                  unsigned width, unsigned height)
{
  pImpl_->copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width,
			    height);
}

WGLWidget::Buffer WGLWidget::createBuffer()
{
  return pImpl_->createBuffer();
}

WGLWidget::ArrayBuffer WGLWidget::createAndLoadArrayBuffer(const std::string &url)
{
  return pImpl_->createAndLoadArrayBuffer(url);
}

WGLWidget::Framebuffer WGLWidget::createFramebuffer()
{
  return pImpl_->createFramebuffer();
}

WGLWidget::Program WGLWidget::createProgram()
{
  return pImpl_->createProgram();
}

WGLWidget::Renderbuffer WGLWidget::createRenderbuffer()
{
  return pImpl_->createRenderbuffer();
}

WGLWidget::Shader WGLWidget::createShader(GLenum shader)
{
  return pImpl_->createShader(shader);
}

WGLWidget::Texture WGLWidget::createTexture()
{
  return pImpl_->createTexture();
}

WGLWidget::Texture WGLWidget::createTextureAndLoad(const std::string &url)
{
  return pImpl_->createTextureAndLoad(url);
}

WPaintDevice* WGLWidget::createPaintDevice(const WLength& width,
					   const WLength& height)
{
  return pImpl_->createPaintDevice(width, height);
}

void WGLWidget::cullFace(GLenum mode)
{
  pImpl_->cullFace(mode);
}

void WGLWidget::deleteBuffer(Buffer buffer)
{
  pImpl_->deleteBuffer(buffer);
}

void WGLWidget::deleteFramebuffer(Framebuffer buffer)
{
  pImpl_->deleteFramebuffer(buffer);
}

void WGLWidget::deleteProgram(Program program)
{
  pImpl_->deleteProgram(program);
}

void WGLWidget::deleteRenderbuffer(Renderbuffer buffer)
{
  pImpl_->deleteRenderbuffer(buffer);
}

void WGLWidget::deleteShader(Shader shader)
{
  pImpl_->deleteShader(shader);
}

void WGLWidget::deleteTexture(Texture texture)
{
  pImpl_->deleteTexture(texture);
}

void WGLWidget::depthFunc(GLenum func)
{
  pImpl_->depthFunc(func);
}

void WGLWidget::depthMask(bool flag)
{
  pImpl_->depthMask(flag);
}

void WGLWidget::depthRange(double zNear, double zFar)
{
  pImpl_->depthRange(zNear, zFar);
}

void WGLWidget::detachShader(Program program, Shader shader)
{
  pImpl_->detachShader(program, shader);
}

void WGLWidget::disable(GLenum cap)
{
  pImpl_->disable(cap);
}

void WGLWidget::disableVertexAttribArray(AttribLocation index)
{
  pImpl_->disableVertexAttribArray(index);
}

void WGLWidget::drawArrays(GLenum mode, int first, unsigned count)
{
  pImpl_->drawArrays(mode, first, count);
}

void WGLWidget::drawElements(GLenum mode, unsigned count,
                             GLenum type, unsigned offset)
{
  pImpl_->drawElements(mode, count, type, offset);
}

void WGLWidget::enable(GLenum cap)
{
  pImpl_->enable(cap);
}

void WGLWidget::enableVertexAttribArray(AttribLocation index)
{
  pImpl_->enableVertexAttribArray(index);
}

void WGLWidget::finish()
{
  pImpl_->finish();
}
void WGLWidget::flush()
{
  pImpl_->flush();
}

void WGLWidget::framebufferRenderbuffer(GLenum target, GLenum attachment,
    GLenum renderbuffertarget, Renderbuffer renderbuffer)
{
  pImpl_->framebufferRenderbuffer(target, attachment, renderbuffertarget,
				  renderbuffer);
}

void WGLWidget::framebufferTexture2D(GLenum target, GLenum attachment,
    GLenum textarget, Texture texture, int level)
{
  pImpl_->framebufferTexture2D(target, attachment, textarget, texture, level);
}

void WGLWidget::frontFace(GLenum mode)
{
  pImpl_->frontFace(mode);
}

void WGLWidget::generateMipmap(GLenum target)
{
  pImpl_->generateMipmap(target);
}

WGLWidget::AttribLocation WGLWidget::getAttribLocation(Program program, const std::string &attrib)
{
  return pImpl_->getAttribLocation(program, attrib);
}

WGLWidget::UniformLocation WGLWidget::getUniformLocation(Program program, const std::string location)
{
  return pImpl_->getUniformLocation(program,  location);
}

void WGLWidget::hint(GLenum target, GLenum mode)
{
  pImpl_->hint(target, mode);
}

void WGLWidget::lineWidth(double width)
{
  pImpl_->lineWidth(width);
}

void WGLWidget::linkProgram(Program program)
{
  pImpl_->linkProgram(program);
}

void WGLWidget::pixelStorei(GLenum pname, int param)
{
  pImpl_->pixelStorei(pname, param);
}

void WGLWidget::polygonOffset(double factor, double units)
{
  pImpl_->polygonOffset(factor, units);
}

void WGLWidget::renderbufferStorage(GLenum target, GLenum internalformat, 
  unsigned width, unsigned height)
{
  pImpl_->renderbufferStorage(target, internalformat, width, height);
}

void WGLWidget::sampleCoverage(double value, bool invert)
{
  pImpl_->sampleCoverage(value, invert);
}

void WGLWidget::scissor(int x, int y, unsigned width, unsigned height)
{
  pImpl_->scissor(x, y, width, height);
}

void WGLWidget::shaderSource(Shader shader, const std::string &src)
{
  pImpl_->shaderSource(shader, src);
}

void WGLWidget::stencilFunc(GLenum func, int ref, unsigned mask)
{
  pImpl_->stencilFunc(func, ref, mask);
}

void WGLWidget::stencilFuncSeparate(GLenum face,
                                    GLenum func, int ref,
                                    unsigned mask)
{
  pImpl_->stencilFuncSeparate(face, func, ref, mask);
}

void WGLWidget::stencilMask(unsigned mask)
{
  pImpl_->stencilMask(mask);
}

void WGLWidget::stencilMaskSeparate(GLenum face, unsigned mask)
{
  pImpl_->stencilMaskSeparate(face, mask);
}

void WGLWidget::stencilOp(GLenum fail, GLenum zfail,
                          GLenum zpass)
{
  pImpl_->stencilOp(fail, zfail, zpass);
}

void WGLWidget::stencilOpSeparate(GLenum face, GLenum fail,
                                  GLenum zfail, GLenum zpass)
{
  pImpl_->stencilOpSeparate(face, fail, zfail, zpass);
}

void WGLWidget::texImage2D(GLenum target, int level, GLenum internalformat, 
                  unsigned width, unsigned height, int border, GLenum format)
{
  pImpl_->texImage2D(target, level, internalformat, width, height, border,
		     format);
}

void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           WImage *image)
{
  pImpl_->texImage2D(target, level, internalformat, format, type, image);
}

void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           WVideo *video)
{
  pImpl_->texImage2D(target, level, internalformat, format, type, video);
}

void WGLWidget::texImage2D(GLenum target, int level, GLenum internalformat,
			   GLenum format, GLenum type, std::string imgFilename)
{
  pImpl_->texImage2D(target, level, internalformat, format, type, imgFilename);
}

void WGLWidget::texImage2D(GLenum target, int level, GLenum internalformat,
			   GLenum format, GLenum type,
			   WPaintDevice *paintdevice)
{
  pImpl_->texImage2D(target, level, internalformat, format, type, paintdevice);
}

// Deprecated!
void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           Texture texture)
{
  pImpl_->texImage2D(target, level, internalformat, format, type, texture);
}

void WGLWidget::texParameteri(GLenum target,
                              GLenum pname,
                              GLenum param)
{
  pImpl_->texParameteri(target, pname, param);
}

void WGLWidget::uniform1f(const UniformLocation &location, double x)
{
  pImpl_->uniform1f(location, x);
}

void WGLWidget::uniform1fv(const UniformLocation &location,
			   const WT_ARRAY float *value)
{
  pImpl_->uniform1fv(location, value);
}

void WGLWidget::uniform1fv(const UniformLocation &location,
			   const JavaScriptVector &v)
{
  pImpl_->uniform1fv(location, v);
}

void WGLWidget::uniform1i(const UniformLocation &location, int x)
{
  pImpl_->uniform1i(location, x);
}

void WGLWidget::uniform1iv(const UniformLocation &location, 
			   const WT_ARRAY int *value)
{
  pImpl_->uniform1iv(location, value);
}

void WGLWidget::uniform2f(const UniformLocation &location, double x, double y)
{
  pImpl_->uniform2f(location, x, y);
}

void WGLWidget::uniform2fv(const UniformLocation &location, 
			   const WT_ARRAY float *value)
{
  pImpl_->uniform2fv(location, value);
}

void WGLWidget::uniform2fv(const UniformLocation &location, 
			   const JavaScriptVector &v)
{
  pImpl_->uniform2fv(location, v);
}

void WGLWidget::uniform2i(const UniformLocation &location, int x, int y)
{
  pImpl_->uniform2i(location, x, y);
}

void WGLWidget::uniform2iv(const UniformLocation &location, 
			   const WT_ARRAY int *value)
{
  pImpl_->uniform2iv(location, value);
}

void WGLWidget::uniform3f(const UniformLocation &location,
			  double x, double y, double z)
{
  pImpl_->uniform3f(location, x, y, z);
}

void WGLWidget::uniform3fv(const UniformLocation &location, 
			   const WT_ARRAY float *value)
{
  pImpl_->uniform3fv(location, value);
}

void WGLWidget::uniform3fv(const UniformLocation &location,
			   const JavaScriptVector &v)
{
  pImpl_->uniform3fv(location, v);
}

void WGLWidget::uniform3i(const UniformLocation &location, int x, int y, int z)
{
  pImpl_->uniform3i(location, x, y, z);
}

void WGLWidget::uniform3iv(const UniformLocation &location, 
			   const WT_ARRAY int *value)
{
  pImpl_->uniform3iv(location, value);
}

void WGLWidget::uniform4f(const UniformLocation &location,
			  double x, double y, double z, double w)
{
  pImpl_->uniform4f(location, x, y, z, w);
}

void WGLWidget::uniform4fv(const UniformLocation &location, 
			   const WT_ARRAY float *value)
{
  pImpl_->uniform4fv(location, value);
}

void WGLWidget::uniform4fv(const UniformLocation &location,
			   const JavaScriptVector &v)
{
  pImpl_->uniform4fv(location, v);
}

void WGLWidget::uniform4i(const UniformLocation &location, int x, int y, 
			  int z, int w)
{
  pImpl_->uniform4i(location, x, y, z, w);
}

void WGLWidget::uniform4iv(const UniformLocation &location, 
			   const WT_ARRAY int *value)
{
  pImpl_->uniform4iv(location, value);
}

void WGLWidget::uniformMatrix2fv(const UniformLocation &location,
				 bool transpose, 
				 const WT_ARRAY double *value)
{
  pImpl_->uniformMatrix2fv(location, transpose, value);
}

void WGLWidget::uniformMatrix2(const UniformLocation &location,
			       const WGenericMatrix<double, 2, 2> &m)
{
  pImpl_->uniformMatrix2(location, m);
}

void WGLWidget::uniformMatrix3fv(const UniformLocation &location,
				 bool transpose, 
				 const WT_ARRAY double *value)
{
  pImpl_->uniformMatrix3fv(location, transpose, value);
}

void WGLWidget::uniformMatrix3(const UniformLocation &location,
			       const WGenericMatrix<double, 3, 3> &m)
{
  pImpl_->uniformMatrix3(location, m);
}

void WGLWidget::uniformMatrix4fv(const UniformLocation &location,
				 bool transpose,
				 const WT_ARRAY double *value)
{
  pImpl_->uniformMatrix4fv(location, transpose, value);
}

void WGLWidget::uniformMatrix4(const UniformLocation &location,
			       const WGenericMatrix<double, 4, 4> &m)
{
  pImpl_->uniformMatrix4(location, m);
}

void WGLWidget::uniformMatrix4(const UniformLocation &location,
			       const JavaScriptMatrix4x4 &jsm)
{
  if (!jsm.initialized())
    throw WException("JavaScriptMatrix4x4: matrix not initialized");

  pImpl_->uniformMatrix4(location, jsm);
}

void WGLWidget::useProgram(Program program)
{
  pImpl_->useProgram(program);
}

void WGLWidget::validateProgram(Program program)
{
  pImpl_->validateProgram(program);
}

void WGLWidget::vertexAttrib1f(AttribLocation location, double x) {
  pImpl_->vertexAttrib1f(location, x);
}

void WGLWidget::vertexAttrib2f(AttribLocation location, double x, double y) {
  pImpl_->vertexAttrib2f(location, x, y);
}

void WGLWidget::vertexAttrib3f(AttribLocation location, double x, double y, 
			       double z) {
  pImpl_->vertexAttrib3f(location, x, y, z);
}

void WGLWidget::vertexAttrib4f(AttribLocation location,
			       double x, double y, double z, double w) {
  pImpl_->vertexAttrib4f(location, x, y, z, w);
}

void WGLWidget::vertexAttribPointer(AttribLocation location, int size,
				    GLenum type, bool normalized,
				    unsigned stride, unsigned offset)
{
  pImpl_->vertexAttribPointer(location, size, type, normalized, stride, offset);
}

void WGLWidget::viewport(int x, int y, unsigned width, unsigned height)
{
  pImpl_->viewport(x, y, width, height);
}

void WGLWidget::clearBinaryResources()
{
  pImpl_->clearBinaryResources();
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::createJavaScriptMatrix4()
{
  WGLWidget::JavaScriptMatrix4x4 mat;
  addJavaScriptMatrix4(mat);
  initJavaScriptMatrix4(mat);

  return mat;
}

void WGLWidget::addJavaScriptMatrix4(JavaScriptMatrix4x4 &mat)
{
  if (mat.hasContext())
    throw WException("The given matrix is already associated with a WGLWidget!");
  mat.assignToContext(jsValues_++, this);

  jsMatrixList_.push_back(jsMatrixMap(mat.id(), WMatrix4x4()));
}

void WGLWidget::initJavaScriptMatrix4(JavaScriptMatrix4x4 &mat)
{
  pImpl_->initJavaScriptMatrix4(mat);
}

WGLWidget::JavaScriptVector WGLWidget::createJavaScriptVector(unsigned length)
{
  WGLWidget::JavaScriptVector vec(length);
  addJavaScriptVector(vec);
  initJavaScriptVector(vec);

  return vec;
}

void WGLWidget::addJavaScriptVector(JavaScriptVector &vec)
{
  if (vec.hasContext())
    throw WException("The given matrix is already associated with a WGLWidget!");
  vec.assignToContext(jsValues_++, this);

  std::vector<float> values;
  for (unsigned i = 0; i < vec.length(); ++i) {
    values.push_back(0.0f);
  }
  jsVectorList_.push_back(jsVectorMap(vec.id(), values));
}

void WGLWidget::initJavaScriptVector(JavaScriptVector &vec)
{
  pImpl_->initJavaScriptVector(vec);
}

void WGLWidget::setJavaScriptMatrix4(JavaScriptMatrix4x4 &jsm,
				     const WGenericMatrix<double, 4, 4> &m)
{
  if (!jsm.initialized())
    throw WException("JavaScriptMatrix4x4: matrix not initialized");
  if (jsm.hasOperations())
    throw WException("JavaScriptMatrix4x4: matrix was already operated on");

  valueChanged_ = true;

  // set the server-side copy
  for (unsigned i = 0; i < jsMatrixList_.size(); i++)
    if (jsMatrixList_[i].id == jsm.id())
      jsMatrixList_[i].serverSideCopy = m;

  pImpl_->setJavaScriptMatrix4(jsm, m);
}

void WGLWidget::setJavaScriptVector(JavaScriptVector &jsv,
				    const std::vector<float> &v)
{
  if (!jsv.initialized())
    throw WException("JavaScriptVector: vector not initialized");

  valueChanged_ = true;

  // set the server-side copy
  for (unsigned i = 0; i < jsVectorList_.size(); i++)
    if (jsVectorList_[i].id == jsv.id())
      jsVectorList_[i].serverSideCopy = v;

  pImpl_->setJavaScriptVector(jsv, v);
}

void WGLWidget::setClientSideMouseHandler(const std::string& handlerCode)
{
  pImpl_->setClientSideMouseHandler(handlerCode);
}

void WGLWidget::setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
                                           double centerX, double centerY, double centerZ,
                                           double uX, double uY, double uZ,
                                           double pitchRate, double yawRate)
{
  pImpl_->setClientSideLookAtHandler(m, centerX, centerY, centerZ, uX, uY, uZ,
				     pitchRate, yawRate);
}

void WGLWidget::setClientSideWalkHandler(const JavaScriptMatrix4x4 &m, double frontStep, double rotStep)
{
  pImpl_->setClientSideWalkHandler(m, frontStep, rotStep);
}

void WGLWidget::setFormData(const FormData& formData)
{
  if (valueChanged_)
    return;

  Http::ParameterValues parVals = formData.values;
  if (Utils::isEmpty(parVals))
    return;
  if (parVals[0] == "undefined")
    return;

  std::vector<std::string> matrices;
  boost::split(matrices, parVals[0], boost::is_any_of(";"));
  for (unsigned i = 0; i < matrices.size(); i++) {
    if (matrices[i] == "")
      break;
    std::vector<std::string> idAndData;
    boost::split(idAndData, matrices[i], boost::is_any_of(":"));
    int id = (int)Wt::asNumber(idAndData[0]);

    unsigned j = 0;
    for (j = 0; j < jsMatrixList_.size(); j++)
      if (jsMatrixList_[j].id == id)
	break;
    if (j == jsMatrixList_.size()) {
      for (j = 0; j < jsVectorList_.size(); j++)
	if (jsVectorList_[j].id == id)
	  break;
      std::vector<float>& vec = jsVectorList_[j].serverSideCopy;
      std::vector<std::string> mData;
      boost::split(mData, idAndData[1], boost::is_any_of(","));
      for (unsigned i1 = 0; i1 < vec.size(); i1++) {
	if (mData[i1] == "Infinity") {
	  vec[i1] = std::numeric_limits<float>::infinity();
	} else if (mData[i1] == "-Infinity") {
	  vec[i1] = -std::numeric_limits<float>::infinity();
	} else {
	  vec[i1] = boost::lexical_cast<float>(mData[i1]);
	}
      }
    } else {
      WMatrix4x4& mat = jsMatrixList_[j].serverSideCopy;
      std::vector<std::string> mData;
      boost::split(mData, idAndData[1], boost::is_any_of(","));
      for (int i1 = 0; i1 < 4; i1++) {
	for (int i2 = 0; i2 < 4; i2++) {
#ifndef WT_TARGET_JAVA
	  mat(i2, i1) = (float)Wt::asNumber(mData[i1*4+i2]);
#else
	  mat.setElement(i2, i1, boost::lexical_cast<float>(mData[i1*4+i2]));
#endif
	}
      }
    }
  }
}

WGLWidget::JavaScriptVector::JavaScriptVector(unsigned length)
  : id_(-1), length_(length), context_(0), initialized_(false)
{}

void WGLWidget::JavaScriptVector
::assignToContext(int id, const WGLWidget *context)
{
  id_ = id;
  jsRef_ = context->glObjJsRef() + ".jsValues[" + 
    boost::lexical_cast<std::string>(id_) + "]";
  context_ = context;
}

#ifndef WT_TARGET_JAVA
WGLWidget::JavaScriptVector
::JavaScriptVector(const WGLWidget::JavaScriptVector &other)
  : id_(other.id()),
    length_(other.length()),
    jsRef_(other.jsRef()),
    context_(other.context_),
    initialized_(other.initialized())
{
}

WGLWidget::JavaScriptVector &
WGLWidget::JavaScriptVector::operator=(const WGLWidget::JavaScriptVector &rhs)
{
  id_ = rhs.id_;
  length_ = rhs.length_;
  jsRef_ = rhs.jsRef_;
  context_ = rhs.context_;
  initialized_ = rhs.initialized_;
  return *this;
}
#endif

std::vector<float> WGLWidget::JavaScriptVector::value() const
{
  if (!hasContext())
    throw WException("JavaScriptVector: vector not assigned to a WGLWidget");

  for (unsigned i = 0; i < context_->jsVectorList_.size(); i++) {
    if (context_->jsVectorList_[i].id == id_) {
      return context_->jsVectorList_[i].serverSideCopy;
    }
  }

  std::vector<float> result;
  for (unsigned i = 0; i < length(); ++i) {
    result.push_back(0.0f);
  }
  return result;
}

WGLWidget::JavaScriptMatrix4x4::JavaScriptMatrix4x4()
  : id_(-1), context_(0), initialized_(false)
{}

void WGLWidget::JavaScriptMatrix4x4::assignToContext(int id, const WGLWidget* context)
{
  id_ = id;
  jsRef_ = context->glObjJsRef() + ".jsValues[" + boost::lexical_cast<std::string>(id_) + "]";
  context_ = context;
}

#ifndef WT_TARGET_JAVA
WGLWidget::JavaScriptMatrix4x4::JavaScriptMatrix4x4(const WGLWidget::JavaScriptMatrix4x4 &other)
  : id_(other.id()),
    jsRef_(other.jsRef()),
    context_(other.context_),
    operations_(other.operations_),
    matrices_(other.matrices_),
    initialized_(other.initialized_)
{
}

WGLWidget::JavaScriptMatrix4x4 &
WGLWidget::JavaScriptMatrix4x4::operator=(const WGLWidget::JavaScriptMatrix4x4 &rhs)
{
  id_ = rhs.id_;
  jsRef_ = rhs.jsRef_;
  context_ = rhs.context_;
  operations_ = rhs.operations_;
  matrices_ = rhs.matrices_;
  initialized_ = rhs.initialized_;
  return *this;
}
#endif

WMatrix4x4 WGLWidget::JavaScriptMatrix4x4::value() const
{
  if (!hasContext())
    throw WException("JavaScriptMatrix4x4: matrix not assigned to a WGLWidget");

  WMatrix4x4 originalCpy;
  for (unsigned i = 0; i<context_->jsMatrixList_.size(); i++)
    if (context_->jsMatrixList_[i].id == id_)
      originalCpy = context_->jsMatrixList_[i].serverSideCopy;

  // apply all operations
  int nbMult = 0;
  for (unsigned i = 0; i<operations_.size(); i++) {
    switch (operations_[i]) {
    case TRANSPOSE:
      originalCpy = originalCpy.transposed();
      break;
    case INVERT:
#ifndef WT_TARGET_JAVA
      originalCpy = 
#endif
	originalCpy.inverted();
      break;
    case MULTIPLY:
#ifndef WT_TARGET_JAVA
      originalCpy = originalCpy * matrices_[nbMult];
#else
      originalCpy.mul(matrices_[nbMult]);
#endif
      nbMult++;
      break;
    }
  }

  return originalCpy;
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::inverted() const
{
  if (!initialized())
    throw WException("JavaScriptMatrix4x4: matrix not initialized");
  WGLWidget::JavaScriptMatrix4x4 copy = WGLWidget::JavaScriptMatrix4x4(*this);
  copy.jsRef_ = WT_CLASS ".glMatrix.mat4.inverse(" +
    jsRef_ + ", " WT_CLASS ".glMatrix.mat4.create())";
  copy.operations_.push_back(INVERT);
  return copy;
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::transposed() const
{
  if (!initialized())
    throw WException("JavaScriptMatrix4x4: matrix not initialized");
  WGLWidget::JavaScriptMatrix4x4 copy = WGLWidget::JavaScriptMatrix4x4(*this);
  copy.jsRef_ = WT_CLASS ".glMatrix.mat4.transpose(" +
    jsRef_ + ", " WT_CLASS ".glMatrix.mat4.create())";
  copy.operations_.push_back(TRANSPOSE);
  return copy;
}

#if !defined(WT_TARGET_JAVA)
WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::operator*(const WGenericMatrix<double, 4, 4> &m) const
#else 
WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::multiply(const WGenericMatrix<double, 4, 4> &m) const
#endif 
{
  if (!initialized())
    throw WException("JavaScriptMatrix4x4: matrix not initialized");
  WGLWidget::JavaScriptMatrix4x4 copy = WGLWidget::JavaScriptMatrix4x4(*this);
  std::stringstream ss;
  ss << WT_CLASS ".glMatrix.mat4.multiply(" << jsRef_ << ",";
  WGenericMatrix<double, 4, 4> t(m.transposed());
#ifndef WT_TARGET_JAVA
  WClientGLWidget::renderfv(ss, t, context_->pImpl_->arrayType());
#else
  renderfv(ss, t, context_->pImpl_->arrayType());
#endif
  ss << ", " WT_CLASS ".glMatrix.mat4.create())";
  copy.jsRef_ = ss.str();
  copy.operations_.push_back(MULTIPLY);
  copy.matrices_.push_back(m);
  return copy;
}

#ifdef WT_TARGET_JAVA
WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::clone() const
{
  WGLWidget::JavaScriptMatrix4x4 copy;
  copy.id_ = id_;
  copy.jsRef_ = jsRef_;
  copy.context_ = context_;
  copy.initialized_ = initialized_;

  copy.operations_ = operations_;
  copy.matrices_ = matrices_;

  return copy;
}
#endif

}
