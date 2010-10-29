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
#include "Wt/WApplication"

#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/WGLWidget.min.js"
#endif

using namespace Wt;

// TODO: for uniform*v, attribute8v, generate the non-v version in js. We
//       will probably end up with less bw and there's no use in allocating
//       an extra array.


const char *WGLWidget::toString(BeginModeEnum mode)
{
  switch(mode) {
    case POINTS: return "ctx.POINTS";
    case LINES: return "ctx.LINES";
    case LINE_LOOP: return "ctx.LINE_LOOP";
    case LINE_STRIP: return "ctx.LINE_STRIP";
    case TRIANGLES: return "ctx.TRIANGLES";
    case TRIANGLE_STRIP: return "ctx.TRIANGLE_STRIP";
    case TRIANGLE_FAN: return "ctx.TRIANGLE_FAN";
  }
  return "BAD_BEGIN_MODE_ENUM";
}
const char *WGLWidget::toString(BlendingFactorEnum e)
{
  switch(e) {
    case BF_ZERO: return "ctx.ZERO";
    case ONE: return "ctx.ONE";
    case SRC_COLOR: return "ctx.SRC_COLOR";
    case ONE_MINUS_SRC_COLOR: return "ctx.ONE_MINUS_SRC_COLOR";
    case SRC_ALPHA: return "ctx.SRC_ALPHA";
    case ONE_MINUS_SRC_ALPHA: return "ctx.ONE_MINUS_SRC_ALPHA";
    case DST_ALPHA: return "ctx.DST_ALPHA";
    case ONE_MINUS_DST_ALPHA: return "ctx.ONE_MINUS_DST_ALPHA";
    case DST_COLOR: return "ctx.DST_COLOR";
    case ONE_MINUS_DST_COLOR: return "ctx.ONE_MINUS_DST_COLOR";
    case CONSTANT_COLOR: return "ctx.CONSTANT_COLOR";
    case ONE_MINUS_CONSTANT_COLOR: return "ctx.ONE_MINUS_CONSTANT_COLOR";
    case CONSTANT_ALPHA: return "ctx.CONSTANT_ALPHA";
    case ONE_MINUS_CONSTANT_ALPHA: return "ctx.ONE_MINUS_CONSTANT_ALPHA";
    case SRC_ALPHA_SATURATE: return "ctx.SRC_ALPHA_SATURATE";
  }
  return "BAD_BLENDINGFACTOR_ENUM";
}
  
  
const char *WGLWidget::toString(BlendEquationModeEnum e)
{
  switch(e) {
    case FUNC_ADD: return "ctx.FUNC_ADD";
    case FUNC_SUBTRACT: return "ctx.FUNC_SUBTRACT";
    case FUNC_REVERSE_SUBTRACT: return "ctx.FUNC_REVERSE_SUBTRACT";
  }
  return "BAD_BLENDEQUATIONMODE_ENUM";
}

