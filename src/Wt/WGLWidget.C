// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGLWidget"
#include "Wt/WEnvironment"
#include "Wt/WHTML5Video"
#include "Wt/WImage"
#include "Wt/WText"
#include "Wt/WWebWidget"
#include "DomElement.h"
#include "Utils.h"
#include "Wt/WApplication"

#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/WGLWidget.min.js"
#include "js/WtGlMatrix.min.js"
#endif

using namespace Wt;

// TODO: for uniform*v, attribute8v, generate the non-v version in js. We
//       will probably end up with less bw and there's no use in allocating
//       an extra array.
// TODO: allow VBO's to be served from a file

const char *WGLWidget::toString(GLenum e)
{
  switch(e) {
    case DEPTH_BUFFER_BIT: return "ctx.DEPTH_BUFFER_BIT";
    case STENCIL_BUFFER_BIT: return "ctx.STENCIL_BUFFER_BIT";
    case COLOR_BUFFER_BIT: return "ctx.COLOR_BUFFER_BIT";
    case POINTS: return "ctx.POINTS";
    case LINES: return "ctx.LINES";
    case LINE_LOOP: return "ctx.LINE_LOOP";
    case LINE_STRIP: return "ctx.LINE_STRIP";
    case TRIANGLES: return "ctx.TRIANGLES";
    case TRIANGLE_STRIP: return "ctx.TRIANGLE_STRIP";
    case TRIANGLE_FAN: return "ctx.TRIANGLE_FAN";
    //case ZERO: return "ctx.ZERO";
    //case ONE: return "ctx.ONE";
    case SRC_COLOR: return "ctx.SRC_COLOR";
    case ONE_MINUS_SRC_COLOR: return "ctx.ONE_MINUS_SRC_COLOR";
    case SRC_ALPHA: return "ctx.SRC_ALPHA";
    case ONE_MINUS_SRC_ALPHA: return "ctx.ONE_MINUS_SRC_ALPHA";
    case DST_ALPHA: return "ctx.DST_ALPHA";
    case ONE_MINUS_DST_ALPHA: return "ctx.ONE_MINUS_DST_ALPHA";
    case DST_COLOR: return "ctx.DST_COLOR";
    case ONE_MINUS_DST_COLOR: return "ctx.ONE_MINUS_DST_COLOR";
    case SRC_ALPHA_SATURATE: return "ctx.SRC_ALPHA_SATURATE";
    case FUNC_ADD: return "ctx.FUNC_ADD";
    case BLEND_EQUATION: return "ctx.BLEND_EQUATION";
    //case BLEND_EQUATION_RGB: return "ctx.BLEND_EQUATION_RGB";
    case BLEND_EQUATION_ALPHA: return "ctx.BLEND_EQUATION_ALPHA";
    case FUNC_SUBTRACT: return "ctx.FUNC_SUBTRACT";
    case FUNC_REVERSE_SUBTRACT: return "ctx.FUNC_REVERSE_SUBTRACT";
    case BLEND_DST_RGB: return "ctx.BLEND_DST_RGB";
    case BLEND_SRC_RGB: return "ctx.BLEND_SRC_RGB";
    case BLEND_DST_ALPHA: return "ctx.BLEND_DST_ALPHA";
    case BLEND_SRC_ALPHA: return "ctx.BLEND_SRC_ALPHA";
    case CONSTANT_COLOR: return "ctx.CONSTANT_COLOR";
    case ONE_MINUS_CONSTANT_COLOR: return "ctx.ONE_MINUS_CONSTANT_COLOR";
    case CONSTANT_ALPHA: return "ctx.CONSTANT_ALPHA";
    case ONE_MINUS_CONSTANT_ALPHA: return "ctx.ONE_MINUS_CONSTANT_ALPHA";
    case BLEND_COLOR: return "ctx.BLEND_COLOR";
    case ARRAY_BUFFER: return "ctx.ARRAY_BUFFER";
    case ELEMENT_ARRAY_BUFFER: return "ctx.ELEMENT_ARRAY_BUFFER";
    case ARRAY_BUFFER_BINDING: return "ctx.ARRAY_BUFFER_BINDING";
    case ELEMENT_ARRAY_BUFFER_BINDING: return "ctx.ELEMENT_ARRAY_BUFFER_BINDING";
    case STREAM_DRAW: return "ctx.STREAM_DRAW";
    case STATIC_DRAW: return "ctx.STATIC_DRAW";
    case DYNAMIC_DRAW: return "ctx.DYNAMIC_DRAW";
    case BUFFER_SIZE: return "ctx.BUFFER_SIZE";
    case BUFFER_USAGE: return "ctx.BUFFER_USAGE";
    case CURRENT_VERTEX_ATTRIB: return "ctx.CURRENT_VERTEX_ATTRIB";
    case FRONT: return "ctx.FRONT";
    case BACK: return "ctx.BACK";
    case FRONT_AND_BACK: return "ctx.FRONT_AND_BACK";
    case CULL_FACE: return "ctx.CULL_FACE";
    case BLEND: return "ctx.BLEND";
    case DITHER: return "ctx.DITHER";
    case STENCIL_TEST: return "ctx.STENCIL_TEST";
    case DEPTH_TEST: return "ctx.DEPTH_TEST";
    case SCISSOR_TEST: return "ctx.SCISSOR_TEST";
    case POLYGON_OFFSET_FILL: return "ctx.POLYGON_OFFSET_FILL";
    case SAMPLE_ALPHA_TO_COVERAGE: return "ctx.SAMPLE_ALPHA_TO_COVERAGE";
    case SAMPLE_COVERAGE: return "ctx.SAMPLE_COVERAGE";
    //case NO_ERROR: return "ctx.NO_ERROR";
    case INVALID_ENUM: return "ctx.INVALID_ENUM";
    case INVALID_VALUE: return "ctx.INVALID_VALUE";
    case INVALID_OPERATION: return "ctx.INVALID_OPERATION";
    case OUT_OF_MEMORY: return "ctx.OUT_OF_MEMORY";
    case CW: return "ctx.CW";
    case CCW: return "ctx.CCW";
    case LINE_WIDTH: return "ctx.LINE_WIDTH";
    case ALIASED_POINT_SIZE_RANGE: return "ctx.ALIASED_POINT_SIZE_RANGE";
    case ALIASED_LINE_WIDTH_RANGE: return "ctx.ALIASED_LINE_WIDTH_RANGE";
    case CULL_FACE_MODE: return "ctx.CULL_FACE_MODE";
    case FRONT_FACE: return "ctx.FRONT_FACE";
    case DEPTH_RANGE: return "ctx.DEPTH_RANGE";
    case DEPTH_WRITEMASK: return "ctx.DEPTH_WRITEMASK";
    case DEPTH_CLEAR_VALUE: return "ctx.DEPTH_CLEAR_VALUE";
    case DEPTH_FUNC: return "ctx.DEPTH_FUNC";
    case STENCIL_CLEAR_VALUE: return "ctx.STENCIL_CLEAR_VALUE";
    case STENCIL_FUNC: return "ctx.STENCIL_FUNC";
    case STENCIL_FAIL: return "ctx.STENCIL_FAIL";
    case STENCIL_PASS_DEPTH_FAIL: return "ctx.STENCIL_PASS_DEPTH_FAIL";
    case STENCIL_PASS_DEPTH_PASS: return "ctx.STENCIL_PASS_DEPTH_PASS";
    case STENCIL_REF: return "ctx.STENCIL_REF";
    case STENCIL_VALUE_MASK: return "ctx.STENCIL_VALUE_MASK";
    case STENCIL_WRITEMASK: return "ctx.STENCIL_WRITEMASK";
    case STENCIL_BACK_FUNC: return "ctx.STENCIL_BACK_FUNC";
    case STENCIL_BACK_FAIL: return "ctx.STENCIL_BACK_FAIL";
    case STENCIL_BACK_PASS_DEPTH_FAIL: return "ctx.STENCIL_BACK_PASS_DEPTH_FAIL";
    case STENCIL_BACK_PASS_DEPTH_PASS: return "ctx.STENCIL_BACK_PASS_DEPTH_PASS";
    case STENCIL_BACK_REF: return "ctx.STENCIL_BACK_REF";
    case STENCIL_BACK_VALUE_MASK: return "ctx.STENCIL_BACK_VALUE_MASK";
    case STENCIL_BACK_WRITEMASK: return "ctx.STENCIL_BACK_WRITEMASK";
    case VIEWPORT: return "ctx.VIEWPORT";
    case SCISSOR_BOX: return "ctx.SCISSOR_BOX";
    case COLOR_CLEAR_VALUE: return "ctx.COLOR_CLEAR_VALUE";
    case COLOR_WRITEMASK: return "ctx.COLOR_WRITEMASK";
    case UNPACK_ALIGNMENT: return "ctx.UNPACK_ALIGNMENT";
    case PACK_ALIGNMENT: return "ctx.PACK_ALIGNMENT";
    case MAX_TEXTURE_SIZE: return "ctx.MAX_TEXTURE_SIZE";
    case MAX_VIEWPORT_DIMS: return "ctx.MAX_VIEWPORT_DIMS";
    case SUBPIXEL_BITS: return "ctx.SUBPIXEL_BITS";
    case RED_BITS: return "ctx.RED_BITS";
    case GREEN_BITS: return "ctx.GREEN_BITS";
    case BLUE_BITS: return "ctx.BLUE_BITS";
    case ALPHA_BITS: return "ctx.ALPHA_BITS";
    case DEPTH_BITS: return "ctx.DEPTH_BITS";
    case STENCIL_BITS: return "ctx.STENCIL_BITS";
    case POLYGON_OFFSET_UNITS: return "ctx.POLYGON_OFFSET_UNITS";
    case POLYGON_OFFSET_FACTOR: return "ctx.POLYGON_OFFSET_FACTOR";
    case TEXTURE_BINDING_2D: return "ctx.TEXTURE_BINDING_2D";
    case SAMPLE_BUFFERS: return "ctx.SAMPLE_BUFFERS";
    case SAMPLES: return "ctx.SAMPLES";
    case SAMPLE_COVERAGE_VALUE: return "ctx.SAMPLE_COVERAGE_VALUE";
    case SAMPLE_COVERAGE_INVERT: return "ctx.SAMPLE_COVERAGE_INVERT";
    case NUM_COMPRESSED_TEXTURE_FORMATS: return "ctx.NUM_COMPRESSED_TEXTURE_FORMATS";
    case COMPRESSED_TEXTURE_FORMATS: return "ctx.COMPRESSED_TEXTURE_FORMATS";
    case DONT_CARE: return "ctx.DONT_CARE";
    case FASTEST: return "ctx.FASTEST";
    case NICEST: return "ctx.NICEST";
    case GENERATE_MIPMAP_HINT: return "ctx.GENERATE_MIPMAP_HINT";
    case BYTE: return "ctx.BYTE";
    case UNSIGNED_BYTE: return "ctx.UNSIGNED_BYTE";
    case SHORT: return "ctx.SHORT";
    case UNSIGNED_SHORT: return "ctx.UNSIGNED_SHORT";
    case INT: return "ctx.INT";
    case UNSIGNED_INT: return "ctx.UNSIGNED_INT";
    case FLOAT: return "ctx.FLOAT";
    case DEPTH_COMPONENT: return "ctx.DEPTH_COMPONENT";
    case ALPHA: return "ctx.ALPHA";
    case RGB: return "ctx.RGB";
    case RGBA: return "ctx.RGBA";
    case LUMINANCE: return "ctx.LUMINANCE";
    case LUMINANCE_ALPHA: return "ctx.LUMINANCE_ALPHA";
    case UNSIGNED_SHORT_4_4_4_4: return "ctx.UNSIGNED_SHORT_4_4_4_4";
    case UNSIGNED_SHORT_5_5_5_1: return "ctx.UNSIGNED_SHORT_5_5_5_1";
    case UNSIGNED_SHORT_5_6_5: return "ctx.UNSIGNED_SHORT_5_6_5";
    case FRAGMENT_SHADER: return "ctx.FRAGMENT_SHADER";
    case VERTEX_SHADER: return "ctx.VERTEX_SHADER";
    case MAX_VERTEX_ATTRIBS: return "ctx.MAX_VERTEX_ATTRIBS";
    case MAX_VERTEX_UNIFORM_VECTORS: return "ctx.MAX_VERTEX_UNIFORM_VECTORS";
    case MAX_VARYING_VECTORS: return "ctx.MAX_VARYING_VECTORS";
    case MAX_COMBINED_TEXTURE_IMAGE_UNITS: return "ctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS";
    case MAX_VERTEX_TEXTURE_IMAGE_UNITS: return "ctx.MAX_VERTEX_TEXTURE_IMAGE_UNITS";
    case MAX_TEXTURE_IMAGE_UNITS: return "ctx.MAX_TEXTURE_IMAGE_UNITS";
    case MAX_FRAGMENT_UNIFORM_VECTORS: return "ctx.MAX_FRAGMENT_UNIFORM_VECTORS";
    case SHADER_TYPE: return "ctx.SHADER_TYPE";
    case DELETE_STATUS: return "ctx.DELETE_STATUS";
    case LINK_STATUS: return "ctx.LINK_STATUS";
    case VALIDATE_STATUS: return "ctx.VALIDATE_STATUS";
    case ATTACHED_SHADERS: return "ctx.ATTACHED_SHADERS";
    case ACTIVE_UNIFORMS: return "ctx.ACTIVE_UNIFORMS";
    case ACTIVE_UNIFORM_MAX_LENGTH: return "ctx.ACTIVE_UNIFORM_MAX_LENGTH";
    case ACTIVE_ATTRIBUTES: return "ctx.ACTIVE_ATTRIBUTES";
    case ACTIVE_ATTRIBUTE_MAX_LENGTH: return "ctx.ACTIVE_ATTRIBUTE_MAX_LENGTH";
    case SHADING_LANGUAGE_VERSION: return "ctx.SHADING_LANGUAGE_VERSION";
    case CURRENT_PROGRAM: return "ctx.CURRENT_PROGRAM";
    case NEVER: return "ctx.NEVER";
    case LESS: return "ctx.LESS";
    case EQUAL: return "ctx.EQUAL";
    case LEQUAL: return "ctx.LEQUAL";
    case GREATER: return "ctx.GREATER";
    case NOTEQUAL: return "ctx.NOTEQUAL";
    case GEQUAL: return "ctx.GEQUAL";
    case ALWAYS: return "ctx.ALWAYS";
    case KEEP: return "ctx.KEEP";
    case REPLACE: return "ctx.REPLACE";
    case INCR: return "ctx.INCR";
    case DECR: return "ctx.DECR";
    case INVERT: return "ctx.INVERT";
    case INCR_WRAP: return "ctx.INCR_WRAP";
    case DECR_WRAP: return "ctx.DECR_WRAP";
    case VENDOR: return "ctx.VENDOR";
    case RENDERER: return "ctx.RENDERER";
    case VERSION: return "ctx.VERSION";
    case NEAREST: return "ctx.NEAREST";
    case LINEAR: return "ctx.LINEAR";
    case NEAREST_MIPMAP_NEAREST: return "ctx.NEAREST_MIPMAP_NEAREST";
    case LINEAR_MIPMAP_NEAREST: return "ctx.LINEAR_MIPMAP_NEAREST";
    case NEAREST_MIPMAP_LINEAR: return "ctx.NEAREST_MIPMAP_LINEAR";
    case LINEAR_MIPMAP_LINEAR: return "ctx.LINEAR_MIPMAP_LINEAR";
    case TEXTURE_MAG_FILTER: return "ctx.TEXTURE_MAG_FILTER";
    case TEXTURE_MIN_FILTER: return "ctx.TEXTURE_MIN_FILTER";
    case TEXTURE_WRAP_S: return "ctx.TEXTURE_WRAP_S";
    case TEXTURE_WRAP_T: return "ctx.TEXTURE_WRAP_T";
    case TEXTURE_2D: return "ctx.TEXTURE_2D";
    case TEXTURE: return "ctx.TEXTURE";
    case TEXTURE_CUBE_MAP: return "ctx.TEXTURE_CUBE_MAP";
    case TEXTURE_BINDING_CUBE_MAP: return "ctx.TEXTURE_BINDING_CUBE_MAP";
    case TEXTURE_CUBE_MAP_POSITIVE_X: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_X";
    case TEXTURE_CUBE_MAP_NEGATIVE_X: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_X";
    case TEXTURE_CUBE_MAP_POSITIVE_Y: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_Y";
    case TEXTURE_CUBE_MAP_NEGATIVE_Y: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_Y";
    case TEXTURE_CUBE_MAP_POSITIVE_Z: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_Z";
    case TEXTURE_CUBE_MAP_NEGATIVE_Z: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_Z";
    case MAX_CUBE_MAP_TEXTURE_SIZE: return "ctx.MAX_CUBE_MAP_TEXTURE_SIZE";
    case TEXTURE0: return "ctx.TEXTURE0";
    case TEXTURE1: return "ctx.TEXTURE1";
    case TEXTURE2: return "ctx.TEXTURE2";
    case TEXTURE3: return "ctx.TEXTURE3";
    case TEXTURE4: return "ctx.TEXTURE4";
    case TEXTURE5: return "ctx.TEXTURE5";
    case TEXTURE6: return "ctx.TEXTURE6";
    case TEXTURE7: return "ctx.TEXTURE7";
    case TEXTURE8: return "ctx.TEXTURE8";
    case TEXTURE9: return "ctx.TEXTURE9";
    case TEXTURE10: return "ctx.TEXTURE10";
    case TEXTURE11: return "ctx.TEXTURE11";
    case TEXTURE12: return "ctx.TEXTURE12";
    case TEXTURE13: return "ctx.TEXTURE13";
    case TEXTURE14: return "ctx.TEXTURE14";
    case TEXTURE15: return "ctx.TEXTURE15";
    case TEXTURE16: return "ctx.TEXTURE16";
    case TEXTURE17: return "ctx.TEXTURE17";
    case TEXTURE18: return "ctx.TEXTURE18";
    case TEXTURE19: return "ctx.TEXTURE19";
    case TEXTURE20: return "ctx.TEXTURE20";
    case TEXTURE21: return "ctx.TEXTURE21";
    case TEXTURE22: return "ctx.TEXTURE22";
    case TEXTURE23: return "ctx.TEXTURE23";
    case TEXTURE24: return "ctx.TEXTURE24";
    case TEXTURE25: return "ctx.TEXTURE25";
    case TEXTURE26: return "ctx.TEXTURE26";
    case TEXTURE27: return "ctx.TEXTURE27";
    case TEXTURE28: return "ctx.TEXTURE28";
    case TEXTURE29: return "ctx.TEXTURE29";
    case TEXTURE30: return "ctx.TEXTURE30";
    case TEXTURE31: return "ctx.TEXTURE31";
    case ACTIVE_TEXTURE: return "ctx.ACTIVE_TEXTURE";
    case REPEAT: return "ctx.REPEAT";
    case CLAMP_TO_EDGE: return "ctx.CLAMP_TO_EDGE";
    case MIRRORED_REPEAT: return "ctx.MIRRORED_REPEAT";
    case FLOAT_VEC2: return "ctx.FLOAT_VEC2";
    case FLOAT_VEC3: return "ctx.FLOAT_VEC3";
    case FLOAT_VEC4: return "ctx.FLOAT_VEC4";
    case INT_VEC2: return "ctx.INT_VEC2";
    case INT_VEC3: return "ctx.INT_VEC3";
    case INT_VEC4: return "ctx.INT_VEC4";
    case BOOL: return "ctx.BOOL";
    case BOOL_VEC2: return "ctx.BOOL_VEC2";
    case BOOL_VEC3: return "ctx.BOOL_VEC3";
    case BOOL_VEC4: return "ctx.BOOL_VEC4";
    case FLOAT_MAT2: return "ctx.FLOAT_MAT2";
    case FLOAT_MAT3: return "ctx.FLOAT_MAT3";
    case FLOAT_MAT4: return "ctx.FLOAT_MAT4";
    case SAMPLER_2D: return "ctx.SAMPLER_2D";
    case SAMPLER_CUBE: return "ctx.SAMPLER_CUBE";
    case VERTEX_ATTRIB_ARRAY_ENABLED: return "ctx.VERTEX_ATTRIB_ARRAY_ENABLED";
    case VERTEX_ATTRIB_ARRAY_SIZE: return "ctx.VERTEX_ATTRIB_ARRAY_SIZE";
    case VERTEX_ATTRIB_ARRAY_STRIDE: return "ctx.VERTEX_ATTRIB_ARRAY_STRIDE";
    case VERTEX_ATTRIB_ARRAY_TYPE: return "ctx.VERTEX_ATTRIB_ARRAY_TYPE";
    case VERTEX_ATTRIB_ARRAY_NORMALIZED: return "ctx.VERTEX_ATTRIB_ARRAY_NORMALIZED";
    case VERTEX_ATTRIB_ARRAY_POINTER: return "ctx.VERTEX_ATTRIB_ARRAY_POINTER";
    case VERTEX_ATTRIB_ARRAY_BUFFER_BINDING: return "ctx.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING";
    case COMPILE_STATUS: return "ctx.COMPILE_STATUS";
    case INFO_LOG_LENGTH: return "ctx.INFO_LOG_LENGTH";
    case SHADER_SOURCE_LENGTH: return "ctx.SHADER_SOURCE_LENGTH";
    case LOW_FLOAT: return "ctx.LOW_FLOAT";
    case MEDIUM_FLOAT: return "ctx.MEDIUM_FLOAT";
    case HIGH_FLOAT: return "ctx.HIGH_FLOAT";
    case LOW_INT: return "ctx.LOW_INT";
    case MEDIUM_INT: return "ctx.MEDIUM_INT";
    case HIGH_INT: return "ctx.HIGH_INT";
    case FRAMEBUFFER: return "ctx.FRAMEBUFFER";
    case RENDERBUFFER: return "ctx.RENDERBUFFER";
    case RGBA4: return "ctx.RGBA4";
    case RGB5_A1: return "ctx.RGB5_A1";
    case RGB565: return "ctx.RGB565";
    case DEPTH_COMPONENT16: return "ctx.DEPTH_COMPONENT16";
    case STENCIL_INDEX: return "ctx.STENCIL_INDEX";
    case STENCIL_INDEX8: return "ctx.STENCIL_INDEX8";
    case DEPTH_STENCIL: return "ctx.DEPTH_STENCIL";
    case RENDERBUFFER_WIDTH: return "ctx.RENDERBUFFER_WIDTH";
    case RENDERBUFFER_HEIGHT: return "ctx.RENDERBUFFER_HEIGHT";
    case RENDERBUFFER_INTERNAL_FORMAT: return "ctx.RENDERBUFFER_INTERNAL_FORMAT";
    case RENDERBUFFER_RED_SIZE: return "ctx.RENDERBUFFER_RED_SIZE";
    case RENDERBUFFER_GREEN_SIZE: return "ctx.RENDERBUFFER_GREEN_SIZE";
    case RENDERBUFFER_BLUE_SIZE: return "ctx.RENDERBUFFER_BLUE_SIZE";
    case RENDERBUFFER_ALPHA_SIZE: return "ctx.RENDERBUFFER_ALPHA_SIZE";
    case RENDERBUFFER_DEPTH_SIZE: return "ctx.RENDERBUFFER_DEPTH_SIZE";
    case RENDERBUFFER_STENCIL_SIZE: return "ctx.RENDERBUFFER_STENCIL_SIZE";
    case FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE: return "ctx.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE";
    case FRAMEBUFFER_ATTACHMENT_OBJECT_NAME: return "ctx.FRAMEBUFFER_ATTACHMENT_OBJECT_NAME";
    case FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL: return "ctx.FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL";
    case FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE: return "ctx.FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE";
    case COLOR_ATTACHMENT0: return "ctx.COLOR_ATTACHMENT0";
    case DEPTH_ATTACHMENT: return "ctx.DEPTH_ATTACHMENT";
    case STENCIL_ATTACHMENT: return "ctx.STENCIL_ATTACHMENT";
    case DEPTH_STENCIL_ATTACHMENT: return "ctx.DEPTH_STENCIL_ATTACHMENT";
    //case NONE: return "ctx.NONE";
    case FRAMEBUFFER_COMPLETE: return "ctx.FRAMEBUFFER_COMPLETE";
    case FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "ctx.FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "ctx.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case FRAMEBUFFER_INCOMPLETE_DIMENSIONS: return "ctx.FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
    case FRAMEBUFFER_UNSUPPORTED: return "ctx.FRAMEBUFFER_UNSUPPORTED";
    case FRAMEBUFFER_BINDING: return "ctx.FRAMEBUFFER_BINDING";
    case RENDERBUFFER_BINDING: return "ctx.RENDERBUFFER_BINDING";
    case MAX_RENDERBUFFER_SIZE: return "ctx.MAX_RENDERBUFFER_SIZE";
    case INVALID_FRAMEBUFFER_OPERATION: return "ctx.INVALID_FRAMEBUFFER_OPERATION";
    case UNPACK_FLIP_Y_WEBGL: return "ctx.UNPACK_FLIP_Y_WEBGL";
    case UNPACK_PREMULTIPLY_ALPHA_WEBGL: return "ctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL";
    case CONTEXT_LOST_WEBGL: return "ctx.CONTEXT_LOST_WEBGL";
    case UNPACK_COLORSPACE_CONVERSION_WEBGL: return "ctx.UNPACK_COLORSPACE_CONVERSION_WEBGL";
    case BROWSER_DEFAULT_WEBGL: return "ctx.BROWSER_DEFAULT_WEBGL";
  }
  return "BAD_GL_ENUM";
}


