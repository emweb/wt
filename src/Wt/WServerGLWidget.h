// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WSERVERGLWIDGET
#define WSERVERGLWIDGET

#include <Wt/WAbstractGLImplementation.h>

namespace Wt {

class WMemoryResource;
class WRasterImage;

class WServerGLWidgetImpl;

class WServerGLWidget : public WAbstractGLImplementation {
public:
  WServerGLWidget(WGLWidget *glInterface);

  virtual ~WServerGLWidget();

  virtual void debugger()
    override;
  virtual void activeTexture(WGLWidget::GLenum texture)
    override;
  virtual void attachShader(WGLWidget::Program program,
			    WGLWidget::Shader shader)
    override;
  virtual void bindAttribLocation(WGLWidget::Program program, unsigned index,
				  const std::string &name)
    override;
  virtual void bindBuffer(WGLWidget::GLenum target, WGLWidget::Buffer buffer)
    override;
  virtual void bindFramebuffer(WGLWidget::GLenum target,
			       WGLWidget::Framebuffer buffer)
    override;
  virtual void bindRenderbuffer(WGLWidget::GLenum target,
				WGLWidget::Renderbuffer buffer)
    override;
  virtual void bindTexture(WGLWidget::GLenum target,
			   WGLWidget::Texture texture)
    override;
  virtual void blendColor(double red, double green, double blue, double alpha)
    override;
  virtual void blendEquation(WGLWidget::GLenum mode)
    override;
  virtual void blendEquationSeparate(WGLWidget::GLenum modeRGB,
				     WGLWidget::GLenum modeAlpha)
    override;
  virtual void blendFunc(WGLWidget::GLenum sfactor, WGLWidget::GLenum dfactor)
    override;
  virtual void blendFuncSeparate(WGLWidget::GLenum srcRGB,
				 WGLWidget::GLenum dstRGB,
				 WGLWidget::GLenum srcAlpha,
				 WGLWidget::GLenum dstAlpha)
    override;
  virtual void bufferData(WGLWidget::GLenum target, int size,
			  WGLWidget::GLenum usage)
    override;
  virtual void bufferData(WGLWidget::GLenum target, WGLWidget::ArrayBuffer res,
			  WGLWidget::GLenum usage)
    override;
  virtual void bufferData(WGLWidget::GLenum target, WGLWidget::ArrayBuffer res,
			  unsigned arrayBufferOffset, unsigned arrayBufferSize,
			  WGLWidget::GLenum usage)
    override;
  virtual void bufferSubData(WGLWidget::GLenum target, unsigned offset,
			     WGLWidget::ArrayBuffer res)
    override;
  virtual void bufferSubData(WGLWidget::GLenum target, unsigned offset,
			     WGLWidget::ArrayBuffer res,
			     unsigned arrayBufferOffset, unsigned size)
    override;
  virtual void bufferDatafv(WGLWidget::GLenum target,
			    const FloatBuffer &v, WGLWidget::GLenum usage,
			    bool binary)
    override;
#ifdef WT_TARGET_JAVA
  virtual void bufferDatafv(WGLWidget::GLenum target,
			    const FloatNotByteBuffer &buffer,
			    WGLWidget::GLenum usage)
    override;
#endif
  virtual void bufferSubDatafv(WGLWidget::GLenum target, unsigned offset,
			       const FloatBuffer &buffer, bool binary)
    override;
#ifdef WT_TARGET_JAVA
  virtual void bufferSubDatafv(WGLWidget::GLenum target, unsigned offset,
			       const FloatNotByteBuffer &buffer)
    override;
#endif

