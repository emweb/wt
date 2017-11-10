// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGLWIDGET_H_
#define WGLWIDGET_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WGenericMatrix.h>
#include <Wt/WMatrix4x4.h>

#ifndef WT_TARGET_JAVA
namespace Wt {
typedef std::vector<float> FloatBuffer;
typedef std::vector<int> IntBuffer;
}
#else
#include <java/buffer>
#endif

namespace Wt {

// define to enable WebGL debug code
#define WT_WGLWIDGET_DEBUG

#ifdef WT_TARGET_JAVA
#define __FUNCTION__ "(unknown)"
#endif

#ifdef WT_WGLWIDGET_DEBUG
#define GLDEBUG do {if (debugging_) {js_ << "\n{var err = ctx.getError(); if(err != ctx.NO_ERROR && err != ctx.CONTEXT_LOST_WEBGL) {alert('error " << __FUNCTION__ << ": ' + err); debugger;}}\n";}} while(false)
  //if (glGetError() != GL_NO_ERROR){}
#define SERVERGLDEBUG do {if (debugging_) {int err = glGetError(); if (err != GL_NO_ERROR){std::cerr << "gl error occured in " << __FUNCTION__ << ": " << err << std::endl;}} } while(false)
//#define GLDEBUG do {js_ << "\n{var err = ctx.getError(); if(err != ctx.NO_ERROR) {debugger;}}\n";} while(false)
#else
#define GLDEBUG
#endif

class WAbstractGLImplementation;
class WPaintDevice;

#if !defined(WT_TARGET_JAVA)
#define WT_WGL_TEMPLATE(...) template < __VA_ARGS__ >
#else
#define WT_WGL_TEMPLATE(...)
#endif

class WHTML5Video;
class WImage;

enum class JsArrayType {
  Array,
  Float32Array
};

/*! \brief Enumeration for render options
 *
 * \sa setRenderOptions()
 */
enum class GLRenderOption {
  ClientSide = 0x1,   //!< Enables client-side rendering
  ServerSide = 0x2,   //!< Enables server-side rendering
  AntiAliasing = 0x4  //!< Enables anti-aliasing
};

/*! \brief Specifies what GL function needs to be updated
 */
enum class GLClientSideRenderer {
  PAINT_GL = 0x1,  //!< refresh paintGL()
  RESIZE_GL = 0x2, //!< refresh resizeGL()
  UPDATE_GL = 0x4  //!< refresh updateGL()
};

/*! \class WGLWidget Wt/WGLWidget.h Wt/WGLWidget.h
 *  \brief GL support class
 *
 * The WGLWidget class is an interface to the HTML5 WebGL infrastructure
 * for client-side rendering, and OpenGL for server-side rendering.
 * Its API is based on the WebGL API. To fully understand WebGL, it is
 * recommended to read the WebGL standard in addition to this documentation.
 *
 * The most recent version of the WebGL specification can be found here:
 * http://www.khronos.org/registry/webgl/specs/latest/1.0/
 *
 * The goal of the WGLWidget class is to provide a method to render 3D
 * structures in the browser, where rendering and rerendering is normally
 * done at the client side in JavaScript without interaction from the server,
 * in order to obtain a smooth user interaction. Unless the scene requires
 * server-side updates, there is no communication with the server.
 *
 * The rendering interface resembles to OpenGL ES, the same standard as
 * WebGL is based on. This is a stripped down version of the normal OpenGL
 * as we usually find them on desktops. Many stateful OpenGL features are
 * not present in OpenGL ES: no modelview and camera transformation stacks,
 * no default lighting models, no support for other rendering methods than
 * through VBOs, ... Therefore much existing example code for OpenGL
 * applications and shaders will not work on WebGL without modifications.
 * The 'learning webgl' web site at http://learningwebgl.com/ is a good
 * starting point to get familiar with WebGL.
 *
 * To use a %WGLWidget, you must derive from it and reimplement
 * the painter methods. Usually, you will always need to implement
 * initializeGL() and paintGL(). Optionally, you may choose to implement
 * resizeGL() (if your widget does not have a fixed size), and updateGL().
 * If you need to modify the painting methods, a repaint is triggered by
 * calling the repaintGL() method. The default behaviour for any of these
 * four painting functions is to do nothing.
 *
 * The four painter methods (initializeGL(), resizeGL(), paintGL() and
 * updateGL()) all record JavaScript which is sent to the
 * browser. The JavaScript code of paintGL() is cached client-side, and
 * may be executed many times, e.g. to repaint a scene from different
 * viewpoints. The JavaScript code of initializeGL(), resizeGL() and
 * updateGL() are intended for OpenGL state updates, and is therefore
 * only executed once on the client and is then discarded.
 *
 * There are four painting methods that you may implement in a
 * specialization of this class. The purpose of these functions is
 * to register what JavaScript code has to be executed to render a
 * scene. Through invocations of the WebGL functions documented below,
 * %Wt records the JavaScript calls that have to be invoked in the
 * browser.
 * <ul>
 * <li><b>initializeGL()</b>: this function is executed after the GL
 *     context has been initialized. It is also executed when the
 *     \c webglcontextrestored signal is fired. You can distinguish between
 *     the first initialization and restoration of context using restoringContext().
 *     This is the ideal location to compose shader programs, send VBO's to
 *     the client, extract uniform and attribute locations, ... Due to
 *     the presence of VBO's, this function may generate a large amount
 *     of data to the client.
 * <li><b>resizeGL()</b>: this function is executed whenever the canvas
 *     dimensions change. A change in canvas size will require you to
 *     invoke the viewport() function again, as well as recalculate the
 *     projection matrices (especially when the aspect ratio has changed).
 *     The resizeGL() function is therefore the ideal location to set those
 *     properties.
 *     The resizeGL() function is invoked automatically on every resize,
 *     and after the first initializeGL() invocation. Additional invocations
 *     may be triggered by calling repaint() with the RESIZE_GL flag.
 * <li><b>paintGL()</b>: this is the main scene drawing function. Through
 *     its execution, %Wt records what has to be done to render a scene,
 *     and it is executed every time that the scene is to be redrawn. You
 *     can use the VBO's and shaders prepared in the initializeGL() phase.
 *     Usually, this function sets uniforms and attributes, links
 *     attributes to VBO's, applies textures, and draws primitives.
 *     You may also create local programs, buffers, ... Remember that this
 *     function is executed a lot of times, so every buffer/program created
 *     in this function should also be destroyed to avoid memory leaks.
 *     This function is transmitted once to the client, and is executed
 *     when the scene needs to be redrawn. Redraws may be triggered from
 *     mouse events, timer triggers, events on e.g. a video element, or
 *     whatever other event.
 *     The paintGL() function can be updated through invoking repaintGL()
 *     with the PAINT_GL flag.
 * <li><b>updateGL()</b>: VBO's, programs, uniforms, GL properties,
 *     or anything else set during intializeGL() are not necessarily
 *     immutable. If you want to change, add, remove or reconfigure those
 *     properties, the execution of an updateGL() function can be triggered
 *     by invoking repaintGL() with the UPDATE_GL flag. This signals that
 *     updateGL() needs to be evaluated - just once. It is possible that
 *     the paintGL() function also requires updates as consequence of the
 *     changes in the updateGL() function; in this case, you should also set
 *     the PAINT_GL flag of repaintGL().
 * </ul>
 *
 * The GL functions are intended to be used exclusively from within the
 * invocation of the four callback functions mentioned above. In order to
 * manually trigger the execution of these function, use the repaintGL().
 *
 * A WGLWidget must be given a size explicitly, or must be put inside a
 * layout manager that manages its width and height. The behaviour of a
 * WGLWidget that was not given a size is undefined.
 *
 * <h3>Binary buffer transfers</h3>
 *
 * In bufferDatafv(), there is an additional boolean argument where you
 * can indicate that you want the data to be transferred to the client in
 * binary form. A WMemoryResource is created for each of these buffers. If
 * you know all previous resources are not required in the client anymore,
 * you can free memory with the method clearBinaryResources() (the memory
 * is also managed, so this is not neccesary). If you want to manage these
 * resources entirely by yourself, the following method can be used.
 *
 * Using createAndLoadArrayBuffer(), you can load an array buffer in
 * binary format from an URL. This will cause the client to fetch the
 * given URL, and make the contents of the file available in an
 * ArrayBuffer, which can then be used by BufferData() to bind them
 * to an OpenGL buffer. This is ideal to load VBO buffers in a faster
 * way, as it avoids converting floats to text strings on the server
 * and then back to floats on the client. You can combine this with
 * the use of WResource (e.g. WMemoryResource) to send an std::vector of
 * vertices to the client. Note that using ArrayBuffer is not possible when
 * you want a fall-back in the form of server-side rendering.
 *
 * <h3>Client side matrices and vectors.</h3>
 *
 * The WGLWidget provides the WGLWidget::JavaScriptMatrix4x4 class as
 * a mechanism to use client-side modifiable matrices in the render
 * functions. These matrices can be used identically to the
 * 'constant', with the advantage that there is no need to have a
 * roundtrip to the server to redraw the scene when they are changed.
 * As such, they are ideal for mouse-based camera manipulations, timer
 * triggered animations, or object manipulations.
 *
 * There's also support for client-side modifiable vectors, with
 * WGLWidget::JavaScriptVector.
 */
class WT_API WGLWidget: public WInteractWidget
{
#ifdef WT_TARGET_JAVA
  typedef float FloatArray;
  typedef int IntArray;
  typedef double MatrixType;
#endif

public:
  /*! \brief Typedef for enum Wt::GLRenderOption */
  typedef GLRenderOption RenderOption;
  /*! \brief Typedef for enum Wt::GLClientSideRenderer */
  typedef GLClientSideRenderer ClientSideRenderer;


  //! Abstract base class for all GL objects
  class WT_API GlObject {
  public:
    GlObject(): id_(-1) {}
    GlObject(int id) : 
      id_(id) {}
    virtual ~GlObject() {}
    
    virtual std::string jsRef() const = 0;

    int getId() const { return id_; }

    void clear() { id_ = -1; }

    bool isNull() const { return id_ == -1; }
   
  private:
    int id_;
  };

  //!  Reference to a WebGLShader class
  class WT_API Shader : public GlObject {
  public:
    Shader() {}

    explicit Shader(int i) : 
      GlObject(i) {}
    
    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("Shader: is null");
      return std::string("ctx.WtShader") + std::to_string(getId());
    }
  };

