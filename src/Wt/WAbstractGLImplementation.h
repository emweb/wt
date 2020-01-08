// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACTGLIMPLEMENTATION_H
#define WABSTRACTGLIMPLEMENTATION_H

#include <Wt/WGLWidget.h>
#include "DomElement.h"

namespace Wt {

class WAbstractGLImplementation : public WObject {
public:
  virtual void debugger() = 0;

  virtual void activeTexture(WGLWidget::GLenum texture) = 0;

  virtual void attachShader(WGLWidget::Program program,
			    WGLWidget::Shader shader) = 0;

  virtual void bindAttribLocation(WGLWidget::Program program, unsigned index,
				  const std::string &name) = 0;

  virtual void bindBuffer(WGLWidget::GLenum target,
			  WGLWidget::Buffer buffer) = 0;

  virtual void bindFramebuffer(WGLWidget::GLenum target,
			       WGLWidget::Framebuffer buffer) = 0;

  virtual void bindRenderbuffer(WGLWidget::GLenum target,
				WGLWidget::Renderbuffer buffer) = 0;

  virtual void bindTexture(WGLWidget::GLenum target,
			   WGLWidget::Texture texture) = 0;

  virtual void blendColor(double red, double green, double blue,
			  double alpha) = 0;

  virtual void blendEquation(WGLWidget::GLenum mode) = 0;

  virtual void blendEquationSeparate(WGLWidget::GLenum modeRGB,
				     WGLWidget::GLenum modeAlpha) = 0;

  virtual void blendFunc(WGLWidget::GLenum sfactor,
			 WGLWidget::GLenum dfactor) = 0;

  virtual void blendFuncSeparate(WGLWidget::GLenum srcRGB,
				 WGLWidget::GLenum dstRGB,
				 WGLWidget::GLenum srcAlpha,
				 WGLWidget::GLenum dstAlpha) = 0;

  virtual void bufferData(WGLWidget::GLenum target, int size, WGLWidget::GLenum usage) = 0;

  virtual void bufferData(WGLWidget::GLenum target, WGLWidget::ArrayBuffer res,
			  WGLWidget::GLenum usage) = 0;

  virtual void bufferData(WGLWidget::GLenum target, WGLWidget::ArrayBuffer res,
			  unsigned arrayBufferOffset, unsigned arrayBufferSize,
			  WGLWidget::GLenum usage) = 0;

  virtual void bufferSubData(WGLWidget::GLenum target, unsigned offset,
			     WGLWidget::ArrayBuffer res) = 0;

  virtual void bufferSubData(WGLWidget::GLenum target, unsigned offset,
			     WGLWidget::ArrayBuffer res,
			     unsigned arrayBufferOffset, unsigned size) = 0;

  virtual void bufferDatafv(WGLWidget::GLenum target, const FloatBuffer &v, WGLWidget::GLenum usage, bool binary) = 0;
#ifdef WT_TARGET_JAVA
  virtual void bufferDatafv(WGLWidget::GLenum target, const FloatNotByteBuffer &buffer, WGLWidget::GLenum usage) = 0;
#endif

  virtual void bufferSubDatafv(WGLWidget::GLenum target, unsigned offset, const FloatBuffer &buffer, bool binary) = 0;
#ifdef WT_TARGET_JAVA
  virtual void bufferSubDatafv(WGLWidget::GLenum target, unsigned offset, const FloatNotByteBuffer &buffer) = 0;
#endif

  virtual void bufferDataiv(WGLWidget::GLenum target, IntBuffer &buffer, WGLWidget::GLenum usage, WGLWidget::GLenum type) = 0;

  virtual void bufferSubDataiv(WGLWidget::GLenum target,
			       unsigned offset, IntBuffer &buffer, 
			       WGLWidget::GLenum type) = 0;

  virtual void clearBinaryResources() = 0;

  virtual void clear(WFlags<WGLWidget::GLenum> mask) = 0;

  virtual void clearColor(double r, double g, double b, double a) = 0;

  virtual void clearDepth(double depth) = 0;

  virtual void clearStencil(int s) = 0;

  virtual void colorMask(bool red, bool green, bool blue, bool alpha) = 0;

  virtual void compileShader(WGLWidget::Shader shader) = 0;

  virtual void copyTexImage2D(WGLWidget::GLenum target, int level,
			      WGLWidget::GLenum internalFormat,
			      int x, int y,
			      unsigned width, unsigned height, 
			      int border) = 0;

  virtual void copyTexSubImage2D(WGLWidget::GLenum target, int level,
				 int xoffset, int yoffset,
				 int x, int y,
				 unsigned width, unsigned height) = 0;

  virtual WGLWidget::Buffer createBuffer() = 0;

  virtual WGLWidget::ArrayBuffer createAndLoadArrayBuffer(const std::string &url) = 0;

  virtual WGLWidget::Framebuffer createFramebuffer() = 0;

  virtual WGLWidget::Program createProgram() = 0;

  virtual WGLWidget::Renderbuffer createRenderbuffer() = 0;

