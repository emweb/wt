/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WClientGLWidget.h"
#include "WebUtils.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WFileResource.h"
#include "Wt/WWebWidget.h"
#include "Wt/WMemoryResource.h"
#include "Wt/WCanvasPaintDevice.h"
#include "Wt/WRasterImage.h"
#include "Wt/WImage.h"
#include "Wt/WVideo.h"

#include <ostream>
#include <fstream>
#include <limits>

namespace Wt {

WClientGLWidget::WClientGLWidget(WGLWidget *glInterface)
  : WAbstractGLImplementation(glInterface),
    shaders_(0),
    programs_(0),
    attributes_(0),
    uniforms_(0),
    buffers_(0),
    arrayBuffers_(0),
    framebuffers_(0),
    renderbuffers_(0),
    textures_(0),
    images_(1), // id 0 is reserved for the deprecated preloading with
                // createTextureAndLoad()
    canvas_(0)
{}

#ifndef WT_TARGET_JAVA
const char *WClientGLWidget::makeFloat(double d, char *buf)
{
  return Utils::round_js_str(d, 6, buf);
}

const char *WClientGLWidget::makeInt(int i, char *buf)
{
  return Utils::itoa(i, buf);
}
#endif

const char *WClientGLWidget::toString(WGLWidget::GLenum e)
{
  switch(e) {
    case WGLWidget::DEPTH_BUFFER_BIT: return "ctx.DEPTH_BUFFER_BIT";
    case WGLWidget::STENCIL_BUFFER_BIT: return "ctx.STENCIL_BUFFER_BIT";
    case WGLWidget::COLOR_BUFFER_BIT: return "ctx.COLOR_BUFFER_BIT";
    case WGLWidget::POINTS: return "ctx.POINTS";
    case WGLWidget::LINES: return "ctx.LINES";
    case WGLWidget::LINE_LOOP: return "ctx.LINE_LOOP";
    case WGLWidget::LINE_STRIP: return "ctx.LINE_STRIP";
    case WGLWidget::TRIANGLES: return "ctx.TRIANGLES";
    case WGLWidget::TRIANGLE_STRIP: return "ctx.TRIANGLE_STRIP";
    case WGLWidget::TRIANGLE_FAN: return "ctx.TRIANGLE_FAN";
    //case WGLWidget::ZERO: return "ctx.ZERO";
    //case WGLWidget::ONE: return "ctx.ONE";
    case WGLWidget::SRC_COLOR: return "ctx.SRC_COLOR";
    case WGLWidget::ONE_MINUS_SRC_COLOR: return "ctx.ONE_MINUS_SRC_COLOR";
    case WGLWidget::SRC_ALPHA: return "ctx.SRC_ALPHA";
    case WGLWidget::ONE_MINUS_SRC_ALPHA: return "ctx.ONE_MINUS_SRC_ALPHA";
    case WGLWidget::DST_ALPHA: return "ctx.DST_ALPHA";
    case WGLWidget::ONE_MINUS_DST_ALPHA: return "ctx.ONE_MINUS_DST_ALPHA";
    case WGLWidget::DST_COLOR: return "ctx.DST_COLOR";
    case WGLWidget::ONE_MINUS_DST_COLOR: return "ctx.ONE_MINUS_DST_COLOR";
    case WGLWidget::SRC_ALPHA_SATURATE: return "ctx.SRC_ALPHA_SATURATE";
    case WGLWidget::FUNC_ADD: return "ctx.FUNC_ADD";
    case WGLWidget::BLEND_EQUATION: return "ctx.BLEND_EQUATION";
    //case WGLWidget::BLEND_EQUATION_RGB: return "ctx.BLEND_EQUATION_RGB";
    case WGLWidget::BLEND_EQUATION_ALPHA: return "ctx.BLEND_EQUATION_ALPHA";
    case WGLWidget::FUNC_SUBTRACT: return "ctx.FUNC_SUBTRACT";
    case WGLWidget::FUNC_REVERSE_SUBTRACT: return "ctx.FUNC_REVERSE_SUBTRACT";
    case WGLWidget::BLEND_DST_RGB: return "ctx.BLEND_DST_RGB";
    case WGLWidget::BLEND_SRC_RGB: return "ctx.BLEND_SRC_RGB";
    case WGLWidget::BLEND_DST_ALPHA: return "ctx.BLEND_DST_ALPHA";
    case WGLWidget::BLEND_SRC_ALPHA: return "ctx.BLEND_SRC_ALPHA";
    case WGLWidget::CONSTANT_COLOR: return "ctx.CONSTANT_COLOR";
    case WGLWidget::ONE_MINUS_CONSTANT_COLOR: return "ctx.ONE_MINUS_CONSTANT_COLOR";
    case WGLWidget::CONSTANT_ALPHA: return "ctx.CONSTANT_ALPHA";
    case WGLWidget::ONE_MINUS_CONSTANT_ALPHA: return "ctx.ONE_MINUS_CONSTANT_ALPHA";
    case WGLWidget::BLEND_COLOR: return "ctx.BLEND_COLOR";
    case WGLWidget::ARRAY_BUFFER: return "ctx.ARRAY_BUFFER";
    case WGLWidget::ELEMENT_ARRAY_BUFFER: return "ctx.ELEMENT_ARRAY_BUFFER";
    case WGLWidget::ARRAY_BUFFER_BINDING: return "ctx.ARRAY_BUFFER_BINDING";
    case WGLWidget::ELEMENT_ARRAY_BUFFER_BINDING: return "ctx.ELEMENT_ARRAY_BUFFER_BINDING";
    case WGLWidget::STREAM_DRAW: return "ctx.STREAM_DRAW";
    case WGLWidget::STATIC_DRAW: return "ctx.STATIC_DRAW";
    case WGLWidget::DYNAMIC_DRAW: return "ctx.DYNAMIC_DRAW";
    case WGLWidget::BUFFER_SIZE: return "ctx.BUFFER_SIZE";
    case WGLWidget::BUFFER_USAGE: return "ctx.BUFFER_USAGE";
    case WGLWidget::CURRENT_VERTEX_ATTRIB: return "ctx.CURRENT_VERTEX_ATTRIB";
    case WGLWidget::FRONT: return "ctx.FRONT";
    case WGLWidget::BACK: return "ctx.BACK";
    case WGLWidget::FRONT_AND_BACK: return "ctx.FRONT_AND_BACK";
    case WGLWidget::CULL_FACE: return "ctx.CULL_FACE";
    case WGLWidget::BLEND: return "ctx.BLEND";
    case WGLWidget::DITHER: return "ctx.DITHER";
    case WGLWidget::STENCIL_TEST: return "ctx.STENCIL_TEST";
    case WGLWidget::DEPTH_TEST: return "ctx.DEPTH_TEST";
    case WGLWidget::SCISSOR_TEST: return "ctx.SCISSOR_TEST";
    case WGLWidget::POLYGON_OFFSET_FILL: return "ctx.POLYGON_OFFSET_FILL";
    case WGLWidget::SAMPLE_ALPHA_TO_COVERAGE: return "ctx.SAMPLE_ALPHA_TO_COVERAGE";
    case WGLWidget::SAMPLE_COVERAGE: return "ctx.SAMPLE_COVERAGE";
    //case WGLWidget::NO_ERROR: return "ctx.NO_ERROR";
    case WGLWidget::INVALID_ENUM: return "ctx.INVALID_ENUM";
    case WGLWidget::INVALID_VALUE: return "ctx.INVALID_VALUE";
    case WGLWidget::INVALID_OPERATION: return "ctx.INVALID_OPERATION";
    case WGLWidget::OUT_OF_MEMORY: return "ctx.OUT_OF_MEMORY";
    case WGLWidget::CW: return "ctx.CW";
    case WGLWidget::CCW: return "ctx.CCW";
    case WGLWidget::LINE_WIDTH: return "ctx.LINE_WIDTH";
    case WGLWidget::ALIASED_POINT_SIZE_RANGE: return "ctx.ALIASED_POINT_SIZE_RANGE";
    case WGLWidget::ALIASED_LINE_WIDTH_RANGE: return "ctx.ALIASED_LINE_WIDTH_RANGE";
    case WGLWidget::CULL_FACE_MODE: return "ctx.CULL_FACE_MODE";
    case WGLWidget::FRONT_FACE: return "ctx.FRONT_FACE";
    case WGLWidget::DEPTH_RANGE: return "ctx.DEPTH_RANGE";
    case WGLWidget::DEPTH_WRITEMASK: return "ctx.DEPTH_WRITEMASK";
    case WGLWidget::DEPTH_CLEAR_VALUE: return "ctx.DEPTH_CLEAR_VALUE";
    case WGLWidget::DEPTH_FUNC: return "ctx.DEPTH_FUNC";
    case WGLWidget::STENCIL_CLEAR_VALUE: return "ctx.STENCIL_CLEAR_VALUE";
    case WGLWidget::STENCIL_FUNC: return "ctx.STENCIL_FUNC";
    case WGLWidget::STENCIL_FAIL: return "ctx.STENCIL_FAIL";
    case WGLWidget::STENCIL_PASS_DEPTH_FAIL: return "ctx.STENCIL_PASS_DEPTH_FAIL";
    case WGLWidget::STENCIL_PASS_DEPTH_PASS: return "ctx.STENCIL_PASS_DEPTH_PASS";
    case WGLWidget::STENCIL_REF: return "ctx.STENCIL_REF";
    case WGLWidget::STENCIL_VALUE_MASK: return "ctx.STENCIL_VALUE_MASK";
    case WGLWidget::STENCIL_WRITEMASK: return "ctx.STENCIL_WRITEMASK";
    case WGLWidget::STENCIL_BACK_FUNC: return "ctx.STENCIL_BACK_FUNC";
    case WGLWidget::STENCIL_BACK_FAIL: return "ctx.STENCIL_BACK_FAIL";
    case WGLWidget::STENCIL_BACK_PASS_DEPTH_FAIL: return "ctx.STENCIL_BACK_PASS_DEPTH_FAIL";
    case WGLWidget::STENCIL_BACK_PASS_DEPTH_PASS: return "ctx.STENCIL_BACK_PASS_DEPTH_PASS";
    case WGLWidget::STENCIL_BACK_REF: return "ctx.STENCIL_BACK_REF";
    case WGLWidget::STENCIL_BACK_VALUE_MASK: return "ctx.STENCIL_BACK_VALUE_MASK";
    case WGLWidget::STENCIL_BACK_WRITEMASK: return "ctx.STENCIL_BACK_WRITEMASK";
    case WGLWidget::VIEWPORT: return "ctx.VIEWPORT";
    case WGLWidget::SCISSOR_BOX: return "ctx.SCISSOR_BOX";
    case WGLWidget::COLOR_CLEAR_VALUE: return "ctx.COLOR_CLEAR_VALUE";
    case WGLWidget::COLOR_WRITEMASK: return "ctx.COLOR_WRITEMASK";
    case WGLWidget::UNPACK_ALIGNMENT: return "ctx.UNPACK_ALIGNMENT";
    case WGLWidget::PACK_ALIGNMENT: return "ctx.PACK_ALIGNMENT";
    case WGLWidget::MAX_TEXTURE_SIZE: return "ctx.MAX_TEXTURE_SIZE";
    case WGLWidget::MAX_VIEWPORT_DIMS: return "ctx.MAX_VIEWPORT_DIMS";
    case WGLWidget::SUBPIXEL_BITS: return "ctx.SUBPIXEL_BITS";
    case WGLWidget::RED_BITS: return "ctx.RED_BITS";
    case WGLWidget::GREEN_BITS: return "ctx.GREEN_BITS";
    case WGLWidget::BLUE_BITS: return "ctx.BLUE_BITS";
    case WGLWidget::ALPHA_BITS: return "ctx.ALPHA_BITS";
    case WGLWidget::DEPTH_BITS: return "ctx.DEPTH_BITS";
    case WGLWidget::STENCIL_BITS: return "ctx.STENCIL_BITS";
    case WGLWidget::POLYGON_OFFSET_UNITS: return "ctx.POLYGON_OFFSET_UNITS";
    case WGLWidget::POLYGON_OFFSET_FACTOR: return "ctx.POLYGON_OFFSET_FACTOR";
    case WGLWidget::TEXTURE_BINDING_2D: return "ctx.TEXTURE_BINDING_2D";
    case WGLWidget::SAMPLE_BUFFERS: return "ctx.SAMPLE_BUFFERS";
    case WGLWidget::SAMPLES: return "ctx.SAMPLES";
    case WGLWidget::SAMPLE_COVERAGE_VALUE: return "ctx.SAMPLE_COVERAGE_VALUE";
    case WGLWidget::SAMPLE_COVERAGE_INVERT: return "ctx.SAMPLE_COVERAGE_INVERT";
    case WGLWidget::NUM_COMPRESSED_TEXTURE_FORMATS: return "ctx.NUM_COMPRESSED_TEXTURE_FORMATS";
    case WGLWidget::COMPRESSED_TEXTURE_FORMATS: return "ctx.COMPRESSED_TEXTURE_FORMATS";
    case WGLWidget::DONT_CARE: return "ctx.DONT_CARE";
    case WGLWidget::FASTEST: return "ctx.FASTEST";
    case WGLWidget::NICEST: return "ctx.NICEST";
    case WGLWidget::GENERATE_MIPMAP_HINT: return "ctx.GENERATE_MIPMAP_HINT";
    case WGLWidget::BYTE: return "ctx.BYTE";
    case WGLWidget::UNSIGNED_BYTE: return "ctx.UNSIGNED_BYTE";
    case WGLWidget::SHORT: return "ctx.SHORT";
    case WGLWidget::UNSIGNED_SHORT: return "ctx.UNSIGNED_SHORT";
    case WGLWidget::INT: return "ctx.INT";
    case WGLWidget::UNSIGNED_INT: return "ctx.UNSIGNED_INT";
    case WGLWidget::FLOAT: return "ctx.FLOAT";
    case WGLWidget::DEPTH_COMPONENT: return "ctx.DEPTH_COMPONENT";
    case WGLWidget::ALPHA: return "ctx.ALPHA";
    case WGLWidget::RGB: return "ctx.RGB";
    case WGLWidget::RGBA: return "ctx.RGBA";
    case WGLWidget::LUMINANCE: return "ctx.LUMINANCE";
    case WGLWidget::LUMINANCE_ALPHA: return "ctx.LUMINANCE_ALPHA";
    case WGLWidget::UNSIGNED_SHORT_4_4_4_4: return "ctx.UNSIGNED_SHORT_4_4_4_4";
    case WGLWidget::UNSIGNED_SHORT_5_5_5_1: return "ctx.UNSIGNED_SHORT_5_5_5_1";
    case WGLWidget::UNSIGNED_SHORT_5_6_5: return "ctx.UNSIGNED_SHORT_5_6_5";
    case WGLWidget::FRAGMENT_SHADER: return "ctx.FRAGMENT_SHADER";
    case WGLWidget::VERTEX_SHADER: return "ctx.VERTEX_SHADER";
    case WGLWidget::MAX_VERTEX_ATTRIBS: return "ctx.MAX_VERTEX_ATTRIBS";
    case WGLWidget::MAX_VERTEX_UNIFORM_VECTORS: return "ctx.MAX_VERTEX_UNIFORM_VECTORS";
    case WGLWidget::MAX_VARYING_VECTORS: return "ctx.MAX_VARYING_VECTORS";
    case WGLWidget::MAX_COMBINED_TEXTURE_IMAGE_UNITS: return "ctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS";
    case WGLWidget::MAX_VERTEX_TEXTURE_IMAGE_UNITS: return "ctx.MAX_VERTEX_TEXTURE_IMAGE_UNITS";
    case WGLWidget::MAX_TEXTURE_IMAGE_UNITS: return "ctx.MAX_TEXTURE_IMAGE_UNITS";
    case WGLWidget::MAX_FRAGMENT_UNIFORM_VECTORS: return "ctx.MAX_FRAGMENT_UNIFORM_VECTORS";
    case WGLWidget::SHADER_TYPE: return "ctx.SHADER_TYPE";
    case WGLWidget::DELETE_STATUS: return "ctx.DELETE_STATUS";
    case WGLWidget::LINK_STATUS: return "ctx.LINK_STATUS";
    case WGLWidget::VALIDATE_STATUS: return "ctx.VALIDATE_STATUS";
    case WGLWidget::ATTACHED_SHADERS: return "ctx.ATTACHED_SHADERS";
    case WGLWidget::ACTIVE_UNIFORMS: return "ctx.ACTIVE_UNIFORMS";
    case WGLWidget::ACTIVE_UNIFORM_MAX_LENGTH: return "ctx.ACTIVE_UNIFORM_MAX_LENGTH";
    case WGLWidget::ACTIVE_ATTRIBUTES: return "ctx.ACTIVE_ATTRIBUTES";
    case WGLWidget::ACTIVE_ATTRIBUTE_MAX_LENGTH: return "ctx.ACTIVE_ATTRIBUTE_MAX_LENGTH";
    case WGLWidget::SHADING_LANGUAGE_VERSION: return "ctx.SHADING_LANGUAGE_VERSION";
    case WGLWidget::CURRENT_PROGRAM: return "ctx.CURRENT_PROGRAM";
    case WGLWidget::NEVER: return "ctx.NEVER";
    case WGLWidget::LESS: return "ctx.LESS";
    case WGLWidget::EQUAL: return "ctx.EQUAL";
    case WGLWidget::LEQUAL: return "ctx.LEQUAL";
    case WGLWidget::GREATER: return "ctx.GREATER";
    case WGLWidget::NOTEQUAL: return "ctx.NOTEQUAL";
    case WGLWidget::GEQUAL: return "ctx.GEQUAL";
    case WGLWidget::ALWAYS: return "ctx.ALWAYS";
    case WGLWidget::KEEP: return "ctx.KEEP";
    case WGLWidget::REPLACE: return "ctx.REPLACE";
    case WGLWidget::INCR: return "ctx.INCR";
    case WGLWidget::DECR: return "ctx.DECR";
    case WGLWidget::INVERT: return "ctx.INVERT";
    case WGLWidget::INCR_WRAP: return "ctx.INCR_WRAP";
    case WGLWidget::DECR_WRAP: return "ctx.DECR_WRAP";
    case WGLWidget::VENDOR: return "ctx.VENDOR";
    case WGLWidget::RENDERER: return "ctx.RENDERER";
    case WGLWidget::VERSION: return "ctx.VERSION";
    case WGLWidget::NEAREST: return "ctx.NEAREST";
    case WGLWidget::LINEAR: return "ctx.LINEAR";
    case WGLWidget::NEAREST_MIPMAP_NEAREST: return "ctx.NEAREST_MIPMAP_NEAREST";
    case WGLWidget::LINEAR_MIPMAP_NEAREST: return "ctx.LINEAR_MIPMAP_NEAREST";
    case WGLWidget::NEAREST_MIPMAP_LINEAR: return "ctx.NEAREST_MIPMAP_LINEAR";
    case WGLWidget::LINEAR_MIPMAP_LINEAR: return "ctx.LINEAR_MIPMAP_LINEAR";
    case WGLWidget::TEXTURE_MAG_FILTER: return "ctx.TEXTURE_MAG_FILTER";
    case WGLWidget::TEXTURE_MIN_FILTER: return "ctx.TEXTURE_MIN_FILTER";
    case WGLWidget::TEXTURE_WRAP_S: return "ctx.TEXTURE_WRAP_S";
    case WGLWidget::TEXTURE_WRAP_T: return "ctx.TEXTURE_WRAP_T";
    case WGLWidget::TEXTURE_2D: return "ctx.TEXTURE_2D";
    case WGLWidget::TEXTURE: return "ctx.TEXTURE";
    case WGLWidget::TEXTURE_CUBE_MAP: return "ctx.TEXTURE_CUBE_MAP";
    case WGLWidget::TEXTURE_BINDING_CUBE_MAP: return "ctx.TEXTURE_BINDING_CUBE_MAP";
    case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_X: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_X";
    case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_X: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_X";
    case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_Y: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_Y";
    case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_Y: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_Y";
    case WGLWidget::TEXTURE_CUBE_MAP_POSITIVE_Z: return "ctx.TEXTURE_CUBE_MAP_POSITIVE_Z";
    case WGLWidget::TEXTURE_CUBE_MAP_NEGATIVE_Z: return "ctx.TEXTURE_CUBE_MAP_NEGATIVE_Z";
    case WGLWidget::MAX_CUBE_MAP_TEXTURE_SIZE: return "ctx.MAX_CUBE_MAP_TEXTURE_SIZE";
    case WGLWidget::TEXTURE0: return "ctx.TEXTURE0";
    case WGLWidget::TEXTURE1: return "ctx.TEXTURE1";
    case WGLWidget::TEXTURE2: return "ctx.TEXTURE2";
    case WGLWidget::TEXTURE3: return "ctx.TEXTURE3";
    case WGLWidget::TEXTURE4: return "ctx.TEXTURE4";
    case WGLWidget::TEXTURE5: return "ctx.TEXTURE5";
    case WGLWidget::TEXTURE6: return "ctx.TEXTURE6";
    case WGLWidget::TEXTURE7: return "ctx.TEXTURE7";
    case WGLWidget::TEXTURE8: return "ctx.TEXTURE8";
    case WGLWidget::TEXTURE9: return "ctx.TEXTURE9";
    case WGLWidget::TEXTURE10: return "ctx.TEXTURE10";
    case WGLWidget::TEXTURE11: return "ctx.TEXTURE11";
    case WGLWidget::TEXTURE12: return "ctx.TEXTURE12";
    case WGLWidget::TEXTURE13: return "ctx.TEXTURE13";
    case WGLWidget::TEXTURE14: return "ctx.TEXTURE14";
    case WGLWidget::TEXTURE15: return "ctx.TEXTURE15";
    case WGLWidget::TEXTURE16: return "ctx.TEXTURE16";
    case WGLWidget::TEXTURE17: return "ctx.TEXTURE17";
    case WGLWidget::TEXTURE18: return "ctx.TEXTURE18";
    case WGLWidget::TEXTURE19: return "ctx.TEXTURE19";
    case WGLWidget::TEXTURE20: return "ctx.TEXTURE20";
    case WGLWidget::TEXTURE21: return "ctx.TEXTURE21";
    case WGLWidget::TEXTURE22: return "ctx.TEXTURE22";
    case WGLWidget::TEXTURE23: return "ctx.TEXTURE23";
    case WGLWidget::TEXTURE24: return "ctx.TEXTURE24";
    case WGLWidget::TEXTURE25: return "ctx.TEXTURE25";
    case WGLWidget::TEXTURE26: return "ctx.TEXTURE26";
    case WGLWidget::TEXTURE27: return "ctx.TEXTURE27";
    case WGLWidget::TEXTURE28: return "ctx.TEXTURE28";
    case WGLWidget::TEXTURE29: return "ctx.TEXTURE29";
    case WGLWidget::TEXTURE30: return "ctx.TEXTURE30";
    case WGLWidget::TEXTURE31: return "ctx.TEXTURE31";
    case WGLWidget::ACTIVE_TEXTURE: return "ctx.ACTIVE_TEXTURE";
    case WGLWidget::REPEAT: return "ctx.REPEAT";
    case WGLWidget::CLAMP_TO_EDGE: return "ctx.CLAMP_TO_EDGE";
    case WGLWidget::MIRRORED_REPEAT: return "ctx.MIRRORED_REPEAT";
    case WGLWidget::FLOAT_VEC2: return "ctx.FLOAT_VEC2";
    case WGLWidget::FLOAT_VEC3: return "ctx.FLOAT_VEC3";
    case WGLWidget::FLOAT_VEC4: return "ctx.FLOAT_VEC4";
    case WGLWidget::INT_VEC2: return "ctx.INT_VEC2";
    case WGLWidget::INT_VEC3: return "ctx.INT_VEC3";
    case WGLWidget::INT_VEC4: return "ctx.INT_VEC4";
    case WGLWidget::BOOL: return "ctx.BOOL";
    case WGLWidget::BOOL_VEC2: return "ctx.BOOL_VEC2";
    case WGLWidget::BOOL_VEC3: return "ctx.BOOL_VEC3";
    case WGLWidget::BOOL_VEC4: return "ctx.BOOL_VEC4";
    case WGLWidget::FLOAT_MAT2: return "ctx.FLOAT_MAT2";
    case WGLWidget::FLOAT_MAT3: return "ctx.FLOAT_MAT3";
    case WGLWidget::FLOAT_MAT4: return "ctx.FLOAT_MAT4";
    case WGLWidget::SAMPLER_2D: return "ctx.SAMPLER_2D";
    case WGLWidget::SAMPLER_CUBE: return "ctx.SAMPLER_CUBE";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_ENABLED: return "ctx.VERTEX_ATTRIB_ARRAY_ENABLED";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_SIZE: return "ctx.VERTEX_ATTRIB_ARRAY_SIZE";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_STRIDE: return "ctx.VERTEX_ATTRIB_ARRAY_STRIDE";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_TYPE: return "ctx.VERTEX_ATTRIB_ARRAY_TYPE";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_NORMALIZED: return "ctx.VERTEX_ATTRIB_ARRAY_NORMALIZED";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_POINTER: return "ctx.VERTEX_ATTRIB_ARRAY_POINTER";
    case WGLWidget::VERTEX_ATTRIB_ARRAY_BUFFER_BINDING: return "ctx.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING";
    case WGLWidget::COMPILE_STATUS: return "ctx.COMPILE_STATUS";
    case WGLWidget::INFO_LOG_LENGTH: return "ctx.INFO_LOG_LENGTH";
    case WGLWidget::SHADER_SOURCE_LENGTH: return "ctx.SHADER_SOURCE_LENGTH";
    case WGLWidget::LOW_FLOAT: return "ctx.LOW_FLOAT";
    case WGLWidget::MEDIUM_FLOAT: return "ctx.MEDIUM_FLOAT";
    case WGLWidget::HIGH_FLOAT: return "ctx.HIGH_FLOAT";
    case WGLWidget::LOW_INT: return "ctx.LOW_INT";
    case WGLWidget::MEDIUM_INT: return "ctx.MEDIUM_INT";
    case WGLWidget::HIGH_INT: return "ctx.HIGH_INT";
    case WGLWidget::FRAMEBUFFER: return "ctx.FRAMEBUFFER";
    case WGLWidget::RENDERBUFFER: return "ctx.RENDERBUFFER";
    case WGLWidget::RGBA4: return "ctx.RGBA4";
    case WGLWidget::RGB5_A1: return "ctx.RGB5_A1";
    case WGLWidget::RGB565: return "ctx.RGB565";
    case WGLWidget::DEPTH_COMPONENT16: return "ctx.DEPTH_COMPONENT16";
    case WGLWidget::STENCIL_INDEX: return "ctx.STENCIL_INDEX";
    case WGLWidget::STENCIL_INDEX8: return "ctx.STENCIL_INDEX8";
    case WGLWidget::DEPTH_STENCIL: return "ctx.DEPTH_STENCIL";
    case WGLWidget::RENDERBUFFER_WIDTH: return "ctx.RENDERBUFFER_WIDTH";
    case WGLWidget::RENDERBUFFER_HEIGHT: return "ctx.RENDERBUFFER_HEIGHT";
    case WGLWidget::RENDERBUFFER_INTERNAL_FORMAT: return "ctx.RENDERBUFFER_INTERNAL_FORMAT";
    case WGLWidget::RENDERBUFFER_RED_SIZE: return "ctx.RENDERBUFFER_RED_SIZE";
    case WGLWidget::RENDERBUFFER_GREEN_SIZE: return "ctx.RENDERBUFFER_GREEN_SIZE";
    case WGLWidget::RENDERBUFFER_BLUE_SIZE: return "ctx.RENDERBUFFER_BLUE_SIZE";
    case WGLWidget::RENDERBUFFER_ALPHA_SIZE: return "ctx.RENDERBUFFER_ALPHA_SIZE";
    case WGLWidget::RENDERBUFFER_DEPTH_SIZE: return "ctx.RENDERBUFFER_DEPTH_SIZE";
    case WGLWidget::RENDERBUFFER_STENCIL_SIZE: return "ctx.RENDERBUFFER_STENCIL_SIZE";
    case WGLWidget::FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE: return "ctx.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE";
    case WGLWidget::FRAMEBUFFER_ATTACHMENT_OBJECT_NAME: return "ctx.FRAMEBUFFER_ATTACHMENT_OBJECT_NAME";
    case WGLWidget::FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL: return "ctx.FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL";
    case WGLWidget::FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE: return "ctx.FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE";
    case WGLWidget::COLOR_ATTACHMENT0: return "ctx.COLOR_ATTACHMENT0";
    case WGLWidget::DEPTH_ATTACHMENT: return "ctx.DEPTH_ATTACHMENT";
    case WGLWidget::STENCIL_ATTACHMENT: return "ctx.STENCIL_ATTACHMENT";
    case WGLWidget::DEPTH_STENCIL_ATTACHMENT: return "ctx.DEPTH_STENCIL_ATTACHMENT";
    //case WGLWidget::NONE: return "ctx.NONE";
    case WGLWidget::FRAMEBUFFER_COMPLETE: return "ctx.FRAMEBUFFER_COMPLETE";
    case WGLWidget::FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "ctx.FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case WGLWidget::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "ctx.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case WGLWidget::FRAMEBUFFER_INCOMPLETE_DIMENSIONS: return "ctx.FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
    case WGLWidget::FRAMEBUFFER_UNSUPPORTED: return "ctx.FRAMEBUFFER_UNSUPPORTED";
    case WGLWidget::FRAMEBUFFER_BINDING: return "ctx.FRAMEBUFFER_BINDING";
    case WGLWidget::RENDERBUFFER_BINDING: return "ctx.RENDERBUFFER_BINDING";
    case WGLWidget::MAX_RENDERBUFFER_SIZE: return "ctx.MAX_RENDERBUFFER_SIZE";
    case WGLWidget::INVALID_FRAMEBUFFER_OPERATION: return "ctx.INVALID_FRAMEBUFFER_OPERATION";
    case WGLWidget::UNPACK_FLIP_Y_WEBGL: return "ctx.UNPACK_FLIP_Y_WEBGL";
    case WGLWidget::UNPACK_PREMULTIPLY_ALPHA_WEBGL: return "ctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL";
    case WGLWidget::CONTEXT_LOST_WEBGL: return "ctx.CONTEXT_LOST_WEBGL";
    case WGLWidget::UNPACK_COLORSPACE_CONVERSION_WEBGL: return "ctx.UNPACK_COLORSPACE_CONVERSION_WEBGL";
    case WGLWidget::BROWSER_DEFAULT_WEBGL: return "ctx.BROWSER_DEFAULT_WEBGL";
  }
  return "BAD_GL_ENUM";
}

void WClientGLWidget::debugger()
{
  js_ << "debugger;\n";
}

void WClientGLWidget::activeTexture(WGLWidget::GLenum texture)
{
  js_ << "ctx.activeTexture(" << toString(texture) << ");";
  GLDEBUG;
}

void WClientGLWidget::attachShader(WGLWidget::Program program,
				   WGLWidget::Shader shader)
{
  js_ << "ctx.attachShader(" << program.jsRef() 
      << ", " << shader.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::bindAttribLocation(WGLWidget::Program program,
					 unsigned index,
					 const std::string &name)
{
  js_ << "ctx.bindAttribLocation(" << program.jsRef() << "," << index
      << "," << WWebWidget::jsStringLiteral(name) << ");";
  GLDEBUG;
}

void WClientGLWidget::bindBuffer(WGLWidget::GLenum target,
				 WGLWidget::Buffer buffer)
{
  js_ << "ctx.bindBuffer(" << toString(target) << "," << buffer.jsRef() << ");";
  currentlyBoundBuffer_ = buffer;
  GLDEBUG;
}

void WClientGLWidget::bindFramebuffer(WGLWidget::GLenum target,
				      WGLWidget::Framebuffer buffer)
{
  js_ << "ctx.bindFramebuffer(" << toString(target) << "," << buffer.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::bindRenderbuffer(WGLWidget::GLenum target,
				       WGLWidget::Renderbuffer buffer)
{
  js_ << "ctx.bindRenderbuffer(" << toString(target) << "," << buffer.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::bindTexture(WGLWidget::GLenum target,
				  WGLWidget::Texture texture)
{
  js_ << "ctx.bindTexture(" << toString(target) << "," 
      << texture.jsRef() << ");";
  currentlyBoundTexture_ = texture;
  GLDEBUG;
}

void WClientGLWidget::blendColor(double red, double green, double blue,
				 double alpha)
{
  char buf[30];
  js_ << "ctx.blendColor(" << makeFloat(red, buf) << ",";
  js_ << makeFloat(green, buf) << ",";
  js_ << makeFloat(blue, buf) << ",";
  js_ << makeFloat(alpha, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::blendEquation(WGLWidget::GLenum mode)
{
  js_ << "ctx.blendEquation(" << toString(mode) << ");";
  GLDEBUG;
}

void WClientGLWidget::blendEquationSeparate(WGLWidget::GLenum modeRGB,
					    WGLWidget::GLenum modeAlpha)
{
  js_ << "ctx.blendEquationSeparate(" << toString(modeRGB) << "," 
    << toString(modeAlpha) << ");";
  GLDEBUG;
}

void WClientGLWidget::blendFunc(WGLWidget::GLenum sfactor,
				WGLWidget::GLenum dfactor)
{
  js_ << "ctx.blendFunc(" << toString(sfactor) << ","
    << toString(dfactor) << ");";
  GLDEBUG;
}

void WClientGLWidget::blendFuncSeparate(WGLWidget::GLenum srcRGB,
					WGLWidget::GLenum dstRGB,
					WGLWidget::GLenum srcAlpha,
					WGLWidget::GLenum dstAlpha)
{
  js_ << "ctx.blendFuncSeparate(" << toString(srcRGB) << ","
    << toString(dstRGB) << "," << toString(srcAlpha) << ","
    << toString(dstAlpha) << ");";
  GLDEBUG;
}

void WClientGLWidget::bufferData(WGLWidget::GLenum target,
				 WGLWidget::ArrayBuffer res,
				 WGLWidget::GLenum usage)
{
  js_ << "ctx.bufferData(" << toString(target) << ",";
  js_ << res.jsRef() << ".data, ";
  js_ << toString(usage) << ");";
  GLDEBUG;
}

void WClientGLWidget::bufferData(WGLWidget::GLenum target,
				 WGLWidget::ArrayBuffer res,
				 unsigned bufferResourceOffset,
				 unsigned bufferResourceSize,
				 WGLWidget::GLenum usage)
{
  js_ << "ctx.bufferData(" << toString(target) << ",";
  js_ << res.jsRef() << ".data.slice(" << bufferResourceOffset << ","
      << bufferResourceOffset + bufferResourceSize << "),";
  js_ << toString(usage) << ");";
  GLDEBUG;
}

void WClientGLWidget::bufferSubData(WGLWidget::GLenum target, unsigned offset,
				    WGLWidget::ArrayBuffer res)
{
  js_ << "ctx.bufferSubData(" << toString(target) << ",";
  js_ << offset << ",";
  js_ << res.jsRef() << ".data);";
  GLDEBUG;
}

void WClientGLWidget::bufferSubData(WGLWidget::GLenum target, unsigned offset,
				    WGLWidget::ArrayBuffer res,
				    unsigned bufferResourceOffset,
				    unsigned bufferResourceSize)
{
  js_ << "ctx.bufferSubData(" << toString(target) << ",";
  js_ << offset << ",";
  js_ << res.jsRef() << ".data.slice("<< bufferResourceOffset <<", "
      << bufferResourceOffset + bufferResourceSize << "));";
  GLDEBUG;
}

void WClientGLWidget::bufferData(WGLWidget::GLenum target, int size,
				 WGLWidget::GLenum usage)
{
  js_ << "ctx.bufferData(" << toString(target) << ",";
  js_ << size << ",";
  js_ << toString(usage) << ");";
  GLDEBUG;
}

void WClientGLWidget::bufferDatafv(WGLWidget::GLenum target,
				   const FloatBuffer &v,
				   WGLWidget::GLenum usage,
				   bool binary)
{
  if (binary) {
    std::unique_ptr<WMemoryResource> res
      (new WMemoryResource("application/octet"));
    auto resPtr = res.get();
    res->setData(Utils::toCharPointer(v), v.size()*sizeof(float));
    binaryResources_.push_back(std::move(res));
    preloadArrayBuffers_.push_back(PreloadArrayBuffer(currentlyBoundBuffer_.jsRef(), resPtr->url()));

    js_ << "ctx.bufferData(" << toString(target) << ",";
    js_ << currentlyBoundBuffer_.jsRef() << ".data, ";
    js_ << toString(usage) << ");";
  } else {
#ifdef WT_TARGET_JAVA
    bufferDatafv(target, v.asFloatBuffer(), usage);
#else
    js_ << "ctx.bufferData(" << toString(target) << ",";
    js_ << "new Float32Array([";
    char buf[30];
    for (unsigned i = 0; i < v.size(); i++) {
      js_ << (i == 0 ? "" : ",") << makeFloat(v[i], buf);
    }
    js_ << "])";
    js_ << ","<< toString(usage) << ");";
#endif
  }
  GLDEBUG;
}

#ifdef WT_TARGET_JAVA
  void WClientGLWidget::bufferDatafv(WGLWidget::GLenum target, const FloatNotByteBuffer &buffer, WGLWidget::GLenum usage)
  {
    js_ << "ctx.bufferData(" << toString(target) << ",";
    js_ << "new Float32Array([";
    char buf[30];
    for (int i=0; i < buffer.size(); i++) {
      js_ << (i == 0 ? "" : ",") << makeFloat(buffer[i], buf);
    }
    js_ << "])";
    js_ << ","<< toString(usage) << ");";
  }
#endif

void WClientGLWidget::bufferDataiv(WGLWidget::GLenum target, IntBuffer &buffer, WGLWidget::GLenum usage, WGLWidget::GLenum type)
{
  js_ << "ctx.bufferData(" << toString(target) << ",";
  renderiv(js_, buffer, type);
  js_ << ","<< toString(usage) << ");";
  GLDEBUG;
}

void WClientGLWidget::bufferSubDatafv(WGLWidget::GLenum target, unsigned offset, const FloatBuffer &buffer, bool binary)
{
  if (binary) {
    std::unique_ptr<WMemoryResource> res
      (new WMemoryResource("application/octet"));
    res->setData(Utils::toCharPointer(buffer), buffer.size()*sizeof(float));
    preloadArrayBuffers_.push_back(PreloadArrayBuffer(currentlyBoundBuffer_.jsRef(), res->url()));
    binaryResources_.push_back(std::move(res));

    js_ << "ctx.bufferSubData(" << toString(target) << ",";
    js_ << offset << ",";
    js_ << currentlyBoundBuffer_.jsRef() << ".data);";
  } else {
#ifdef WT_TARGET_JAVA
    bufferSubDatafv(target, offset, buffer.asFloatBuffer());
#else
    js_ << "ctx.bufferSubData(" << toString(target) << ",";
    js_ << offset << ",";
    js_ << "new Float32Array([";
    char buf[30];
    for (unsigned i = 0; i < buffer.size(); i++) {
      js_ << (i == 0 ? "" : ",") << makeFloat(buffer[i], buf);
    }
    js_ << "])";
    js_ << ");";
#endif
  }
  GLDEBUG;
}

#ifdef WT_TARGET_JAVA
void WClientGLWidget::bufferSubDatafv(WGLWidget::GLenum target, unsigned offset, const FloatNotByteBuffer &buffer)
{
  js_ << "ctx.bufferSubData(" << toString(target) << ",";
  js_ << offset << ",";
  js_ << "new Float32Array([";
  char buf[30];
  for (int i=0; i < buffer.size(); i++) {
    js_ << (i == 0 ? "" : ",") << makeFloat(buffer[i], buf);
  }
  js_ << "])";
  js_ << ");";
}
#endif

void WClientGLWidget::bufferSubDataiv(WGLWidget::GLenum target,
				      unsigned offset, IntBuffer &buffer, 
				      WGLWidget::GLenum type)
{
  js_ << "ctx.bufferSubData(" << toString(target) << "," << offset << ",";
  renderiv(js_, buffer, type);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::clearBinaryResources()
{
  binaryResources_.clear();
}

void WClientGLWidget::clear(WFlags<WGLWidget::GLenum> mask)
{
  js_ << "ctx.clear(";
  if (mask & WGLWidget::COLOR_BUFFER_BIT) js_ << "ctx.COLOR_BUFFER_BIT|";
  if (mask & WGLWidget::DEPTH_BUFFER_BIT) js_ << "ctx.DEPTH_BUFFER_BIT|";
  if (mask & WGLWidget::STENCIL_BUFFER_BIT) js_ << "ctx.STENCIL_BUFFER_BIT|";

  js_ << "0);";
  GLDEBUG;
}

void WClientGLWidget::clearColor(double r, double g, double b, double a)
{
  char buf[30];
  js_ << "ctx.clearColor(" << makeFloat(r, buf) << ",";
  js_ << makeFloat(g, buf) << ",";
  js_ << makeFloat(b, buf) << ",";
  js_ << makeFloat(a, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::clearDepth(double depth)
{
  char buf[30];
  js_ << "ctx.clearDepth(" << makeFloat(depth, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::clearStencil(int s)
{
  js_ << "ctx.clearStencil(" << s << ");";
  GLDEBUG;
}

void WClientGLWidget::colorMask(bool red, bool green, bool blue, bool alpha)
{
  js_ << "ctx.colorMask(" << (red ? "true" : "false") << ","
      << (green ? "true" : "false") << ","
      << (blue ? "true" : "false") << ","
      << (alpha ? "true" : "false") << ");";
  GLDEBUG;
}

void WClientGLWidget::compileShader(WGLWidget::Shader shader)
{
  js_ << "ctx.compileShader(" << shader.jsRef() << ");";
  js_ << "if (!ctx.getShaderParameter(" << shader.jsRef()
      << ", ctx.COMPILE_STATUS)) {"
      << "alert(ctx.getShaderInfoLog(" << shader.jsRef() << "));}";
  GLDEBUG;
}

void WClientGLWidget::copyTexImage2D(WGLWidget::GLenum target, int level,
				     WGLWidget::GLenum internalFormat,
				     int x, int y,
				     unsigned width, unsigned height,
				     int border)
{
  js_ << "ctx.copyTexImage2D(" << toString(target) << "," << level << ","
    << toString(internalFormat) << "," << x << "," << y << ","
    << width << "," << height << "," << border << ");";
  GLDEBUG;
}

void WClientGLWidget::copyTexSubImage2D(WGLWidget::GLenum target, int level,
					int xoffset, int yoffset,
					int x, int y,
					unsigned width, unsigned height)
{
  js_ << "ctx.copyTexSubImage2D(" << toString(target) << "," << level << ","
    << xoffset << "," << yoffset << "," << x << "," << y << ","
    << width << "," << height << ");";
  GLDEBUG;
}

WGLWidget::Buffer WClientGLWidget::createBuffer()
{
  WGLWidget::Buffer retval(buffers_++);
  js_ << "if (!" << retval.jsRef() << "){";
  js_ << retval.jsRef() << "=ctx.createBuffer();";
  js_ << "\n}";
  GLDEBUG;
  return retval;
}

WGLWidget::ArrayBuffer WClientGLWidget::createAndLoadArrayBuffer(const std::string &url)
{
  WGLWidget::ArrayBuffer retval(arrayBuffers_++);
  preloadArrayBuffers_.push_back(PreloadArrayBuffer(retval.jsRef(), url));
  return retval;
}

WGLWidget::Framebuffer WClientGLWidget::createFramebuffer()
{
  WGLWidget::Framebuffer retval(framebuffers_++);
  js_ << retval.jsRef() << "=ctx.createFramebuffer();";
  GLDEBUG;
  return retval;
}

WGLWidget::Program WClientGLWidget::createProgram()
{
  WGLWidget::Program retval(programs_++);
  js_ << retval.jsRef() << "=ctx.createProgram();";
  GLDEBUG;
  return retval;
}

WGLWidget::Renderbuffer WClientGLWidget::createRenderbuffer()
{
  WGLWidget::Renderbuffer retval(renderbuffers_++);
  js_ << retval.jsRef() << "=ctx.createRenderbuffer();";
  GLDEBUG;
  return retval;
}

WGLWidget::Shader WClientGLWidget::createShader(WGLWidget::GLenum shader)
{
  WGLWidget::Shader retval(shaders_++);
  js_ << retval.jsRef() << "=ctx.createShader(" << toString(shader) << ");";
  GLDEBUG;
  return retval;
}

WGLWidget::Texture WClientGLWidget::createTexture()
{
  WGLWidget::Texture retval(textures_++);
  js_ << "if (!" << retval.jsRef() << "){";
  js_ << retval.jsRef() << "=ctx.createTexture();";
  js_ << "\n}";
  GLDEBUG;
  return retval;
}

WGLWidget::Texture WClientGLWidget::createTextureAndLoad(const std::string &url)
{
  WGLWidget::Texture retval(textures_++);
  preloadImages_.push_back(PreloadImage(retval.jsRef(), url, 0));
  return retval;
}

std::unique_ptr<WPaintDevice>
WClientGLWidget::createPaintDevice(const WLength& width,
				   const WLength& height)
{
  return std::unique_ptr<WPaintDevice>(new WCanvasPaintDevice(width, height));
}

void WClientGLWidget::cullFace(WGLWidget::GLenum mode)
{
  js_ << "ctx.cullFace(" << toString(mode) << ");";
  GLDEBUG;
}

void WClientGLWidget::deleteBuffer(WGLWidget::Buffer buffer)
{
  if ((unsigned)buffer.getId() >= buffers_) return;
  js_ << "ctx.deleteBuffer(" << buffer.jsRef() << ");";
  js_ << "delete " << buffer.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::deleteFramebuffer(WGLWidget::Framebuffer buffer)
{
  if ((unsigned)buffer.getId() >= framebuffers_) return;
  js_ << "ctx.deleteFramebuffer(" << buffer.jsRef() << ");";
  js_ << "delete " << buffer.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::deleteProgram(WGLWidget::Program program)
{
  if ((unsigned)program.getId() >= programs_) return;
  js_ << "ctx.deleteProgram(" << program.jsRef() << ");";
  js_ << "delete " << program.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::deleteRenderbuffer(WGLWidget::Renderbuffer buffer)
{
  if ((unsigned)buffer.getId() >= renderbuffers_) return;
  js_ << "ctx.deleteRenderbuffer(" << buffer.jsRef() << ");";
  js_ << "delete " << buffer.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::deleteShader(WGLWidget::Shader shader)
{
  if ((unsigned)shader.getId() >= shaders_) return;
  js_ << "ctx.deleteShader(" << shader.jsRef() << ");";
  js_ << "delete " << shader.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::deleteTexture(WGLWidget::Texture texture)
{
  if ((unsigned)texture.getId() >= textures_) return;
  js_ << "ctx.deleteTexture(" << texture.jsRef() << ");";
  js_ << "delete " << texture.jsRef() << ";";
  GLDEBUG;
}

void WClientGLWidget::depthFunc(WGLWidget::GLenum func)
{
  js_ << "ctx.depthFunc(" << toString(func) << ");";
  GLDEBUG;
}

void WClientGLWidget::depthMask(bool flag)
{
  js_ << "ctx.depthMask(" << (flag ? "true" : "false") << ");";
  GLDEBUG;
}

void WClientGLWidget::depthRange(double zNear, double zFar)
{
  char buf[30];
  js_ << "ctx.depthRange(" << makeFloat(zNear, buf) << ",";
  js_ << makeFloat(zFar, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::detachShader(WGLWidget::Program program,
				   WGLWidget::Shader shader)
{
  if ((unsigned)program.getId() >= programs_ ||
      (unsigned)shader.getId() >= shaders_)
    return;
  js_ << "if (" << program.jsRef() << " && " << shader.jsRef() << ") { ctx.detachShader(" << program.jsRef() << "," << shader.jsRef() << "); }";
  GLDEBUG;
}

void WClientGLWidget::disable(WGLWidget::GLenum cap)
{
  js_ << "ctx.disable(" << toString(cap) << ");";
  GLDEBUG;
}

void WClientGLWidget::disableVertexAttribArray(WGLWidget::AttribLocation index)
{
  js_ << "ctx.disableVertexAttribArray(" << index.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::drawArrays(WGLWidget::GLenum mode,
				 int first, unsigned count)
{
  js_ << "ctx.drawArrays(" << toString(mode) << "," << first << "," << count << ");";
  GLDEBUG;
}

void WClientGLWidget::drawElements(WGLWidget::GLenum mode, unsigned count, WGLWidget::GLenum type, unsigned offset)
{
  js_ << "ctx.drawElements(" << toString(mode) << "," << count << ","
      << toString(type) << "," << offset << ");";
  GLDEBUG;
}

void WClientGLWidget::enable(WGLWidget::GLenum cap)
{
  js_ << "ctx.enable(" << toString(cap) << ");";
  GLDEBUG;
}

void WClientGLWidget::enableVertexAttribArray(WGLWidget::AttribLocation index)
{
  js_ << "ctx.enableVertexAttribArray(" << index.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::finish()
{
  js_ << "ctx.finish();";
  GLDEBUG;
}

void WClientGLWidget::flush()
{
  js_ << "ctx.flush();";
  GLDEBUG;
}

void WClientGLWidget
::framebufferRenderbuffer(WGLWidget::GLenum target,
			  WGLWidget::GLenum attachment,
			  WGLWidget::GLenum renderbuffertarget,
			  WGLWidget::Renderbuffer renderbuffer)
{
  js_ << "ctx.framebufferRenderbuffer(" << toString(target) << ","
      << toString(attachment) << "," << toString(renderbuffertarget) << ","
      << renderbuffer.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::framebufferTexture2D(WGLWidget::GLenum target,
					   WGLWidget::GLenum attachment,
					   WGLWidget::GLenum textarget,
					   WGLWidget::Texture texture,
					   int level)
{
  js_ << "ctx.framebufferTexture2D(" << toString(target) << ","
      << toString(attachment) << "," << toString(textarget) << ","
      << texture.jsRef() << "," << level << ");";
  GLDEBUG;
}

void WClientGLWidget::frontFace(WGLWidget::GLenum mode)
{
  js_ << "ctx.frontFace(" << toString(mode) << ");";
  GLDEBUG;
}

void WClientGLWidget::generateMipmap(WGLWidget::GLenum target)
{
  js_ << "ctx.generateMipmap(" << toString(target) << ");";
  GLDEBUG;
}

WGLWidget::AttribLocation WClientGLWidget::getAttribLocation(WGLWidget::Program program, const std::string &attrib)
{
  WGLWidget::AttribLocation retval(attributes_++);
  js_ << retval.jsRef() << "=ctx.getAttribLocation(" << program.jsRef()
      << "," << WWebWidget::jsStringLiteral(attrib) << ");";
  GLDEBUG;
  return retval;
}

WGLWidget::UniformLocation WClientGLWidget::getUniformLocation(WGLWidget::Program program, const std::string &location)
{
  WGLWidget::UniformLocation retval(uniforms_++);
  js_ << retval.jsRef() << "=ctx.getUniformLocation(" << program.jsRef() 
      << "," << WWebWidget::jsStringLiteral(location) << ");";
  GLDEBUG;
  return retval;
}

void WClientGLWidget::hint(WGLWidget::GLenum target, WGLWidget::GLenum mode)
{
  js_ << "ctx.hint(" << toString(target) << "," << toString(mode) << ");";
  GLDEBUG;
}

void WClientGLWidget::lineWidth(double width)
{
  char buf[30];
  if (!wApp->environment().agentIsIE())
    js_ << "ctx.lineWidth(" << makeFloat(width, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::linkProgram(WGLWidget::Program program)
{
  js_ << "ctx.linkProgram(" << program.jsRef() << ");";
  js_ << "if(!ctx.getProgramParameter(" << program.jsRef() 
      << ",ctx.LINK_STATUS)){"
      << "alert('Could not initialize shaders: ' + ctx.getProgramInfoLog(" 
      << program.jsRef() << "));}";
  GLDEBUG;
}

void WClientGLWidget::pixelStorei(WGLWidget::GLenum pname, int param)
{
  js_ << "ctx.pixelStorei(" << toString(pname) << "," << param << ");";
  GLDEBUG;
}

void WClientGLWidget::polygonOffset(double factor, double units) {
  char buf[30];
  js_ << "ctx.polygonOffset(" << makeFloat(factor, buf) << ",";
  js_ << makeFloat(units, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::renderbufferStorage(WGLWidget::GLenum target,
					  WGLWidget::GLenum internalformat, 
					  unsigned width, unsigned height)
{
  js_ << "ctx.renderbufferStorage(" << toString(target) << ","
      << toString(internalformat) << "," << width << "," << height << ");";
  GLDEBUG;
}

void WClientGLWidget::sampleCoverage(double value, bool invert)
{
  char buf[30];
  js_ << "ctx.sampleCoverage(" << makeFloat(value, buf) << ","
      << (invert ? "true" : "false") << ");";
  GLDEBUG;
}

void WClientGLWidget::scissor(int x, int y, unsigned width, unsigned height)
{
  js_ << "ctx.scissor(" << x << "," << y << ","
      << width << "," << height << ");";
  GLDEBUG;
}

void WClientGLWidget::shaderSource(WGLWidget::Shader shader,
				   const std::string &src)
{
  js_ << "ctx.shaderSource(" << shader.jsRef() << "," 
      << WWebWidget::jsStringLiteral(src) << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilFunc(WGLWidget::GLenum func, int ref,
				  unsigned mask)
{
  js_ << "ctx.stencilFunc(" << toString(func) << "," << ref << "," << mask << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilFuncSeparate(WGLWidget::GLenum face,
					  WGLWidget::GLenum func, int ref,
					  unsigned mask)
{
  js_ << "ctx.stencilFuncSeparate(" << toString(face) << "," << toString(func)
      << "," << ref << "," << mask << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilMask(unsigned mask)
{
  js_ << "ctx.stencilMask(" << mask << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilMaskSeparate(WGLWidget::GLenum face, unsigned mask)
{
  js_ << "ctx.stencilMaskSeparate(" << toString(face) << "," << mask << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilOp(WGLWidget::GLenum fail, WGLWidget::GLenum zfail,
				WGLWidget::GLenum zpass)
{
  js_ << "ctx.stencilOp(" << toString(fail) << "," << toString(zfail) << ","
      << toString(zpass) << ");";
  GLDEBUG;
}

void WClientGLWidget::stencilOpSeparate(WGLWidget::GLenum face,
					WGLWidget::GLenum fail,
					WGLWidget::GLenum zfail,
					WGLWidget::GLenum zpass)
{
  js_ << "ctx.stencilOpSeparate(" << toString(face) << ","
      << toString(fail) << "," << toString(zfail) << ","
      << toString(zpass) << ");";
  GLDEBUG;
}

void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat, 
				 unsigned width, unsigned height, int border,
				 WGLWidget::GLenum format)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << width << "," << height << ","
      << border << "," << toString(format) << "," << toString(WGLWidget::UNSIGNED_BYTE)
      << ",null);";
  GLDEBUG;
}

void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WImage *image)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << toString(format) << "," << toString(type)
      << "," << image->jsRef() << ");";
  GLDEBUG;
}


void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WVideo *video)
{
  js_ << "if (" << video->jsMediaRef()<< ")"
    " ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << toString(format) << "," << toString(type)
      << "," << video->jsMediaRef() << ");";
  GLDEBUG;
}

// // >>> Issues with CORS
// void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
// 				 WGLWidget::GLenum internalformat,
// 				 WGLWidget::GLenum format,
// 				 WGLWidget::GLenum type,
// 				 WLink *image)
// {
//   unsigned imgNb = images_++;
//   preloadImages_.push_back(PreloadImage(currentlyBoundTexture_.jsRef(),
// 					image->url(), imgNb));

//   js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
//       << toString(internalformat) << "," << toString(format) 
//       << "," << toString(type)
//       << "," << currentlyBoundTexture_.jsRef() << ".image" << imgNb  << ");";
//   GLDEBUG;
// }

void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 std::string image)
{
  unsigned imgNb = images_++;
  // note: does not necessarily have to be a png (also tested jpg)
  std::unique_ptr<WFileResource> imgFile(new WFileResource("image/png", image));
  preloadImages_.push_back(PreloadImage(currentlyBoundTexture_.jsRef(),
					imgFile->url(), imgNb));
#ifndef WT_TARGET_JAVA
  addChild(std::move(imgFile));
#endif // WT_TARGET_JAVA

  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << toString(format) 
      << "," << toString(type)
      << "," << currentlyBoundTexture_.jsRef() << ".image" << imgNb  << ");";
  GLDEBUG;
}

void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WPaintDevice *paintdevice)
{
  unsigned imgNb = images_++;
  if (dynamic_cast<WCanvasPaintDevice*>(paintdevice) != nullptr) {
    WCanvasPaintDevice *cpd
      = dynamic_cast<WCanvasPaintDevice*>(paintdevice);
    std::string jsRef = currentlyBoundTexture_.jsRef() + "Canvas";
    js_ << jsRef << "=document.createElement('canvas');";
    js_ << jsRef << ".width=" << cpd->width().value() << ";";
    js_ << jsRef << ".height=" << cpd->height().value() << ";";
    js_ << "var f = function(myCanvas) {";
    cpd->renderPaintCommands(js_, "myCanvas");
    js_ << "};";
    js_ << "f(" << jsRef << ");";
    js_ << currentlyBoundTexture_.jsRef() << ".image" << imgNb << "=" << jsRef << ";";
    js_ << "delete " << jsRef << ";";
  }
#ifdef WT_HAS_WRASTERIMAGE
  else if (dynamic_cast<WRasterImage*>(paintdevice) != nullptr) {
    WRasterImage *rpd = dynamic_cast<WRasterImage*>(paintdevice);
    rpd->done();
    std::unique_ptr<WResource> mr = rpdToMemResource(rpd);
    preloadImages_.push_back(PreloadImage(currentlyBoundTexture_.jsRef(),
					  mr->url(), imgNb));
#ifndef WT_TARGET_JAVA
    addChild(std::move(mr));
#endif // WT_TARGET_JAVA
  }
#endif

  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << toString(format) 
      << "," << toString(type)
      << "," << currentlyBoundTexture_.jsRef() << ".image" << imgNb << ");";
  GLDEBUG;
}

#ifndef WT_TARGET_JAVA
std::unique_ptr<WResource> WClientGLWidget::rpdToMemResource(WRasterImage *rpd)
{
  std::stringstream ss;
  rpd->write(ss);
  std::unique_ptr<WMemoryResource> mr(new WMemoryResource("image/png"));
  mr->setData(reinterpret_cast<const unsigned char*>(ss.str().c_str()),
	      ss.str().size());
  return std::move(mr);
}
#endif

void WClientGLWidget::texImage2D(WGLWidget::GLenum target, int level,
				 WGLWidget::GLenum internalformat,
				 WGLWidget::GLenum format,
				 WGLWidget::GLenum type,
				 WGLWidget::Texture texture)
{
  js_ << "ctx.texImage2D(" << toString(target) << "," << level << ","
      << toString(internalformat) << "," << toString(format) 
      << "," << toString(type)
      << "," << texture.jsRef() << ".image0);";
  GLDEBUG;
}

void WClientGLWidget::texParameteri(WGLWidget::GLenum target,
				    WGLWidget::GLenum pname,
				    WGLWidget::GLenum param)
{
  js_ << "ctx.texParameteri(" << toString(target) << "," << toString(pname)
    << "," << toString(param) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform1f(const WGLWidget::UniformLocation &location,
				double x)
{
  char buf[30];
  js_ << "ctx.uniform1f(" << location.jsRef() 
      << "," << makeFloat(x, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform1fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  js_ << "ctx.uniform1fv(" << location.jsRef() << ",";
  renderfv(js_, value, 1, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform1fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  js_ << "ctx.uniform1fv(" << location.jsRef() << ","
      << v.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform1i(const WGLWidget::UniformLocation &location,
				int x)
{
  char buf[30];
  js_ << "ctx.uniform1i(" << location.jsRef() << ",";
  js_ << makeInt(x, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform1iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  js_ << "ctx.uniform1iv(" << location.jsRef() << ",";
  renderiv(js_, value, 1, WGLWidget::INT);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform2f(const WGLWidget::UniformLocation &location,
				double x, double y)
{
  char buf[30];
  js_ << "ctx.uniform2f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform2fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  js_ << "ctx.uniform2fv(" << location.jsRef() << ",";
  renderfv(js_, value, 2, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform2fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  js_ << "ctx.uniform2fv(" << location.jsRef() << ","
      << v.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform2i(const WGLWidget::UniformLocation &location,
				int x, int y)
{
  char buf[30];
  js_ << "ctx.uniform2i(" << location.jsRef() << ",";
  js_ << makeInt(x, buf) << ",";
  js_ << makeInt(y, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform2iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  js_ << "ctx.uniform2iv(" << location.jsRef() << ",";
  renderiv(js_, value, 2, WGLWidget::INT);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform3f(const WGLWidget::UniformLocation &location,
				double x, double y, double z)
{
  char buf[30];
  js_ << "ctx.uniform3f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ",";
  js_ << makeFloat(z, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform3fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  js_ << "ctx.uniform3fv(" << location.jsRef() << ",";
  renderfv(js_, value, 3, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform3fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  js_ << "ctx.uniform3fv(" << location.jsRef() << ","
      << v.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform3i(const WGLWidget::UniformLocation &location,
				int x, int y, int z)
{
  char buf[30];
  js_ << "ctx.uniform3i(" << location.jsRef() << ",";
  js_ << makeInt(x, buf) << ",";
  js_ << makeInt(y, buf) << ",";
  js_ << makeInt(z, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform3iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  js_ << "ctx.uniform3iv(" << location.jsRef() << ",";
  renderiv(js_, value, 3, WGLWidget::INT);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform4f(const WGLWidget::UniformLocation &location,
				double x, double y, double z, double w)
{
  char buf[30];
  js_ << "ctx.uniform4f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ",";
  js_ << makeFloat(z, buf) << ",";
  js_ << makeFloat(w, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform4fv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY float *value)
{
  js_ << "ctx.uniform4fv(" << location.jsRef() << ",";
  renderfv(js_, value, 4, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform4fv(const WGLWidget::UniformLocation &location,
				 const WGLWidget::JavaScriptVector &v)
{
  js_ << "ctx.uniform4fv(" << location.jsRef() << ","
      << v.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::uniform4i(const WGLWidget::UniformLocation &location,
				int x, int y, int z, int w)
{
  char buf[30];
  js_ << "ctx.uniform4i(" << location.jsRef() << ",";
  js_ << makeInt(x, buf) << ",";
  js_ << makeInt(y, buf) << ",";
  js_ << makeInt(z, buf) << ",";
  js_ << makeInt(w, buf) << ");";
  GLDEBUG;
}
  
void WClientGLWidget::uniform4iv(const WGLWidget::UniformLocation &location,
				 const WT_ARRAY int *value)
{
  js_ << "ctx.uniform4iv(" << location.jsRef() << ",";
  renderiv(js_, value, 4, WGLWidget::INT);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix2fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  js_ << "ctx.uniformMatrix2fv(" << location.jsRef() << ","
      << (transpose?"true":"false") << ",";
  renderfv(js_, value, 4, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix2(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 2, 2> &m)
{
  js_ << "ctx.uniformMatrix2fv(" << location.jsRef() << ",false,";
  WGenericMatrix<double, 2, 2> t(m.transposed());
  renderfv(js_, t, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix3fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  js_ << "ctx.uniformMatrix3fv(" << location.jsRef() << ","
      << (transpose?"true":"false") << ",";
  renderfv(js_, value, 9, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix3(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 3, 3> &m)
{
  js_ << "ctx.uniformMatrix3fv(" << location.jsRef() << ",false,";
  WGenericMatrix<double, 3, 3> t(m.transposed());
  renderfv(js_, t, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix4fv(const WGLWidget::UniformLocation &location,
				       bool transpose,
				       const WT_ARRAY double *value)
{
  js_ << "ctx.uniformMatrix4fv(" << location.jsRef() << ","
      << (transpose?"true":"false") << ",";
  renderfv(js_, value, 16, JsArrayType::Float32Array);
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix4(const WGLWidget::UniformLocation &location,
				     const WGenericMatrix<double, 4, 4> &m) {
  js_ << "ctx.uniformMatrix4fv(" << location.jsRef() << ",false,";
  js_ << "new Float32Array([";
  char buf[30];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) { // col-major
      js_ << ((i == 0 && j == 0) ? "" : ",") << makeFloat(m.at(j, i), buf);
    }
  }
  js_ << "])";
  js_ << ");";
  GLDEBUG;
}

void WClientGLWidget::uniformMatrix4(const WGLWidget::UniformLocation &location,
				     const WGLWidget::JavaScriptMatrix4x4 &m)
{
  js_ << "ctx.uniformMatrix4fv(" << location.jsRef() << ",false,";
  js_ << m.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::useProgram(WGLWidget::Program program)
{
  js_ << "ctx.useProgram(" << program.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::validateProgram(WGLWidget::Program program)
{
  js_ << "ctx.validateProgram(" << program.jsRef() << ");";
  GLDEBUG;
}

void WClientGLWidget::vertexAttrib1f(WGLWidget::AttribLocation location, double x)
{
  char buf[30];
  js_ << "ctx.vertexAttrib1f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::vertexAttrib2f(WGLWidget::AttribLocation location,
				     double x, double y)
{
  char buf[30];
  js_ << "ctx.vertexAttrib2f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::vertexAttrib3f(WGLWidget::AttribLocation location,
				     double x, double y, double z)
{
  char buf[30];
  js_ << "ctx.vertexAttrib3f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ",";
  js_ << makeFloat(z, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::vertexAttrib4f(WGLWidget::AttribLocation location,
				     double x, double y, double z, double w)
{
  char buf[30];
  js_ << "ctx.vertexAttrib4f(" << location.jsRef() << ",";
  js_ << makeFloat(x, buf) << ",";
  js_ << makeFloat(y, buf) << ",";
  js_ << makeFloat(z, buf) << ",";
  js_ << makeFloat(w, buf) << ");";
  GLDEBUG;
}

void WClientGLWidget::vertexAttribPointer(WGLWidget::AttribLocation location,
					  int size,
					  WGLWidget::GLenum type,
					  bool normalized,
					  unsigned stride, unsigned offset)
{
  js_ << "ctx.vertexAttribPointer(" << location.jsRef() << "," << size << "," << toString(type)
    << "," << (normalized?"true":"false") << "," << stride << "," << offset << ");";
  GLDEBUG;
}

void WClientGLWidget::viewport(int x, int y, unsigned width, unsigned height)
{
  js_ << "ctx.viewport(" << x << "," << y << "," << width << "," << height << ");";
  GLDEBUG;
}

void WClientGLWidget::initJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &mat)
{
  if (!mat.hasContext())
    glInterface_->addJavaScriptMatrix4(mat);
  else if (mat.context_ != glInterface_)
    throw WException("JavaScriptMatrix4x4: associated WGLWidget is not equal to the WGLWidget it's being initialized in");

  WGenericMatrix<double, 4, 4> m = mat.value();
  js_ << mat.jsRef() << "=";
  renderfv(js_, m, JsArrayType::Float32Array);
  js_ << ";";

  mat.initialize();
}

void WClientGLWidget::setJavaScriptMatrix4(WGLWidget::JavaScriptMatrix4x4 &jsm,
					   const WGenericMatrix<double, 4, 4> &m)
{
  js_ << WT_CLASS ".glMatrix.mat4.set(";
  WGenericMatrix<double, 4, 4> t(m.transposed());
  renderfv(js_, t, JsArrayType::Float32Array);
  js_ << ", " << jsm.jsRef() << ");";
}

void WClientGLWidget::initJavaScriptVector(WGLWidget::JavaScriptVector &vec)
{
  if (!vec.hasContext())
    glInterface_->addJavaScriptVector(vec);
  else if (vec.context_ != glInterface_)
    throw WException("JavaScriptVector: associated WGLWidget is not equal to the WGLWidget it's being initialized in");

  std::vector<float> v = vec.value();
  js_ << vec.jsRef() << "= new Float32Array([";
  for (unsigned i = 0; i < vec.length(); ++i) {
    std::string val;
    if (v[i] == std::numeric_limits<float>::infinity()) {
      val = "Infinity";
    } else if (v[i] == -std::numeric_limits<float>::infinity()) {
      val = "-Infinity";
    } else {
      val = std::to_string(v[i]);
    }
    if (i != 0)
      js_ << ",";
    js_ << val;
  }
  js_ << "]);";

  vec.initialize();
}

void WClientGLWidget::setJavaScriptVector(WGLWidget::JavaScriptVector &jsv,
					  const std::vector<float> &v)
{
  if (jsv.length() != v.size())
    throw WException("Trying to set a JavaScriptVector with "
		     "incompatible length!");
  for (unsigned i = 0; i < jsv.length(); ++i) {
    std::string val;
    if (v[i] == std::numeric_limits<float>::infinity()) {
      val = "Infinity";
    } else if (v[i] == -std::numeric_limits<float>::infinity()) {
      val = "-Infinity";
    } else {
      val = std::to_string(v[i]);
    }
    js_ << jsv.jsRef() << "[" << i << "] = " << val << ";";
  }
}

void WClientGLWidget::setClientSideMouseHandler(const std::string& handlerCode)
{
  js_ << "obj.setMouseHandler(" << handlerCode << ");";
}

void WClientGLWidget::setClientSideLookAtHandler(const WGLWidget::JavaScriptMatrix4x4 &m,
						 double centerX, double centerY, double centerZ,
						 double uX, double uY, double uZ,
						 double pitchRate, double yawRate)
{
  js_ << "obj.setMouseHandler(new obj.LookAtMouseHandler("
     << m.jsRef()
     << ",[" << centerX << "," << centerY << "," << centerZ << "],"
     << "[" << uX << "," << uY << "," << uZ << "],"
     << pitchRate << "," << yawRate << "));";
}

void WClientGLWidget::setClientSideWalkHandler(const WGLWidget::JavaScriptMatrix4x4 &m,
				double frontStep, double rotStep)
{
  js_ << "obj.setMouseHandler(new obj.WalkMouseHandler("
      << m.jsRef() << ","
      << frontStep << "," << rotStep << "));";
}

JsArrayType WClientGLWidget::arrayType() const
{
  return JsArrayType::Float32Array;
}

void WClientGLWidget::injectJS(const std::string & jsString)
{
  js_ << jsString;
}

void WClientGLWidget::restoreContext(const std::string &jsRef)
{
  std::stringstream tmp;
  tmp << "{var o = " << glObjJsRef(jsRef) << ";\n";

  shaders_ = 0;
  programs_ = 0;
  attributes_ = 0;
  uniforms_ = 0;
  for (unsigned i = 0; i < buffers_; ++i) {
    tmp << "o.ctx.WtBuffer" << i << "=null;";
  }
  buffers_ = 0;
  arrayBuffers_ = 0;

  framebuffers_ = 0;
  renderbuffers_ = 0;
  for (unsigned i = 0; i < textures_; ++i) {
    tmp << "o.ctx.WtTexture" << i << "=null;";
  }
  textures_ = 0;
  images_ = 0;
  canvas_ = 0;

  initializeGL(jsRef, tmp);
  tmp << "o.initialized = false;}";
  WApplication::instance()->doJavaScript(tmp.str());
}

void WClientGLWidget::initializeGL(const std::string &jsRef, std::stringstream &ss)
{
  js_.str("");
  glInterface_->initializeGL();
  ss <<
    """o.initializeGL=function(){\n"
    """var obj=" << glObjJsRef(jsRef) << ";\n"
    """var ctx=obj.ctx; if(!ctx) return;\n" <<
    "" << js_.str() <<
    """obj.initialized = true;\n"
    // updates are queued until initialization is complete
    """var key;\n"
    """for(key in obj.updates) obj.updates[key]();\n"
    """obj.updates = new Array();\n"
    // Similar, resizeGL is not executed until initialized
    """obj.resizeGL();\n"
    "};\n";
}

void WClientGLWidget::render(const std::string& jsRef, WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    std::stringstream tmp;
    tmp <<
      "{\n"
      """var o = new " WT_CLASS ".WGLWidget(" << wApp->javaScriptClass() << "," << jsRef << ");\n"
#ifndef WT_TARGET_JAVA
      """o.discoverContext(function(){" << webglNotAvailable_.createCall({}) 
#else // WT_TARGET_JAVA
      """o.discoverContext(function(){" << webglNotAvailable_.createCall() 
#endif // WT_TARGET_JAVA
	<< "}, "
	<< (glInterface_->renderOptions_.test(GLRenderOption::AntiAliasing)
	    ? "true" : "false") << ");\n";

    initializeGL(jsRef, tmp);
    tmp << "}\n";
    WApplication::instance()->doJavaScript(tmp.str());
  }

  if (updateGL_ || updateResizeGL_ || updatePaintGL_) {
    std::stringstream tmp;
    tmp <<
      "var o = " << glObjJsRef(jsRef) << ";\n"
      "if(o.ctx){\n";
    if (updateGL_) {
      js_.str("");
      glInterface_->updateGL();
      tmp << "var update =function(){\n"
        "var obj=" << glObjJsRef(jsRef) << ";\n"
        "var ctx=obj.ctx;if (!ctx) return;\n"
        << js_.str() << "\n};\n"
        // cannot execute updates before initializeGL is executed
        "o.updates.push(update);";
    }
    if (updateResizeGL_) {
      js_.str("");
      glInterface_->resizeGL(renderWidth_, renderHeight_);
      tmp << "o.resizeGL=function(){\n"
        "var obj=" << glObjJsRef(jsRef) << ";\n"
        "var ctx=obj.ctx;if (!ctx) return;\n"
        << js_.str() << "};";
    }
    if (updatePaintGL_) {
      js_.str("");
      glInterface_->paintGL();
      // TG: cannot overwrite paint before update is executed
      // therefore overwrite paintgl in a deferred function through the
      // tasks queue
      tmp << "var updatePaint = function(){\n";
      tmp << "var obj=" << glObjJsRef(jsRef) << ";\n";

      tmp << "obj.paintGL=function(){\n"
        "var obj=" << glObjJsRef(jsRef) << ";\n"
        "var ctx=obj.ctx;if (!ctx) return;\n"
        << js_.str() << "};";

      tmp << "};\n";
      tmp << "o.updates.push(updatePaint);";


    }
    js_.str("");
    // Make sure textures are loaded before we render
    tmp << "}\n";

    // Preloading images and buffers
    const bool preloadingSomething = preloadImages_.size()>0 || preloadArrayBuffers_.size() >0;

    if (preloadingSomething)  {
      if (preloadImages_.size() > 0) {
	tmp <<
	  "o.preloadingTextures++;"
	  "new " << wApp->javaScriptClass() << "._p_.ImagePreloader([";
	for (unsigned i = 0; i < preloadImages_.size(); ++i) {
	  if (i != 0)
	    tmp << ',';
	  tmp << '\'' << wApp->resolveRelativeUrl(preloadImages_[i].url) << '\'';
	}
	tmp <<
	  "],function(images){\n"
	  "var o=" << glObjJsRef(jsRef) << ";\n"
	  "var ctx=null;\n"
	  "if(o) ctx=o.ctx;\n"
	  "if(ctx == null) return;\n";
	for (unsigned i = 0; i < preloadImages_.size(); ++i) {
	  std::string texture = preloadImages_[i].jsRef;
	  tmp << texture << "=ctx.createTexture();\n"
	      << texture << ".image" << preloadImages_[i].id
	      << "=images[" << i << "];\n";
	}
	tmp << "o.preloadingTextures--;\n"
	  << "o.handlePreload();\n"
	  << "});";

	preloadImages_.clear();
      }

      // now check for buffers to preload
      if (preloadArrayBuffers_.size() > 0) {
	tmp <<
	  "o.preloadingBuffers++;"
	  "new " << wApp->javaScriptClass() << "._p_.ArrayBufferPreloader([";
	for (unsigned i = 0; i < preloadArrayBuffers_.size(); ++i) {
	  if (i != 0)
	    tmp << ',';
	  tmp << '\'' << preloadArrayBuffers_[i].url << '\'';
	}
	tmp <<
	  "],function(bufferResources){\n"
	  "var o=" << glObjJsRef(jsRef) << ";\n"
	  "var ctx=null;\n"
	  " if(o) ctx=o.ctx;\n"
	  "if(ctx == null) return;\n";
	for (unsigned i = 0; i < preloadArrayBuffers_.size(); ++i) {
	  std::string bufferResource = preloadArrayBuffers_[i].jsRef;
	  // setup datatype
	  tmp << bufferResource << " = ctx.createBuffer();";
	  // set the data
	  tmp << "if (bufferResources[" << i << "]==null){";
	  tmp << bufferResource << ".data=[];\n";
	  tmp << "}else{";
	  tmp << bufferResource << ".data=bufferResources[" << i << "];\n";
	  tmp << "}";
	}
	tmp << "o.preloadingBuffers--;"
	  << "o.handlePreload();\n"
	  << "});";

	preloadArrayBuffers_.clear();
      }
    } else {
      // No textures or buffers to load - go and paint
      tmp << "o.handlePreload();";
    }
    WApplication::instance()->doJavaScript(tmp.str());
    updateGL_ = updatePaintGL_ = updateResizeGL_ = false;
  }

  //repaintGL(GLClientSideRenderer::PAINT_GL | GLClientSideRenderer::RESIZE_GL);
}

std::string WClientGLWidget::glObjJsRef(const std::string& jsRef)
{
  return "(function(){"
    "var r = " + jsRef + ";"
    "var o = r ? r.wtObj : null;"
    "return o ? o : {ctx: null};"
    "})()";
}

}
