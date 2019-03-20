#include "Wt/WApplication.h"
#include "Wt/WServerGLWidget.h"

#include "Wt/WClientGLWidget.h"
#include "Wt/WMemoryResource.h"
#include "Wt/WPainter.h"
#include "Wt/WRasterImage.h"
#include "Wt/WWebWidget.h"
#include "Wt/Http/Response.h"

#include <fstream>

#ifdef WT_WIN32
#define WIN32_GL
#define FRAMEBUFFER_RENDERING
#elif defined(__APPLE__)
#define APPLE_GL
#define FRAMEBUFFER_RENDERING
#else
#define X11_GL
#endif

#include <GL/glew.h>
#include <GL/gl.h>

#ifdef X11_GL
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

#ifdef WIN32_GL
#include <GL/wglew.h>
#include <windows.h>
#endif

#ifdef APPLE_GL
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#endif

namespace {
  GLenum serverGLenum(Wt::WGLWidget::GLenum e);
}

namespace Wt {

  namespace {
    class WGLImageResource final : public WMemoryResource {
    public:
      WGLImageResource() : WMemoryResource()
      { }

      virtual void handleRequest(const Http::Request &request,
				 Http::Response &response) override
      {
	response.addHeader("Cache-Control", "max-age=60");
	WMemoryResource::handleRequest(request, response);
      }
    };
  }

class WServerGLWidgetImpl {
public:
  WServerGLWidgetImpl(bool antialiasingEnabled);
  ~WServerGLWidgetImpl();

  void makeCurrent();
  void unmakeCurrent();
  void resize(int width, int height);
#ifdef FRAMEBUFFER_RENDERING
  void initReadBuffer();
  void setDrawBuffer();
  void initializeRenderbuffers();
  GLuint framebuffer() const;
  GLuint renderbuffer() const;
  GLuint depthbuffer() const;
#endif
private:
#ifdef WIN32_GL
  HWND wnd_;
  HDC hdc_;
  HGLRC ctx_;
#endif
#ifdef X11_GL
  Display *display_;
  GLXContext ctx_;
  //  Window win_;
  GLXPbuffer pbuffer_;
  GLXFBConfig bestFbc_;
  Colormap cmap_;
  XVisualInfo *vi_;
  XSetWindowAttributes swa_;
#endif
#ifdef APPLE_GL
  CGLContextObj context_;
#endif
#ifdef FRAMEBUFFER_RENDERING
  int width_, height_;
  GLuint framebuffer_, renderbuffer_, depthbuffer_;
  GLuint framebufferRead_, renderbufferRead_;
#endif
};

#ifdef X11_GL
WServerGLWidgetImpl::WServerGLWidgetImpl(bool antialiasingEnabled):
  display_(nullptr)
{
  display_ = XOpenDisplay(nullptr);

  if ( !display_ )
  {
    throw WException("WServerGLWidget.C: failed to open X display.\n");
  }

  int sampleBuffers;
  int samples;
  if (antialiasingEnabled) {
    sampleBuffers = 1;
    samples = 4;
  } else {
    sampleBuffers = 0;
    samples = 0;
  }

  // Get a matching FB config
  int visual_attribs[] =
    {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_DIRECT_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      GLX_SAMPLE_BUFFERS  , sampleBuffers,
      GLX_SAMPLES         , samples,
      None
    };

  int fbcount;
  GLXFBConfig *fbc = glXChooseFBConfig( display_, DefaultScreen( display_ ),
                                        visual_attribs, &fbcount );
  if ( !fbc )
  {
    XCloseDisplay( display_ );
    throw WException("WServerGLWidget: Failed to retrieve framebuffer configuration.\n");
  }

  bestFbc_ = fbc[0];

  // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
  XFree( fbc );

  // NOTE: It is not necessary to create or make current to a context before
  // calling glXGetProcAddressARB
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

  int context_attribs[] =
    {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 0,
      None
    };

  ctx_ = glXCreateContextAttribsARB( display_, bestFbc_, nullptr,
				    True, context_attribs );

  // Sync to ensure any errors generated are processed.
  XSync( display_, False );
  if ( ! ctx_ ) {
    XCloseDisplay( display_ );
    throw WException("WServerGLWidget: Failed to create an OpenGL context.\n");
  }

  int pbufferAttribs[] = {
    GLX_PBUFFER_WIDTH,  600,
    GLX_PBUFFER_HEIGHT, 600,
    None
  };
  pbuffer_ = glXCreatePbuffer( display_, bestFbc_, pbufferAttribs );

  // Sync to ensure any errors generated are processed.
  XSync( display_, False );

  if ( !pbuffer_ ) {
    glXDestroyContext( display_, ctx_ );
    XCloseDisplay( display_ );
    throw WException("WServerGLWidget: Failed to create an OpenGL pBuffer.\n" );
  }

  glXMakeCurrent( display_, pbuffer_, ctx_ );
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    glXDestroyContext( display_, ctx_ );
    XCloseDisplay( display_ );
    glXMakeCurrent( display_, 0, nullptr );
    throw WException("WServerGLWidget: problem with GLEW initialization.\n");
  }
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SPRITE);
  glXMakeCurrent( display_, 0, nullptr );
}

WServerGLWidgetImpl::~WServerGLWidgetImpl()
{
  glXDestroyContext( display_, ctx_ );
  XCloseDisplay( display_ );
}

void WServerGLWidgetImpl::makeCurrent()
{
  glXMakeCurrent(display_, pbuffer_, ctx_);
}

void WServerGLWidgetImpl::unmakeCurrent()
{
  glXMakeCurrent(display_, 0, nullptr);
}

void WServerGLWidgetImpl::resize(int width, int height)
{
  glXDestroyPbuffer(display_, pbuffer_);
  int pbufferAttribs[] = {
    GLX_PBUFFER_WIDTH,  width,
    GLX_PBUFFER_HEIGHT, height,
    None
  };
  pbuffer_ = glXCreatePbuffer(display_, bestFbc_, pbufferAttribs);
  // resize X-window
  // XResizeWindow(display_, win_, (uint)renderWidth_, (uint)renderHeight_);
}
#endif

#ifdef APPLE_GL
WServerGLWidgetImpl::WServerGLWidgetImpl(bool antialiasingEnabled):
  width_(0),
  height_(0),
  framebuffer_(-1)
{
  CGLPixelFormatAttribute attributes[4] = {
    kCGLPFAAccelerated,   // no software rendering
    kCGLPFAOpenGLProfile, // core profile with the version stated below
    (CGLPixelFormatAttribute) kCGLOGLPVersion_Legacy,
    //    (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
    (CGLPixelFormatAttribute) 0
  };
  CGLPixelFormatObj pix;
  CGLError errorCode;
  GLint num; // stores the number of possible pixel formats
  CGLError err;

  err = CGLChoosePixelFormat(attributes, &pix, &num);
  if (err != kCGLNoError) {
    throw WException("WServerGLWidget cannot select proper pixel format: "
		     + std::to_string(err));
  }

  err = CGLCreateContext(pix, NULL, &context_);
  CGLDestroyPixelFormat(pix);
  if (err != kCGLNoError) {
    throw WException("WServerGLWidget cannot create context"
		     + std::to_string(err));
  }

  makeCurrent();

  // Experimental or glew doesn't work with core profile >= 3.2
  glewExperimental = 1;
  GLenum res = glewInit();
  if (res == GLEW_OK && GLEW_ARB_framebuffer_object) {
    initializeRenderbuffers();
  } else {
    unmakeCurrent();
    CGLSetCurrentContext(NULL);
    CGLDestroyContext(context_);
    throw WException("WServerGLWidget cannot create offscreen framebuffers "
		     + std::to_string(err));
  }
}

WServerGLWidgetImpl::~WServerGLWidgetImpl()
{
  CGLSetCurrentContext(NULL);
  CGLDestroyContext(context_);
}

void WServerGLWidgetImpl::makeCurrent()
{
  CGLError err = CGLSetCurrentContext(context_);
  if (err != kCGLNoError) {
    throw WException("WServerGLWidget cannot make context active"
		     + std::to_string(err));
  }
}

void WServerGLWidgetImpl::unmakeCurrent()
{
  CGLSetCurrentContext(NULL);
}
#endif