  virtual WGLWidget::Shader createShader(WGLWidget::GLenum shader) = 0;

  virtual WGLWidget::Texture createTexture() = 0;

  virtual WGLWidget::Texture createTextureAndLoad(const std::string &url) = 0;

  virtual std::unique_ptr<WPaintDevice>
    createPaintDevice(const WLength& width, const WLength& height) = 0;

  virtual void cullFace(WGLWidget::GLenum mode) = 0;

  virtual void deleteBuffer(WGLWidget::Buffer buffer) = 0;

  virtual void deleteFramebuffer(WGLWidget::Framebuffer buffer) = 0;

  virtual void deleteProgram(WGLWidget::Program program) = 0;

  virtual void deleteRenderbuffer(WGLWidget::Renderbuffer buffer) = 0;

  virtual void deleteShader(WGLWidget::Shader shader) = 0;

  virtual void deleteTexture(WGLWidget::Texture texture) = 0;

  virtual void depthFunc(WGLWidget::GLenum func) = 0;

  virtual void depthMask(bool flag) = 0;

  virtual void depthRange(double zNear, double zFar) = 0;

  virtual void detachShader(WGLWidget::Program program,
			    WGLWidget::Shader shader) = 0;

  virtual void disable(WGLWidget::GLenum cap) = 0;

  virtual void disableVertexAttribArray(WGLWidget::AttribLocation index) = 0;

  virtual void drawArrays(WGLWidget::GLenum mode, int first, unsigned count) = 0;

  virtual void drawElements(WGLWidget::GLenum mode, unsigned count, WGLWidget::GLenum type, unsigned offset) = 0;

  virtual void enable(WGLWidget::GLenum cap) = 0;

  virtual void enableVertexAttribArray(WGLWidget::AttribLocation index) = 0;
  
  virtual void finish() = 0;

  virtual void flush() = 0;

  virtual void framebufferRenderbuffer(WGLWidget::GLenum target,
				       WGLWidget::GLenum attachment,
				       WGLWidget::GLenum renderbuffertarget,
				       WGLWidget::Renderbuffer renderbuffer) = 0;

  virtual void framebufferTexture2D(WGLWidget::GLenum target,
				    WGLWidget::GLenum attachment,
				    WGLWidget::GLenum textarget,
				    WGLWidget::Texture texture, int level) = 0;

  virtual void frontFace(WGLWidget::GLenum mode) = 0;

  virtual void generateMipmap(WGLWidget::GLenum target) = 0;

  virtual WGLWidget::AttribLocation getAttribLocation(WGLWidget::Program program, const std::string &attrib) = 0;

  virtual WGLWidget::UniformLocation getUniformLocation(WGLWidget::Program program, const std::string &location) = 0;

  virtual void hint(WGLWidget::GLenum target, WGLWidget::GLenum mode) = 0;

  virtual void lineWidth(double width) = 0;

  virtual void linkProgram(WGLWidget::Program program) = 0;

  virtual void pixelStorei(WGLWidget::GLenum pname, int param) = 0;

  virtual void polygonOffset(double factor, double units) = 0;

  virtual void renderbufferStorage(WGLWidget::GLenum target,
				   WGLWidget::GLenum internalformat, 
				   unsigned width, unsigned height) = 0;

  virtual void sampleCoverage(double value, bool invert) = 0;

  virtual void scissor(int x, int y, unsigned width, unsigned height) = 0;

  virtual void shaderSource(WGLWidget::Shader shader,
			    const std::string &src) = 0;

  virtual void stencilFunc(WGLWidget::GLenum func, int ref, unsigned mask) = 0;

  virtual void stencilFuncSeparate(WGLWidget::GLenum face,
				   WGLWidget::GLenum func, int ref,
				   unsigned mask) = 0;

  virtual void stencilMask(unsigned mask) = 0;

  virtual void stencilMaskSeparate(WGLWidget::GLenum face, unsigned mask) = 0;

  virtual void stencilOp(WGLWidget::GLenum fail, WGLWidget::GLenum zfail,
			 WGLWidget::GLenum zpass) = 0;