const char *WGLWidget::toString(BufferEnum target)
{
  switch (target) {
    case ARRAY_BUFFER: return "ctx.ARRAY_BUFFER";
    case ELEMENT_ARRAY_BUFFER: return "ctx.ELEMENT_ARRAY_BUFFER";
  }
  return "BAD_BUFFER_ENUM";
}
const char *WGLWidget::toString(BufferUsageEnum usage)
{
  switch(usage) {
    case STREAM_DRAW: return "ctx.STREAM_DRAW";
    case STATIC_DRAW: return "ctx.STATIC_DRAW";
    case DYNAMIC_DRAW: return "ctx.DYNAMIC_DRAW";
  }
  return "BAD_BUFFER_USAGE_ENUM";
}
const char *WGLWidget::toString(CullFaceModeEnum e)
{
  switch(e) {
    case FRONT: return "ctx.FRONT";
    case BACK: return "ctx.BACK";
    case FRONT_AND_BACK: return "ctx.FRONT_AND_BACK";
  }
  return "BAD_CULLFACEMODE_ENUM";
}
const char *WGLWidget::toString(EnableCapEnum cap)
{
  switch(cap) {
    case CULL_FACE: return "ctx.CULL_FACE";
    case BLEND: return "ctx.BLEND";
    case DITHER: return "ctx.DITHER";
    case STENCIL_TEST: return "ctx.STENCIL_TEST";
    case DEPTH_TEST: return "ctx.DEPTH_TEST";
    case SCISSOR_TEST: return "ctx.SCISSOR_TEST";
    case POLYGON_OFFSET_FILL: return "ctx.POLYGON_OFFSET_FILL";
    case SAMPLE_ALPHA_TO_COVERAGE: return "ctx.SAMPLE_ALPHA_TO_COVERAGE";
    case SAMPLE_COVERAGE: return "ctx.SAMPLE_COVERAGE";
  }
  return "BAD_ENABLE_CAP_ENUM";
}
const char *WGLWidget::toString(FrontFaceDirectionEnum e)
{
  switch(e) {
    case CW: return "ctx.CW";
    case CCW: return "ctx.CCW";
  }
  return "BAD_FRONTFACEDIRECTION_ENUM";
}
const char *WGLWidget::toString(ParameterEnum e)
{
  switch(e) {
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
    case P_SCISSOR_TEST: return "ctx.SCISSOR_TEST";
    case COLOR_CLEAR_VALUE: return "ctx.COLOR_CLEAR_VALUE";
    case COLOR_WRITEMASK: return "ctx.COLOR_WRITEMASK";
    case UNPACK_ALIGNMENT: return "ctx.UNPACK_ALIGNMENT";
    case UNPACK_FLIP_Y_WEBGL: return "ctx.UNPACK_FLIP_Y_WEBGL";
    case UNPACK_PREMULTIPLY_ALPHA_WEBGL: return "ctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL";
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
    case POLYGON_OFFSET_FILL: return "ctx.POLYGON_OFFSET_FILL";
    case POLYGON_OFFSET_FACTOR: return "ctx.POLYGON_OFFSET_FACTOR";
    case TEXTURE_BINDING_2D: return "ctx.TEXTURE_BINDING_2D";
    case SAMPLE_BUFFERS: return "ctx.SAMPLE_BUFFERS";
    case SAMPLES: return "ctx.SAMPLES";
    case SAMPLE_COVERAGE_VALUE: return "ctx.SAMPLE_COVERAGE_VALUE";
    case SAMPLE_COVERAGE_INVERT: return "ctx.SAMPLE_COVERAGE_INVERT";
  }
  return "BAD_PARAMETER_ENUM";
}
const char *WGLWidget::toString(HintModeEnum e)
{
  switch(e) {
    case DONT_CARE: return "ctx.DONT_CARE";
    case FASTEST: return "ctx.FASTEST";
    case NICEST: return "ctx.NICEST";
  }
  return "BAD_HINTMODE_ENUM";
}
const char *WGLWidget::toString(HintTargetEnum e)
{
  switch(e) {
    case GENERATE_MIPMAP_HINT: return "ctx.GENERATE_MIPMAP_HINT";
  }
  return "BAD_HINTTARGET_ENUM";
}
const char *WGLWidget::toString(DataTypeEnum type)
{
  switch(type) {
    case BYTE: return "ctx.BYTE";
    case DT_UNSIGNED_BYTE: return "ctx.UNSIGNED_BYTE";
    case SHORT: return "ctx.SHORT";
    case UNSIGNED_SHORT: return "ctx.UNSIGNED_SHORT";
    case INT: return "ctx.INT";
    case UNSIGNED_INT: return "ctx.UNSIGNED_INT";
    case FLOAT: return "ctx.FLOAT";
  }
  return "BAD_DATA_TYPE_ENUM";
}
const char *WGLWidget::toString(PixelFormatEnum e)
{
  switch(e) {
    case DEPTH_COMPONENT: return "ctx.DEPTH_COMPONENT";
    case ALPHA: return "ctx.ALPHA";
    case RGB: return "ctx.RGB";
    case RGBA: return "ctx.RGBA";
    case LUMINANCE: return "ctx.LUMINANCE";
    case LUMINANCE_ALPHA: return "ctx.LUMINANCE_ALPHA";
  }
  return "BAD_PIXELFORMAT_ENUM";
}