#ifdef WIN32_GL
WServerGLWidgetImpl::WServerGLWidgetImpl(bool antialiasingEnabled)
  : framebuffer_(-1)
{
  // Window properties
  WNDCLASSEX wndClass;
  wndClass.cbSize         = sizeof(WNDCLASSEX);
  wndClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wndClass.lpfnWndProc    = DefWindowProc;
  wndClass.cbClsExtra     = 0;
  wndClass.cbWndExtra     = 0;
  wndClass.hInstance      = 0;
  wndClass.hIcon          = 0;
  wndClass.hCursor        = LoadCursor(0, IDC_ARROW);
  wndClass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndClass.lpszMenuName   = 0;
  wndClass.lpszClassName  = "WndClass";
  wndClass.hIconSm        = 0;
  RegisterClassEx(&wndClass);

  // Window decorations are substracted from the size of the drawable area,
  // which is not what we want, so disable them completely.
  DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
  // Create the window. Position and size it.
  wnd_ = CreateWindowEx(0,
    "WndClass",
    "",
    style,
    CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
    0, 0, 0, 0);
  // comment out line below to show server side render window
  //ShowWindow(wnd_, SW_SHOW);
  hdc_ = GetDC(wnd_);

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion   = 1;
  pfd.dwFlags    = /*PFD_DOUBLEBUFFER |*/ PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;
  pfd.iPixelType = PFD_TYPE_RGBA;

  int pixelFormat = ChoosePixelFormat(hdc_, &pfd);
  if (pixelFormat == 0)
    throw WException("WServerGLWidget: Failed to find suitable pixel format.\n" );

  BOOL bResult = SetPixelFormat (hdc_, pixelFormat, &pfd);

  if (!bResult)
    throw WException("WServerGLWidget: Failed to set suitable pixel format.\n" );

  HGLRC tempContext = wglCreateContext(hdc_);
  wglMakeCurrent(hdc_, tempContext);

  GLenum err = glewInit();

  if (GLEW_OK != err)
    throw WException("WServerGLWidget: GLEW failed to initialize.\n" );

  int attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
    WGL_CONTEXT_FLAGS_ARB, 0,
    0
  };

  int sampleBuffersARB = antialiasingEnabled ? GL_TRUE : GL_FALSE;

  // check for anti-aliasing
  if (wglewIsSupported("WGL_ARB_multisample") == 1) {
    float fAttributes[] = {0,0};
    int iAttributes[] = {
      WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
      WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB,24,
      WGL_ALPHA_BITS_ARB,8,
      WGL_DEPTH_BITS_ARB,16,
      WGL_STENCIL_BITS_ARB,0,
      //WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
      WGL_SAMPLE_BUFFERS_ARB,sampleBuffersARB,
      WGL_SAMPLES_ARB, 2,
      0,0};
    int newPixelFormat;
    UINT numFormats;
    bool valid = wglChoosePixelFormatARB(hdc_, iAttributes, fAttributes, 1, &newPixelFormat, &numFormats);
    if (valid && numFormats > 1) {
      pixelFormat = newPixelFormat;

      // D'oh! we have to re-create the window (cannot change pixel format)
      // simply redo all the work
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(tempContext);
      ReleaseDC(wnd_, hdc_);
      DestroyWindow(wnd_);
      wnd_ = CreateWindowEx(0,
	"WndClass",
	"",
	style,
	CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
	0, 0, 0, 0);
      // comment out line below to show server side render window
      //ShowWindow(wnd_, SW_SHOW);
      hdc_ = GetDC(wnd_);
      BOOL bResult = SetPixelFormat (hdc_, pixelFormat, &pfd);
      if (!bResult)
	throw WException("WServerGLWidget: Failed to set multisample pixel format.\n" );
      wglCreateContext(hdc_);
      wglMakeCurrent(hdc_, tempContext);
    }
  }

  if(wglewIsSupported("WGL_ARB_create_context") == 1) {
    ctx_ = wglCreateContextAttribsARB(hdc_, 0, attribs);
    wglMakeCurrent(NULL,NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(hdc_, ctx_);
  } else {
    throw WException("WServerGLWidget: Failed to create a 3.x context.\n" );
  }

  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glGenRenderbuffers(1, &renderbuffer_);
  glGenRenderbuffers(1, &depthbuffer_);
  glGenFramebuffers(1, &framebufferRead_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferRead_);
  glGenRenderbuffers(1, &renderbufferRead_);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  unmakeCurrent();
  resize(100, 100); // whatever
}

WServerGLWidgetImpl::~WServerGLWidgetImpl()
{
  wglMakeCurrent(NULL, NULL);
  if(ctx_) {
    wglDeleteContext(ctx_);
    ctx_ = 0;
  }
  ReleaseDC(wnd_, hdc_);
  DestroyWindow(wnd_);
}

void WServerGLWidgetImpl::makeCurrent()
{
  if (!wglMakeCurrent(hdc_, ctx_))
    throw WException("WServerGLWidget: makeCurrent() failed");
}

void WServerGLWidgetImpl::unmakeCurrent()
{
  if (!wglMakeCurrent(NULL,NULL))
    throw WException("WServerGLWidget: unmakeCurrent() failed");

}
#endif

#ifdef FRAMEBUFFER_RENDERING
void WServerGLWidgetImpl::initializeRenderbuffers()
{
  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glGenRenderbuffers(1, &renderbuffer_);
  glGenRenderbuffers(1, &depthbuffer_);
  glGenFramebuffers(1, &framebufferRead_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferRead_);
  glGenRenderbuffers(1, &renderbufferRead_);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  unmakeCurrent();
  resize(100, 100); // whatever
}

GLuint WServerGLWidgetImpl::framebuffer() const
{
  return framebuffer_;
}

GLuint WServerGLWidgetImpl::renderbuffer() const
{
  return renderbuffer_;
}

GLuint WServerGLWidgetImpl::depthbuffer() const
{
  return depthbuffer_;
}

void WServerGLWidgetImpl::resize(int width, int height)
{
  if (width == width_ && height == height_)
    return;
  makeCurrent();
  int status;
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			    GL_RENDERBUFFER, renderbuffer_);

  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			    GL_RENDERBUFFER, depthbuffer_);
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  glBindFramebuffer(GL_FRAMEBUFFER, framebufferRead_);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbufferRead_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			    GL_RENDERBUFFER, renderbufferRead_);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  unmakeCurrent();
  if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    throw WException("WServerGLWidget: resize failed\n");
  width_ = width;
  height_ = height;
}

void WServerGLWidgetImpl::initReadBuffer()
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferRead_);
  glBlitFramebuffer(0,0,width_,height_,0,0,width_,height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBindFramebufferEXT(GL_FRAMEBUFFER, framebufferRead_);
}

void WServerGLWidgetImpl::setDrawBuffer()
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}
#endif

WServerGLWidget::WServerGLWidget(WGLWidget *glInterface)
  : WAbstractGLImplementation(glInterface),
    raster_(0),
    memres_(new WGLImageResource())
{
  try {
    impl_ = new WServerGLWidgetImpl
      (glInterface_->renderOptions_.test(GLRenderOption::AntiAliasing));
  } catch (WException &e) {
    delete memres_;
    throw e;
  }
}

WServerGLWidget::~WServerGLWidget()
{
  delete raster_;
  delete memres_;
  delete impl_;
}

void WServerGLWidget::debugger()
{
  // only functional in WClientGLWidget
}

void WServerGLWidget::activeTexture(WGLWidget::GLenum texture)
{
  glActiveTexture(serverGLenum(texture));
  SERVERGLDEBUG;
}