  virtual void bufferDataiv(WGLWidget::GLenum target, IntBuffer &buffer,
			    WGLWidget::GLenum usage, WGLWidget::GLenum type)
    override;
  virtual void bufferSubDataiv(WGLWidget::GLenum target,
			       unsigned offset, IntBuffer &buffer, 
			       WGLWidget::GLenum type)
    override;
  virtual void clearBinaryResources()
    override;
  virtual void clear(WFlags<WGLWidget::GLenum> mask)
    override;
  virtual void clearColor(double r, double g, double b, double a)
    override;
  virtual void clearDepth(double depth)
    override;
  virtual void clearStencil(int s)
    override;
  virtual void colorMask(bool red, bool green, bool blue, bool alpha)
    override;
  virtual void compileShader(WGLWidget::Shader shader)
    override;
  virtual void copyTexImage2D(WGLWidget::GLenum target, int level,
			      WGLWidget::GLenum internalFormat,
			      int x, int y,
			      unsigned width, unsigned height, 
			      int border)
    override;
  virtual void copyTexSubImage2D(WGLWidget::GLenum target, int level,
				 int xoffset, int yoffset,
				 int x, int y,
				 unsigned width, unsigned height)
    override;
  virtual WGLWidget::Buffer createBuffer()
    override;
  virtual WGLWidget::ArrayBuffer createAndLoadArrayBuffer
    (const std::string &url)
    override;
  virtual WGLWidget::Framebuffer createFramebuffer()
    override;
  virtual WGLWidget::Program createProgram()
    override;
  virtual WGLWidget::Renderbuffer createRenderbuffer()
    override;
  virtual WGLWidget::Shader createShader(WGLWidget::GLenum shader)
    override;
  virtual WGLWidget::Texture createTexture()
    override;
  virtual WGLWidget::Texture createTextureAndLoad(const std::string &url)
    override;
  virtual std::unique_ptr<WPaintDevice> createPaintDevice(const WLength& width,
							  const WLength& height)
    override;
  virtual void cullFace(WGLWidget::GLenum mode)
    override;
  virtual void deleteBuffer(WGLWidget::Buffer buffer)
    override;
  virtual void deleteFramebuffer(WGLWidget::Framebuffer buffer)
    override;
  virtual void deleteProgram(WGLWidget::Program program)
    override;
  virtual void deleteRenderbuffer(WGLWidget::Renderbuffer buffer)
    override;
  virtual void deleteShader(WGLWidget::Shader shader)
    override;
  virtual void deleteTexture(WGLWidget::Texture texture)
    override;
  virtual void depthFunc(WGLWidget::GLenum func)
    override;
  virtual void depthMask(bool flag)
    override;
  virtual void depthRange(double zNear, double zFar)
    override;
  virtual void detachShader(WGLWidget::Program program,
			    WGLWidget::Shader shader)
    override;
  virtual void disable(WGLWidget::GLenum cap)
    override;
  virtual void disableVertexAttribArray(WGLWidget::AttribLocation index)
    override;
  virtual void drawArrays(WGLWidget::GLenum mode, int first, unsigned count)
    override;
  virtual void drawElements(WGLWidget::GLenum mode, unsigned count,
			    WGLWidget::GLenum type, unsigned offset)
    override;
  virtual void enable(WGLWidget::GLenum cap)
    override;
  virtual void enableVertexAttribArray(WGLWidget::AttribLocation index)
    override;
  virtual void finish()
    override;
  virtual void flush()
    override;
  virtual void framebufferRenderbuffer(WGLWidget::GLenum target,
				       WGLWidget::GLenum attachment,
				       WGLWidget::GLenum renderbuffertarget,
				       WGLWidget::Renderbuffer renderbuffer)
    override;
  virtual void framebufferTexture2D(WGLWidget::GLenum target,
				    WGLWidget::GLenum attachment,
				    WGLWidget::GLenum textarget,
				    WGLWidget::Texture texture, int level)
    override;
  virtual void frontFace(WGLWidget::GLenum mode)
    override;
  virtual void generateMipmap(WGLWidget::GLenum target)
    override;
  virtual WGLWidget::AttribLocation getAttribLocation
   (WGLWidget::Program program, const std::string &attrib)
    override;
  virtual WGLWidget::UniformLocation getUniformLocation
    (WGLWidget::Program program, const std::string &location)
    override;
  virtual void hint(WGLWidget::GLenum target, WGLWidget::GLenum mode)
    override;
  virtual void lineWidth(double width)
    override;
  virtual void linkProgram(WGLWidget::Program program)
    override;
  virtual void pixelStorei(WGLWidget::GLenum pname, int param)
    override;
  virtual void polygonOffset(double factor, double units)
    override;
  virtual void renderbufferStorage(WGLWidget::GLenum target,
				   WGLWidget::GLenum internalformat,
				   unsigned width, unsigned height)
    override;
  virtual void sampleCoverage(double value, bool invert)
    override;
  virtual void scissor(int x, int y, unsigned width, unsigned height)
    override;
  virtual void shaderSource(WGLWidget::Shader shader, const std::string &src)
    override;
  virtual void stencilFunc(WGLWidget::GLenum func, int ref, unsigned mask)
    override;
  virtual void stencilFuncSeparate(WGLWidget::GLenum face,
				   WGLWidget::GLenum func, int ref,
				   unsigned mask)
    override;
  virtual void stencilMask(unsigned mask)
    override;
  virtual void stencilMaskSeparate(WGLWidget::GLenum face, unsigned mask)
    override;
  virtual void stencilOp(WGLWidget::GLenum fail, WGLWidget::GLenum zfail,
			 WGLWidget::GLenum zpass)
    override;
  virtual void stencilOpSeparate(WGLWidget::GLenum face, WGLWidget::GLenum fail,
				 WGLWidget::GLenum zfail,
				 WGLWidget::GLenum zpass)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  unsigned width, unsigned height, int border,
			  WGLWidget::GLenum format)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WImage *image)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WVideo *video)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  std::string image)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WPaintDevice *paintdevice)
    override;
  virtual void texImage2D(WGLWidget::GLenum target, int level,
			  WGLWidget::GLenum internalformat,
			  WGLWidget::GLenum format, WGLWidget::GLenum type,
			  WGLWidget::Texture texture)
    override;
  virtual void texParameteri(WGLWidget::GLenum target, WGLWidget::GLenum pname,
			     WGLWidget::GLenum param)
    override;
  virtual void uniform1f(const WGLWidget::UniformLocation &location, double x)
    override;
  virtual void uniform1fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value)
    override;
  virtual void uniform1fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v)
    override;
  virtual void uniform1i(const WGLWidget::UniformLocation &location,
			 int x)
    override;
  virtual void uniform1iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value)
    override;
  virtual void uniform2f(const WGLWidget::UniformLocation &location,
			 double x, double y)
    override;
  virtual void uniform2fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value)
    override;
  virtual void uniform2fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v)
    override;
  virtual void uniform2i(const WGLWidget::UniformLocation &location,
			 int x, int y)
    override;
  virtual void uniform2iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value)
    override;
  virtual void uniform3f(const WGLWidget::UniformLocation &location,
			 double x, double y, double z)
    override;
  virtual void uniform3fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value)
    override;
  virtual void uniform3fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v)
    override;
  virtual void uniform3i(const WGLWidget::UniformLocation &location,
			 int x, int y, int z)
    override;
  virtual void uniform3iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value)
    override;
  virtual void uniform4f(const WGLWidget::UniformLocation &location,
			 double x, double y, double z, double w)
    override;
  virtual void uniform4fv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY float *value)
    override;
  virtual void uniform4fv(const WGLWidget::UniformLocation &location,
			  const WGLWidget::JavaScriptVector &v)
    override;
  virtual void uniform4i(const WGLWidget::UniformLocation &location,
			 int x, int y, int z, int w)
    override;
  virtual void uniform4iv(const WGLWidget::UniformLocation &location,
			  const WT_ARRAY int *value)
    override;
  virtual void uniformMatrix2fv(const WGLWidget::UniformLocation &location,
				bool transpose, const WT_ARRAY double *value)
    override;
  virtual void uniformMatrix2(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 2, 2> &m)
    override;
  virtual void uniformMatrix3fv(const WGLWidget::UniformLocation &location,
				bool transpose,	const WT_ARRAY double *value)
    override;
  virtual void uniformMatrix3(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 3, 3> &m)
    override;
  virtual void uniformMatrix4fv(const WGLWidget::UniformLocation &location,
				bool transpose, const WT_ARRAY double *value)
    override;
  virtual void uniformMatrix4(const WGLWidget::UniformLocation &location,
			      const WGenericMatrix<double, 4, 4> &m)
    override;
  virtual void uniformMatrix4(const WGLWidget::UniformLocation &location,
			      const WGLWidget::JavaScriptMatrix4x4 &m)
    override;
  virtual void useProgram(WGLWidget::Program program)
    override;
  virtual void validateProgram(WGLWidget::Program program)
    override;
  virtual void vertexAttrib1f(WGLWidget::AttribLocation location, double x)
    override;
  virtual void vertexAttrib2f(WGLWidget::AttribLocation location,
			      double x, double y)
    override;
  virtual void vertexAttrib3f(WGLWidget::AttribLocation location,
			      double x, double y, double z)
    override;
  virtual void vertexAttrib4f(WGLWidget::AttribLocation location,
			      double x, double y, double z, double w)
    override;
  virtual void vertexAttribPointer(WGLWidget::AttribLocation location, int size,
				   WGLWidget::GLenum type, bool normalized,
				   unsigned stride, unsigned offset)
    override;
  virtual void viewport(int x, int y, unsigned width, unsigned height)
    override;
  virtual void initJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm)
    override;
  virtual void setJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm,
				    const WGenericMatrix<double, 4, 4> &m)
    override;
  virtual void initJavaScriptVector(WGLWidget::JavaScriptVector &jsv)
    override;
  virtual void setJavaScriptVector(WGLWidget::JavaScriptVector &jsv,
				   const std::vector<float> &v)
    override;
  virtual void setClientSideMouseHandler(const std::string& handlerCode)
    override;
  virtual void setClientSideLookAtHandler
    (const WGLWidget::JavaScriptMatrix4x4 &m, double ctrX, double ctrY,
     double ctrZ, double uX, double uY, double uZ, double pitchRate,
     double yawRate)
    override;
  virtual void setClientSideWalkHandler
    (const WGLWidget::JavaScriptMatrix4x4 &m, double frontStep, double rotStep)
    override;
  virtual JsArrayType arrayType() const
    override;
  virtual void injectJS(const std::string & jsString)
    override;
  virtual void restoreContext(const std::string &jsRef)
    override;
  virtual void render(const std::string& jsRef, WFlags<RenderFlag> flags)
    override;

  std::string glObjJsRef(const std::string& jsRef);

private:
  WServerGLWidgetImpl *impl_;
  WRasterImage *raster_;
  WMemoryResource *memres_;

  std::stringstream js_;
};

}

#endif