  //! Reference to a WebGLProgram class
  class WT_API Program : public GlObject {
  public:
    Program() {}
    explicit Program(int i) : 
      GlObject(i) {}
    
    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("Program: is null");
      return std::string("ctx.WtProgram") + std::to_string(getId());
    }
  };

  //! Reference to a shader attribute location
  class WT_API AttribLocation : public GlObject {
  public:
    AttribLocation() {}
    explicit AttribLocation(int i) : 
      GlObject(i) {}
    
    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("AttribLocation: is null");
      return std::string("ctx.WtAttrib") + std::to_string(getId());
    }
  };

  //! Reference to a WebGLBuffer class
  class WT_API Buffer : public GlObject {
  public:
    Buffer() {}
    explicit Buffer(int i) : 
      GlObject(i) {}
    
    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("Buffer: is null");
      return std::string("ctx.WtBuffer") + std::to_string(getId());
    }
  };

  //! Reference to a WebGLUniformLocation class
  class WT_API UniformLocation : public GlObject {
  public:
    UniformLocation() {}
    explicit UniformLocation(int i) : 
      GlObject(i) {}
    
    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("UniformLocation: is null");
      return std::string("ctx.WtUniform") + std::to_string(getId());
    }
  };

  //! Reference to a WebGLTexture class
  class WT_API Texture: public GlObject {
  public:
    Texture()
      : url_("")
      {}
    explicit Texture(int i) : 
      GlObject(i) {}

    virtual std::string jsRef() const override
    {
      if (isNull()) {
	// throw WException("Texture: is null");
	return "null";
      }
      return std::string("ctx.WtTexture") + std::to_string(getId());
    }

    void setUrl(std::string url) {
      url_ = url;
    }

    const std::string& url() const {
      return url_;
    }

  private:
    std::string url_;
  };

  //! Reference to a WebGLFramebuffer class
  class WT_API Framebuffer : public GlObject {
  public:
    Framebuffer() {}
    Framebuffer(int i) :
      GlObject(i) {}

    virtual std::string jsRef() const override
    {
      if (isNull()) {
	//throw WException("FrameBuffer: is null");
	return "null";
      }
      return "ctx.WtFramebuffer" + std::to_string(getId());
    }
  };

  //! Reference to a WebGLRenderbuffer class
  class WT_API Renderbuffer : public GlObject {
  public:
    Renderbuffer() {}
    Renderbuffer(int i) :
      GlObject(i) {}

    virtual std::string jsRef() const override
    {
      if (isNull()) {
	//throw WException("RenderBuffer: is null");
	return "null";
      }
      return "ctx.WtRenderbuffer" + std::to_string(getId());
    }
  };

  /*!
   * \brief Reference to a javascript %ArrayBuffer class
   */
  class WT_API ArrayBuffer : public GlObject {
  public:
    ArrayBuffer() {}
    ArrayBuffer(int i) :
      GlObject(i) {}

    virtual std::string jsRef() const override
    {
      if (isNull())
	throw WException("ArrayBuffer: is null");
      return "ctx.WtBufferResource" + std::to_string(getId());
    }
  };

  /*! \brief A client-side JavaScript vector
   *
   * Using a JavaScriptVector, GL parameters can be modified without
   * communication with the server. The value of the JavaScriptMatrix4x4
   * is updated server-side whenever an event is sent to the server.
   *
   * The JavaScriptVector is represented in JavaScript as an array, either
   * as a Float32Array or as a plain JavaScript array.
   */
  class WT_API JavaScriptVector {
  public:
    /** \brief Create a temporarily invalid JavaScriptVector.
     *
     *  Should be added to a %WGLWidget with
     *  WGLWidget::addJavaScriptVector(), and initialized with
     *  WGLWidget::initJavaScriptVector().
     */
    JavaScriptVector(unsigned length);

#ifndef WT_TARGET_JAVA
    JavaScriptVector(const JavaScriptVector &other);

    JavaScriptVector &operator=(const JavaScriptVector &rhs);
#endif

    int id() const { return id_; }

    /** \brief Returns whether this %JavaScriptVector has been initialized.
     */
    bool initialized() const { return initialized_; }

    /** \brief Returns whether this %JavaScriptVector has been assigned
     *         to a WGLWidget.
     */
    bool hasContext() const { return context_ != nullptr; }

    /** \brief Returns the length (number of items) of this JavaScriptVector.
     */
    unsigned length() const { return length_; }

    /** \brief Returns the JavaScript reference to this JavaScriptVector.
     *
     * In order to get a valid JavaScript reference, this vector
     * should have been added to a WGLWidget.
     */
    std::string jsRef() const
    {
      if (!hasContext())
	throw WException("JavaScriptVector: does not belong to a "
			 "WGLWidget yet");
      return jsRef_;
    }

    /** \brief Returns the current server-side value.
     *
     * Client-side changes to the JavaScriptVector are automatically
     * synchronized.
     */
    std::vector<float> value() const;

#ifdef WT_TARGET_JAVA
    JavaScriptVector clone() const;
#endif
  private:
    void assignToContext(int id, const WGLWidget* context);
    void initialize() { initialized_ = true; }
    int id_;
    unsigned length_;
    std::string jsRef_;
    const WGLWidget* context_;
    bool initialized_;

    friend class WGLWidget;
    friend class WServerGLWidget;
    friend class WClientGLWidget;
  };

  /*! \brief A client-side JavaScript matrix
   *
   * A JavaScriptMatrix has methods that make it possible to do client-side
   * calculations on matrices.
   *
   * Using a JavaScriptMatrix4x4, GL parameters can be modified without
   * communication with the server. The value of the JavaScriptMatrix4x4
   * is updated server-side whenever an event is sent to the server.
   *
   * Important: only the jsRef() of the return value from a call to
   * WGLWidget::createJavaScriptMatrix() is a variable name that can be used
   * in custom JavaScript to modify a matrix from external scripts.
   * The jsRef() of return values of operations refer to unnamed temporary
   * objects - rvalues in C++-lingo.
   *
   * The JavaScriptMatrix4x4 is represented in JavaScript as an array of 16
   * elements. This array represents the values of the matrix in column-major
   * order. It is either a Float32Array or a plain JavaScript array.
   */
  class WT_API JavaScriptMatrix4x4 {
  public:
    /** \brief Creates a temporarily invalid JavaScriptMatrix4x4.
     *
     *  Should be added to a %WGLWidget with
     *  WGLWidget::addJavaScriptMatrix4, and initialized with
     *  WGLWidget::initJavaScriptMatrix4.
     */
    JavaScriptMatrix4x4();

#ifndef WT_TARGET_JAVA
    JavaScriptMatrix4x4(const JavaScriptMatrix4x4 &other);

    JavaScriptMatrix4x4 &operator=(const JavaScriptMatrix4x4 &rhs);
#endif

    int id() const { return id_; }

    /** \brief Returns whether this JavaScriptMatrix4x4 has been initialized.
     */
    bool initialized() const { return initialized_; }

    /** \brief Returns whether this JavaScriptMatrix4x4 has been assigned
     *         to a WGLWidget.
     */
    bool hasContext() const { return context_ != nullptr; }

    /** \brief Returns the JavaScript reference to this JavaScriptMatrix4x4.
     *
     * In order to get a valid JavaScript reference, this matrix should
     * have been added to a WGLWidget.
     */
    std::string jsRef() const
    {
      if (!hasContext())
	throw WException("JavaScriptMatrix4x4: does not belong to a WGLWidget yet");
      return jsRef_;
    }

    /** \brief Returns the current server-side value.
     *
     * Client-side changes to the JavaScriptMatrix4x4 are automatically
     * synchronized.
     */
    WMatrix4x4 value() const;

    JavaScriptMatrix4x4 inverted() const;
    JavaScriptMatrix4x4 transposed() const;

#if !defined(WT_TARGET_JAVA)
    JavaScriptMatrix4x4 operator*(const WGenericMatrix<double, 4, 4> &m) const;
#else
    JavaScriptMatrix4x4 multiply(const WGenericMatrix<MatrixType, 4, 4> &m) const;
#endif

#ifdef WT_TARGET_JAVA
    JavaScriptMatrix4x4 clone() const;
#endif

  private:
    void assignToContext(int i, const WGLWidget* context);
    void initialize() { initialized_ = true; }
    bool hasOperations() const { return operations_.size() > 0;}

    // void invert();
    // void transpose();
    // void mul(const WGenericMatrix<double, 4, 4> &m);

    int id_;
    std::string jsRef_;
    const WGLWidget* context_;

    enum class op {TRANSPOSE, INVERT, MULTIPLY};
    std::vector<op> operations_;
    std::vector< WGenericMatrix<double, 4, 4> > matrices_;

    bool initialized_;

    friend class WGLWidget;
    friend class WServerGLWidget;
    friend class WClientGLWidget;
  };

  /*! \brief Construct a GL widget.
   *
   * Before the first rendering, you must apply a size to the WGLWidget.
   */
  WGLWidget();

  /*! \brief Destructor
   */
  ~WGLWidget();

  /*! \brief Sets the rendering option.
   *
   * Use this method to configure whether client-side and/or
   * server-side rendering can be used, and whether anti-aliasing
   * should be enabled. The actual choice is also based
   * on availability (respectively client-side or server-side).
   *
   * The default value is to try both ClientSide or ServerSide rendering,
   * and to enable anti-aliasing if available.
   *
   * \note Options must be set before the widget is being rendered.
   */
  void setRenderOptions(WFlags<GLRenderOption> options);

protected:

  /*! \brief Initialize the GL state when the widget is first shown.
   *
   * initializeGL() is called when the widget is first rendered, and
   * when the webglcontextrestored signal is fired. You can distinguish
   * between the first initialization and context restoration using
   * restoringContext().
   * It usually creates most of the GL related state: shaders, VBOs,
   * uniform locations, ...
   *
   * If this state is to be updated during the lifetime of the widget,
   * you should specialize the updateGL() to accomodate for this.
   */
  virtual void initializeGL();

  /*! \brief Act on resize events
   *
   * Usually, this method only contains functions to set the viewport
   * and the projection matrix (as this is aspect ration dependent).
   *
   * resizeGL() is rendered after initializeGL, and whenever widget is
   * resized. After this method finishes, the widget is repainted with
   * the cached client-side paint function.
   */
  virtual void resizeGL(int width, int height);

  /*! \brief Update the client-side painting function.
   *
   * This method is invoked client-side when a repaint is required,
   * i.e. when the repaintSlot() (a JavaScript-side JSlot) is triggered.
   * Typical examples are: after mouse-based camera movements, after
   * a timed update of a camera or an object's position, after
   * a resize event (resizeGL() will also be called then), after
   * an animation event, ... In many cases, this function will be
   * executed client-side many many times.
   *
   * Using the GL functions from this class, you construct a scene.
   * The implementation tracks all JavaScript calls that need to be
   * performed to draw the scenes, and will replay them verbatim on
   * every trigger of the repaintSlot(). There are a few mechanisms
   * that may be employed to change what is rendered without updating
   * the paintGL() cache:
   * <ul>
   * <li>Client-side matrices may be used to change camera viewpoints,
   *     manipilate separate object's model transformation matrices, ...
   *     </li>
   * <li>Shader sources can be updated without requiring the paint
   *     function to be renewed</li>
   * </ul>
   *
   * Updating the paintGL() cache is usually not too expensive; the VBOs,
   * which are large in many cases, are already at the client side, while
   * the paintGL() code only draws the VBOs. Of course, if you have to
   * draw many separate objects, the paintGL() JS code may become large
   * and updating is more expensive.
   *
   * In order to update the paintGL() cache, call repaintGL() with
   * the PAINT_GL parameter, which will cause the invocation of this
   * method.
   */
  virtual void paintGL();

  /*! \brief Update state set in initializeGL()
   *
   * Invoked when repaint is called with the UPDATE_GL call.
   *
   * This is intended to be executed when you want to change programs,
   * 'constant' uniforms, or even VBO's, ... without resending already
   * initialized data. It is a mechanism to make changes to what you've
   * set in intializeGL(). For every server-side invocation of this method,
   * the result will be rendered client-side exactly once.
   */
  virtual void updateGL();