WGLWidget::WGLWidget(WContainerWidget *parent):
  WInteractWidget(parent),
  renderWidth_(100),
  renderHeight_(100),
  updatePaintGL_(true),
  updateResizeGL_(true),
  updateGL_(false),
  sizeChanged_(true),
  alternative_(0),
  shaders_(0),
  programs_(0),
  attributes_(0),
  uniforms_(0),
  buffers_(0),
  textures_(0),
  matrices_(0),
  webglNotAvailable_(this, "webglNotAvailable"),
  webGlNotAvailable_(false),
  mouseWentDownSlot_("function(){}", this),
  mouseWentUpSlot_("function(){}", this),
  mouseDraggedSlot_("function(){}", this),
  mouseWheelSlot_("function(){}", this),
  repaintSlot_("function() {"
    "var o = " + this->glObjJsRef() + ";"
    "if(o.ctx) o.paintGL();"
    "}", this)

{
  setInline(false);
  setLayoutSizeAware(true);
  // A canvas must have a size set.
  resize(100, 100);
  webglNotAvailable_.connect(this, &WGLWidget::webglNotAvailable);

  mouseWentDown().connect(mouseWentDownSlot_);
  mouseWentUp().connect(mouseWentUpSlot_);
  mouseDragged().connect(mouseDraggedSlot_);
  mouseWheel().connect(mouseWheelSlot_);
  setAlternativeContent(new WText("Your browser does not support WebGL"));
}