const char *WGLWidget::toString(PixelTypeEnum e)
{
  switch(e) {
    case PT_UNSIGNED_BYTE: return "ctx.UNSIGNED_BYTE";
    case UNSIGNED_SHORT_4_4_4_4: return "ctx.UNSIGNED_SHORT_4_4_4_4";
    case UNSIGNED_SHORT_5_5_5_1: return "ctx.UNSIGNED_SHORT_5_5_5_1";
    case UNSIGNED_SHORT_5_6_5: return "ctx.UNSIGNED_SHORT_5_6_5";
  }
  return "BAD_PIXELTYPE_ENUM";
}
const char *WGLWidget::toString(StencilFunctionEnum f)
{
  switch(f) {
    case NEVER: return "ctx.NEVER";
    case LESS: return "ctx.LESS";
    case EQUAL: return "ctx.EQUAL";
    case LEQUAL: return "ctx.LEQUAL";
    case GREATER: return "ctx.GREATER";
    case NOTEQUAL: return "ctx.NOTEQUAL";
    case GEQUAL: return "ctx.GEQUAL";
    case ALWAYS: return "ctx.ALWAYS";
  }
  return "BAD_STENCIL_FN_ENUM";
}
const char *WGLWidget::toString(StencilOpEnum e)
{
  switch(e) {
    case SO_ZERO: return "ctx.ZERO";
    case KEEP: return "ctx.KEEP";
    case REPLACE: return "ctx.REPLACE";
    case INCR: return "ctx.INCR";
    case DECR: return "ctx.DECR";
    case INVERT: return "ctx.INVERT";
    case INCR_WRAP: return "ctx.INCR_WRAP";
    case DECR_WRAP: return "ctx.DECR_WRAP";
  }
  return "BAD_STENCILOP_ENUM";
}
const char *WGLWidget::toString(TextureFilterEnum e)
{
  switch (e) {
    case NEAREST: return "ctx.NEAREST";
    case LINEAR: return "ctx.LINEAR";
    case NEAREST_MIPMAP_NEAREST: return "ctx.NEAREST_MIPMAP_NEAREST";
    case LINEAR_MIPMAP_NEAREST: return "ctx.LINEAR_MIPMAP_NEAREST";
    case NEAREST_MIPMAP_LINEAR: return "ctx.NEAREST_MIPMAP_LINEAR";
    case LINEAR_MIPMAP_LINEAR: return "ctx.LINEAR_MIPMAP_LINEAR";
  }
  return "BAD_TEXTUREFILTER_ENUM";
}

const char *WGLWidget::toString(TextureParameterNameEnum e)
{
  switch (e) {
    case TEXTURE_MAG_FILTER: return "ctx.TEXTURE_MAG_FILTER";
    case TEXTURE_MIN_FILTER: return "ctx.TEXTURE_MIN_FILTER";
    case TEXTURE_WRAP_S: return "ctx.TEXTURE_WRAP_S";
    case TEXTURE_WRAP_T: return "ctx.TEXTURE_WRAP_T";
  }
  return "BAD_TEXTUREPARAMETERNAME_ENUM";
};
const char *WGLWidget::toString(TextureTargetEnum e)
{
  switch (e) {
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
  }
  return "BAD_TEXTURETARGET_ENUM";
}
const char *WGLWidget::toString(TextureUnitEnum e)
{
  switch (e) {
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
    //case ACTIVE_TEXTURE: return "ctx.ACTIVE_TEXTURE";
  }
  return "BAD_TEXTUREUNIT_ENUM";
}
const char *WGLWidget::toString(TextureWrapModeEnum e)
{
  switch (e) {
    case REPEAT: return "ctx.REPEAT";
    case CLAMP_TO_EDGE: return "ctx.CLAMP_TO_EDGE";
    case MIRRORED_REPEAT: return "ctx.MIRRORED_REPEAT";
  }
  return "BAD_TEXTUREWRAPMODE_ENUM";
}

WGLWidget::WGLWidget(WContainerWidget *parent):
  WInteractWidget(parent),
  shaders_(0),
  programs_(0),
  attributes_(0),
  uniforms_(0),
  buffers_(0),
  textures_(0),
  matrices_(0),
  mouseWentDownSlot_("function(){}", this),
  mouseWentUpSlot_("function(){}", this),
  mouseDraggedSlot_("function(){}", this),
  mouseWheelSlot_("function(){}", this)

{
  setInline(false);
  mouseWentDown().connect(mouseWentDownSlot_);
  mouseWentUp().connect(mouseWentUpSlot_);
  mouseDragged().connect(mouseDraggedSlot_);
  mouseWheel().connect(mouseWheelSlot_);
}