public:

  /*! \brief Request invocation of resizeGL, paintGL and/or updateGL.
   *
   * If invoked with PAINT_GL, the client-side cached paint function
   * is updated. If invoked with RESIZE_GL or UPDATE_GL, the code
   * will be executed once.
   *
   * If invoked with multiple flags set, the order of execution will be
   * updateGL(), resizeGL(), paintGL().
   */
  void repaintGL(WFlags<GLClientSideRenderer> which);

  /*! \brief Returns whether a lost context is in the process of being restored.
   *
   * You can check for this in initializeGL(), to handle the first
   * initialization and restoration of context differently.
   */
  bool restoringContext() const { return restoringContext_; }

  void resize(const WLength &width, const WLength &height) override;

  /*! \brief The enormous GLenum
   *
   * This enum contains all numeric constants defined by the WebGL
   * standard, see:
   * http://www.khronos.org/registry/webgl/specs/latest/1.0/#WEBGLRENDERINGCONTEXT
   */
  enum GLenum {
    /* ClearBufferMask */
    DEPTH_BUFFER_BIT               = 0x00000100,
    STENCIL_BUFFER_BIT             = 0x00000400,
    COLOR_BUFFER_BIT               = 0x00004000,
    
    /* BeginMode */
    POINTS                         = 0x0000,
    LINES                          = 0x0001,
    LINE_LOOP                      = 0x0002,
    LINE_STRIP                     = 0x0003,
    TRIANGLES                      = 0x0004,
    TRIANGLE_STRIP                 = 0x0005,
    TRIANGLE_FAN                   = 0x0006,
    
    /* AlphaFunction (not supported in ES20) */
    /*      NEVER */
    /*      LESS */
    /*      EQUAL */
    /*      LEQUAL */
    /*      GREATER */
    /*      NOTEQUAL */
    /*      GEQUAL */
    /*      ALWAYS */
    
    /* BlendingFactorDest */
    ZERO                           = 0x0,
    ONE                            = 0x1,
    SRC_COLOR                      = 0x0300,
    ONE_MINUS_SRC_COLOR            = 0x0301,
    SRC_ALPHA                      = 0x0302,
    ONE_MINUS_SRC_ALPHA            = 0x0303,
    DST_ALPHA                      = 0x0304,
    ONE_MINUS_DST_ALPHA            = 0x0305,
    
    /* BlendingFactorSrc */
    /*      ZERO */
    /*      ONE */
    DST_COLOR                      = 0x0306,
    ONE_MINUS_DST_COLOR            = 0x0307,
    SRC_ALPHA_SATURATE             = 0x0308,
    /*      SRC_ALPHA */
    /*      ONE_MINUS_SRC_ALPHA */
    /*      DST_ALPHA */
    /*      ONE_MINUS_DST_ALPHA */
    
    /* BlendEquationSeparate */
    FUNC_ADD                       = 0x8006,
    BLEND_EQUATION                 = 0x8009,
    BLEND_EQUATION_RGB             = 0x8009,   /* same as BLEND_EQUATION */
    BLEND_EQUATION_ALPHA           = 0x883D,
    
    /* BlendSubtract */
    FUNC_SUBTRACT                  = 0x800A,
    FUNC_REVERSE_SUBTRACT          = 0x800B,
    
    /* Separate Blend Functions */
    BLEND_DST_RGB                  = 0x80C8,
    BLEND_SRC_RGB                  = 0x80C9,
    BLEND_DST_ALPHA                = 0x80CA,
    BLEND_SRC_ALPHA                = 0x80CB,
    CONSTANT_COLOR                 = 0x8001,
    ONE_MINUS_CONSTANT_COLOR       = 0x8002,
    CONSTANT_ALPHA                 = 0x8003,
    ONE_MINUS_CONSTANT_ALPHA       = 0x8004,
    BLEND_COLOR                    = 0x8005,
    
    /* Buffer Objects */
    ARRAY_BUFFER                   = 0x8892,
    ELEMENT_ARRAY_BUFFER           = 0x8893,
    ARRAY_BUFFER_BINDING           = 0x8894,
    ELEMENT_ARRAY_BUFFER_BINDING   = 0x8895,
    
    STREAM_DRAW                    = 0x88E0,
    STATIC_DRAW                    = 0x88E4,
    DYNAMIC_DRAW                   = 0x88E8,
    
    BUFFER_SIZE                    = 0x8764,
    BUFFER_USAGE                   = 0x8765,
    
    CURRENT_VERTEX_ATTRIB          = 0x8626,
    
    /* CullFaceMode */
    FRONT                          = 0x0404,
    BACK                           = 0x0405,
    FRONT_AND_BACK                 = 0x0408,
    
    /* DepthFunction */
    /*      NEVER */
    /*      LESS */
    /*      EQUAL */
    /*      LEQUAL */
    /*      GREATER */
    /*      NOTEQUAL */
    /*      GEQUAL */
    /*      ALWAYS */
    
    /* EnableCap */
    /* TEXTURE_2D */
    CULL_FACE                      = 0x0B44,
    BLEND                          = 0x0BE2,
    DITHER                         = 0x0BD0,
    STENCIL_TEST                   = 0x0B90,
    DEPTH_TEST                     = 0x0B71,
    SCISSOR_TEST                   = 0x0C11,
    POLYGON_OFFSET_FILL            = 0x8037,
    SAMPLE_ALPHA_TO_COVERAGE       = 0x809E,
    SAMPLE_COVERAGE                = 0x80A0,
    
    /* ErrorCode */
    NO_ERROR                       = 0x0,
    INVALID_ENUM                   = 0x0500,
    INVALID_VALUE                  = 0x0501,
    INVALID_OPERATION              = 0x0502,
    OUT_OF_MEMORY                  = 0x0505,
    
    /* FrontFaceDirection */
    CW                             = 0x0900,
    CCW                            = 0x0901,
    
    /* GetPName */
    LINE_WIDTH                     = 0x0B21,
    ALIASED_POINT_SIZE_RANGE       = 0x846D,
    ALIASED_LINE_WIDTH_RANGE       = 0x846E,
    CULL_FACE_MODE                 = 0x0B45,
    FRONT_FACE                     = 0x0B46,
    DEPTH_RANGE                    = 0x0B70,
    DEPTH_WRITEMASK                = 0x0B72,
    DEPTH_CLEAR_VALUE              = 0x0B73,
    DEPTH_FUNC                     = 0x0B74,
    STENCIL_CLEAR_VALUE            = 0x0B91,
    STENCIL_FUNC                   = 0x0B92,
    STENCIL_FAIL                   = 0x0B94,
    STENCIL_PASS_DEPTH_FAIL        = 0x0B95,
    STENCIL_PASS_DEPTH_PASS        = 0x0B96,
    STENCIL_REF                    = 0x0B97,
    STENCIL_VALUE_MASK             = 0x0B93,
    STENCIL_WRITEMASK              = 0x0B98,
    STENCIL_BACK_FUNC              = 0x8800,
    STENCIL_BACK_FAIL              = 0x8801,
    STENCIL_BACK_PASS_DEPTH_FAIL   = 0x8802,
    STENCIL_BACK_PASS_DEPTH_PASS   = 0x8803,
    STENCIL_BACK_REF               = 0x8CA3,
    STENCIL_BACK_VALUE_MASK        = 0x8CA4,
    STENCIL_BACK_WRITEMASK         = 0x8CA5,
    VIEWPORT                       = 0x0BA2,
    SCISSOR_BOX                    = 0x0C10,
    /*      SCISSOR_TEST */
    COLOR_CLEAR_VALUE              = 0x0C22,
    COLOR_WRITEMASK                = 0x0C23,
    UNPACK_ALIGNMENT               = 0x0CF5,
    PACK_ALIGNMENT                 = 0x0D05,
    MAX_TEXTURE_SIZE               = 0x0D33,
    MAX_VIEWPORT_DIMS              = 0x0D3A,
    SUBPIXEL_BITS                  = 0x0D50,
    RED_BITS                       = 0x0D52,
    GREEN_BITS                     = 0x0D53,
    BLUE_BITS                      = 0x0D54,
    ALPHA_BITS                     = 0x0D55,
    DEPTH_BITS                     = 0x0D56,
    STENCIL_BITS                   = 0x0D57,
    POLYGON_OFFSET_UNITS           = 0x2A00,
    /*      POLYGON_OFFSET_FILL */
    POLYGON_OFFSET_FACTOR          = 0x8038,
    TEXTURE_BINDING_2D             = 0x8069,
    SAMPLE_BUFFERS                 = 0x80A8,
    SAMPLES                        = 0x80A9,
    SAMPLE_COVERAGE_VALUE          = 0x80AA,
    SAMPLE_COVERAGE_INVERT         = 0x80AB,
    
    /* GetTextureParameter */
    /*      TEXTURE_MAG_FILTER */
    /*      TEXTURE_MIN_FILTER */
    /*      TEXTURE_WRAP_S */
    /*      TEXTURE_WRAP_T */
    
    NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2,
    COMPRESSED_TEXTURE_FORMATS     = 0x86A3,
    
    /* HintMode */
    DONT_CARE                      = 0x1100,
    FASTEST                        = 0x1101,
    NICEST                         = 0x1102,
    
    /* HintTarget */
    GENERATE_MIPMAP_HINT            = 0x8192,
    
    /* DataType */
    BYTE                           = 0x1400,
    UNSIGNED_BYTE                  = 0x1401,
    SHORT                          = 0x1402,
    UNSIGNED_SHORT                 = 0x1403,
    INT                            = 0x1404,
    UNSIGNED_INT                   = 0x1405,
    FLOAT                          = 0x1406,
    
    /* PixelFormat */
    DEPTH_COMPONENT                = 0x1902,
    ALPHA                          = 0x1906,
    RGB                            = 0x1907,
    RGBA                           = 0x1908,
    LUMINANCE                      = 0x1909,
    LUMINANCE_ALPHA                = 0x190A,
    
    /* PixelType */
    /*      UNSIGNED_BYTE */
    UNSIGNED_SHORT_4_4_4_4         = 0x8033,
    UNSIGNED_SHORT_5_5_5_1         = 0x8034,
    UNSIGNED_SHORT_5_6_5           = 0x8363,
    
    /* Shaders */
    FRAGMENT_SHADER                  = 0x8B30,
    VERTEX_SHADER                    = 0x8B31,
    MAX_VERTEX_ATTRIBS               = 0x8869,
    MAX_VERTEX_UNIFORM_VECTORS       = 0x8DFB,
    MAX_VARYING_VECTORS              = 0x8DFC,
    MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D,
    MAX_VERTEX_TEXTURE_IMAGE_UNITS   = 0x8B4C,
    MAX_TEXTURE_IMAGE_UNITS          = 0x8872,
    MAX_FRAGMENT_UNIFORM_VECTORS     = 0x8DFD,
    SHADER_TYPE                      = 0x8B4F,
    DELETE_STATUS                    = 0x8B80,
    LINK_STATUS                      = 0x8B82,
    VALIDATE_STATUS                  = 0x8B83,
    ATTACHED_SHADERS                 = 0x8B85,
    ACTIVE_UNIFORMS                  = 0x8B86,
    ACTIVE_UNIFORM_MAX_LENGTH        = 0x8B87,
    ACTIVE_ATTRIBUTES                = 0x8B89,
    ACTIVE_ATTRIBUTE_MAX_LENGTH      = 0x8B8A,
    SHADING_LANGUAGE_VERSION         = 0x8B8C,
    CURRENT_PROGRAM                  = 0x8B8D,
    
    /* StencilFunction */
    NEVER                          = 0x0200,
    LESS                           = 0x0201,
    EQUAL                          = 0x0202,
    LEQUAL                         = 0x0203,
    GREATER                        = 0x0204,
    NOTEQUAL                       = 0x0205,
    GEQUAL                         = 0x0206,
    ALWAYS                         = 0x0207,
    
    /* StencilOp */
    /*      ZERO */
    KEEP                           = 0x1E00,
    REPLACE                        = 0x1E01,
    INCR                           = 0x1E02,
    DECR                           = 0x1E03,
    INVERT                         = 0x150A,
    INCR_WRAP                      = 0x8507,
    DECR_WRAP                      = 0x8508,
    
    /* StringName */
    VENDOR                         = 0x1F00,
    RENDERER                       = 0x1F01,
    VERSION                        = 0x1F02,
    
    /* TextureMagFilter */
    NEAREST                        = 0x2600,
    LINEAR                         = 0x2601,
    
    /* TextureMinFilter */
    /*      NEAREST */
    /*      LINEAR */
    NEAREST_MIPMAP_NEAREST         = 0x2700,
    LINEAR_MIPMAP_NEAREST          = 0x2701,
    NEAREST_MIPMAP_LINEAR          = 0x2702,
    LINEAR_MIPMAP_LINEAR           = 0x2703,
    
    /* TextureParameterName */
    TEXTURE_MAG_FILTER             = 0x2800,
    TEXTURE_MIN_FILTER             = 0x2801,
    TEXTURE_WRAP_S                 = 0x2802,
    TEXTURE_WRAP_T                 = 0x2803,
    
    /* TextureTarget */
    TEXTURE_2D                     = 0x0DE1,
    TEXTURE                        = 0x1702,
    
    TEXTURE_CUBE_MAP               = 0x8513,
    TEXTURE_BINDING_CUBE_MAP       = 0x8514,
    TEXTURE_CUBE_MAP_POSITIVE_X    = 0x8515,
    TEXTURE_CUBE_MAP_NEGATIVE_X    = 0x8516,
    TEXTURE_CUBE_MAP_POSITIVE_Y    = 0x8517,
    TEXTURE_CUBE_MAP_NEGATIVE_Y    = 0x8518,
    TEXTURE_CUBE_MAP_POSITIVE_Z    = 0x8519,
    TEXTURE_CUBE_MAP_NEGATIVE_Z    = 0x851A,
    MAX_CUBE_MAP_TEXTURE_SIZE      = 0x851C,
    
    /* TextureUnit */
    TEXTURE0                       = 0x84C0,
    TEXTURE1                       = 0x84C1,
    TEXTURE2                       = 0x84C2,
    TEXTURE3                       = 0x84C3,
    TEXTURE4                       = 0x84C4,
    TEXTURE5                       = 0x84C5,
    TEXTURE6                       = 0x84C6,
    TEXTURE7                       = 0x84C7,
    TEXTURE8                       = 0x84C8,
    TEXTURE9                       = 0x84C9,
    TEXTURE10                      = 0x84CA,
    TEXTURE11                      = 0x84CB,
    TEXTURE12                      = 0x84CC,
    TEXTURE13                      = 0x84CD,
    TEXTURE14                      = 0x84CE,
    TEXTURE15                      = 0x84CF,
    TEXTURE16                      = 0x84D0,
    TEXTURE17                      = 0x84D1,
    TEXTURE18                      = 0x84D2,
    TEXTURE19                      = 0x84D3,
    TEXTURE20                      = 0x84D4,
    TEXTURE21                      = 0x84D5,
    TEXTURE22                      = 0x84D6,
    TEXTURE23                      = 0x84D7,
    TEXTURE24                      = 0x84D8,
    TEXTURE25                      = 0x84D9,
    TEXTURE26                      = 0x84DA,
    TEXTURE27                      = 0x84DB,
    TEXTURE28                      = 0x84DC,
    TEXTURE29                      = 0x84DD,
    TEXTURE30                      = 0x84DE,
    TEXTURE31                      = 0x84DF,
    ACTIVE_TEXTURE                 = 0x84E0,
    
    /* TextureWrapMode */
    REPEAT                         = 0x2901,
    CLAMP_TO_EDGE                  = 0x812F,
    MIRRORED_REPEAT                = 0x8370,
    
    /* Uniform Types */
    FLOAT_VEC2                     = 0x8B50,
    FLOAT_VEC3                     = 0x8B51,
    FLOAT_VEC4                     = 0x8B52,
    INT_VEC2                       = 0x8B53,
    INT_VEC3                       = 0x8B54,
    INT_VEC4                       = 0x8B55,
    BOOL                           = 0x8B56,
    BOOL_VEC2                      = 0x8B57,
    BOOL_VEC3                      = 0x8B58,
    BOOL_VEC4                      = 0x8B59,
    FLOAT_MAT2                     = 0x8B5A,
    FLOAT_MAT3                     = 0x8B5B,
    FLOAT_MAT4                     = 0x8B5C,
    SAMPLER_2D                     = 0x8B5E,
    SAMPLER_CUBE                   = 0x8B60,
    
    /* Vertex Arrays */
    VERTEX_ATTRIB_ARRAY_ENABLED        = 0x8622,
    VERTEX_ATTRIB_ARRAY_SIZE           = 0x8623,
    VERTEX_ATTRIB_ARRAY_STRIDE         = 0x8624,
    VERTEX_ATTRIB_ARRAY_TYPE           = 0x8625,
    VERTEX_ATTRIB_ARRAY_NORMALIZED     = 0x886A,
    VERTEX_ATTRIB_ARRAY_POINTER        = 0x8645,
    VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,
    
    /* Shader Source */
    COMPILE_STATUS                 = 0x8B81,
    INFO_LOG_LENGTH                = 0x8B84,
    SHADER_SOURCE_LENGTH           = 0x8B88,
    
    /* Shader Precision-Specified Types */
    LOW_FLOAT                      = 0x8DF0,
    MEDIUM_FLOAT                   = 0x8DF1,
    HIGH_FLOAT                     = 0x8DF2,
    LOW_INT                        = 0x8DF3,
    MEDIUM_INT                     = 0x8DF4,
    HIGH_INT                       = 0x8DF5,
    
    /* Framebuffer Object. */
    FRAMEBUFFER                    = 0x8D40,
    RENDERBUFFER                   = 0x8D41,
    
    RGBA4                          = 0x8056,
    RGB5_A1                        = 0x8057,
    RGB565                         = 0x8D62,
    DEPTH_COMPONENT16              = 0x81A5,
    STENCIL_INDEX                  = 0x1901,
    STENCIL_INDEX8                 = 0x8D48,
    DEPTH_STENCIL                  = 0x84F9,
    
    RENDERBUFFER_WIDTH             = 0x8D42,
    RENDERBUFFER_HEIGHT            = 0x8D43,
    RENDERBUFFER_INTERNAL_FORMAT   = 0x8D44,
    RENDERBUFFER_RED_SIZE          = 0x8D50,
    RENDERBUFFER_GREEN_SIZE        = 0x8D51,
    RENDERBUFFER_BLUE_SIZE         = 0x8D52,
    RENDERBUFFER_ALPHA_SIZE        = 0x8D53,
    RENDERBUFFER_DEPTH_SIZE        = 0x8D54,
    RENDERBUFFER_STENCIL_SIZE      = 0x8D55,
    
    FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           = 0x8CD0,
    FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           = 0x8CD1,
    FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         = 0x8CD2,
    FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3,
    
    COLOR_ATTACHMENT0              = 0x8CE0,
    DEPTH_ATTACHMENT               = 0x8D00,
    STENCIL_ATTACHMENT             = 0x8D20,
    DEPTH_STENCIL_ATTACHMENT       = 0x821A,
    
    NONE                           = 0x0,
    
    FRAMEBUFFER_COMPLETE                      = 0x8CD5,
    FRAMEBUFFER_INCOMPLETE_ATTACHMENT         = 0x8CD6,
    FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7,
    FRAMEBUFFER_INCOMPLETE_DIMENSIONS         = 0x8CD9,
    FRAMEBUFFER_UNSUPPORTED                   = 0x8CDD,
    
    FRAMEBUFFER_BINDING            = 0x8CA6,
    RENDERBUFFER_BINDING           = 0x8CA7,
    MAX_RENDERBUFFER_SIZE          = 0x84E8,
    
    INVALID_FRAMEBUFFER_OPERATION  = 0x0506,
    
    /* WebGL-specific enums */
    UNPACK_FLIP_Y_WEBGL            = 0x9240,
    UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0x9241,
    CONTEXT_LOST_WEBGL             = 0x9242,
    UNPACK_COLORSPACE_CONVERSION_WEBGL = 0x9243,
    BROWSER_DEFAULT_WEBGL          = 0x9244,
  };
  
  void debugger();  

  /*! @name GL methods
   * The GL methods are mostly 1-on-1 translated to the identical
   * JavaScript call in WebGL. You can use the GL methods in your resizeGL(),
   * paintGL() and updateGL() specializations. Wt takes care that data,
   * arguments, ... are transfered to the client side and that the equivalent
   * JavaScript WebGL funtion is executed when using client-side rendering.
   * When using server-side rendering, the appropriate OpenGL functions
   * are called.
   * @{
   */


  /*! \brief GL function to activate an existing texture
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glActiveTexture.xml">
   * glActiveTexture() OpenGL ES manpage</a>
   */
  void activeTexture(GLenum texture);

  /*! \brief GL function to attach a shader to a program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glAttachShader.xml">
   * glAttachShader() OpenGL ES manpage</a>
   */
  void attachShader(Program program, Shader shader);

  /*! \brief GL function to bind an attribute to a given location
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBindAttribLocation.xml">
   * glBindAttribLocation() OpenGL ES manpage</a>
   */
  void bindAttribLocation(Program program, unsigned index,
                          const std::string &name);

  /*! \brief GL function to bind a buffer to a target
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBindBuffer.xml">
   * glBindBuffer() OpenGL ES manpage</a>
   */
  void bindBuffer(GLenum target, Buffer buffer);

  /*! \brief GL function to bind a frame buffer to a target
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBindFramebuffer.xml">
   * glBindFramebuffer() OpenGL ES manpage</a>
   */
  void bindFramebuffer(GLenum target, Framebuffer framebuffer);

  /*! \brief GL function to bind a render buffer to a target
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBindRenderbuffer.xml">
   * glBindRenderbuffer() OpenGL ES manpage</a>
   */
  void bindRenderbuffer(GLenum target, Renderbuffer renderbuffer);

  /*! \brief GL function to bind a texture to a target
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBindTexture.xml">
   * glBindTexture() OpenGL ES manpage</a>
   */
  void bindTexture(GLenum target, Texture texture);

  /*! \brief GL function to set the blending color
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBlendColor.xml">
   * glBlendColor() OpenGL ES manpage</a>
   */
  void blendColor(double red, double green, double blue, double alpha);

  /*! \brief GL function to set the blending equation
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBlendEquation.xml">
   * glBlendEquation() OpenGL ES manpage</a>
   */
  void blendEquation(GLenum mode);

  /*! \brief GL function that sets separate blending functions for RGB and alpha
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBlendEquationSeparate.xml">
   * glBlendEquationSeparate() OpenGL ES manpage</a>
   */
  void blendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

  /*! \brief GL function to configure the blending function
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBlendFunc.xml">
   * glBlendFunc() OpenGL ES manpage</a>
   */
  void blendFunc(GLenum sfactor, GLenum dfactor);

  /*! \brief GL function that configures the blending function
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBlendFuncSeparate.xml">
   * glBlendFuncSeparate() OpenGL ES manpage</a>
   */
  void blendFuncSeparate(GLenum srcRGB, GLenum dstRGB, 
                         GLenum srcAlpha, GLenum dstAlpha);

  /*! \brief glBufferData - create and initialize a buffer object's data store
   *
   * Set the size of the currently bound WebGLBuffer object for the passed target.
   * The buffer is initialized to 0.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   */
  void bufferData(GLenum target, int size, GLenum usage);

  /*! \brief glBufferData - create and initialize a buffer object's data store
   *         from an ArrayBuffer
   *
   * Set the size and contents of the currently bound WebGLBuffer object to
   * be a copy of the given ArrayBuffer.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   *
   * Note: an ArrayBuffer refers to a javascript object, which cannot be
   * used for server-side rendering. If a server-side fallback will be used,
   * then bufferDatafv() should be used with the additional boolean argument to
   * indicate binary transfer of the data in case of client-side rendering.
   *
   * \sa createAndLoadArrayBuffer
   */
  void bufferData(GLenum target, ArrayBuffer res, GLenum usage);

  /*! \brief glBufferData - create and initialize a buffer object's data store
   *         from an ArrayBuffer
   *
   * Set the size of the currently bound WebGLBuffer object to
   * arrayBufferSize, and copy the contents of the ArrayBuffer to
   * the buffer, starting at the given offset.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   *
   * Note: not functional for a server-side fall-back (see bufferData(GLenum target, ArrayBuffer res, GLenum usage) for more info)
   *
   * \sa createAndLoadArrayBuffer
   */
  void bufferData(GLenum target, ArrayBuffer res, unsigned arrayBufferOffset,
                  unsigned arrayBufferSize, GLenum usage);

  /*! \brief Initialize a buffer object's data store from an ArrayBuffer
   *
   * Load the data of the currently bound WebGLBuffer object from
   * the given ArrayBuffer. The first byte of the resource data will
   * be written at the given offset of the currently bound buffer.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   *
   * Note: not functional for a server-side fall-back (see bufferData(GLenum target, ArrayBuffer res, GLenum usage) for more info)
   *
   * \sa createAndLoadArrayBuffer
   */
  void bufferSubData(GLenum target, unsigned offset, ArrayBuffer res);

  /*! \brief Initialize a buffer object's data store from an ArrayBuffer
   *
   * Load the data of the currently bound WebGLBuffer object from
   * the given ArrayBuffer. The byte at position arrayBufferOffset
   * will be written to the currently bound buffer at position offset.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   *
   * Note: not functional for a server-side fall-back (see bufferData(GLenum target, ArrayBuffer res, GLenum usage) for more info)
   *
   * \sa createAndLoadArrayBuffer
   */
  void bufferSubData(GLenum target, unsigned offset, ArrayBuffer res,
                     unsigned arrayBufferOffset, unsigned size);