WGLWidget::~WGLWidget()
{
}

void WGLWidget::setAlternativeContent(WWidget *alternative)
{
  if (alternative_)
    delete alternative_;
  alternative_ = alternative;
  if (alternative_)
    addChild(alternative_);
}

void WGLWidget::webglNotAvailable()
{
  std::cout << "WebGL Not available in client!\n";
  webGlNotAvailable_ = true;
}

std::string WGLWidget::renderRemoveJs()
{
  if (webGlNotAvailable_) {
    // The canvas was already deleted client-side
    return alternative_->webWidget()->renderRemoveJs();
  } else {
    // Nothing special, behave as usual
    return WInteractWidget::renderRemoveJs();
  }
}

std::string WGLWidget::glObjJsRef()
{
  return "(function(){"
    "var r = " + jsRef() + ";"
    "var o = r ? jQuery.data(r,'obj') : null;"
    "return o ? o : {ctx: null};"
    "})()";
}

char *WGLWidget::makeFloat(double d, char *buf)
{
  return Utils::round_str(d, 6, buf);
}

char *WGLWidget::makeInt(int i, char *buf)
{
  return Utils::itoa(i, buf);
}

DomElement *WGLWidget::createDomElement(WApplication *app)
{
  DomElement *result = 0;
  if (app->environment().agentIsIElt(9) ||
      app->environment().agent() == WEnvironment::MobileWebKitAndroid) {
    // Shortcut: IE misbehaves when it encounters a canvas element
    result = DomElement::createNew(DomElement_DIV);
    if (alternative_)
      result->addChild(alternative_->createSDomElement(app));
  } else {
    result = DomElement::createNew(DomElement_CANVAS);;
    if (alternative_) {
      result->addChild(alternative_->createSDomElement(app));
    }
  }
  setId(result, app);

  std::stringstream tmp;
    tmp <<
      "{\n"
      """var o = new " WT_CLASS ".WGLWidget(" << app->javaScriptClass() << "," << jsRef() << ");\n"
      """o.discoverContext(function(){" << webglNotAvailable_.createCall() << "});\n";

    js_.str("");
    initializeGL();
    tmp <<
      """o.initializeGL=function(){\n"
      //"""debugger;\n"
      """var obj=" << glObjJsRef() << ";\n"
      """var ctx=obj.ctx;\n" <<
      "" << js_.str() <<
      """o.initialized = true;\n"
      // updates are queued until initialization is complete
      """var key;\n"
      """for(key in o.updates) o.updates[key]();\n"
      """o.updates = new Array();\n"
      // Similar, resizeGL is not executed until initialized
      """o.resizeGL();\n"
      "};\n"
      "}\n";
  tmp << delayedJavaScript_.str();
  delayedJavaScript_.str("");
  result->callJavaScript(tmp.str());

  repaintGL(PAINT_GL | RESIZE_GL);

  updateDom(*result, true);


  return result;
}

void WGLWidget::repaintGL(WFlags<ClientSideRenderer> which)
{
  if (which & PAINT_GL)
    updatePaintGL_ = true;
  if (which & RESIZE_GL)
    updateResizeGL_ = true;
  if (which & UPDATE_GL)
    updateGL_ = true;
  if (which != 0)
    repaint(Wt::RepaintPropertyAttribute); // FIXME: correct?
}

void WGLWidget::updateDom(DomElement &element, bool all)
{
  if (webGlNotAvailable_)
    return;

  DomElement *el = &element;
  if (all || sizeChanged_) {
    el->setAttribute("width", boost::lexical_cast<std::string>(renderWidth_));
    el->setAttribute("height", boost::lexical_cast<std::string>(renderHeight_));
    sizeChanged_ = false;
  }
  if (updateGL_ || updateResizeGL_ || updatePaintGL_) {
    std::stringstream tmp;
    tmp <<
      "var o = " << glObjJsRef() << ";\n"
      "if(o.ctx){\n";
    if (updateGL_) {
      js_.str("");
      updateGL();
      tmp << "var update =function(){\n"
        "var obj=" << glObjJsRef() << ";\n"
        "var ctx=obj.ctx;\n"
        << js_.str() << "\n};\n"
        // cannot execute updates before initializeGL is executed
        "o.updates.push(update);";
    }
    if (updateResizeGL_) {
      js_.str("");
      resizeGL(renderWidth_, renderHeight_);
      tmp << "o.resizeGL=function(){\n"
        "var obj=" << glObjJsRef() << ";\n"
        "var ctx=obj.ctx;\n"
        << js_.str() << "};";
    }
    if (updatePaintGL_) {
      js_.str("");
      paintGL();
      tmp << "o.paintGL=function(){\n"
        "var obj=" << glObjJsRef() << ";\n"
        "var ctx=obj.ctx;\n"
        << js_.str() << "};";
    }
    js_.str("");
    // Make sure textures are loaded before we render
    tmp << "}\n";
    if (preloadImages_.size() > 0) {
      tmp << "new Wt._p_.ImagePreloader([";
      for (unsigned i = 0; i < preloadImages_.size(); ++i) {
        if (i != 0)
          tmp << ',';
        tmp << '\'' << fixRelativeUrl(preloadImages_[i].second) << '\'';
      }
      tmp <<
        "],function(images){\n"
        "var o=" << glObjJsRef() << ";\n"
        "var ctx=null;\n"
        " if(o) ctx=o.ctx;\n"
        "if(ctx == null) return;\n";
      for (unsigned i = 0; i < preloadImages_.size(); ++i) {
        std::string texture = preloadImages_[i].first;
        tmp << texture << "=ctx.createTexture();\n"
          << texture << ".image=images[" << i << "];\n";
      }
      tmp <<
        "if(o.initialized){"
        // Delay calling of update() to after textures are loaded
        """var key;"
        """for(key in o.updates) o.updates[key]();"
        """o.updates = new Array();"
        // Delay calling of resizeGL() to after updates are executed
        """o.resizeGL();"
        """o.paintGL();"
        "} else {"
        // initializeGL will call updates and resizeGL
        """o.initializeGL();\n"
        """o.paintGL();\n"
        "}});";
    } else {
      // No textures to load - go and paint
      tmp <<
        "if(o.initialized) {"
        """var key;"
        """for(key in o.updates) o.updates[key]();"
        """o.updates = new Array();"
        """o.resizeGL();"
        """o.paintGL();"
        "} else {"
        """o.initializeGL();"
        """o.paintGL();"
        "}";
    }
    el->callJavaScript(tmp.str());
    updateGL_ = updatePaintGL_ = updateResizeGL_ = false;
  }

  WInteractWidget::updateDom(element, all);
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

void WGLWidget::resize(const WLength &width, const WLength &height)
{
  WInteractWidget::resize(width, height);
  layoutSizeChanged(width.value(), height.value());
}

void WGLWidget::layoutSizeChanged(int width, int height)
{
  renderWidth_ = width;
  renderHeight_ = height;
  sizeChanged_ = true;
  repaint(Wt::RepaintPropertyAttribute);
  repaintGL(RESIZE_GL);
}

void WGLWidget::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *GLMATRIX_JS = "js/WtGlMatrix.js";

  if (!app->javaScriptLoaded(GLMATRIX_JS)) {
    LOAD_JAVASCRIPT(app, GLMATRIX_JS, "glMatrix", wtjs2);
    app->setJavaScriptLoaded(GLMATRIX_JS);
  }

  const char *THIS_JS = "js/WGLWidget.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WGLWidget", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

}

void WGLWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();
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

void WGLWidget::activeTexture(GLenum texture)
{
  js_ << "ctx.activeTexture(" << toString(texture) << ");";
  GLDEBUG;
}

void WGLWidget::attachShader(Program program, Shader shader)
{
  js_ << "ctx.attachShader(" << program << ", " << shader << ");";
  GLDEBUG;
}

void WGLWidget::bindAttribLocation(Program program, unsigned index, const std::string &name)
{
  js_ << "ctx.bindAttribLocation(" << program << "," << index
    << "," << jsStringLiteral(name) << ");";
  GLDEBUG;
}

void WGLWidget::bindBuffer(GLenum target, Buffer buffer)
{
  js_ << "ctx.bindBuffer(" << toString(target) << "," << buffer << ");";
  GLDEBUG;
}

void WGLWidget::bindTexture(GLenum target, Texture texture)
{
  js_ << "ctx.bindTexture(" << toString(target) << "," << texture << ");";
  GLDEBUG;
}

void WGLWidget::blendColor(double red, double green, double blue, double alpha)
{
  char buf[30];
  js_ << "ctx.blendColor(" << makeFloat(red, buf) << ",";
  js_ << makeFloat(green, buf) << ",";
  js_ << makeFloat(blue, buf) << ",";
  js_ << makeFloat(alpha, buf) << ");";
  GLDEBUG;
}

void WGLWidget::blendEquation(GLenum mode)
{
  js_ << "ctx.blendEquation(" << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::blendEquationSeparate(GLenum modeRGB,
                                      GLenum modeAlpha)
{
  js_ << "ctx.blendEquationSeparate(" << toString(modeRGB) << "," 
    << toString(modeAlpha) << ");";
  GLDEBUG;
}

void WGLWidget::blendFunc(GLenum sfactor, GLenum dfactor)
{
  js_ << "ctx.blendFunc(" << toString(sfactor) << ","
    << toString(dfactor) << ");";
  GLDEBUG;
}

void WGLWidget::blendFuncSeparate(GLenum srcRGB,
                                  GLenum dstRGB,
                                  GLenum srcAlpha,
                                  GLenum dstAlpha)
{
  js_ << "ctx.blendFuncSeparate(" << toString(srcRGB) << ","
    << toString(dstRGB) << "," << toString(srcAlpha) << ","
    << toString(dstAlpha) << ");";
  GLDEBUG;
}

void WGLWidget::clear(WFlags<GLenum> mask)
{
  js_ << "ctx.clear(";
  if (mask & COLOR_BUFFER_BIT) js_ << "ctx.COLOR_BUFFER_BIT|";
  if (mask & DEPTH_BUFFER_BIT) js_ << "ctx.DEPTH_BUFFER_BIT|";
  if (mask & STENCIL_BUFFER_BIT) js_ << "ctx.STENCIL_BUFFER_BIT|";

  js_ << "0);";
  GLDEBUG;
}

void WGLWidget::clearColor(double r, double g, double b, double a)
{
  char buf[30];
  js_ << "ctx.clearColor(" << makeFloat(r, buf) << ",";
  js_ << makeFloat(g, buf) << ",";
  js_ << makeFloat(b, buf) << ",";
  js_ << makeFloat(a, buf) << ");";
  GLDEBUG;
}

void WGLWidget::clearDepth(double depth)
{
  char buf[30];
  js_ << "ctx.clearDepth(" << makeFloat(depth, buf) << ");";
  GLDEBUG;
}

void WGLWidget::clearStencil(int s)
{
  js_ << "ctx.clearStencil(" << s << ");";
  GLDEBUG;
}

void WGLWidget::colorMask(bool red, bool green, bool blue, bool alpha)
{
  js_ << "ctx.colorMask(" << (red ? "true" : "false") << ","
    << (green ? "true" : "false") << ","
    << (blue ? "true" : "false") << ","
    << (alpha ? "true" : "false") << ");";
  GLDEBUG;
}

void WGLWidget::compileShader(Shader shader)
{
  js_ << "ctx.compileShader(" << shader << ");";
  js_ << "if (!ctx.getShaderParameter(" << shader << ", ctx.COMPILE_STATUS)) {"
    << "alert(ctx.getShaderInfoLog(" << shader << "));}";
  GLDEBUG;
}

void WGLWidget::copyTexImage2D(GLenum target, int level,
                               GLenum internalFormat,
                               int x, int y,
                               unsigned width, unsigned height, 
                               int border)
{
  js_ << "ctx.copyTexImage2D(" << toString(target) << "," << level << ","
    << toString(internalFormat) << "," << x << "," << y << ","
    << width << "," << height << "," << border << ");";
  GLDEBUG;
}

void WGLWidget::copyTexSubImage2D(GLenum target, int level,
                                  int xoffset, int yoffset,
                                  int x, int y,
                                  unsigned width, unsigned height)
{
  js_ << "ctx.copyTexSubImage2D(" << toString(target) << "," << level << ","
    << xoffset << "," << yoffset << "," << x << "," << y << ","
    << width << "," << height << ");";
  GLDEBUG;
}

WGLWidget::Buffer WGLWidget::createBuffer()
{
  Buffer retval = "ctx.WtBuffer" + boost::lexical_cast<std::string>(buffers_++);
  js_ << retval << "=ctx.createBuffer();";
  GLDEBUG;
  return retval;
}

WGLWidget::Program WGLWidget::createProgram()
{
  Program retval = "ctx.WtProgram" + boost::lexical_cast<std::string>(programs_++);
  js_ << retval << "=ctx.createProgram();";
  GLDEBUG;
  return retval;
}

WGLWidget::Shader WGLWidget::createShader(GLenum shader)
{
  Shader retval = "ctx.WtShader" + boost::lexical_cast<std::string>(shaders_++);
  js_ << retval << "=ctx.createShader(" << toString(shader) << ");";
  GLDEBUG;
  return retval;
}

WGLWidget::Texture WGLWidget::createTexture()
{
  Texture retval = "ctx.WtTexture" + boost::lexical_cast<std::string>(textures_++);
  js_ << retval << "=ctx.createTexture();";
  GLDEBUG;
  return retval;
}

WGLWidget::Texture WGLWidget::createTextureAndLoad(const std::string &url)
{
  Texture retval = "ctx.WtTexture" + boost::lexical_cast<std::string>(textures_++);
  preloadImages_.push_back(std::make_pair<std::string, std::string>(retval, url));
  GLDEBUG;
  return retval;

}

void WGLWidget::cullFace(GLenum mode)
{
  js_ << "ctx.cullFace(" << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::deleteBuffer(Buffer buffer)
{
  js_ << "ctx.deleteBuffer(" << buffer << ");";
  GLDEBUG;
}

void WGLWidget::deleteProgram(Program program)
{
  js_ << "ctx.deleteProgram(" << program << ");";
  GLDEBUG;
}

void WGLWidget::deleteShader(Shader shader)
{
  js_ << "ctx.deleteShader(" << shader << ");";
  GLDEBUG;
}

void WGLWidget::deleteTexture(Texture texture)
{
  js_ << "ctx.deleteTexture(" << texture << ");";
  GLDEBUG;
}

void WGLWidget::depthFunc(GLenum func)
{
  js_ << "ctx.depthFunc(" << toString(func) << ");";
  GLDEBUG;
}

void WGLWidget::depthMask(bool flag)
{
  js_ << "ctx.depthMask(" << (flag ? "true" : "false") << ");";
  GLDEBUG;
}

void WGLWidget::depthRange(double zNear, double zFar)
{
  char buf[30];
  js_ << "ctx.depthRange(" << makeFloat(zNear, buf) << ",";
  js_ << makeFloat(zFar, buf) << ");";
  GLDEBUG;
}

void WGLWidget::detachShader(Program program, Shader shader)
{
  js_ << "ctx.detachShader(" << program << "," << shader << ");";
  GLDEBUG;
}

void WGLWidget::disable(GLenum cap)
{
  js_ << "ctx.disable(" << toString(cap) << ");";
  GLDEBUG;
}

void WGLWidget::drawArrays(GLenum mode, int first, unsigned count)
{
  js_ << "ctx.drawArrays(" << toString(mode) << "," << first << "," << count << ");";
  GLDEBUG;
}

void WGLWidget::drawElements(GLenum mode, unsigned count,
                             GLenum type, unsigned offset)
{
  js_ << "ctx.drawElements(" << toString(mode) << "," << count << ","
    << toString(type) << "," << offset << ");";
}

void WGLWidget::enable(GLenum cap)
{
  js_ << "ctx.enable(" << toString(cap) << ");";
  GLDEBUG;
}

void WGLWidget::enableVertexAttribArray(AttribLocation index)
{
  js_ << "ctx.enableVertexAttribArray(" << index << ");";
  GLDEBUG;
}

void WGLWidget::finish()
{
  js_ << "ctx.finish();";
  GLDEBUG;
}
void WGLWidget::flush()
{
  js_ << "ctx.flush();";
  GLDEBUG;
}

void WGLWidget::frontFace(GLenum mode)
{
  js_ << "ctx.frontFace(" << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::generateMipmap(GLenum target)
{
  js_ << "ctx.generateMipmap(" << toString(target) << ");";
  GLDEBUG;
}

WGLWidget::AttribLocation WGLWidget::getAttribLocation(Program program, const std::string &attrib)
{
  AttribLocation retval = "ctx.WtAttrib" + boost::lexical_cast<std::string>(attributes_++);
  js_ << retval << "=ctx.getAttribLocation(" << program << "," << jsStringLiteral(attrib) << ");";
  GLDEBUG;
  return retval;
}

WGLWidget::UniformLocation WGLWidget::getUniformLocation(Program program, const std::string location)
{
  UniformLocation retval = "ctx.WtUniform" + boost::lexical_cast<std::string>(uniforms_++);
  js_ << retval << "=ctx.getUniformLocation(" << program << "," << jsStringLiteral(location) << ");";
  GLDEBUG;
  return retval;
}

void WGLWidget::hint(GLenum target, GLenum mode)
{
  js_ << "ctx.hint(" << toString(target) << "," << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::lineWidth(double width)
{
  char buf[30];
  js_ << "ctx.lineWidth(" << makeFloat(width, buf) << ");";
  GLDEBUG;
}

void WGLWidget::linkProgram(Program program)
{
  js_ << "ctx.linkProgram(" << program << ");";
  js_ << "if(!ctx.getProgramParameter(" << program << ",ctx.LINK_STATUS)){"
    << "alert('Could not initialize shaders: ' + ctx.getProgramInfoLog(" << program << "));}";
  GLDEBUG;
}

void WGLWidget::pixelStorei(GLenum pname, int param)
{
  js_ << "ctx.pixelStorei(" << toString(pname) << "," << param << ");";
  GLDEBUG;
}

void WGLWidget::polygonOffset(double factor, double units)
{
  char buf[30];
  js_ << "ctx.polygonOffset(" << makeFloat(factor, buf) << ",";
  js_ << makeFloat(units, buf) << ");";
  GLDEBUG;
}
void WGLWidget::sampleCoverage(double value, bool invert)
{
  char buf[30];
  js_ << "ctx.sampleCoverage(" << makeFloat(value, buf) << ","
    << (invert ? "true" : "false") << ");";
  GLDEBUG;
}

void WGLWidget::scissor(int x, int y, unsigned width, unsigned height)
{
  js_ << "ctx.scissor(" << x << "," << y << ","
    << width << "," << height << ");";
  GLDEBUG;
}

void WGLWidget::shaderSource(Shader shader, const std::string &src)
{
  js_ << "ctx.shaderSource(" << shader << "," << jsStringLiteral(src) << ");";
  GLDEBUG;
}

void WGLWidget::stencilFunc(GLenum func, int ref, unsigned mask)
{
  js_ << "ctx.stencilFunc(" << toString(func) << "," << ref << "," << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilFuncSeparate(GLenum face,
                                    GLenum func, int ref,
                                    unsigned mask)
{
  js_ << "ctx.stencilFuncSeparate(" << toString(face) << "," << toString(func)
    << "," << ref << "," << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilMask(unsigned mask)
{
  js_ << "ctx.stencilMask(" << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilMaskSeparate(GLenum face, unsigned mask)
{
  js_ << "ctx.stencilMaskSeparate(" << toString(face) << "," << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilOp(GLenum fail, GLenum zfail,
                          GLenum zpass)
{
  js_ << "ctx.stencilOp(" << toString(fail) << "," << toString(zfail) << ","
    << toString(zpass) << ");";
  GLDEBUG;
}
void WGLWidget::stencilOpSeparate(GLenum face, GLenum fail,
                                  GLenum zfail, GLenum zpass)
{
  js_ << "ctx.stencilOpSeparate(" << toString(face) << ","
    << toString(fail) << "," << toString(zfail) << ","
    << toString(zpass) << ");";
  GLDEBUG;
}

void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           WImage *image)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
    << toString(internalformat) << "," << toString(format) << "," << toString(type)
    << "," << image->jsRef() << ");";
  GLDEBUG;
}
#if 0
void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           W)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
    << toString(internalformat) << "," << toString(format) << "," << toString(type)
    << "," << image << ");";
  GLDEBUG;
}
#endif
void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           WHTML5Video *video)
{
  js_ << "if (" << video->jsMediaRef()<< ")"
    " ctx.texImage2D(" << toString(target) << "," << level << ","
    << toString(internalformat) << "," << toString(format) << "," << toString(type)
    << "," << video->jsMediaRef() << ");";
  GLDEBUG;
}
void WGLWidget::texImage2D(GLenum target, int level,
                           GLenum internalformat,
                           GLenum format, GLenum type,
                           Texture texture)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
    << toString(internalformat) << "," << toString(format) << "," << toString(type)
    << "," << texture << ".image);";
  GLDEBUG;
}

void WGLWidget::texParameteri(GLenum target,
                              GLenum pname,
                              GLenum param)
{
  js_ << "ctx.texParameteri(" << toString(target) << "," << toString(pname)
    << "," << toString(param) << ");";
  GLDEBUG;
}

void WGLWidget::useProgram(Program program)
{
  js_ << "ctx.useProgram(" << program << ");";
  GLDEBUG;
}

void WGLWidget::validateProgram(Program program)
{
  js_ << "ctx.validateProgram(" << program << ");";
  GLDEBUG;
}

void WGLWidget::vertexAttribPointer(AttribLocation location, int size,
                                 GLenum type, bool normalized,
                                 unsigned stride, unsigned offset)
{
  js_ << "ctx.vertexAttribPointer(" << location << "," << size << "," << toString(type)
    << "," << (normalized?"true":"false") << "," << stride << "," << offset << ");";
  GLDEBUG;
}

void WGLWidget::viewport(int x, int y, unsigned width, unsigned height)
{
  js_ << "ctx.viewport(" << x << "," << y << "," << width << "," << height << ");";
  GLDEBUG;
}

void WGLWidget::connectJavaScript(Wt::EventSignalBase &s,
                                  const std::string &methodName)
{
  std::string jsFunction =
    "function(obj, event){"
    """var o=" + glObjJsRef() + ";"
    """if(o) o." + methodName + "(obj,event);"
    "}";
  s.connect(jsFunction);
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::createJavaScriptMatrix4()
{
  WGenericMatrix<double, 4, 4> m; // unit matrix
  JavaScriptMatrix4x4 retval("ctx.WtMatrix" + boost::lexical_cast<std::string>(matrices_++));
  js_ << retval.jsRef() << "=";
  renderfv(js_, m.data().begin(), m.data().end());
  js_ << ";";

  GLDEBUG;
  return retval;
}

void WGLWidget::setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
                                           double centerX, double centerY, double centerZ,
                                           double uX, double uY, double uZ,
                                           double pitchRate, double yawRate)
{
  mouseWentDownSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseDown(o, e);}");
  mouseWentUpSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseUp(o, e);}");
  mouseDraggedSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseDragLookAt(o, e);}");
  mouseWheelSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseWheelLookAt(o, e);}");
  js_ <<
    "obj.setLookAtParams("
    << m.jsRef()
    << ",[" << centerX << "," << centerY << "," << centerZ << "],"
    << "[" << uX << "," << uY << "," << uZ << "],"
    << pitchRate << "," << yawRate << ");";
//  if (this->isRendered()) {
//    doJavaScript(ss.str());
//  } else {
//    delayedJavaScript_ << ss.str();
//  }
}

void WGLWidget::setClientSideWalkHandler(const JavaScriptMatrix4x4 &m, double frontStep, double rotStep)
{
  mouseWentDownSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseDown(o, e);}");
  mouseWentUpSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseUp(o, e);}");
  mouseDraggedSlot_.setJavaScript("function(o, e){" + glObjJsRef() + ".mouseDragWalk(o, e);}");
  mouseWheelSlot_.setJavaScript("function(o, e){}");
  std::stringstream ss;
  ss << "(function(){var o = " << glObjJsRef() << ";"
    "if(o.ctx == null) return;"
    "o.setWalkParams(o."
    << m.jsRef() << ","
    << frontStep << "," << rotStep << ");})()";
  if (this->isRendered()) {
    doJavaScript(ss.str());
  } else {
    delayedJavaScript_ << ss.str();
  }
}

WGLWidget::JavaScriptMatrix4x4::JavaScriptMatrix4x4(const std::string &jsVariable):
  jsRef_(jsVariable)
{
}

WGLWidget::JavaScriptMatrix4x4::JavaScriptMatrix4x4()
{
}

WGLWidget::JavaScriptMatrix4x4::JavaScriptMatrix4x4(const WGLWidget::JavaScriptMatrix4x4 &other):
  jsRef_(other.jsRef())
{
}

WGLWidget::JavaScriptMatrix4x4 &
WGLWidget::JavaScriptMatrix4x4::operator=(const WGLWidget::JavaScriptMatrix4x4 &rhs)
{
  jsRef_ = rhs.jsRef_;
  return *this;
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::inverted() const
{
  return WGLWidget::JavaScriptMatrix4x4(WT_CLASS ".glMatrix.mat4.inverse(" +
    jsRef_ + ", " WT_CLASS ".glMatrix.mat4.create())");
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::transposed() const
{
  return WGLWidget::JavaScriptMatrix4x4(WT_CLASS ".glMatrix.mat4.transpose(" +
    jsRef_ + ", " WT_CLASS ".glMatrix.mat4.create())");
}

WGLWidget::JavaScriptMatrix4x4 WGLWidget::JavaScriptMatrix4x4::operator*(const WGenericMatrix<double, 4, 4> &m) const
{
  std::stringstream ss;
  ss << WT_CLASS ".glMatrix.mat4.multiply(" << jsRef_ << ",";
  WGenericMatrix<double, 4, 4> t(m.transposed());
  renderfv(ss, t.data().begin(), t.data().end());
  ss << ", " WT_CLASS ".glMatrix.mat4.create())";
  return WGLWidget::JavaScriptMatrix4x4(ss.str());
}