WGLWidget::~WGLWidget()
{
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
  app->require("glMatrix.js");
  DomElement *result = DomElement::createNew(DomElement_CANVAS);;

  result->setAttribute("width", boost::lexical_cast<std::string>(width().value()));
  result->setAttribute("height", boost::lexical_cast<std::string>(height().value()));
  setId(result, app);
  updateDom(*result, true);


  std::stringstream tmp;
  tmp <<
    "new " WT_CLASS ".WGLWidget(" << app->javaScriptClass() << "," << jsRef() << ");"
    //"jQuery.data(" << jsRef() << ",'obj').ctx=ctx;"
    "var o = jQuery.data(" + jsRef() + ",'obj');\n"
    "o.discoverContext();\n"
    "console.log('o.ctx: ' + o.ctx);\n"
    "if(o.ctx){\n"
    """var ctx = o.ctx;\n";
  js_.str("");
  initializeGL();
  tmp << "o.initializeGl=function(){\nvar ctx=jQuery.data("
    << jsRef() << ",'obj').ctx;\n" << js_.str() << "\n};\n";
  js_.str("");
  paintGL();
  tmp << "o.paintGl=function(){\nvar ctx=jQuery.data("
    << jsRef() << ",'obj').ctx;\n" << js_.str() << "};";
  js_.str("");
  // Make sure textures are loaded before we render
  tmp << "}\n";
  if (textures_ > 0) {
    tmp << "new Wt._p_.ImagePreloader([";
    for (unsigned i = 0; i < preloadImages_.size(); ++i) {
      if (i != 0)
	tmp << ',';
      tmp << '\'' << fixRelativeUrl(preloadImages_[i].second) << '\'';
    }
    tmp <<
      "],function(images){";
    for (unsigned i = 0; i < preloadImages_.size(); ++i) {
      std::string texture = preloadImages_[i].first;
      tmp << texture << "=o.ctx.createTexture();\n"
        << texture << ".image=images[" << i << "];";
    }
    tmp << "o.initializeGl();"
           "o.paintGl();"
         "});";
  } else {
    // No textures to load - go and paint
    tmp << "o.initializeGl();o.paintGl();";
  }


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

void WGLWidget::defineJavaScript()
{
  WApplication *app = WApplication::instance();

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

void WGLWidget::activeTexture(TextureUnitEnum texture)
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

void WGLWidget::bindBuffer(BufferEnum target, Buffer buffer)
{
  js_ << "ctx.bindBuffer(" << toString(target) << "," << buffer << ");";
  GLDEBUG;
}

void WGLWidget::bindTexture(TextureTargetEnum target, Texture texture)
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

void WGLWidget::blendEquation(BlendEquationModeEnum mode)
{
  js_ << "ctx.blendEquation(" << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::blendEquationSeparate(BlendEquationModeEnum modeRGB,
                                      BlendEquationModeEnum modeAlpha)
{
  js_ << "ctx.blendEquationSeparate(" << toString(modeRGB) << "," 
    << toString(modeAlpha) << ");";
  GLDEBUG;
}

void WGLWidget::blendFunc(BlendingFactorEnum sfactor, BlendingFactorEnum dfactor)
{
  js_ << "ctx.blendFunc(" << toString(sfactor) << ","
    << toString(dfactor) << ");";
  GLDEBUG;
}

void WGLWidget::blendFuncSeparate(BlendingFactorEnum srcRGB,
                                  BlendingFactorEnum dstRGB,
                                  BlendingFactorEnum srcAlpha,
                                  BlendingFactorEnum dstAlpha)
{
  js_ << "ctx.blendFuncSeparate(" << toString(srcRGB) << ","
    << toString(dstRGB) << "," << toString(srcAlpha) << ","
    << toString(dstAlpha) << ");";
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

void WGLWidget::copyTexImage2D(TextureTargetEnum target, int level,
                               PixelFormatEnum internalFormat,
                               int x, int y,
                               unsigned width, unsigned height, 
                               int border)
{
  js_ << "ctx.copyTexImage2D(" << toString(target) << "," << level << ","
    << toString(internalFormat) << "," << x << "," << y << ","
    << width << "," << height << "," << border << ");";
  GLDEBUG;
}

void WGLWidget::copyTexSubImage2D(TextureTargetEnum target, int level,
                                  int xoffset, int yoffset,
                                  int x, int y,
                                  unsigned width, unsigned height)
{
  js_ << "ctx.copyTexSubImage2D(" << toString(target) << "," << level << ","
    << xoffset << "," << yoffset << "," << x << "," << y << ","
    << width << "," << height << ");";
  GLDEBUG;
}

Buffer WGLWidget::createBuffer()
{
  Buffer retval = "ctx.WtBuffer" + boost::lexical_cast<std::string>(buffers_++);
  js_ << retval << "=ctx.createBuffer();";
  GLDEBUG;
  return retval;
}

Program WGLWidget::createProgram()
{
  Program retval = "ctx.WtProgram" + boost::lexical_cast<std::string>(programs_++);
  js_ << retval << "=ctx.createProgram();";
  GLDEBUG;
  return retval;
}

Shader WGLWidget::createShader(ShaderEnum shader)
{
  Shader retval = "ctx.WtShader" + boost::lexical_cast<std::string>(shaders_++);
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
  GLDEBUG;
  return retval;
}

Texture WGLWidget::createTexture()
{
  Texture retval = "ctx.WtTexture" + boost::lexical_cast<std::string>(textures_++);
  js_ << retval << "=ctx.createTexture();";
  GLDEBUG;
  return retval;
}

Texture WGLWidget::createTextureAndLoad(const std::string &url)
{
  Texture retval = "WtTexture" + boost::lexical_cast<std::string>(textures_++);
  preloadImages_.push_back(std::make_pair<std::string, std::string>(retval, url));
  GLDEBUG;
  return retval;

}

void WGLWidget::cullFace(CullFaceModeEnum mode)
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

void WGLWidget::depthFunc(StencilFunctionEnum func)
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

void WGLWidget::disable(EnableCapEnum cap)
{
  js_ << "ctx.disable(" << toString(cap) << ");";
  GLDEBUG;
}

void WGLWidget::drawArrays(BeginModeEnum mode, int first, unsigned count)
{
  js_ << "ctx.drawArrays(" << toString(mode) << "," << first << "," << count << ");";
  GLDEBUG;
}

void WGLWidget::drawElements(BeginModeEnum mode, unsigned count,
                             DataTypeEnum type, unsigned offset)
{
  js_ << "ctx.drawElements(" << toString(mode) << "," << count << ","
    << toString(type) << "," << offset << ");";
}

void WGLWidget::enable(EnableCapEnum cap)
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

void WGLWidget::frontFace(FrontFaceDirectionEnum mode)
{
  js_ << "ctx.frontFace(" << toString(mode) << ");";
  GLDEBUG;
}

void WGLWidget::generateMipmap(TextureTargetEnum target)
{
  js_ << "ctx.generateMipmap(" << toString(target) << ");";
  GLDEBUG;
}

AttribLocation WGLWidget::getAttribLocation(Program program, const std::string &attrib)
{
  AttribLocation retval = "ctx.WtAttrib" + boost::lexical_cast<std::string>(attributes_++);
  js_ << retval << "=ctx.getAttribLocation(" << program << "," << jsStringLiteral(attrib) << ");";
  GLDEBUG;
  return retval;
}

UniformLocation WGLWidget::getUniformLocation(Program program, const std::string location)
{
  UniformLocation retval = "ctx.WtUniform" + boost::lexical_cast<std::string>(uniforms_++);
  js_ << retval << "=ctx.getUniformLocation(" << program << "," << jsStringLiteral(location) << ");";
  GLDEBUG;
  return retval;
}

void WGLWidget::hint(HintTargetEnum target, HintModeEnum mode)
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
    << "alert('Could not initialize shaders');}";
  GLDEBUG;
}

void WGLWidget::pixelStorei(ParameterEnum pname, int param)
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

void WGLWidget::stencilFunc(StencilFunctionEnum func, int ref, unsigned mask)
{
  js_ << "ctx.stencilFunc(" << toString(func) << "," << ref << "," << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilFuncSeparate(CullFaceModeEnum face,
                                    StencilFunctionEnum func, int ref,
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
void WGLWidget::stencilMaskSeparate(CullFaceModeEnum face, unsigned mask)
{
  js_ << "ctx.stencilMaskSeparate(" << toString(face) << "," << mask << ");";
  GLDEBUG;
}
void WGLWidget::stencilOp(StencilOpEnum fail, StencilOpEnum zfail,
                          StencilOpEnum zpass)
{
  js_ << "ctx.stencilOp(" << toString(fail) << "," << toString(zfail) << ","
    << toString(zpass) << ");";
  GLDEBUG;
}
void WGLWidget::stencilOpSeparate(CullFaceModeEnum face, StencilOpEnum fail,
                                  StencilOpEnum zfail, StencilOpEnum zpass)
{
  js_ << "ctx.stencilOpSeparate(" << toString(face) << ","
    << toString(fail) << "," << toString(zfail) << ","
    << toString(zpass) << ");";
  GLDEBUG;
}

void WGLWidget::texImage2D(TextureTargetEnum target, int level,
                           PixelFormatEnum internalformat,
                           PixelFormatEnum format, PixelTypeEnum type,
                           Texture texture)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
    << toString(internalformat) << "," << toString(format) << "," << toString(type)
    << "," << texture << ".image);";
  GLDEBUG;
}

void WGLWidget::texParameteri(TextureTargetEnum target,
                              TextureParameterNameEnum pname,
                              TextureWrapModeEnum param)
{
  js_ << "ctx.texParameteri(" << toString(target) << "," << toString(pname)
    << "," << toString(param) << ");";
  GLDEBUG;
}
void WGLWidget::texParameteri(TextureTargetEnum target,
                              TextureParameterNameEnum pname,
                              TextureFilterEnum param)
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
                                 DataTypeEnum type, bool normalized,
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
    """var o=jQuery.data(" + jsRef() + ",'obj');"
    """if(o) o." + methodName + "(obj,event);"
    "}";
  s.connect(jsFunction);
}

JavaScriptMatrix4x4 WGLWidget::createJavaScriptMatrix4()
{
  WGenericMatrix<double, 4, 4> m; // unit matrix
  JavaScriptMatrix4x4 retval = "ctx.WtMatrix" + boost::lexical_cast<std::string>(matrices_++);
  js_ << retval << "=";
  renderfv(m.data().begin(), m.data().end());
  js_ << ";";

  return retval;
}

void WGLWidget::setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
                                           double centerX, double centerY, double centerZ,
                                           double uX, double uY, double uZ,
                                           double pitchRate, double yawRate)
{
  mouseWentDownSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseDown(o, e);}");
  mouseWentUpSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseUp(o, e);}");
  mouseDraggedSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseDragLookAt(o, e);}");
  mouseWheelSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseWheelLookAt(o, e);}");
  std::stringstream ss;
  ss << "jQuery.data(" << jsRef() << ",'obj').setLookAtParams("
    << m
    << ",[" << centerX << "," << centerY << "," << centerZ << "],"
    << "[" << uX << "," << uY << "," << uZ << "],"
    << pitchRate << "," << yawRate << ");";
  doJavaScript(ss.str());
}

void WGLWidget::setClientSideWalkHandler(const JavaScriptMatrix4x4 &m, double frontStep, double rotStep)
{
  mouseWentDownSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseDown(o, e);}");
  mouseWentUpSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseUp(o, e);}");
  mouseDraggedSlot_.setJavaScript("function(o, e){jQuery.data(" + jsRef() + ",'obj').mouseDragWalk(o, e);}");
  mouseWheelSlot_.setJavaScript("function(o, e){}");
  std::stringstream ss;
  ss << "jQuery.data(" << jsRef() << ",'obj').setWalkParams("
    << m << ","
    << frontStep << "," << rotStep << ");";
  doJavaScript(ss.str());
}

void WGLWidget::uniformNormalMatrix4(const UniformLocation &u,
                                     const JavaScriptMatrix4x4 &jsm,
                                     const WGenericMatrix<double, 4, 4> &mm)
{
  js_ << "ctx.uniformMatrix4fv(" << u << ",false,mat4.transpose(mat4.inverse(mat4.multiply(mat4.create(" << jsm << "), ";
  WGenericMatrix<double, 4, 4> t(mm.transposed());
  renderfv(t.data().begin(), t.data().end());
  js_ << "))));";
}