#if !defined(WT_TARGET_JAVA)
  /*! \brief GL function that loads float or double data in a VBO
   *
   * Unlike the C version, we can't accept a void * here. We must be able
   * to interpret the buffer's data in order to transmit it to the JS side.
   *
   * Later we may also want versions with strides and offsets to cope with
   * more complex buffer layouts that we typically see on desktop WebGL apps;
   * suggestions to improve this are welcome
   *
   * Note: prefer bufferDatafv(GLenum target, const std::vector<float> &buffer, GLenum usage, bool binary)
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   */
  template<typename Iterator>
    void bufferDatafv(GLenum target, const Iterator begin, const Iterator end, GLenum usage, bool binary = false) {
    std::vector<float> data;
    data.reserve(end-begin);
    for (Iterator i = begin; i != end; ++i) {
      data.push_back((float)*i);
    }

    bufferDatafv(target, data, usage, binary);
  }

  /*! \brief GL function that loads integer data in a VBO
   *
   * Note: prefer bufferDataiv(GLenum target, std::vector<int> &buffer, GLenum usage, GLenum type)
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   */
  template<typename Iterator>
  void bufferDataiv(GLenum target, const Iterator begin, const Iterator end,
                    GLenum usage, GLenum type)
  {
    std::vector<int> data;
    data.reserve(end-begin);
    for (Iterator i = begin; i != end; i++)
      data.push_back(*i);

    bufferDataiv(target, data, usage, type);
  }

  /*! \brief GL function that updates an existing VBO with new float or double
   * data
   *
   * Note: prefer bufferSubDatafv(GLenum target, unsigned offset, const std::vector<float> &buffer, bool binary)
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   */
  template<typename Iterator>
  void bufferSubDatafv(GLenum target, unsigned offset, const Iterator begin, const Iterator end, bool binary = false)
  {
    std::vector<float> data;
    data.reserve(end-begin);
    for (Iterator i = begin; i != end; ++i)
      data.push_back(*i);
    
    bufferSubDatafv(target, offset, data, binary);
  }

  /*! \brief GL function that updates an existing VBO with new integer data
   *
   * Note: prefer void bufferSubDataiv(GLenum target, unsigned offset, std::vector<int> &buffer, GLenum type)
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   */
  template<typename Iterator>
  void bufferSubDataiv(GLenum target, unsigned offset, const Iterator begin, Iterator end, GLenum type)
  {
    std::vector<int> data;
    data.reserve(end-begin);
    for (Iterator i = begin; i != end; i++)
      data.push_back(*i);

    bufferSubDataiv(target, offset, data, type);
  }