  virtual void stencilOpSeparate(WGLWidget::GLenum face, WGLWidget::GLenum fail,
				 WGLWidget::GLenum zfail, WGLWidget::GLenum zpass) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat, 
			  unsigned width, unsigned height, int border,
			  WGLWidget::GLenum format) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WImage *image) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WVideo *video) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  std::string image) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WPaintDevice *paintdevice) = 0;

  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WGLWidget::Texture texture) = 0;

  virtual void texParameteri(WGLWidget::GLenum target, WGLWidget::GLenum pname,
			     WGLWidget::GLenum param) = 0;

  virtual void uniform1f(const WGLWidget::UniformLocation &location,
			 double x) = 0;

  virtual void uniform1fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value) = 0;

  virtual void uniform1fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v) = 0;

  virtual void uniform1i(const WGLWidget::UniformLocation &location,
			 int x) = 0;

  virtual void uniform1iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value) = 0;

  virtual void uniform2f(const WGLWidget::UniformLocation &location,
			 double x, double y) = 0;

  virtual void uniform2fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value) = 0;

  virtual void uniform2fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v) = 0;

  virtual void uniform2i(const WGLWidget::UniformLocation &location,
			 int x, int y) = 0;
  
  virtual void uniform2iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value) = 0;
  
  virtual void uniform3f(const WGLWidget::UniformLocation &location,
			 double x, double y, double z) = 0;
  
  virtual void uniform3fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value) = 0;

  virtual void uniform3fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v) = 0;

  virtual void uniform3i(const WGLWidget::UniformLocation &location,
			 int x, int y, int z) = 0;

  virtual void uniform3iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value) = 0;

  virtual void uniform4f(const WGLWidget::UniformLocation &location,
			 double x, double y, double z, double w) = 0;

  virtual void uniform4fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value) = 0;

  virtual void uniform4fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v) = 0;

  virtual void uniform4i(const WGLWidget::UniformLocation &location,
			 int x, int y, int z, int w) = 0;
  
  virtual void uniform4iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value) = 0;

  virtual void uniformMatrix2fv(const WGLWidget::UniformLocation &location,
				bool transpose,
				const WT_ARRAY double *value) = 0;

  virtual void uniformMatrix2(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 2, 2> &m) = 0;

  virtual void uniformMatrix3fv(const WGLWidget::UniformLocation &location,
				bool transpose,
				const WT_ARRAY double *value) = 0;

  virtual void uniformMatrix3(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 3, 3> &m) = 0;

  virtual void uniformMatrix4fv(const WGLWidget::UniformLocation &location,
				bool transpose,
				const WT_ARRAY double *value) = 0;

  virtual void uniformMatrix4(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 4, 4> &m) = 0;

  virtual void uniformMatrix4(const WGLWidget::UniformLocation &location,
			      const WGLWidget::JavaScriptMatrix4x4 &m) = 0;

  virtual void useProgram(WGLWidget::Program program) = 0;

  virtual void validateProgram(WGLWidget::Program program) = 0;

  virtual void vertexAttrib1f(WGLWidget::AttribLocation location, double x) = 0;

  virtual void vertexAttrib2f(WGLWidget::AttribLocation location,
			      double x, double y) = 0;

  virtual void vertexAttrib3f(WGLWidget::AttribLocation location,
			      double x, double y, double z) = 0;

  virtual void vertexAttrib4f(WGLWidget::AttribLocation location,
			      double x, double y, double z, double w) = 0;

  virtual void vertexAttribPointer(WGLWidget::AttribLocation location, int size,
				   WGLWidget::GLenum type, bool normalized,
				   unsigned stride, unsigned offset) = 0;

  virtual void viewport(int x, int y, unsigned width, unsigned height) = 0;

  virtual void initJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm) = 0;

  virtual void setJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm,
				    const WGenericMatrix<double, 4, 4> &m) = 0;

  virtual void initJavaScriptVector(WGLWidget::JavaScriptVector &jsv) = 0;

  virtual void setJavaScriptVector(WGLWidget::JavaScriptVector &jsv,
				   const std::vector<float> &v) = 0;

  virtual void setClientSideMouseHandler(const std::string& handlerCode) = 0;

  virtual void setClientSideLookAtHandler(const WGLWidget::JavaScriptMatrix4x4 &m,
					  double ctrX, double ctrY, double ctrZ,
					  double uX, double uY, double uZ,
					  double pitchRate, double yawRate) = 0;
    
  virtual void setClientSideWalkHandler(const WGLWidget::JavaScriptMatrix4x4 &m,
					double frontStep, double rotStep) = 0;

  void layoutSizeChanged(int width, int height);

  void repaintGL(WFlags<GLClientSideRenderer> which);
  
  // void mouseDownHandler(WMouseEvent e);
  // void mouseUpHandler(WMouseEvent e);
  // void mouseDragHandler(WMouseEvent e);
  // void mouseWheelHandler(WMouseEvent e);

  void enableClientErrorChecks(bool enable) {
#ifdef WT_WGLWIDGET_DEBUG
    debugging_ = enable;
#endif
  }

  virtual JsArrayType arrayType() const = 0;

  virtual void injectJS(const std::string & jsString) = 0;

  virtual void restoreContext(const std::string &jsRef) = 0;

  virtual void render(const std::string& jsRef, WFlags<RenderFlag> flags)  = 0;

  void updateDom(DomElement *el, bool all) {
    if (all || sizeChanged_) {
      el->setAttribute("width", std::to_string(renderWidth_));
      el->setAttribute("height", std::to_string(renderHeight_));
      sizeChanged_ = false;
    }
  }

protected:
  WAbstractGLImplementation(WGLWidget *glInterface);

  WGLWidget *glInterface_;
  bool updateGL_, updateResizeGL_, updatePaintGL_;

  int renderWidth_;
  int renderHeight_;
  bool sizeChanged_;
#ifdef WT_WGLWIDGET_DEBUG
  bool debugging_;
#endif

  JSignal<> webglNotAvailable_;
};

}

#endif