void WServerGLWidget::attachShader(WGLWidget::Program program, WGLWidget::Shader shader)
{
  glAttachShader(program.getId(), shader.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::bindAttribLocation(WGLWidget::Program program,
					 unsigned index,
					 const std::string &name)
{
  glBindAttribLocation(program.getId(), index, name.c_str());
  SERVERGLDEBUG;
}

void WServerGLWidget::bindBuffer(WGLWidget::GLenum target, WGLWidget::Buffer buffer)
{
  glBindBuffer(serverGLenum(target), buffer.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::bindFramebuffer(WGLWidget::GLenum target, WGLWidget::Framebuffer buffer)
{
  if (buffer.isNull()) {
#ifdef FRAMEBUFFER_RENDERING
    glBindFramebuffer(serverGLenum(target), impl_->framebuffer());
#else
    glBindFramebuffer(serverGLenum(target), 0);
#endif
  } else {
    glBindFramebuffer(serverGLenum(target), buffer.getId());
  }
  SERVERGLDEBUG;
}

void WServerGLWidget::bindRenderbuffer(WGLWidget::GLenum target, WGLWidget::Renderbuffer buffer)
{
  if (buffer.isNull()) {
#ifdef FRAMEBUFFER_RENDERING
    glBindRenderbuffer(serverGLenum(target), impl_->renderbuffer());
#else
    glBindRenderbuffer(serverGLenum(target), 0);
#endif
  } else {
    glBindRenderbuffer(serverGLenum(target), buffer.getId());
  }
  SERVERGLDEBUG;
}

void WServerGLWidget::bindTexture(WGLWidget::GLenum target, WGLWidget::Texture texture)
{
  glBindTexture(serverGLenum(target), texture.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::blendColor(double red, double green, double blue, double alpha)
{
  glBlendColor((GLfloat) red, (GLfloat) green, (GLfloat) blue, (GLfloat) alpha);
  SERVERGLDEBUG;
}

void WServerGLWidget::blendEquation(WGLWidget::GLenum mode)
{
  glBlendEquation(serverGLenum(mode));
  SERVERGLDEBUG;
}

void WServerGLWidget::blendEquationSeparate(WGLWidget::GLenum modeRGB,
					    WGLWidget::GLenum modeAlpha)
{
  glBlendEquationSeparate(serverGLenum(modeRGB), serverGLenum(modeAlpha));
  SERVERGLDEBUG;
}

void WServerGLWidget::blendFunc(WGLWidget::GLenum sfactor,
				WGLWidget::GLenum dfactor)
{
  glBlendFunc(serverGLenum(sfactor), serverGLenum(dfactor));
  SERVERGLDEBUG;
}

void WServerGLWidget::blendFuncSeparate(WGLWidget::GLenum srcRGB,
					WGLWidget::GLenum dstRGB,
					WGLWidget::GLenum srcAlpha,
					WGLWidget::GLenum dstAlpha)
{
  glBlendFuncSeparate(serverGLenum(srcRGB), serverGLenum(dstRGB),
		      serverGLenum(srcAlpha), serverGLenum(dstAlpha));
  SERVERGLDEBUG;
}

void WServerGLWidget::bufferData(WGLWidget::GLenum target,
				 WGLWidget::ArrayBuffer res,
				 unsigned bufferResourceOffset,
				 unsigned bufferResourceSize,
				 WGLWidget::GLenum usage)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::bufferData(WGLWidget::GLenum target,
				 WGLWidget::ArrayBuffer res,
				 WGLWidget::GLenum usage)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::bufferSubData(Wt::WGLWidget::GLenum target,
				    unsigned offset,
				    WGLWidget::ArrayBuffer res)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::bufferSubData(Wt::WGLWidget::GLenum target,
				    unsigned offset,
				    WGLWidget::ArrayBuffer res,
				    unsigned bufferResourceOffset,
				    unsigned bufferResourceSize)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::bufferData(WGLWidget::GLenum target, int size,
				 WGLWidget::GLenum usage)
{
  glBufferData(serverGLenum(target), (GLsizeiptr)size, 0, serverGLenum(usage));
  SERVERGLDEBUG;
}

void WServerGLWidget::bufferDatafv(WGLWidget::GLenum target, const FloatBuffer &buffer, WGLWidget::GLenum usage, bool binary)
{
  glBufferData(serverGLenum(target), buffer.size()*4, &buffer[0],
	       serverGLenum(usage));
  SERVERGLDEBUG;
}

void WServerGLWidget::bufferDataiv(WGLWidget::GLenum target,
				   IntBuffer &buffer, WGLWidget::GLenum usage,
				   WGLWidget::GLenum type)
{
  std::vector<short> shortbuffer;
  for (unsigned i = 0; i < buffer.size(); i++) {
    shortbuffer.push_back(buffer[i]);
  }

  glBufferData(serverGLenum(target), shortbuffer.size()*2, &shortbuffer[0],
	       serverGLenum(usage));
  SERVERGLDEBUG;
}

void WServerGLWidget::bufferSubDatafv(WGLWidget::GLenum target, unsigned offset,
				      const FloatBuffer &buffer, bool binary)
{
  glBufferSubData(serverGLenum(target), (GLintptr)offset, buffer.size()*4,
		  &buffer[0]);
}

void WServerGLWidget::bufferSubDataiv(WGLWidget::GLenum target,
				      unsigned offset, IntBuffer &buffer,
				      WGLWidget::GLenum type)
{
  std::vector<short> shortbuffer;
  for (unsigned i = 0; i < buffer.size(); i++) {
    shortbuffer.push_back(buffer[i]);
  }

  glBufferSubData(serverGLenum(target), offset, buffer.size()*2,
		  &shortbuffer[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::clear(WFlags<WGLWidget::GLenum> mask)
{
  GLbitfield glmask = 0;
  if (mask.test(WGLWidget::COLOR_BUFFER_BIT))
    glmask |= GL_COLOR_BUFFER_BIT;
  if (mask.test(WGLWidget::DEPTH_BUFFER_BIT))
    glmask |= GL_DEPTH_BUFFER_BIT;
  if (mask.test(WGLWidget::STENCIL_BUFFER_BIT))
    glmask |= GL_STENCIL_BUFFER_BIT;

  glClear(glmask);
  SERVERGLDEBUG;
}

void WServerGLWidget::clearColor(double r, double g, double b, double a)
{
  glClearColor((float)r, (float)g, (float)b, (float)a);
  SERVERGLDEBUG;
}

void WServerGLWidget::clearDepth(double depth)
{
  glClearDepth(depth);
  SERVERGLDEBUG;
}

void WServerGLWidget::clearStencil(int s)
{
  glClearStencil(s);
  SERVERGLDEBUG;
}

void WServerGLWidget::colorMask(bool red, bool green, bool blue, bool alpha)
{
  glColorMask(red, green, blue, alpha);
  SERVERGLDEBUG;
}

void WServerGLWidget::compileShader(WGLWidget::Shader shader)
{
  glCompileShader(shader.getId());
  SERVERGLDEBUG;
  GLint params = 0;
  glGetShaderiv(shader.getId(), GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE) {
    std::cerr << "Shader " << shader.getId() << " compilation failed" << std::endl;
    char buffer[32*1024];
    glGetShaderInfoLog(shader.getId(), sizeof(buffer), nullptr, buffer);
    std::cerr << buffer << std::endl;
  }
  SERVERGLDEBUG;
}

void WServerGLWidget::copyTexImage2D(WGLWidget::GLenum target, int level,
				     WGLWidget::GLenum internalFormat,
				     int x, int y,
				     unsigned width, unsigned height,
				     int border)
{
  glCopyTexImage2D(serverGLenum(target), level, serverGLenum(internalFormat),
		   x, y, width, height, border);
  SERVERGLDEBUG;
}

void WServerGLWidget::copyTexSubImage2D(WGLWidget::GLenum target, int level,
					int xoffset, int yoffset,
					int x, int y,
					unsigned width, unsigned height)
{
  glCopyTexSubImage2D(serverGLenum(target), level, xoffset, yoffset, x, y,
		      width, height);
  SERVERGLDEBUG;
}

WGLWidget::Buffer WServerGLWidget::createBuffer()
{
  GLuint id[1];
  glGenBuffers(1, id);
  SERVERGLDEBUG;
  return WGLWidget::Buffer((int)id[0]);
}

WGLWidget::ArrayBuffer WServerGLWidget::createAndLoadArrayBuffer(const std::string &url)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

WGLWidget::Framebuffer WServerGLWidget::createFramebuffer()
{
  GLuint id[1];
  glGenFramebuffers(1, id);
  SERVERGLDEBUG;
  return WGLWidget::Framebuffer((int)id[0]);
}

WGLWidget::Program WServerGLWidget::createProgram()
{
  GLuint id = glCreateProgram();
  SERVERGLDEBUG;
  return WGLWidget::Program((int)id);
}

WGLWidget::Renderbuffer WServerGLWidget::createRenderbuffer()
{
  GLuint id[1];
  glGenRenderbuffers(1, id);
  SERVERGLDEBUG;
  return WGLWidget::Renderbuffer((int)id[0]);
}

WGLWidget::Shader WServerGLWidget::createShader(WGLWidget::GLenum shader)
{
  GLuint id = glCreateShader(serverGLenum(shader));
  SERVERGLDEBUG;
  return WGLWidget::Shader((int)id);
}

WGLWidget::Texture WServerGLWidget::createTexture()
{
  GLuint id[1];
  glGenTextures(1, id);
  SERVERGLDEBUG;
  return WGLWidget::Texture((int)id[0]);
}

WGLWidget::Texture WServerGLWidget::createTextureAndLoad(const std::string &url)
{
  GLuint id[1];
  glGenTextures(1, id);
  SERVERGLDEBUG;
  WGLWidget::Texture tex((int)id[0]);
  tex.setUrl(url);
  return tex;
}

std::unique_ptr<WPaintDevice> WServerGLWidget
::createPaintDevice(const WLength& width, const WLength& height)
{
  return std::unique_ptr<WRasterImage>(new WRasterImage("png", width, height));
}

void WServerGLWidget::cullFace(WGLWidget::GLenum mode)
{
  glCullFace(serverGLenum(mode));
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteBuffer(WGLWidget::Buffer buffer)
{
  unsigned int bufArray[1];
  bufArray[0] = buffer.getId();
  glDeleteBuffers(1, bufArray);
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteFramebuffer(WGLWidget::Framebuffer buffer)
{
  unsigned int bufArray[1];
  bufArray[0] = buffer.getId();
  glDeleteFramebuffers(1, bufArray);
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteProgram(WGLWidget::Program program)
{
  glDeleteProgram(program.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteRenderbuffer(WGLWidget::Renderbuffer buffer)
{
  unsigned int bufArray[1];
  bufArray[0] = buffer.getId();
  glDeleteRenderbuffers(1, bufArray);
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteShader(WGLWidget::Shader shader)
{
  glDeleteShader(shader.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::deleteTexture(WGLWidget::Texture texture)
{
  unsigned int texArray[1];
  texArray[0] = texture.getId();
  glDeleteTextures(1, texArray);
  SERVERGLDEBUG;
}

void WServerGLWidget::depthFunc(WGLWidget::GLenum func)
{
  glDepthFunc(serverGLenum(func));
  SERVERGLDEBUG;
}

void WServerGLWidget::depthMask(bool flag)
{
  glDepthMask(flag);
  SERVERGLDEBUG;
}

void WServerGLWidget::depthRange(double zNear, double zFar)
{
  glDepthRange(zNear, zFar);
  SERVERGLDEBUG;
}

void WServerGLWidget::detachShader(WGLWidget::Program program, WGLWidget::Shader shader)
{
  glDetachShader(program.getId(), shader.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::disable(WGLWidget::GLenum cap)
{
  glDisable(serverGLenum(cap));
  SERVERGLDEBUG;
}

void WServerGLWidget::disableVertexAttribArray(WGLWidget::AttribLocation index)
{
  glDisableVertexAttribArray(index.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::drawArrays(WGLWidget::GLenum mode, int first, unsigned count)
{
  glDrawArrays(serverGLenum(mode), first, count);
  SERVERGLDEBUG;
}

void WServerGLWidget::drawElements(WGLWidget::GLenum mode, unsigned count,
				   WGLWidget::GLenum type, unsigned offset)
{
  glDrawElements(serverGLenum(mode), count, serverGLenum(type), nullptr);
  SERVERGLDEBUG;
}

void WServerGLWidget::enable(WGLWidget::GLenum cap)
{
  glEnable(serverGLenum(cap));
  SERVERGLDEBUG;
}

void WServerGLWidget::enableVertexAttribArray(WGLWidget::AttribLocation index)
{
  glEnableVertexAttribArray(index.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::finish()
{
  glFinish();
  SERVERGLDEBUG;
}
void WServerGLWidget::flush()
{
  glFlush();
  SERVERGLDEBUG;
}

void WServerGLWidget::framebufferRenderbuffer(WGLWidget::GLenum target,
					      WGLWidget::GLenum attachment,
					      WGLWidget::GLenum renderbuffertarget,
					      WGLWidget::Renderbuffer renderbuffer)
{
  glFramebufferRenderbuffer(serverGLenum(target), serverGLenum(attachment),
			    serverGLenum(renderbuffertarget),
			    renderbuffer.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::framebufferTexture2D(WGLWidget::GLenum target,
					   WGLWidget::GLenum attachment,
					   WGLWidget::GLenum textarget,
					   WGLWidget::Texture texture,
					   int level)
{
  glFramebufferTexture2D(serverGLenum(target), serverGLenum(attachment),
			 serverGLenum(textarget), texture.getId(), level);
  SERVERGLDEBUG;
}

void WServerGLWidget::frontFace(WGLWidget::GLenum mode)
{
  glFrontFace(serverGLenum(mode));
  SERVERGLDEBUG;
}

void WServerGLWidget::generateMipmap(WGLWidget::GLenum target)
{
  glGenerateMipmap(serverGLenum(target));
  SERVERGLDEBUG;
}

WGLWidget::AttribLocation WServerGLWidget::getAttribLocation(WGLWidget::Program program, const std::string &attrib)
{
  GLint id = glGetAttribLocation(program.getId(), attrib.c_str());
  SERVERGLDEBUG;
  return WGLWidget::AttribLocation((int)id);
}

WGLWidget::UniformLocation WServerGLWidget::getUniformLocation(WGLWidget::Program program, const std::string &location)
{
  GLint id = glGetUniformLocation(program.getId(), location.c_str());
  SERVERGLDEBUG;
  return WGLWidget::UniformLocation((int)id);
}

void WServerGLWidget::hint(WGLWidget::GLenum target, WGLWidget::GLenum mode)
{
  glHint(serverGLenum(target), serverGLenum(mode));
  SERVERGLDEBUG;
}

void WServerGLWidget::lineWidth(double width)
{
  glLineWidth((GLfloat)width);
  SERVERGLDEBUG;
}

void WServerGLWidget::linkProgram(WGLWidget::Program program)
{
  glLinkProgram(program.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::pixelStorei(WGLWidget::GLenum pname, int param)
{
  if (pname == WGLWidget::UNPACK_FLIP_Y_WEBGL ||
      pname == WGLWidget::UNPACK_PREMULTIPLY_ALPHA_WEBGL ||
      pname == WGLWidget::UNPACK_COLORSPACE_CONVERSION_WEBGL)
    return;

  glPixelStorei(serverGLenum(pname), param);
  SERVERGLDEBUG;
}

void WServerGLWidget::polygonOffset(double factor, double units)
{
  glPolygonOffset((GLfloat)factor, (GLfloat)units);
  SERVERGLDEBUG;
}

void WServerGLWidget::renderbufferStorage(WGLWidget::GLenum target, WGLWidget::GLenum internalformat,
  unsigned width, unsigned height)
{
  glRenderbufferStorage(serverGLenum(target), serverGLenum(internalformat),
			width, height);
  SERVERGLDEBUG;
}

void WServerGLWidget::sampleCoverage(double value, bool invert)
{
  glSampleCoverage((GLfloat) value, invert);
  SERVERGLDEBUG;
}

void WServerGLWidget::scissor(int x, int y, unsigned width, unsigned height)
{
  glScissor(x, y, width, height);
  SERVERGLDEBUG;
}

void WServerGLWidget::shaderSource(WGLWidget::Shader shader,
				   const std::string &src)
{
  const char* csrc = src.c_str();
  const GLchar** string = &csrc;

  int psize = src.length();
  const GLint* length = &psize;

  glShaderSource(shader.getId(), 1, string, length);
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilFunc(WGLWidget::GLenum func, int ref,
				  unsigned mask)
{
  glStencilFunc(serverGLenum(func), ref, mask);
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilFuncSeparate(WGLWidget::GLenum face,
					  WGLWidget::GLenum func, int ref,
					  unsigned mask)
{
  glStencilFuncSeparate(serverGLenum(face), serverGLenum(func), ref, mask);
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilMask(unsigned mask)
{
  glStencilMask(mask);
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilMaskSeparate(WGLWidget::GLenum face, unsigned mask)
{
  glStencilMaskSeparate(serverGLenum(face), mask);
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilOp(WGLWidget::GLenum fail, WGLWidget::GLenum zfail,
				WGLWidget::GLenum zpass)
{
  glStencilOp(serverGLenum(fail), serverGLenum(zfail), serverGLenum(zpass));
  SERVERGLDEBUG;
}

void WServerGLWidget::stencilOpSeparate(WGLWidget::GLenum face,
					WGLWidget::GLenum fail,
					WGLWidget::GLenum zfail,
					WGLWidget::GLenum zpass)
{
  glStencilOpSeparate(serverGLenum(face),
		      serverGLenum(fail),
		      serverGLenum(zfail),
		      serverGLenum(zpass));
  SERVERGLDEBUG;
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target,
				 int level, WGLWidget::GLenum internalformat,
				 unsigned width, unsigned height, int border,
				 WGLWidget::GLenum format)
{
  glTexImage2D(serverGLenum(target), level, serverGLenum(internalformat),
	       width, height, border, serverGLenum(format), GL_UNSIGNED_BYTE,
	       nullptr);
  SERVERGLDEBUG;
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WImage *image)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WVideo *video)
{
  throw WException("WServerGLWidget: this operation is not supported in server-side rendering");
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 std::string imgFilename)
{
  WPainter::Image image(WApplication::instance()->docRoot().append("/")
			.append(imgFilename), imgFilename);
  WRasterImage raster("png", image.width(), image.height());
  WPainter decoder(&raster);
  decoder.drawImage(WPointF(), image);

  int width = (int)raster.width().value();
  int height = (int)raster.height().value();

  std::vector<unsigned char> imgdata(width*height*4); // 4 bytes per pixel
  raster.getPixels(&imgdata[0]);

  // flip image over Y axis
  uint32_t *word = (uint32_t *)&imgdata[0];
  for (int r = 0; r < height/2; r++) {
    for (int c = 0; c < width; c++) {
      uint32_t tmp = word[r*width + c];
      word[r*width + c] = word[(height - r - 1)*width + c];
      word[(height - r - 1)*width + c] = tmp;
    }
  }

  glTexImage2D(serverGLenum(target), level, GL_RGBA, width, height, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, &imgdata[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WPaintDevice *paintdevice)
{
  WRasterImage *rpd = dynamic_cast<WRasterImage*>(paintdevice);
  if (!rpd)
    return;

  int width = (int)rpd->width().value();
  int height = (int)rpd->height().value();

  std::vector<unsigned char> imgdata(width*height*4);
  rpd->getPixels(&imgdata[0]);

  // flip image over Y axis
  uint32_t *word = (uint32_t *)&imgdata[0];
  for (int r = 0; r < height/2; r++) {
    for (int c = 0; c < width; c++) {
      uint32_t tmp = word[r*width + c];
      word[r*width + c] = word[(height - r - 1)*width + c];
      word[(height - r - 1)*width + c] = tmp;
    }
  }

  glTexImage2D(serverGLenum(target), level, GL_RGBA, width, height, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, &imgdata[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WGLWidget::Texture texture)
{
  texImage2D(target, level, internalformat, format, type, texture.url());
}

void WServerGLWidget::texParameteri(WGLWidget::GLenum target,
				    WGLWidget::GLenum pname,
				    WGLWidget::GLenum param)
{
  glTexParameteri(serverGLenum(target), serverGLenum(pname), serverGLenum(param));
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform1f(const WGLWidget::UniformLocation &location, double x)
{
  glUniform1f(location.getId(), (GLfloat)x);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform1fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  glUniform1fv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform1fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  glUniform1fv(location.getId(), 1, &v.value()[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform1i(const WGLWidget::UniformLocation &location,
				int x)
{
  glUniform1i(location.getId(), x);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform1iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  glUniform1iv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform2f(const WGLWidget::UniformLocation &location,
				double x, double y)
{
  glUniform2f(location.getId(), (GLfloat)x, (GLfloat)y);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform2fv(const WGLWidget::UniformLocation &location,
			   const WT_ARRAY float *value)
{
  glUniform2fv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform2fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  glUniform2fv(location.getId(), 1, &v.value()[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform2i(const WGLWidget::UniformLocation &location,
				int x, int y)
{
  glUniform2i(location.getId(), x, y);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform2iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  glUniform2iv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform3f(const WGLWidget::UniformLocation &location,
				double x, double y, double z)
{
  glUniform3f(location.getId(), (GLfloat)x, (GLfloat)y, (GLfloat)z);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform3fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  glUniform3fv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform3fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  glUniform3fv(location.getId(), 1, &v.value()[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform3i(const WGLWidget::UniformLocation &location,
				int x, int y, int z)
{
  glUniform3i(location.getId(), x, y, z);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform3iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  glUniform3iv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform4f(const WGLWidget::UniformLocation &location,
				double x, double y, double z, double w)
{
  glUniform4f(location.getId(), (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform4fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  glUniform4fv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform4fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  glUniform4fv(location.getId(), 1, &v.value()[0]);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform4i(const WGLWidget::UniformLocation &location,
				int x, int y, int z, int w)
{
  glUniform4i(location.getId(), x, y, z, w);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniform4iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  glUniform4iv(location.getId(), 1, value);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix2fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  float mat[4];
  for (int i=0; i<4; i++){
    mat[i] = (GLfloat)value[i];
  }

  glUniformMatrix2fv(location.getId(), 1, transpose, mat);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix2(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 2, 2> &m)
{
  float mat[4];
  for (int i=0; i<2; i++){
    for (int j=0; j<2; j++) {
      mat[i*2+j] = (GLfloat)m(j, i);
    }
  }

  glUniformMatrix2fv(location.getId(), 1, false, mat);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix3fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  float mat[9];
  for (int i=0; i<9; i++){
    mat[i] = (GLfloat)value[i];
  }

  glUniformMatrix2fv(location.getId(), 1, transpose, mat);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix3(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 3, 3> &m)
{
  float mat[9];
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++) {
      mat[i*3+j] = (GLfloat)m(j, i);
    }
  }

  glUniformMatrix2fv(location.getId(), 1, false, mat);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix4fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  float mat[16];
  for (int i=0; i<16; i++){
    mat[i] = (GLfloat)value[i];
  }

  glUniformMatrix2fv(location.getId(), 1, transpose, mat);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix4(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 4, 4> &m)
{
  float data[16];
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      data[i*4+j] = (float)m(j,i);
    }
  }

  glUniformMatrix4fv(location.getId(), 1, false, data);
  SERVERGLDEBUG;
}

void WServerGLWidget::uniformMatrix4(const WGLWidget::UniformLocation &location,
				     const WGLWidget::JavaScriptMatrix4x4 &jsm)
{
  WMatrix4x4 mat = jsm.value();

  float data[16];
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      data[i*4+j] = (float)mat(j, i);
    }
  }

  glUniformMatrix4fv(location.getId(), 1, false, data);
  SERVERGLDEBUG;
}

void WServerGLWidget::useProgram(WGLWidget::Program program)
{
  glUseProgram(program.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::validateProgram(WGLWidget::Program program)
{
  glValidateProgram(program.getId());
  SERVERGLDEBUG;
}

void WServerGLWidget::vertexAttrib1f(WGLWidget::AttribLocation location,
				     double x)
{
  glVertexAttrib1f(location.getId(), (GLfloat) x);
  SERVERGLDEBUG;
}

void WServerGLWidget::vertexAttrib2f(WGLWidget::AttribLocation location,
				     double x, double y)
{
  glVertexAttrib2f(location.getId(), (GLfloat)x, (GLfloat)y);
  SERVERGLDEBUG;
}

void WServerGLWidget::vertexAttrib3f(WGLWidget::AttribLocation location,
				     double x, double y, double z)
{
  glVertexAttrib3f(location.getId(), (GLfloat)x, (GLfloat)y, (GLfloat)z);
  SERVERGLDEBUG;
}

void WServerGLWidget::vertexAttrib4f(WGLWidget::AttribLocation location,
				     double x, double y, double z, double w)
{
  glVertexAttrib4f(location.getId(), (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
  SERVERGLDEBUG;
}

void WServerGLWidget::vertexAttribPointer(WGLWidget::AttribLocation location,
					  int size, WGLWidget::GLenum type,
					  bool normalized, unsigned stride,
					  unsigned offset)
{
  glVertexAttribPointer(location.getId(), size, serverGLenum(type),
			normalized, stride, nullptr);
  SERVERGLDEBUG;
}

void WServerGLWidget::viewport(int x, int y, unsigned width, unsigned height)
{
  glViewport(x, y, width, height);
  SERVERGLDEBUG;
}

void WServerGLWidget::clearBinaryResources()
{
  // only in WClientGLWidget
}

void WServerGLWidget::initJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &mat)
{
  if (!mat.hasContext())
    glInterface_->addJavaScriptMatrix4(mat);
  else if (mat.context_ != glInterface_)
    throw WException("JavaScriptMatrix4x4: associated WGLWidget is not equal to the WGLWidget it's being initialized in");
  if (mat.initialized())
    throw WException("JavaScriptMatrix4x4: matrix already initialized");

  WGenericMatrix<double, 4, 4> m = mat.value();
  js_ << mat.jsRef() << "=";
  WClientGLWidget::renderfv(js_, m, JsArrayType::Array);
  js_ << ";";

  mat.initialize();
}

void WServerGLWidget::setJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm,
					   const WGenericMatrix<double, 4, 4> &m)
{
  // transpose it for javascript
  WMatrix4x4 transposed;
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      transposed(i, j) = m(j, i);
    }
  }

  // set it in javascript
  js_ << WT_CLASS ".glMatrix.mat4.set(";
  WClientGLWidget::renderfv(js_, transposed, JsArrayType::Array);
  js_ << ", " << jsm.jsRef() << ");";
}

void WServerGLWidget::initJavaScriptVector(WGLWidget::JavaScriptVector &vec)
{
  if (!vec.hasContext())
    glInterface_->addJavaScriptVector(vec);
  else if (vec.context_ != glInterface_)
    throw WException("JavaScriptVector: associated WGLWidget is not equal to the WGLWidget it's being initialized in");
  if (vec.initialized())
    throw WException("JavaScriptVector: vector already initialized");

  std::vector<float> v = vec.value();
  js_ << vec.jsRef() << "= [";
  for (unsigned i = 0; i < vec.length(); ++i) {
    if (i != 0)
      js_ << ",";
    std::string val;
    if (v[i] == std::numeric_limits<float>::infinity()) {
      val = "Infinity";
    } else if (v[i] == -std::numeric_limits<float>::infinity()) {
      val = "-Infinity";
    } else {
      val = std::to_string(v[i]);
    }
    js_ << val;
  }
  js_ << "];";

  vec.initialize();
}

void WServerGLWidget::setJavaScriptVector(WGLWidget::JavaScriptVector &jsv,
					  const std::vector<float> &v)
{
  if (jsv.length() != v.size())
    throw WException("Trying to set a JavaScriptVector with incompatible length!");
  for (unsigned i = 0; i < jsv.length(); ++i) {
    std::string val;
    if (v[i] == std::numeric_limits<float>::infinity()) {
      val = "Number.POSITIVE_INFINITY";
    } else if (v[i] == -std::numeric_limits<float>::infinity()) {
      val = "Number.NEGATIVE_INFINITY";
    } else {
      val = std::to_string(v[i]);
    }
    js_ << jsv.jsRef() << "[" << i << "] = " << val << ";";
  }
}

void WServerGLWidget::setClientSideMouseHandler(const std::string& handlerCode)
{
  js_ << "obj.setMouseHandler(" << handlerCode << ");";
}

void WServerGLWidget::setClientSideLookAtHandler(const WGLWidget::JavaScriptMatrix4x4 &m,
                                           double centerX, double centerY, double centerZ,
                                           double uX, double uY, double uZ,
                                           double pitchRate, double yawRate)
{
  js_ << "obj.setMouseHandler(new obj.LookAtMouseHandler("
    << m.jsRef() << ",[" << centerX << "," << centerY << "," << centerZ << "],["
    << uX << "," << uY << "," << uZ << "],"
    << pitchRate << "," << yawRate << "));";
}

void WServerGLWidget::setClientSideWalkHandler(const WGLWidget::JavaScriptMatrix4x4 &m, double frontStep, double rotStep)
{
  js_ << "obj.setMouseHandler(new obj.WalkMouseHandler("
      << m.jsRef() << "," << frontStep << "," << rotStep << "));\n";
}

JsArrayType WServerGLWidget::arrayType() const
{
  return JsArrayType::Array;
}

std::string WServerGLWidget::glObjJsRef(const std::string& jsRef)
{
  return "(function(){"
    "var r = " + jsRef + ";"
    "var o = r ? r.wtObj : null;"
    "return o ? o : {ctx: null};"
    "})()";
}

void WServerGLWidget::render(const std::string& jsRef, WFlags<RenderFlag> flags)
{
  //boost::timer::auto_cpu_timer t;
  if (updateResizeGL_ && sizeChanged_) {
    impl_->resize(renderWidth_, renderHeight_);
    delete raster_;
    raster_ = nullptr; // will be re-initialized later

  }

  impl_->makeCurrent();
  glEnable(GL_MULTISAMPLE_ARB);

  if (flags.test(RenderFlag::Full)) {
    std::stringstream ss;
    ss << "{\nvar obj = new " WT_CLASS ".WGLWidget("
       << WApplication::instance()->javaScriptClass() << ","
       << jsRef << ");\n";

    js_.str("");
    glInterface_->initializeGL();
    ss << js_.str().c_str();
    ss << "obj.paintGL = function(){\n"
       << Wt::WApplication::instance()->javaScriptClass() << ".emit(" << jsRef << ", " << WWebWidget::jsStringLiteral(std::string("repaintSignal")) << ");"
       << "}";
    ss << "}";

    glInterface_->doJavaScript(ss.str());
    updatePaintGL_ = true;
  }
  if (updateGL_) {
    js_.str("");
    js_ << "var obj=" << glObjJsRef(jsRef) << ";\n";
    glInterface_->updateGL();
    glInterface_->doJavaScript(js_.str());
    updateGL_ = false;
  }
  if (updateResizeGL_) {
    js_.str("");
    js_ << "var obj=" << glObjJsRef(jsRef) << ";\n";
    glInterface_->resizeGL(renderWidth_, renderHeight_);
    glInterface_->doJavaScript(js_.str());
    updateResizeGL_ = false;
  }
  if (updatePaintGL_) {
    js_.str("");
    js_ << "var obj=" << glObjJsRef(jsRef) << ";\n";
    glInterface_->paintGL();
    glInterface_->doJavaScript(js_.str());
    updatePaintGL_ = false;
  }

  glFinish();
  glFlush();

  //impl_->swapBuffers();

  {
  //boost::timer::auto_cpu_timer t;

  // paint a picture with the framebuffer 0
  std::vector<unsigned char> pixelData(renderWidth_ * renderHeight_ * 4);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  SERVERGLDEBUG;
#ifdef FRAMEBUFFER_RENDERING
  impl_->initReadBuffer();
#endif
  glReadPixels(0, 0, renderWidth_, renderHeight_, GL_RGBA,
	       GL_UNSIGNED_BYTE, &pixelData[0]);
  SERVERGLDEBUG;
#ifdef FRAMEBUFFER_RENDERING
  impl_->setDrawBuffer();
#endif

  if (!raster_)
    raster_ = new WRasterImage("png", renderWidth_, renderHeight_);
  int idx = 0;

  WColor pixel;
  for (int i=0; i < renderHeight_; i++) {
    for (int j=0; j < renderWidth_; j++) {
      pixel.setRgb((int)pixelData[idx+0],
		   (int)pixelData[idx+1],
		   (int)pixelData[idx+2],
		   (int)pixelData[idx+3]);
      idx += 4;
      raster_->setPixel(j, renderHeight_-1-i, pixel);
    }
  }
  std::stringstream sstream;
  raster_->write(sstream);
  std::string tmp = sstream.str();
  memres_->setData(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.size());
  }
  memres_->generateUrl();
  impl_->unmakeCurrent();

  std::stringstream ss;
  ss << jsRef << ".wtObj.loadImage(" << WWebWidget::jsStringLiteral(memres_->url()) << ");";
  glInterface_->doJavaScript(ss.str());
}

void WServerGLWidget::injectJS(const std::string & jsString)
{ }

void WServerGLWidget::restoreContext(const std::string &jsRef)
{ }

}

namespace {

using namespace Wt;
GLenum serverGLenum(WGLWidget::GLenum e)
{
  switch(e) {
  case WGLWidget::DEPTH_BUFFER_BIT:
    return GL_DEPTH_BUFFER_BIT;
  case WGLWidget::STENCIL_BUFFER_BIT:
    return GL_STENCIL_BUFFER_BIT;
  case WGLWidget::COLOR_BUFFER_BIT:
    return GL_COLOR_BUFFER_BIT;
  case WGLWidget::POINTS:
    return GL_POINTS;
  case WGLWidget::LINES:
    return GL_LINES;
  case WGLWidget::LINE_LOOP:
    return GL_LINE_LOOP;
  case WGLWidget::LINE_STRIP:
    return GL_LINE_STRIP;
  case WGLWidget::TRIANGLES:
    return GL_TRIANGLES;
  case WGLWidget::TRIANGLE_STRIP:
    return GL_TRIANGLE_STRIP;
  case WGLWidget::TRIANGLE_FAN:
    return GL_TRIANGLE_FAN;
  // case WGLWidget::ZERO:
  //   return GL_ZERO;
  // case WGLWidget::ONE:
  //   return GL_ONE;
  case WGLWidget::SRC_COLOR:
    return GL_SRC_COLOR;
  case WGLWidget::ONE_MINUS_SRC_COLOR:
    return GL_ONE_MINUS_SRC_COLOR;
  case WGLWidget::SRC_ALPHA:
    return GL_SRC_ALPHA;
  case WGLWidget::ONE_MINUS_SRC_ALPHA:
    return GL_ONE_MINUS_SRC_ALPHA;
  case WGLWidget::DST_ALPHA:
    return GL_DST_ALPHA;
  case WGLWidget::ONE_MINUS_DST_ALPHA:
    return GL_ONE_MINUS_DST_ALPHA;
  case WGLWidget::DST_COLOR:
    return GL_DST_COLOR;
  case WGLWidget::ONE_MINUS_DST_COLOR:
    return GL_ONE_MINUS_DST_COLOR;
  case WGLWidget::SRC_ALPHA_SATURATE:
    return GL_SRC_ALPHA_SATURATE;
  case WGLWidget::FUNC_ADD:
    return GL_FUNC_ADD;
  case WGLWidget::BLEND_EQUATION:
    return GL_BLEND_EQUATION;
  // case WGLWidget::BLEND_EQUATION_RGB:
  //   return GL_BLEND_EQUATION_RGB;
  case WGLWidget::BLEND_EQUATION_ALPHA:
    return GL_BLEND_EQUATION_ALPHA;
  case WGLWidget::FUNC_SUBTRACT:
    return GL_FUNC_SUBTRACT;
  case WGLWidget::FUNC_REVERSE_SUBTRACT:
    return GL_FUNC_REVERSE_SUBTRACT;
  case WGLWidget::BLEND_DST_RGB:
    return GL_BLEND_DST_RGB;
  case WGLWidget::BLEND_SRC_RGB:
    return GL_BLEND_SRC_RGB;
  case WGLWidget::BLEND_DST_ALPHA:
    return GL_BLEND_DST_ALPHA;
  case WGLWidget::BLEND_SRC_ALPHA:
    return GL_BLEND_SRC_ALPHA;
  case WGLWidget::CONSTANT_COLOR:
    return GL_CONSTANT_COLOR;
  case WGLWidget::ONE_MINUS_CONSTANT_COLOR:
    return GL_ONE_MINUS_CONSTANT_COLOR;
  case WGLWidget::CONSTANT_ALPHA:
    return GL_CONSTANT_ALPHA;
  case WGLWidget::ONE_MINUS_CONSTANT_ALPHA:
    return GL_ONE_MINUS_CONSTANT_ALPHA;
  case WGLWidget::BLEND_COLOR:
    return GL_BLEND_COLOR;
  case WGLWidget::ARRAY_BUFFER:
    return GL_ARRAY_BUFFER;
  case WGLWidget::ELEMENT_ARRAY_BUFFER:
    return GL_ELEMENT_ARRAY_BUFFER;
  case WGLWidget::ARRAY_BUFFER_BINDING:
    return GL_ARRAY_BUFFER_BINDING;
  case WGLWidget::ELEMENT_ARRAY_BUFFER_BINDING:
    return GL_ELEMENT_ARRAY_BUFFER_BINDING;
  case WGLWidget::STREAM_DRAW:
    return GL_STREAM_DRAW;
  case WGLWidget::STATIC_DRAW:
    return GL_STATIC_DRAW;
  case WGLWidget::DYNAMIC_DRAW:
    return GL_DYNAMIC_DRAW;
  case WGLWidget::BUFFER_SIZE:
    return GL_BUFFER_SIZE;
  case WGLWidget::BUFFER_USAGE:
    return GL_BUFFER_USAGE;
  case WGLWidget::CURRENT_VERTEX_ATTRIB:
    return GL_CURRENT_VERTEX_ATTRIB;
  case WGLWidget::FRONT:
    return GL_FRONT;
  case WGLWidget::BACK:
    return GL_BACK;
  case WGLWidget::FRONT_AND_BACK:
    return GL_FRONT_AND_BACK;
  case WGLWidget::CULL_FACE:
    return GL_CULL_FACE;
  case WGLWidget::BLEND:
    return GL_BLEND;
  case WGLWidget::DITHER:
    return GL_DITHER;
  case WGLWidget::STENCIL_TEST:
    return GL_STENCIL_TEST;
  case WGLWidget::DEPTH_TEST:
    return GL_DEPTH_TEST;
  case WGLWidget::SCISSOR_TEST:
    return GL_SCISSOR_TEST;
  case WGLWidget::POLYGON_OFFSET_FILL:
    return GL_POLYGON_OFFSET_FILL;
  case WGLWidget::SAMPLE_ALPHA_TO_COVERAGE:
    return GL_SAMPLE_ALPHA_TO_COVERAGE;
  case WGLWidget::SAMPLE_COVERAGE:
    return GL_SAMPLE_COVERAGE;
  // case WGLWidget::NO_ERROR:
  //   return GL_NO_ERROR;
  case WGLWidget::INVALID_ENUM:
    return GL_INVALID_ENUM;
  case WGLWidget::INVALID_VALUE:
    return GL_INVALID_VALUE;
  case WGLWidget::INVALID_OPERATION:
    return GL_INVALID_OPERATION;
  case WGLWidget::OUT_OF_MEMORY:
    return GL_OUT_OF_MEMORY;
  case WGLWidget::CW:
    return GL_CW;
  case WGLWidget::CCW:
    return GL_CCW;
  case WGLWidget::LINE_WIDTH:
    return GL_LINE_WIDTH;
  case WGLWidget::ALIASED_POINT_SIZE_RANGE:
    return GL_ALIASED_POINT_SIZE_RANGE;
  case WGLWidget::ALIASED_LINE_WIDTH_RANGE:
    return GL_ALIASED_LINE_WIDTH_RANGE;
  case WGLWidget::CULL_FACE_MODE:
    return GL_CULL_FACE_MODE;
  case WGLWidget::FRONT_FACE:
    return GL_FRONT_FACE;
  case WGLWidget::DEPTH_RANGE:
    return GL_DEPTH_RANGE;
  case WGLWidget::DEPTH_WRITEMASK:
    return GL_DEPTH_WRITEMASK;
  case WGLWidget::DEPTH_CLEAR_VALUE:
    return GL_DEPTH_CLEAR_VALUE;
  case WGLWidget::DEPTH_FUNC:
    return GL_DEPTH_FUNC;
  case WGLWidget::STENCIL_CLEAR_VALUE:
    return GL_STENCIL_CLEAR_VALUE;
  case WGLWidget::STENCIL_FUNC:
    return GL_STENCIL_FUNC;
  case WGLWidget::STENCIL_FAIL:
    return GL_STENCIL_FAIL;
  case WGLWidget::STENCIL_PASS_DEPTH_FAIL:
    return GL_STENCIL_PASS_DEPTH_FAIL;
  case WGLWidget::STENCIL_PASS_DEPTH_PASS:
    return GL_STENCIL_PASS_DEPTH_PASS;
  case WGLWidget::STENCIL_REF:
    return GL_STENCIL_REF;
  case WGLWidget::STENCIL_VALUE_MASK:
    return GL_STENCIL_VALUE_MASK;
  case WGLWidget::STENCIL_WRITEMASK:
    return GL_STENCIL_WRITEMASK;
  case WGLWidget::STENCIL_BACK_FUNC:
    return GL_STENCIL_BACK_FUNC;
  case WGLWidget::STENCIL_BACK_FAIL:
    return GL_STENCIL_BACK_FAIL;
  case WGLWidget::STENCIL_BACK_PASS_DEPTH_FAIL:
    return GL_STENCIL_BACK_PASS_DEPTH_FAIL;
  case WGLWidget::STENCIL_BACK_PASS_DEPTH_PASS:
    return GL_STENCIL_BACK_PASS_DEPTH_PASS;
  case WGLWidget::STENCIL_BACK_REF:
    return GL_STENCIL_BACK_REF;
  case WGLWidget::STENCIL_BACK_VALUE_MASK:
    return GL_STENCIL_BACK_VALUE_MASK;
  case WGLWidget::STENCIL_BACK_WRITEMASK:
    return GL_STENCIL_BACK_WRITEMASK;
  case WGLWidget::VIEWPORT:
    return GL_VIEWPORT;
  case WGLWidget::SCISSOR_BOX:
    return GL_SCISSOR_BOX;
  case WGLWidget::COLOR_CLEAR_VALUE:
    return GL_COLOR_CLEAR_VALUE;
  case WGLWidget::COLOR_WRITEMASK:
    return GL_COLOR_WRITEMASK;
  case WGLWidget::UNPACK_ALIGNMENT:
    return GL_UNPACK_ALIGNMENT;
  case WGLWidget::PACK_ALIGNMENT:
    return GL_PACK_ALIGNMENT;
  case WGLWidget::MAX_TEXTURE_SIZE:
    return GL_MAX_TEXTURE_SIZE;
  case WGLWidget::MAX_VIEWPORT_DIMS:
    return GL_MAX_VIEWPORT_DIMS;
  case WGLWidget::SUBPIXEL_BITS:
    return GL_SUBPIXEL_BITS;
  case WGLWidget::RED_BITS:
    return GL_RED_BITS;
  case WGLWidget::GREEN_BITS:
    return GL_GREEN_BITS;
  case WGLWidget::BLUE_BITS:
    return GL_BLUE_BITS;
  case WGLWidget::ALPHA_BITS:
    return GL_ALPHA_BITS;
  case WGLWidget::DEPTH_BITS:
    return GL_DEPTH_BITS;
  case WGLWidget::STENCIL_BITS:
    return GL_STENCIL_BITS;
  case WGLWidget::POLYGON_OFFSET_UNITS:
    return GL_POLYGON_OFFSET_UNITS;
  case WGLWidget::POLYGON_OFFSET_FACTOR:
    return GL_POLYGON_OFFSET_FACTOR;
  case WGLWidget::TEXTURE_BINDING_2D:
    return GL_TEXTURE_BINDING_2D;
  case WGLWidget::SAMPLE_BUFFERS:
    return GL_SAMPLE_BUFFERS;
  case WGLWidget::SAMPLES:
    return GL_SAMPLES;
  case WGLWidget::SAMPLE_COVERAGE_VALUE:
    return GL_SAMPLE_COVERAGE_VALUE;
  case WGLWidget::SAMPLE_COVERAGE_INVERT:
    return GL_SAMPLE_COVERAGE_INVERT;
  case WGLWidget::NUM_COMPRESSED_TEXTURE_FORMATS:
    return GL_NUM_COMPRESSED_TEXTURE_FORMATS;
  case WGLWidget::COMPRESSED_TEXTURE_FORMATS:
    return GL_COMPRESSED_TEXTURE_FORMATS;
  case WGLWidget::DONT_CARE:
    return GL_DONT_CARE;
  case WGLWidget::FASTEST:
    return GL_FASTEST;
  case WGLWidget::NICEST:
    return GL_NICEST;
  case WGLWidget::GENERATE_MIPMAP_HINT:
    return GL_GENERATE_MIPMAP_HINT;
  case WGLWidget::BYTE:
    return GL_BYTE;
  case WGLWidget::UNSIGNED_BYTE:
    return GL_UNSIGNED_BYTE;
  case WGLWidget::SHORT:
    return GL_SHORT;
  case WGLWidget::UNSIGNED_SHORT:
    return GL_UNSIGNED_SHORT;
  case WGLWidget::INT:
    return GL_INT;
  case WGLWidget::UNSIGNED_INT:
    return GL_UNSIGNED_INT;
  case WGLWidget::FLOAT:
    return GL_FLOAT;
  case WGLWidget::DEPTH_COMPONENT:
    return GL_DEPTH_COMPONENT;
  case WGLWidget::ALPHA:
    return GL_ALPHA;
  case WGLWidget::RGB:
    return GL_RGB;
  case WGLWidget::RGBA:
    return GL_RGBA;
  case WGLWidget::LUMINANCE:
    return GL_LUMINANCE;
  case WGLWidget::LUMINANCE_ALPHA:
    return GL_LUMINANCE_ALPHA;
  case WGLWidget::UNSIGNED_SHORT_4_4_4_4:
    return GL_UNSIGNED_SHORT_4_4_4_4;
  case WGLWidget::UNSIGNED_SHORT_5_5_5_1:
    return GL_UNSIGNED_SHORT_5_5_5_1;
  case WGLWidget::UNSIGNED_SHORT_5_6_5:
    return GL_UNSIGNED_SHORT_5_6_5;
  case WGLWidget::FRAGMENT_SHADER:
    return GL_FRAGMENT_SHADER;
  case WGLWidget::VERTEX_SHADER:
    return GL_VERTEX_SHADER;
  case WGLWidget::MAX_VERTEX_ATTRIBS:
    return GL_MAX_VERTEX_ATTRIBS;
  case WGLWidget::MAX_VERTEX_UNIFORM_VECTORS:
    return GL_MAX_VERTEX_UNIFORM_VECTORS;
  case WGLWidget::MAX_VARYING_VECTORS:
    return GL_MAX_VARYING_VECTORS;
  case WGLWidget::MAX_COMBINED_TEXTURE_IMAGE_UNITS:
    return GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
  case WGLWidget::MAX_VERTEX_TEXTURE_IMAGE_UNITS:
    return GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS;
  case WGLWidget::MAX_TEXTURE_IMAGE_UNITS:
    return GL_MAX_TEXTURE_IMAGE_UNITS;
  case WGLWidget::MAX_FRAGMENT_UNIFORM_VECTORS:
    return GL_MAX_FRAGMENT_UNIFORM_VECTORS;
  case WGLWidget::SHADER_TYPE:
    return GL_SHADER_TYPE;
  case WGLWidget::DELETE_STATUS:
    return GL_DELETE_STATUS;
  case WGLWidget::LINK_STATUS:
    return GL_LINK_STATUS;
  case WGLWidget::VALIDATE_STATUS:
    return GL_VALIDATE_STATUS;
  case WGLWidget::ATTACHED_SHADERS:
    return GL_ATTACHED_SHADERS;
  case WGLWidget::ACTIVE_UNIFORMS:
    return GL_ACTIVE_UNIFORMS;
  case WGLWidget::ACTIVE_UNIFORM_MAX_LENGTH:
    return GL_ACTIVE_UNIFORM_MAX_LENGTH;
  case WGLWidget::ACTIVE_ATTRIBUTES:
    return GL_ACTIVE_ATTRIBUTES;
  case WGLWidget::ACTIVE_ATTRIBUTE_MAX_LENGTH:
    return GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
  case WGLWidget::SHADING_LANGUAGE_VERSION:
    return GL_SHADING_LANGUAGE_VERSION;
  case WGLWidget::CURRENT_PROGRAM:
    return GL_CURRENT_PROGRAM;
  case WGLWidget::NEVER:
    return GL_NEVER;
  case WGLWidget::LESS:
    return GL_LESS;
  case WGLWidget::EQUAL:
    return GL_EQUAL;
  case WGLWidget::LEQUAL:
    return GL_LEQUAL;
  case WGLWidget::GREATER:
    return GL_GREATER;
  case WGLWidget::NOTEQUAL:
    return GL_NOTEQUAL;
  case WGLWidget::GEQUAL:
    return GL_GEQUAL;
  case WGLWidget::ALWAYS:
    return GL_ALWAYS;
  case WGLWidget::KEEP:
    return GL_KEEP;
  case WGLWidget::REPLACE:
    return GL_REPLACE;
  case WGLWidget::INCR:
    return GL_INCR;
  case WGLWidget::DECR:
    return GL_DECR;
  case WGLWidget::INVERT:
    return GL_INVERT;
  case WGLWidget::INCR_WRAP:
    return GL_INCR_WRAP;
  case WGLWidget::DECR_WRAP:
    return GL_DECR_WRAP;
  case WGLWidget::VENDOR:
    return GL_VENDOR;
  case WGLWidget::RENDERER:
    return GL_RENDERER;
  case WGLWidget::VERSION:
    return GL_VERSION;
  case WGLWidget::NEAREST:
    return GL_NEAREST;
  case WGLWidget::LINEAR:
    return GL_LINEAR;
  case WGLWidget::NEAREST_MIPMAP_NEAREST:
    return GL_NEAREST_MIPMAP_NEAREST;
  case WGLWidget::LINEAR_MIPMAP_NEAREST:
    return GL_LINEAR_MIPMAP_NEAREST;
  case WGLWidget::NEAREST_MIPMAP_LINEAR:
    return GL_NEAREST_MIPMAP_LINEAR;
  case WGLWidget::LINEAR_MIPMAP_LINEAR:
    return GL_LINEAR_MIPMAP_LINEAR;
  case WGLWidget::TEXTURE_MAG_FILTER:
    return GL_TEXTURE_MAG_FILTER;
  case WGLWidget::TEXTURE_MIN_FILTER:
    return GL_TEXTURE_MIN_FILTER;
  case WGLWidget::TEXTURE_WRAP_S:
    return GL_TEXTURE_WRAP_S;
  case WGLWidget::TEXTURE_WRAP_T:
    return GL_TEXTURE_WRAP_T;
  case WGLWidget::TEXTURE_2D:
    return GL_TEXTURE_2D;
  case WGLWidget::TEXTURE:
    return GL_TEXTURE;
  case WGLWidget::TEXTURE_CUBE_MAP:
    return GL_TEXTURE_CUBE_MAP;
  case WGLWidget::TEXTURE_BINDING_CUBE_MAP:
    return GL_TEXTURE_BINDING_CUBE_MAP;
  case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_X:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_X:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
  case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_Y:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
  case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_Y:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
  case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_Z:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
  case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_Z:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  case WGLWidget::MAX_CUBE_MAP_TEXTURE_SIZE:
    return GL_MAX_CUBE_MAP_TEXTURE_SIZE;
  case WGLWidget::TEXTURE0:
    return GL_TEXTURE0;
  case WGLWidget::TEXTURE1:
    return GL_TEXTURE1;
  case WGLWidget::TEXTURE2:
    return GL_TEXTURE2;
  case WGLWidget::TEXTURE3:
    return GL_TEXTURE3;
  case WGLWidget::TEXTURE4:
    return GL_TEXTURE4;
  case WGLWidget::TEXTURE5:
    return GL_TEXTURE5;
  case WGLWidget::TEXTURE6:
    return GL_TEXTURE6;
  case WGLWidget::TEXTURE7:
    return GL_TEXTURE7;
  case WGLWidget::TEXTURE8:
    return GL_TEXTURE8;
  case WGLWidget::TEXTURE9:
    return GL_TEXTURE9;
  case WGLWidget::TEXTURE10:
    return GL_TEXTURE10;
  case WGLWidget::TEXTURE11:
    return GL_TEXTURE11;
  case WGLWidget::TEXTURE12:
    return GL_TEXTURE12;
  case WGLWidget::TEXTURE13:
    return GL_TEXTURE13;
  case WGLWidget::TEXTURE14:
    return GL_TEXTURE14;
  case WGLWidget::TEXTURE15:
    return GL_TEXTURE15;
  case WGLWidget::TEXTURE16:
    return GL_TEXTURE16;
  case WGLWidget::TEXTURE17:
    return GL_TEXTURE17;
  case WGLWidget::TEXTURE18:
    return GL_TEXTURE18;
  case WGLWidget::TEXTURE19:
    return GL_TEXTURE19;
  case WGLWidget::TEXTURE20:
    return GL_TEXTURE20;
  case WGLWidget::TEXTURE21:
    return GL_TEXTURE21;
  case WGLWidget::TEXTURE22:
    return GL_TEXTURE22;
  case WGLWidget::TEXTURE23:
    return GL_TEXTURE23;
  case WGLWidget::TEXTURE24:
    return GL_TEXTURE24;
  case WGLWidget::TEXTURE25:
    return GL_TEXTURE25;
  case WGLWidget::TEXTURE26:
    return GL_TEXTURE26;
  case WGLWidget::TEXTURE27:
    return GL_TEXTURE27;
  case WGLWidget::TEXTURE28:
    return GL_TEXTURE28;
  case WGLWidget::TEXTURE29:
    return GL_TEXTURE29;
  case WGLWidget::TEXTURE30:
    return GL_TEXTURE30;
  case WGLWidget::TEXTURE31:
    return GL_TEXTURE31;
  case WGLWidget::ACTIVE_TEXTURE:
    return GL_ACTIVE_TEXTURE;
  case WGLWidget::REPEAT:
    return GL_REPEAT;
  case WGLWidget::CLAMP_TO_EDGE:
    return GL_CLAMP_TO_EDGE;
  case WGLWidget::MIRRORED_REPEAT:
    return GL_MIRRORED_REPEAT;
  case WGLWidget::FLOAT_VEC2:
    return GL_FLOAT_VEC2;
  case WGLWidget::FLOAT_VEC3:
    return GL_FLOAT_VEC3;
  case WGLWidget::FLOAT_VEC4:
    return GL_FLOAT_VEC4;
  case WGLWidget::INT_VEC2:
    return GL_INT_VEC2;
  case WGLWidget::INT_VEC3:
    return GL_INT_VEC3;
  case WGLWidget::INT_VEC4:
    return GL_INT_VEC4;
  case WGLWidget::BOOL:
    return GL_BOOL;
  case WGLWidget::BOOL_VEC2:
    return GL_BOOL_VEC2;
  case WGLWidget::BOOL_VEC3:
    return GL_BOOL_VEC3;
  case WGLWidget::BOOL_VEC4:
    return GL_BOOL_VEC4;
  case WGLWidget::FLOAT_MAT2:
    return GL_FLOAT_MAT2;
  case WGLWidget::FLOAT_MAT3:
    return GL_FLOAT_MAT3;
  case WGLWidget::FLOAT_MAT4:
    return GL_FLOAT_MAT4;
  case WGLWidget::SAMPLER_2D:
    return GL_SAMPLER_2D;
  case WGLWidget::SAMPLER_CUBE:
    return GL_SAMPLER_CUBE;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_ENABLED:
    return GL_VERTEX_ATTRIB_ARRAY_ENABLED;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_SIZE:
    return GL_VERTEX_ATTRIB_ARRAY_SIZE;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_STRIDE:
    return GL_VERTEX_ATTRIB_ARRAY_STRIDE;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_TYPE:
    return GL_VERTEX_ATTRIB_ARRAY_TYPE;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_NORMALIZED:
    return GL_VERTEX_ATTRIB_ARRAY_NORMALIZED;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_POINTER:
    return GL_VERTEX_ATTRIB_ARRAY_POINTER;
  case WGLWidget::VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    return GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING;
  case WGLWidget::COMPILE_STATUS:
    return GL_COMPILE_STATUS;
  case WGLWidget::INFO_LOG_LENGTH:
    return GL_INFO_LOG_LENGTH;
  case WGLWidget::SHADER_SOURCE_LENGTH:
    return GL_SHADER_SOURCE_LENGTH;
  case WGLWidget::LOW_FLOAT:
    return GL_LOW_FLOAT;
  case WGLWidget::MEDIUM_FLOAT:
    return GL_MEDIUM_FLOAT;
  case WGLWidget::HIGH_FLOAT:
    return GL_HIGH_FLOAT;
  case WGLWidget::LOW_INT:
    return GL_LOW_INT;
  case WGLWidget::MEDIUM_INT:
    return GL_MEDIUM_INT;
  case WGLWidget::HIGH_INT:
    return GL_HIGH_INT;
  case WGLWidget::FRAMEBUFFER:
    return GL_FRAMEBUFFER;
  case WGLWidget::RENDERBUFFER:
    return GL_RENDERBUFFER;
  case WGLWidget::RGBA4:
    return GL_RGBA4;
  case WGLWidget::RGB5_A1:
    return GL_RGB5_A1;
  case WGLWidget::RGB565:
    return GL_RGB565;
  case WGLWidget::DEPTH_COMPONENT16:
    return GL_DEPTH_COMPONENT16;
  case WGLWidget::STENCIL_INDEX:
    return GL_STENCIL_INDEX;
  case WGLWidget::STENCIL_INDEX8:
    return GL_STENCIL_INDEX8;
  case WGLWidget::DEPTH_STENCIL:
    return GL_DEPTH_STENCIL;
  case WGLWidget::RENDERBUFFER_WIDTH:
    return GL_RENDERBUFFER_WIDTH;
  case WGLWidget::RENDERBUFFER_HEIGHT:
    return GL_RENDERBUFFER_HEIGHT;
  case WGLWidget::RENDERBUFFER_INTERNAL_FORMAT:
    return GL_RENDERBUFFER_INTERNAL_FORMAT;
  case WGLWidget::RENDERBUFFER_RED_SIZE:
    return GL_RENDERBUFFER_RED_SIZE;
  case WGLWidget::RENDERBUFFER_GREEN_SIZE:
    return GL_RENDERBUFFER_GREEN_SIZE;
  case WGLWidget::RENDERBUFFER_BLUE_SIZE:
    return GL_RENDERBUFFER_BLUE_SIZE;
  case WGLWidget::RENDERBUFFER_ALPHA_SIZE:
    return GL_RENDERBUFFER_ALPHA_SIZE;
  case WGLWidget::RENDERBUFFER_DEPTH_SIZE:
    return GL_RENDERBUFFER_DEPTH_SIZE;
  case WGLWidget::RENDERBUFFER_STENCIL_SIZE:
    return GL_RENDERBUFFER_STENCIL_SIZE;
  case WGLWidget::FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
    return GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE;
  case WGLWidget::FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
    return GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME;
  case WGLWidget::FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
    return GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL;
  case WGLWidget::FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
    return GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE;
  case WGLWidget::COLOR_ATTACHMENT0:
    return GL_COLOR_ATTACHMENT0;
  case WGLWidget::DEPTH_ATTACHMENT:
    return GL_DEPTH_ATTACHMENT;
  case WGLWidget::STENCIL_ATTACHMENT:
    return GL_STENCIL_ATTACHMENT;
  case WGLWidget::DEPTH_STENCIL_ATTACHMENT:
    return GL_DEPTH_STENCIL_ATTACHMENT;
  // case WGLWidget::NONE:
  //   return GL_NONE;
  case WGLWidget::FRAMEBUFFER_COMPLETE:
    return GL_FRAMEBUFFER_COMPLETE;
  case WGLWidget::FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
  case WGLWidget::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
  // case WGLWidget::FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
  //   return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
  case WGLWidget::FRAMEBUFFER_UNSUPPORTED:
    return GL_FRAMEBUFFER_UNSUPPORTED;
  case WGLWidget::FRAMEBUFFER_BINDING:
    return GL_FRAMEBUFFER_BINDING;
  case WGLWidget::RENDERBUFFER_BINDING:
    return GL_RENDERBUFFER_BINDING;
  case WGLWidget::MAX_RENDERBUFFER_SIZE:
    return GL_MAX_RENDERBUFFER_SIZE;
  case WGLWidget::INVALID_FRAMEBUFFER_OPERATION:
    return GL_INVALID_FRAMEBUFFER_OPERATION;
  default:
    return -1;
  }
}

}