#endif

  /*! \brief GL function that loads float or double data in a VBO
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   */
#ifdef WT_TARGET_JAVA
  void bufferDatafv(GLenum target, const FloatBuffer &buffer, GLenum usage, bool binary = false);
  void bufferDatafv(GLenum target, const FloatNotByteBuffer &buffer, GLenum usage);
#else
  void bufferDatafv(GLenum target, const std::vector<float> &buffer, GLenum usage, bool binary = false);
#endif

  /*! \brief remove all binary buffer resources
   * 
   * Removes all WMemoryResources that were allocated when calling
   * bufferDatafv with binary=true. This is not required, since the resources
   * are also managed, but if you are sure they will not be used anymore in
   * the client, this can help free some memory.
   */
  void clearBinaryResources();

  /*! \brief GL function that updates an existing VBO with new integer data
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/glBufferData.xml">
   * glBufferData() OpenGL ES manpage</a>
   */
#ifdef WT_TARGET_JAVA
  void bufferDataiv(GLenum target, IntBuffer &buffer, GLenum usage,
		    GLenum type);
#else
  void bufferDataiv(GLenum target, std::vector<int> &buffer, GLenum usage,
		    GLenum type);
#endif
  
  /*! \brief GL function that updates an existing VBO with new float data
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   */
#ifdef WT_TARGET_JAVA
  void bufferSubDatafv(GLenum target, unsigned offset,
		       const FloatBuffer &buffer, bool binary = false);

  void bufferSubDatafv(GLenum target, unsigned offset,
		       const FloatNotByteBuffer &buffer);
#else
  void bufferSubDatafv(GLenum target, unsigned offset,
		       const std::vector<float> &buffer, bool binary = false);
#endif

  /*! \brief GL function that loads integer data in a VBO
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/glBufferSubData.xml">
   * glBufferSubData() OpenGL ES manpage</a>
   */
#ifdef WT_TARGET_JAVA
  void bufferSubDataiv(GLenum target,
		       unsigned offset, IntBuffer &buffer, 
		       GLenum type);
#else
  void bufferSubDataiv(GLenum target,
		       unsigned offset, std::vector<int> &buffer, 
		       GLenum type);
#endif

  //GLenum checkFramebufferStatus(GLenum target);

  /*! \brief GL function that clears the given buffers
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glClear.xml">
   * glClear() OpenGL ES manpage</a>
   */
  void clear(WFlags<GLenum> mask);

  /*! \brief GL function that sets the clear color of the color buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glClearColor.xml">
   * glClearColor() OpenGL ES manpage</a>
   */
  void clearColor(double r, double g, double b, double a);

  /*! \brief GL function that configures the depth to be set when the
   * depth buffer is cleared
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glClearDepthf.xml">
   * glClearDepthf() OpenGL ES manpage</a>
   */
  void clearDepth(double depth);

  /*! \brief GL function
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glClearStencil.xml">
   * glClearStencil() OpenGL ES manpage</a>
   */
  void clearStencil(int s);

  /*! \brief GL function
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glColorMask.xml">
   * glColorMask() OpenGL ES manpage</a>
   */
  void colorMask(bool red, bool green, bool blue, bool alpha);

  /*! \brief GL function to compile a shader
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCompileShader.xml">
   * glCompileShader() OpenGL ES manpage</a>
   */
  void compileShader(Shader shader);

  /*! \brief GL function to copy a texture image
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCopyTexImage2D.xml">
   * glCopyTexImage2D() OpenGL ES manpage</a>
   */
  void copyTexImage2D(GLenum target, int level,
                      GLenum internalformat,
                      int x, int y,
                      unsigned width, unsigned height, 
                      int border);

  /*! \brief GL function that copies a part of a texture image
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCopyTexSubImage2D.xml">
   * glCopyTexSubImage2D() OpenGL ES manpage</a>
   */
  void copyTexSubImage2D(GLenum target, int level,
                         int xoffset, int yoffset,
                         int x, int y,
                         unsigned width, unsigned height);

  /*! \brief GL function that creates an empty VBO
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenBuffers.xml">
   * glGenBuffers() OpenGL ES manpage</a>
   */
  Buffer createBuffer();

  /*! \brief GL function that creates a frame buffer object
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenFramebuffers.xml">
   * glGenFramebuffers() OpenGL ES manpage</a>
   */
  Framebuffer createFramebuffer();

  /*! \brief GL function that creates an empty program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCreateProgram.xml">
   * glCreateProgram() OpenGL ES manpage</a>
   */
  Program createProgram();

  /*! \brief GL function that creates a render buffer object
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenRenderbuffers.xml">
   * glGenRenderbuffers() OpenGL ES manpage</a>
   */
  Renderbuffer createRenderbuffer();

  /*! \brief GL function that creates an empty shader
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCreateShader.xml">
   * glCreateShader() OpenGL ES manpage</a>
   */
  Shader createShader(GLenum shader);

  /*! \brief GL function that creates an empty texture
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenTextures.xml">
   * glGenTextures() OpenGL ES manpage</a>
   */
  Texture createTexture();

  /*! \brief GL function that creates an image texture
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenTextures.xml">
   * glGenTextures() OpenGL ES manpage</a>
   */
  Texture createTextureAndLoad(const std::string &url);

  /*! \brief returns an paintdevice that can be used to paint a GL texture
   * 
   * If the client has a webGL enabled browser this function returns a
   * WCanvasPaintDevice.
   *
   * \if cpp
   * If server-side rendering is used as fallback then
   * this function returns a WRasterImage.
   * \endif
   * \if java
   * If server-side rendering is used as fallback then
   * this function returns a {@link WRasterPaintDevice}.
   * \endif
   */
  std::unique_ptr<WPaintDevice> createPaintDevice(const WLength& width,
						  const WLength& height);

  /*! \brief GL function that configures the backface culling mode
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glCullFace.xml">
   * glCullFace() OpenGL ES manpage</a>
   */
  void cullFace(GLenum mode);

  /*! \brief GL function that deletes a VBO
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteBuffers.xml">
   * glDeleteBuffers() OpenGL ES manpage</a>
   */
  void deleteBuffer(Buffer buffer);

  /*! \brief GL function that deletes a frame buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteFramebuffers.xml">
   * glDeleteFramebuffers() OpenGL ES manpage</a>
   */
  void deleteFramebuffer(Framebuffer framebuffer);

  /*! \brief GL function that deletes a program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteProgram.xml">
   * glDeleteProgram() OpenGL ES manpage</a>
   */
  void deleteProgram(Program program);

  /*! \brief GL function that deletes a render buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteRenderbuffers.xml">
   * glDeleteRenderbuffers() OpenGL ES manpage</a>
   */
  void deleteRenderbuffer(Renderbuffer renderbuffer);

  /*! \brief GL function that depetes a shader
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteShader.xml">
   * glDeleteShader() OpenGL ES manpage</a>
   */
  void deleteShader(Shader shader);

  /*! \brief GL function that deletes a texture
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDeleteTextures.xml">
   * glDeleteTextures() OpenGL ES manpage</a>
   */
  void deleteTexture(Texture texture);

  /*! \brief GL function to set the depth test function
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDepthFunc.xml">
   * glDepthFunc() OpenGL ES manpage</a>
   */
  void depthFunc(GLenum func);

  /*! \brief GL function that enables or disables writing to the depth buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDepthMask.xml">
   * glDepthMask() OpenGL ES manpage</a>
   */
  void depthMask(bool flag);

  /*! \brief GL function that specifies to what range the normalized [-1,1] z
   * values should match.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDepthRangef.xml">
   * glDepthRangef() OpenGL ES manpage</a>
   */
  void depthRange(double zNear, double zFar);

  /*! \brief GL function that detaches a shader from a program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDetachShader.xml">
   * glDetachShader() OpenGL ES manpage</a>
   */
  void detachShader(Program program, Shader shader);

  /*! \brief GL function to disable features
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDisable.xml">
   * glDisable() OpenGL ES manpage</a>
   */
  void disable(GLenum cap);

  /*! \brief GL function to disable the vertex attribute array
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDisableVertexAttribArray.xml">
   * glDisableVertexAttribArray() OpenGL ES manpage</a>
   */
  void disableVertexAttribArray(AttribLocation index);

  /*! \brief GL function to draw a VBO
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDrawArrays.xml">
   * glDrawArrays() OpenGL ES manpage</a>
   */
  void drawArrays(GLenum mode, int first, unsigned count);

  /*! \brief GL function to draw indexed VBOs
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glDrawElements.xml">
   * glDrawElements() OpenGL ES manpage</a>
   */
  void drawElements(GLenum mode, unsigned count, GLenum type, unsigned offset);

  /*! \brief GL function to enable features
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glEnable.xml">
   * glEnable() OpenGL ES manpage</a>
   */
  void enable(GLenum cap);

  /*! \brief GL function to enable the vertex attribute array
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glEnableVertexAttribArray.xml">
   * glEnableVertexAttribArray() OpenGL ES manpage</a>
   */
  void enableVertexAttribArray(AttribLocation index);

  /*! \brief GL function to wait until given commands are executed
   *
   * This call is transfered to JS, but the server will never wait on
   * this call.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glFinish.xml">
   * glFinish() OpenGL ES manpage</a>
   */
  void finish();

  /*! \brief GL function to force execution of GL commands in finite time.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glFlush.xml">
   * glFlush() OpenGL ES manpage</a>
   */
  void flush();

  /*! \brief GL function to attach the given renderbuffer to the currently
   *         bound frame buffer.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glFramebufferRenderbuffer.xml">
   * glFramebufferRenderbuffer() OpenGL ES manpage</a>
   */
  void framebufferRenderbuffer(GLenum target, GLenum attachment, 
                               GLenum renderbuffertarget, 
                               Renderbuffer renderbuffer);

  /*! \brief GL function to render directly into a texture image.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glFramebufferTexture2D.xml">
   * glFramebufferTexture2D() OpenGL ES manpage</a>
   */
  void framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, 
                            Texture texture, int level);

  /*! \brief GL function that specifies which side of a triangle is the
   * front side
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glFrontFace.xml">
   * glFrontFace() OpenGL ES manpage</a>
   */
  void frontFace(GLenum mode);

  /*! \brief GL function that generates a set of mipmaps for a texture
   * object.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGenerateMipmap.xml">
   * glGenerateMipmap() OpenGL ES manpage</a>
   */
  void generateMipmap(GLenum target);

  //WebGLActiveInfo getActiveAttrib(WebGLProgram program, GLuint index);
  //WebGLActiveInfo getActiveUniform(WebGLProgram program, GLuint index);
  //WebGLShader[ ] getAttachedShaders(WebGLProgram program);

  /*! \brief GL function to retrieve an attribute's location in a Program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGetAttribLocation.xml">
   * glGetAttribLocation() OpenGL ES manpage</a>
   */
  AttribLocation getAttribLocation(Program program, const std::string &attrib);

  //any getParameter(GLenum pname);
  //any getBufferParameter(GLenum target, GLenum pname);

  //GLenum getError();

  //any getFramebufferAttachmentParameter(GLenum target, GLenum attachment, 
  //                                      GLenum pname);
  //any getProgramParameter(WebGLProgram program, GLenum pname);
  //DOMString getProgramInfoLog(WebGLProgram program);
  //any getRenderbufferParameter(GLenum target, GLenum pname);
  //any getShaderParameter(WebGLShader shader, GLenum pname);
  //DOMString getShaderInfoLog(WebGLShader shader);

  //DOMString getShaderSource(WebGLShader shader);

  //any getTexParameter(GLenum target, GLenum pname);

  //any getUniform(WebGLProgram program, WebGLUniformLocation location);

  /*! \brief GL function to retrieve a Uniform's location in a Program.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glGetUniformLocation.xml">
   * glGetUniformLocation() OpenGL ES manpage</a>
   */
  UniformLocation getUniformLocation(Program program, const std::string &location);

  //any getVertexAttrib(GLuint index, GLenum pname);

  //GLsizeiptr getVertexAttribOffset(GLuint index, GLenum pname);

  /*! \brief GL function to give hints to the render pipeline
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glHint.xml">
   * glHint() OpenGL ES manpage</a>
   */
  void hint(GLenum target, GLenum mode);

  //GLboolean isBuffer(WebGLBuffer buffer);
  //GLboolean isEnabled(GLenum cap);
  //GLboolean isFramebuffer(WebGLFramebuffer framebuffer);
  //GLboolean isProgram(WebGLProgram program);
  //GLboolean isRenderbuffer(WebGLRenderbuffer renderbuffer);
  //GLboolean isShader(WebGLShader shader);
  //GLboolean isTexture(WebGLTexture texture);

  /*! \brief GL function to set the line width
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glLineWidth.xml">
   * glLineWidth() OpenGL ES manpage</a>
   */
  void lineWidth(double width);

  /*! \brief GL function to link a program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glLinkProgram.xml">
   * glLinkProgram() OpenGL ES manpage</a>
   */
  void linkProgram(Program program);

  /*! \brief GL function to set the pixel storage mode
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glPixelStorei.xml">
   * glPixelStorei() OpenGL ES manpage</a>
   */
  void pixelStorei(GLenum pname, int param);

  /*! \brief GL function to apply modifications to Z values
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glPolygonOffset.xml">
   * glPolygonOffset() OpenGL ES manpage</a>
   */
  void polygonOffset(double factor, double units);

  //void readPixels(GLint x, GLint y, GLsizei width, GLsizei height, 
  //                GLenum format, GLenum type, ArrayBufferView pixels);

  /*! \brief GL function to allocate the appropriate amount of memory for
   *         a render buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glRenderbufferStorage.xml">
   * glSampleCoverage() OpenGL ES manpage</a>
   */
  void renderbufferStorage(GLenum target, GLenum internalformat, 
                           unsigned width, unsigned height);

  /*! \brief GL function to set multisample parameters
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glSampleCoverage.xml">
   * glSampleCoverage() OpenGL ES manpage</a>
   */
  void sampleCoverage(double value, bool invert);

  /*! \brief GL function to define the scissor box
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glScissor.xml">
   * glScissor() OpenGL ES manpage</a>
   */
  void scissor(int x, int y, unsigned width, unsigned height);

  /*! \brief GL function to set a shader's source code
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glShaderSource.xml">
   * glShaderSource() OpenGL ES manpage</a>
   */
  void shaderSource(Shader shader, const std::string &src);

  /*! \brief GL function to set stencil test parameters
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilFunc.xml">
   * glStencilFunc() OpenGL ES manpage</a>
   */
  void stencilFunc(GLenum func, int ref, unsigned mask);

  /*! \brief GL function to set stencil test parameters for front and/or
   * back stencils
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilFuncSeparate.xml">
   * glStencilFuncSeparate() OpenGL ES manpage</a>
   */
  void stencilFuncSeparate(GLenum face, GLenum func, int ref, unsigned mask);

  /*! \brief GL function to control which bits are to be written in the stencil
   * buffer
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilMask.xml">
   * glStencilMask() OpenGL ES manpage</a>
   */
  void stencilMask(unsigned mask);

  /*! \brief GL function to control which bits are written to the front and/or
   * back stencil buffers
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilMaskSeparate.xml">
   * glStencilMaskSeparate() OpenGL ES manpage</a>
   */
  void stencilMaskSeparate(GLenum face, unsigned mask);

  /*! \brief GL function to set stencil test actions
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilOp.xml">
   * glStencilOp() OpenGL ES manpage</a>
   */
  void stencilOp(GLenum fail, GLenum zfail, GLenum zpass);

  /*! \brief GL function to set front and/or back stencil test actions
   * separately
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glStencilOpSeparate.xml">
   * glStencilOpSeparate() OpenGL ES manpage</a>
   */
  void stencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);

  //void texImage2D(TextureTargetEnum target, int level, PixelFormatEnum internalformat, 
  //                GLsizei width, GLsizei height, GLint border, PixelFormatEnum format, 
  //                GLenum type, ArrayBufferView pixels);
  /*! \brief GL function to reserve space for a 2D texture, without specifying its
   *         contents.
   *
   * This corresponds to calling the WebGL function
   * void texImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width,
   * GLsizei height, GLint border, GLenum format, GLenum type, ArrayBufferView pixels)
   * with null as last parameters. The value of 'type' is then of no importance and is
   * therefore omitted from this function.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   */
  void texImage2D(GLenum target, int level, GLenum internalformat, 
                  unsigned width, unsigned height, int border, GLenum format);

  //void texImage2D(TextureTargetEnum target, int level, PixelFormatEnum internalformat,
  //                PixelFormatEnum format, GLenum type, ImageData pixels);

  /*! \brief GL function to load a 2D texture from a WImage
   *
   * Note: WImage must be loaded before this function is executed.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   */
  void texImage2D(GLenum target, int level, GLenum internalformat,
                  GLenum format, GLenum type, WImage *image);

  //void texImage2D(TextureTargetEnum target, int level, PixelFormatEnum internalformat,
  //                PixelFormatEnum format, GLenum type, HTMLCanvasElement canvas);

  /*! \brief GL function to load a 2D texture from a WVideo
   *
   * Note: the video must be loaded prior to calling this function. The
   * current frame is used as texture image.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   */
  void texImage2D(GLenum target, int level, GLenum internalformat,
		  GLenum format, GLenum type, WVideo *video);

  /*! \brief GL function to load a 2D texture from a file
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   */
  void texImage2D(GLenum target, int level, GLenum internalformat,
                  GLenum format, GLenum type, std::string filename);

  /*! \brief GL function to load a 2D texture from a WPaintDevice
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   *
   * \sa createPaintDevice()
   */
  void texImage2D(GLenum target, int level, GLenum internalformat,
		  GLenum format, GLenum type,
		  WPaintDevice *paintdevice);

  /*! \brief GL function to load a 2D texture loaded with createTextureAndLoad()
   *
   * This function must only be used for textures created with
   * createTextureAndLoad()
   *
   * Note: the WGLWidget implementation will delay rendering until
   * all textures created with createTextureAndLoad() are loaded in the
   * browser.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexImage2D.xml">
   * glTexImage2D() OpenGL ES manpage</a>
   */
  void texImage2D(GLenum target, int level, GLenum internalformat,
                  GLenum format, GLenum type, Texture texture);

  //void texParameterf(TextureTargetEnum target,
  //                   TextureParameterNameEnum pname, double param);

  /*! \brief GL function to set texture parameters
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glTexParameter.xml">
   * glTexParameter() OpenGL ES manpage</a>
   */
  void texParameteri(GLenum target, GLenum pname, GLenum param);

  //void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, 
  //                   GLsizei width, GLsizei height, 
  //                   GLenum format, GLenum type, ArrayBufferView pixels);
  //void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, 
  //                   GLenum format, GLenum type, ImageData pixels);
  //void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, 
  //                   GLenum format, GLenum type, HTMLImageElement image);
  //void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, 
  //                   GLenum format, GLenum type, HTMLCanvasElement canvas);
  //void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, 
  //                   GLenum format, GLenum type, HTMLVideoElement video);


  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform1f(const UniformLocation &location, double x);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void uniform1fv(const UniformLocation &location, 
		  const WT_ARRAY FloatArray *value)
#ifndef WT_TARGET_JAVA
 {
    float out[1];
    for (int i=0; i<1; i++) {
      out[i] = (float)value[i];
    }

    uniform1fv(location, out);
  }

  /// \cond
  void uniform1fv(const UniformLocation &location, 
		  const WT_ARRAY float *value);
  /// \endcond
#else
  ;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform1fv(const UniformLocation &location,
		  const JavaScriptVector &v);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform1i(const UniformLocation &location, int x);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename IntArray)
  void uniform1iv(const UniformLocation &location, 
		  const WT_ARRAY IntArray *value)
#ifndef WT_TARGET_JAVA
{
    int out[1];
    for (int i=0; i<1; i++) {
      out[i] = (int)value[i];
    }

    uniform1iv(location, out);
  }

  /// \cond
  void uniform1iv(const UniformLocation &location, 
		  const WT_ARRAY int *value);
  /// \endcond
#else
  ;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform2f(const UniformLocation &location, double x, double y);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void uniform2fv(const UniformLocation &location, 
		  const WT_ARRAY FloatArray *value)
#ifndef WT_TARGET_JAVA
{
  float out[2];
  for (int i=0; i<2; i++) {
    out[i] = (float)value[i];
  }
  
  uniform2fv(location, out);
}

  /// \cond
void uniform2fv(const UniformLocation &location, 
		const WT_ARRAY float *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform2fv(const UniformLocation &location,
		  const JavaScriptVector &v);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform2i(const UniformLocation &location, int x, int y);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename IntArray)
  void uniform2iv(const UniformLocation &location, 
		  const WT_ARRAY IntArray *value)
#ifndef WT_TARGET_JAVA
  {
    int out[2];
    for (int i=0; i<2; i++) {
      out[i] = (int)value[i];
    }
    
    uniform2iv(location, out);
  }

/// \cond
void uniform2iv(const UniformLocation &location, 
		  const WT_ARRAY int *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform3f(const UniformLocation &location,
                 double x, double y, double z);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void uniform3fv(const UniformLocation &location, 
		  const WT_ARRAY FloatArray *value)
#ifndef WT_TARGET_JAVA
  {
    float out[3];
    for (int i=0; i<3; i++) {
      out[i] = (float)value[i];
    }
    
    uniform3fv(location, out);
  }

/// \cond
  void uniform3fv(const UniformLocation &location, 
		  const WT_ARRAY float *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform3fv(const UniformLocation &location,
		  const JavaScriptVector &v);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform3i(const UniformLocation &location, int x, int y, int z);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename IntArray)
  void uniform3iv(const UniformLocation &location, 
		  const WT_ARRAY IntArray *value)
#ifndef WT_TARGET_JAVA
  {
    int out[3];
    for (int i=0; i<3; i++) {
      out[i] = (int)value[i];
    }
    
    uniform3iv(location, out);
  }

/// \cond
  void uniform3iv(const UniformLocation &location, 
		  const WT_ARRAY int *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform4f(const UniformLocation &location,
                 double x, double y, double z, double w);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void uniform4fv(const UniformLocation &location, 
		  const WT_ARRAY FloatArray *value)
#ifndef WT_TARGET_JAVA
  {
    float out[4];
    for (int i=0; i<4; i++) {
      out[i] = (float)value[i];
    }
    
    uniform4fv(location, out);
  }

/// \cond
  void uniform4fv(const UniformLocation &location, 
		  const WT_ARRAY float *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform4fv(const UniformLocation &location,
		  const JavaScriptVector &v);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniform4i(const UniformLocation &location, int x, int y, int z, int w);

  /*! \brief GL function to set the value of a uniform variable of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename IntArray)
  void uniform4iv(const UniformLocation &location, 
		  const WT_ARRAY IntArray *value)
#ifndef WT_TARGET_JAVA
  {
    int out[4];
    for (int i=0; i<4; i++) {
      out[i] = (int)value[i];
    }
    
    uniform4iv(location, out);
  }

/// \cond
  void uniform4iv(const UniformLocation &location, 
		  const WT_ARRAY int *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * Attention: The OpenGL ES specification states that transpose MUST be
   * false.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix2fv(const UniformLocation &location,
                        bool transpose, 
			const WT_ARRAY MatrixType *value)
#ifndef WT_TARGET_JAVA
  {
    double out[4];
    for (int i=0; i<4; i++) {
      out[i] = (double)value[i];
    }
    uniformMatrix2fv(location, transpose, out);
  }

/// \cond
  void uniformMatrix2fv(const UniformLocation &location,
                        bool transpose, 
			const WT_ARRAY double *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * This function renders the matrix in the proper row/column order.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix2(const UniformLocation &location,
                      const WGenericMatrix<MatrixType, 2, 2> &m)
#ifndef WT_TARGET_JAVA
  {
    WGenericMatrix<double, 2, 2> out;
    for (int i=0; i<2; i++) {
      for (int j=0; j<2; j++) {
	out.at(i, j) = (double)m.at(i, j);
      }
    }
    uniformMatrix2(location, out);
  }

/// \cond
  void uniformMatrix2(const UniformLocation &location,
                      const WGenericMatrix<double, 2, 2> &m);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * Attention: The OpenGL ES specification states that transpose MUST be
   * false.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix3fv(const UniformLocation &location, bool transpose, 
			const WT_ARRAY MatrixType *value)
#ifndef WT_TARGET_JAVA
  {
    double out[9];
    for (int i=0; i<9; i++) {
      out[i] = (double)value[i];
    }
    uniformMatrix3fv(location, transpose, out);
  }

/// \cond
  void uniformMatrix3fv(const UniformLocation &location, bool transpose, 
			const WT_ARRAY double *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * This function renders the matrix in the proper row/column order.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix3(const UniformLocation &location,
		      const WGenericMatrix<MatrixType, 3, 3> &m)
#ifndef WT_TARGET_JAVA
  {
    WGenericMatrix<double, 3, 3> out;
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
	out.at(i, j) = (double)m.at(i, j);
      }
    }
    uniformMatrix3(location, out);
  }

/// \cond
  void uniformMatrix3(const UniformLocation &location,
		      const WGenericMatrix<double, 3, 3> &m);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * Attention: The OpenGL ES specification states that transpose MUST be
   * false.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix4fv(const UniformLocation &location, bool transpose,
                        const WT_ARRAY MatrixType *value)
#ifndef WT_TARGET_JAVA
  {
    double out[16];
    for (int i=0; i<16; i++) {
      out[i] = (double)value[i];
    }
    uniformMatrix4fv(location, transpose, out);
  }

/// \cond
  void uniformMatrix4fv(const UniformLocation &location, bool transpose,
                        const WT_ARRAY double *value);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * This function renders the matrix in the proper row/column order.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void uniformMatrix4(const UniformLocation &location,
                      const WGenericMatrix<MatrixType, 4, 4> &m)
#ifndef WT_TARGET_JAVA
  {
    WGenericMatrix<double, 4, 4> out;
    for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
	out.at(i, j) = (double)m.at(i, j);
      }
    }
    uniformMatrix4(location, out);
  }

/// \cond
  void uniformMatrix4(const UniformLocation &location,
                      const WGenericMatrix<double, 4, 4> &m);
/// \endcond
#else
;
#endif

  /*! \brief GL function to set the value of a uniform matrix of the current
   * program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUniform.xml">
   * glUniform() OpenGL ES manpage</a>
   */
  void uniformMatrix4(const UniformLocation &location,
                      const JavaScriptMatrix4x4 &m);


  /*! \brief GL function to set the current active shader program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glUseProgram.xml">
   * glUseProgram() OpenGL ES manpage</a>
   */
  void useProgram(Program program);

  /*! \brief GL function to validate a program
   *
   * implementation note: there is currently not yet a method to read
   * out the validation result.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glValidateProgram.xml">
   * glValidateProgram() OpenGL ES manpage</a>
   */
  void validateProgram(Program program);

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  void vertexAttrib1f(AttribLocation location, double x);

#ifndef WT_TARGET_JAVA
  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void vertexAttrib1fv(AttribLocation location, 
		       const WT_ARRAY FloatArray *values) {
    vertexAttrib1f(location, values[0]);
  }
#endif

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  void vertexAttrib2f(AttribLocation location, double x, double y);

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void vertexAttrib2fv(AttribLocation location, 
		       const WT_ARRAY FloatArray *values) {
    vertexAttrib2f(location, values[0], values[1]);
  }

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  void vertexAttrib3f(AttribLocation location, double x, double y, double z);

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void vertexAttrib3fv(AttribLocation location, 
		       const WT_ARRAY FloatArray *values) {
    vertexAttrib3f(location, values[0], values[1], values[2]);
  }

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  void vertexAttrib4f(AttribLocation location,
                      double x, double y, double z, double w);

  /*! \brief GL function to set the value of an attribute of the current program
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttrib.xml">
   * glVertexAttrib() OpenGL ES manpage</a>
   */
  WT_WGL_TEMPLATE(typename FloatArray)
  void vertexAttrib4fv(AttribLocation location, 
		       const WT_ARRAY FloatArray *values) {
    vertexAttrib4f(location, values[0], values[1], values[2], values[3]);
  }

  /*! \brief GL function to bind a VBO to an attribute
   *
   * This function links the given attribute to the VBO currently bound
   * to the ARRAY_BUFFER target.
   *
   * The size parameter specifies the number of components per attribute (1
   * to 4). The type parameter is also used to determine the size of each
   * component.
   *
   * The size of a float is 8 bytes.
   *
   * In WGLWidget, the size of an int is 4 bytes.
   *
   * The stride is in bytes.
   *
   * The maximum stride is 255.
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glVertexAttribPointer.xml">
   * glVertexAttribPointer() OpenGL ES manpage</a>
   */
  void vertexAttribPointer(AttribLocation location, int size,
    GLenum type, bool normalized, unsigned stride, unsigned offset);

  /*! \brief GL function to set the viewport
   *
   * <a href="http://www.khronos.org/opengles/sdk/2.0/docs/man/xhtml/glViewport.xml">
   * glViewport() OpenGL ES manpage</a>
   */
  void viewport(int x, int y, unsigned width, unsigned height);

  /*! @}
   */

  /*! @name Client-side vectors and matrices
   *
   * These methods can be used to modify
   * vectors and matrices client-side, to change
   * the GL uniforms without a roundtrip to the server.
   * @{
   */

  /*! \brief Create a matrix that can be manipulated in client-side
   * JavaScript
   *
   * This is a shorthand for creating a JavaScriptMatrix4x4,
   * then adding it to a WGLWidget with addJavaScriptMatrix4,
   * and initializing it with initJavaScriptMatrix4.
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  JavaScriptMatrix4x4 createJavaScriptMatrix4();

  /*! \brief Register a matrix with this WGLWidget.
   *
   * You can call this outside of resizeGL(), paintGL(), updateGL()
   * or initializeGL() methods. After a JavaScriptMatrix4x4 is
   * added to a WGLWidget, its jsRef() becomes valid, and can be
   * used in a JSlot, for example.
   */
  void addJavaScriptMatrix4(JavaScriptMatrix4x4 &m);

  /*! \brief Initialize the client-side JavaScript for the
   * given JavaScriptMatrix4x4
   *
   * If the given matrix is not associated with a widget yet,
   * it will be added to this widget.
   *
   * If the given matrix has already been added to a WGLWidget,
   * then this WGLWidget should be the same as the one you call
   * initJavaScriptMatrix4 on.
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  void initJavaScriptMatrix4(JavaScriptMatrix4x4 &m);

  /*! \brief Set the value of a client-side JavaScript matrix created by
   * createJavaScriptMatrix4x4()
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  WT_WGL_TEMPLATE(typename MatrixType)
  void setJavaScriptMatrix4(JavaScriptMatrix4x4 &jsm,
                            const WGenericMatrix<MatrixType, 4, 4> &m)
#ifndef WT_TARGET_JAVA
  {
    WGenericMatrix<double, 4, 4> out;
    for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
	out.at(i, j) = (double)m.at(i, j);
      }
    }
    setJavaScriptMatrix4(jsm, out);
  }

/// \cond
  void setJavaScriptMatrix4(JavaScriptMatrix4x4 &jsm,
                            const WGenericMatrix<double, 4, 4> &m);
/// \endcond
#else
;
#endif

  /*! \brief Create a vector of a certain length that can be manipulated
   * in client-side JavaScript
   *
   * This is a shorthand for creating a JavaScriptVector,
   * then adding it to a WGLWidget with addJavaScriptVector,
   * and initializing it with initJavaScriptVector.
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  JavaScriptVector createJavaScriptVector(unsigned length);

  /*! \brief Register a vector with this WGLWidget.
   *
   * You can call this outside of resizeGL(), paintGL(), updateGL()
   * or initializeGL() methods. After a JavaScriptVector is
   * added to a WGLWidget, its jsRef() becomes valid, and can be
   * used in a JSlot, for example.
   */
  void addJavaScriptVector(JavaScriptVector &v);

  /*! \brief Initialize the client-side JavaScript for the
   * given JavaScriptVector
   *
   * If the given vector is not associated with a widget yet,
   * it will be added to this widget.
   *
   * If the given vector has already been added to a WGLWidget,
   * then this WGLWidget should be the same as the one you call
   * initJavaScriptVector on.
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  void initJavaScriptVector(JavaScriptVector &v);

  /*! \brief Set the value of a client-side JavaScript vector created by
   * createJavaScriptVector()
   *
   * This method should only be called in initializeGL(),
   * updateGL() or resizeGL().
   */
  void setJavaScriptVector(JavaScriptVector &jsv,
			   const std::vector<float> &v);

  /*! @}
   */

  /*! \brief Set a custom mouse handler based on the given JavaScript code.
   *
   * The handler code should be JavaScript code that produces an object when evaluated.
   *
   * A mouse handler is an object that can implement one or more of the following functions:
   *
   * - <b>setTarget(target)</b>: This is called immediately when the mouse handler is added with an
   *                       object that uniquely identifies the WGLWidget, and a paintGL() method.
   * - <b>mouseDown(o, event)</b>: To handle the \c mousedown event. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c MouseEvent.
   * - <b>mouseUp(o, event)</b>: To handle the \c mouseup event. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c MouseEvent.
   * - <b>mouseDrag(o, event)</b>: Called when the mouse is dragged. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c MouseEvent.
   * - <b>mouseMove(o, event)</b>: Called when the mouse is moved. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c MouseEvent.
   * - <b>mouseWheel(o, event)</b>: Called when the mouse wheel is used. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c MouseEvent.
   * - <b>touchStart(o, event)</b>: To handle the \c touchstart event. \c o is the \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c TouchEvent.
   * - <b>touchEnd(o, event)</b>: To handle the \c touchend event. \c o is this \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c TouchEvent.
   * - <b>touchMoved(o, event)</b>: To handle the \c touchmove event. \c o is this \c &lt;canvas> (client-side rendering)
   *			    or \c &lt;img> (server-side rendering) element corresponding to this WGLWidget. \c event is
   *			    the \c TouchEvent.
   *
   *
   * For example, if we wanted to scale some object when we scroll, we could create
   * a JavaScriptMatrix4x4 called \p transform_:
   *
   * \if cpp
   * \code
private:
  JavaScriptMatrix4x4 transform_;
     \endcode
   * \endif
   *
   * \if java
   * \code
private JavaScriptMatrix4x4 transform_;
     \endcode
   * \endif
   *
   * We can add this in the constructor:
   *
   * \if cpp
   * \code
addJavaScriptMatrix4(transform_);
     \endcode
   * \endif
   *
   * \if java
   * \code
transform_ = new JavaScriptMatrix4x4();
addJavaScriptMatrix4(transform_);
     \endcode
   * \endif
   *
   * Then, in initializeGL(), we can initialize it and set the value:
   *
   * \if cpp
   * \code
initJavaScriptMatrix4(transform_);
setJavaScriptMatrix4(transform_, WMatrix4x4()); // Set to identity matrix
     \endcode
   * \endif
   *
   * \if java
   * \code
initJavaScriptMatrix4(transform_);
setJavaScriptMatrix4(transform_, new WMatrix4x4()); // Set to identity matrix
     \endcode
   * \endif
   *
   * Then, still in initializeGL(), we can set a mouse handler as such:
   *
   * \if cpp
   * \code
setClientSideMouseHandler(std::string("(function(){") +
  "var MouseHandler = function(transform) {"
    "var target = null;"
    "this.setTarget = function(newTarget) {"
      "target = newTarget;"
    "};"
    "this.mouseWheel = function(o, event) {"
      "var fix = jQuery.event.fix(event);"
      "fix.preventDefault();"
      "fix.stopPropagation();"
      "var d = wheelDelta(event);"
      "var s = Math.pow(1.2, d);"
      "transform[0] *= s;" // Scale X
      "transform[5] *= s;" // Scale Y
      "transform[10] *= s;" // Scale Z
      "target.paintGL();" // Repaint
    "};"
    "function wheelDelta(e) {"
      "var delta = 0;"
      "if (e.wheelDelta) {"
	"delta = e.wheelDelta > 0 ? 1 : -1;"
      "} else if (e.detail) {"
	"delta = e.detail < 0 ? 1 : -1;"
      "}"
      "return delta;"
    "}"
  "};"
  "return new MouseHandler(" + transform_.jsRef() + ");"
"})()");
     \endcode
   * \endif
   *
   * \if java
   * \code
setClientSideMouseHandler("(function(){") +
  "var MouseHandler = function(transform) {" +
    "var target = null;" +
    "this.setTarget = function(newTarget) {" +
      "target = newTarget;" +
    "};" +
    "this.mouseWheel = function(o, event) {" +
      "var fix = jQuery.event.fix(event);" +
      "fix.preventDefault();" +
      "fix.stopPropagation();" +
      "var d = wheelDelta(event);" +
      "var s = Math.pow(1.2, d);" +
      "transform[0] *= s;" + // Scale X
      "transform[5] *= s;" + // Scale Y
      "transform[10] *= s;" + // Scale Z
      "target.paintGL();" + // Repaint
    "};" +
    "function wheelDelta(e) {" +
      "var delta = 0;" +
      "if (e.wheelDelta) {" +
	"delta = e.wheelDelta > 0 ? 1 : -1;" +
      "} else if (e.detail) {" +
	"delta = e.detail < 0 ? 1 : -1;" +
      "}" +
      "return delta;" +
    "}" +
  "};" +
  "return new MouseHandler(" + transform_.jsRef() + ");" +
"})()");
     \endcode
   * \endif
   *
   * All that's left to do then is to use this transform somewhere as a uniform variable,
   * see getUniformLocation() and uniformMatrix4().
   */
  void setClientSideMouseHandler(const std::string& handlerCode);

  /*! \brief Add a mouse handler to the widget that looks at a given point
   *
   * This will allow a user to change client-side matrix m with the
   * mouse. M is a model transformation matrix, representing the viewpoint of
   * the camera.
   *
   * Through mouse operations, the camera can be changed by the user, but
   * (lX, lY, lZ) will always be at the center of the display, (uX, uY, uZ)
   * is considered to be the up direction, and the distance of the camera to
   * (lX, lY, lZ) will never change.
   *
   * Pressing the left mouse button and moving the mouse left/right will
   * rotate the camera around the up (uX, uY, uZ) direction. Moving up/down
   * will tilt the camera (causing it to move up/down to keep the lookpoint
   * centered). The scroll wheel simulates zooming by scaling the scene.
   *
   * pitchRate and yawRate control how much the camera will move per mouse
   * pixel.
   *
   * Usually this method is called after setting a camera transformation
   * with a client-side matrix in initializeGL(). However, this function
   * may also be called from outside the intializeGL()/paintGL()/updateGL()
   * methods (but not before m was initialized).
   */
  void setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
                                  double lX, double lY, double lZ,
                                  double uX, double uY, double uZ,
                                  double pitchRate, double yawRate);

  /*! \brief Add a mouse handler to the widget that allows 'walking' in
   * the scene
   *
   * This will allow a user to change client-side matrix m with the
   * mouse. M is a model transformation matrix, representing the viewpoint of
   * the camera.
   *
   * Through mouse operations, the camera can be changed by the user, as if
   * he is walking around on a plane. 
   *
   * Pressing the left mouse button and moving the mouse left/right will
   * rotate the camera around Y axis. Moving the mouse up/down will move
   * the camera in the Z direction (walking forward/backward).
   * centered).
   *
   * frontStep and rotStep control how much the camera will move per mouse
   * pixel.
   */
  void setClientSideWalkHandler(const JavaScriptMatrix4x4 &m,
                                double frontStep, double rotStep);

  /*! \brief Sets the content to be displayed when WebGL is not available.
   *
   * If %Wt cannot create a working WebGL context, this content will be
   * shown to the user. This may be a text explanation, or a pre-rendered
   * image, or a video, a flash movie, ...
   *
   * The default is a widget that explains to the user that he has no
   * WebGL support.
   */
  void setAlternativeContent(std::unique_ptr<WWidget> alternative);

  /*! \brief A JavaScript slot that repaints the widget when triggered.
   *
   * This is useful for client-side initiated repaints. You may e.g. use this
   * if you write your own client-side mouse handler, or if you updated
   * a texture, or if you're playing a video texture.
   */
  JSlot &repaintSlot() { return repaintSlot_; }

  /*! \brief enable client-side error messages (read detailed doc!)
   *
   * This option will add client-side code to check the result of every
   * WebGL call, and will popup an error dialog if a WebGL call returned
   * an error. The JavaScript then invokes the client-side debugger. This
   * code is intended to test your application, and should not be used in
   * production.
   */
  void enableClientErrorChecks(bool enable = true);

  /*! \brief Inject JavaScript into the current js-stream.
   *
   * Careful: this method directly puts the given jsString into
   * the JavaScript stream, whatever state it current has.
   * For example, if called in initGL(), it will put the jsString
   * into the client-side initGL() code.
   */
  void injectJS(const std::string & jsString);

  void webglNotAvailable();

protected:
  virtual DomElementType domElementType() const override;
  virtual DomElement *createDomElement(WApplication *app) override;
  virtual void getDomChanges(std::vector<DomElement *>& result, WApplication *app) override;
  virtual void updateDom(DomElement &element, bool all) override;

  virtual void render(WFlags<RenderFlag> flags) override;
  virtual std::string renderRemoveJs(bool recursive) override;

  virtual void layoutSizeChanged(int width, int height) override;

  virtual void setFormData(const FormData& formData) override;

  virtual void contextRestored();

private:

  WFlags<GLRenderOption> renderOptions_;
  std::unique_ptr<WAbstractGLImplementation> pImpl_;
  struct jsMatrixMap {
    int id;
    WMatrix4x4 serverSideCopy;

    jsMatrixMap(int matId, const WMatrix4x4& ssCopy)
      : id(matId), serverSideCopy(ssCopy)
    {}
  };
  struct jsVectorMap {
    int id;
    std::vector<float> serverSideCopy;

    jsVectorMap(int vecId, const std::vector<float> &ssCopy)
      : id(vecId), serverSideCopy(ssCopy)
    {}
  };
  std::vector<jsMatrixMap> jsMatrixList_;
  std::vector<jsVectorMap> jsVectorList_;
  unsigned jsValues_;
  std::stringstream js_;
  JSignal<> repaintSignal_;

  std::string glObjJsRef() const;

  std::unique_ptr<WWidget> alternative_;

  // If client detects that a WebGL context cannot be created, it fires this
  // signal, and the handler sets wegGlNotAvailable_ to true. If this happens,
  // the client side will have deleted the canvas widget.
  JSignal<> webglNotAvailable_;
  bool webGlNotAvailable_;

  JSignal<> contextRestored_;
  bool restoringContext_;

  bool valueChanged_;

  JSlot mouseWentDownSlot_;
  JSlot mouseWentUpSlot_;
  JSlot mouseDraggedSlot_;
  JSlot mouseMovedSlot_;
  JSlot mouseWheelSlot_;
  JSlot touchStarted_;
  JSlot touchEnded_;
  JSlot touchMoved_;
  JSlot repaintSlot_;

  void defineJavaScript();

#ifdef WT_TARGET_JAVA
static void renderfv(std::ostream &os, WGenericMatrix<MatrixType, 4, 4> t,
		     JsArrayType type);
#endif

  friend class WGLWidget::JavaScriptMatrix4x4;
  friend class WClientGLWidget;
  friend class WServerGLWidget;
};

W_DECLARE_OPERATORS_FOR_FLAGS(WGLWidget::GLenum)
W_DECLARE_OPERATORS_FOR_FLAGS(GLClientSideRenderer)
W_DECLARE_OPERATORS_FOR_FLAGS(GLRenderOption)
}

#undef WT_WGL_TEMPLATE

#endif
