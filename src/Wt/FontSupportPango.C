/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFont.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WPointF.h"
#include "Wt/WRectF.h"
#include "Wt/WTransform.h"
#include "Wt/FontSupport.h"

#include "WebUtils.h"

#ifdef WT_THREADED
#include <thread>
#include <mutex>
#endif // WT_THREADED

#include <boost/algorithm/string.hpp>
#include <pango/pango.h>
#include <pango/pangoft2.h>

#ifndef DOXYGEN_ONLY

namespace {

const double EPSILON = 1e-4;

bool isEpsilonMore(double x, double limit) {
  return x - EPSILON > limit;
}

double pangoUnitsToDouble(const int u) 
{
  return ((double)u) / PANGO_SCALE;
}

int pangoUnitsFromDouble(const double u)
{
  return (int) (u * PANGO_SCALE);
}

static Wt::FontSupport::EnabledFontFormats enabledFontFormats = Wt::FontSupport::AnyFont;

void addTrueTypePattern(FcPattern *pattern, gpointer data)
{
  if (enabledFontFormats == Wt::FontSupport::TrueTypeOnly) {
    FcChar8 font_format[] = "TrueType";
    FcPatternAddString(pattern, "fontformat", font_format);
  }
}

/*
 * A global font map, since this one leaks as hell, and it cannot stand
 * being used in thread local storage since cleanup (with thread exit)
 * doesn't work properly.
 *
 * It is not clear what operations involving the font map are thread-safe,
 * so here we serialize everything using a mutex.
 */

PangoFontMap *pangoFontMap = nullptr;

#ifdef WT_THREADED
std::recursive_mutex pangoMutex;
#define PANGO_LOCK std::unique_lock<std::recursive_mutex> lock(pangoMutex);
#else
#define PANGO_LOCK
#endif // WT_THREADED
}

namespace Wt {

FontSupport::Bitmap::Bitmap(int width, int height)
  : width_(width),
    height_(height),
    pitch_((width_ + 3) & -4)
{
  buffer_ = new unsigned char[height_ * pitch_];
  memset(buffer_, 0, height_ * pitch_);
}

FontSupport::Bitmap::~Bitmap()
{
  delete[] buffer_;
}

FontSupport::FontMatch::FontMatch(PangoFont *font, PangoFontDescription *desc)
  : font_(font), desc_(desc)
{ }

std::string FontSupport::FontMatch::fileName() const
{
  return FontSupport::fontPath(font_);
}

FontSupport::FontSupport(WPaintDevice *paintDevice, EnabledFontFormats enabledFontFormats)
  : device_(paintDevice),
    enabledFontFormats_(enabledFontFormats),
    cache_(10)
{
  PANGO_LOCK;

  if (!pangoFontMap) {
    pangoFontMap = pango_ft2_font_map_new();

    pango_ft2_font_map_set_default_substitute(PANGO_FT2_FONT_MAP(pangoFontMap),
	addTrueTypePattern, NULL, NULL);
  }

#if PANGO_VERSION_MAJOR > 1 || PANGO_VERSION_MINOR > 21
  context_ = pango_font_map_create_context(pangoFontMap);
#else
  context_ = pango_ft2_font_map_create_context(PANGO_FT2_FONT_MAP(pangoFontMap));
#endif

  currentFont_ = nullptr;
}

FontSupport::~FontSupport()
{
  PANGO_LOCK;

  for (MatchCache::iterator i = cache_.begin(); i != cache_.end(); ++i) {
    if (i->match) {
      g_object_unref(i->match);
      pango_font_description_free(i->desc);
    }
  }

  g_object_unref(context_);
}

bool FontSupport::canRender() const
{
  return true;
}

void FontSupport::setDevice(WPaintDevice *paintDevice)
{
  device_ = paintDevice;
}

PangoFontDescription *FontSupport::createFontDescription(const WFont& f) const
{
  std::string s = f.specificFamilies().toUTF8();

  /* Quick fix for Mac OS X (Times is a special font?) */
  boost::trim(s);
  if (Utils::lowerCase(s) == "times")
    s = "Times New Roman";

  if (f.genericFamily() != FontFamily::Default) {
    if (!s.empty())
      s += ',';

    switch (f.genericFamily()) {
    case FontFamily::Serif: s += "serif"; break;
    case FontFamily::SansSerif: s += "sans"; break;
    case FontFamily::Cursive: s += "cursive"; break;
    case FontFamily::Fantasy: s += "fantasy"; break;
    case FontFamily::Monospace: s += "monospace"; break;
    default: break;
    }
  }

  if (f.weightValue() < 300)
    s += " ultra-light";
  else if (f.weightValue() < 400)
    s += " light";
  else if (f.weightValue() >= 700)
    s += " bold";
  else if (f.weightValue() >= 800)
    s += " ultra-bold";
  else if (f.weightValue() >= 900)
    s += " heavy";

  switch (f.style()) {
  case FontStyle::Italic: s += " italic"; break;
  case FontStyle::Oblique: s += " oblique"; break;
  default: break;
  }

  switch (f.variant()) {
  case FontVariant::SmallCaps: s += " small-caps"; break;
  default: break;
  }

  s += " " + std::to_string((int)(f.sizeLength().toPixels() * 0.75 * 96/72));

  return pango_font_description_from_string(s.c_str());
}

FontSupport::FontMatch FontSupport::matchFont(const WFont& f) const
{
  for (MatchCache::iterator i = cache_.begin(); i != cache_.end(); ++i) {
    if (i->font == f) {
      cache_.splice(cache_.begin(), cache_, i); // implement LRU
      return FontMatch(i->match, i->desc);
    }
  }

  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  PangoFontDescription *desc = createFontDescription(f);

  PangoFont *match = pango_font_map_load_font(pangoFontMap, context_, desc);
  pango_context_set_font_description(context_, desc); // for layoutText()

  if (cache_.back().match) {
    g_object_unref(cache_.back().match);
    pango_font_description_free(cache_.back().desc);
  }

  cache_.pop_back();
  cache_.push_front(Matched(f, match, desc));

  return FontMatch(match, desc);
}

void FontSupport::addFontCollection(const std::string& directory,
				    bool recursive)
{
}

bool FontSupport::busy() const
{
  return currentFont_;
}

std::string FontSupport::drawingFontPath() const
{
  return fontPath(currentFont_);
}

std::string FontSupport::fontPath(PangoFont *font)
{
  PANGO_LOCK;

  PangoFcFont *f = (PangoFcFont *)font;
  FT_Face face = pango_fc_font_lock_face(f);

  std::string result;
  if (face->stream->pathname.pointer)
    result = (const char *)face->stream->pathname.pointer;

  pango_fc_font_unlock_face(f);

  return result;
}

GList *FontSupport::layoutText(const WFont& font,
			       const std::string& utf8,
			       std::vector<PangoGlyphString *>& glyphs,
			       int& width)
{
  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  FontMatch match = matchFont(font);
  PangoAttrList *attrs = pango_attr_list_new();

  pango_context_set_font_description(context_, match.pangoFontDescription());
  GList *items
    = pango_itemize(context_, utf8.c_str(), 0, utf8.length(), attrs, nullptr);

  width = 0;

  for (GList *elem = items; elem; elem = elem->next) {
    PangoItem *item = (PangoItem *)elem->data;
    PangoAnalysis *analysis = &item->analysis;

    PangoGlyphString *gl = pango_glyph_string_new();

    pango_shape(utf8.c_str() + item->offset, item->length, analysis, gl);

    glyphs.push_back(gl);

    if (device_) {
      currentFont_ = analysis->font;

      WTextItem textItem
	= device_->measureText(WString::fromUTF8(utf8.substr(item->offset,
							     item->length)),
			       -1, false);

      width += pangoUnitsFromDouble(textItem.width());

      currentFont_ = nullptr;
    } else
      width += pango_glyph_string_get_width(gl);
  }

  pango_attr_list_unref(attrs);

  return items;
}

void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   WFlags<AlignmentFlag> flags, const WString& text)
{
  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  std::string utf8 = text.toUTF8();

  std::vector<PangoGlyphString *> glyphs;
  int width;

  GList *items = layoutText(font, utf8, glyphs, width);

  AlignmentFlag hAlign = flags & AlignHorizontalMask;
  AlignmentFlag vAlign = flags & AlignVerticalMask;

  /* FIXME handle bidi ! */

  double x;
  switch (hAlign) {
  case AlignmentFlag::Left:
    x = rect.left();
    break;
  case AlignmentFlag::Right:
    x = rect.right() - pangoUnitsToDouble(width);
    break;
  case AlignmentFlag::Center:
    x = rect.center().x() - pangoUnitsToDouble(width/2);
    break;
  default:
    x = 0;
  }

  unsigned i = 0;
  for (GList *elem = items; elem; elem = elem->next) {
    PangoItem *item = (PangoItem *)elem->data;
    PangoAnalysis *analysis = &item->analysis;

    PangoGlyphString *gl = glyphs[i++];

    currentFont_ = analysis->font;

    /*
     * Note, we are actually ignoring the selected glyphs here, which
     * is a pitty and possibly wrong if the device does not make the
     * same selection !
     */
    WString s = WString::fromUTF8(utf8.substr(item->offset,
                                              item->length));

    device_->drawText(WRectF(x, rect.y(),
			     1000, rect.height()),
		      AlignmentFlag::Left | vAlign, TextFlag::SingleLine,
		      s, nullptr);

    WTextItem textItem = device_->measureText(s, -1, false);

    x += textItem.width();

    pango_item_free(item);
    pango_glyph_string_free(gl);
  }

  g_list_free(items);

  currentFont_ = nullptr;
}

WFontMetrics FontSupport::fontMetrics(const WFont& font)
{
  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  PangoFont *pangoFont = matchFont(font).pangoFont();
  PangoFontMetrics *metrics = pango_font_get_metrics(pangoFont, nullptr);

  double ascent
    = pangoUnitsToDouble(pango_font_metrics_get_ascent(metrics));
  double descent 
    = pangoUnitsToDouble(pango_font_metrics_get_descent(metrics));

  double leading = (ascent + descent) - font.sizeLength(12).toPixels();

  // ascent < leading is an odd thing. it happens with a font like
  // Cursive.
  if (ascent > leading)
    ascent -= leading;
  else
    leading = 0;

  WFontMetrics result(font, leading, ascent, descent);

  pango_font_metrics_unref(metrics);

  return result;
}

WTextItem FontSupport::measureText(const WFont& font, const WString& text,
				   double maxWidth, bool wordWrap)
{
  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  /*
   * Note: accurate measuring on a bitmap requires that the transformation
   * is applied, because hinting may push chars to boundaries e.g. when
   * rotated (or scaled too?)
   */
  std::string utf8 = text.toUTF8();
  const char *s = utf8.c_str();

  if (wordWrap) {
    int utflen = g_utf8_strlen(s, -1);
    PangoLogAttr *attrs = new PangoLogAttr[utflen + 1];
    PangoLanguage *language = pango_language_from_string("en-US");

    pango_get_log_attrs(s, utf8.length(), -1, language, attrs, utflen + 1);

    double w = 0, nextW = -1;

    int current = 0;
    int measured = 0;
    int end = 0;

    bool maxWidthReached = false;

    for (int i = 0; i < utflen + 1; ++i) {
      if (i == utflen || attrs[i].is_line_break) {
	int cend = g_utf8_offset_to_pointer(s, end) - s;

	WTextItem ti
	  = measureText(font, WString::fromUTF8(utf8.substr(measured,
							    cend - measured)),
			-1, false);

	if (isEpsilonMore(w + ti.width(), maxWidth)) {
	  nextW = ti.width();
	  maxWidthReached = true;
	  break;
	} else {
	  measured = cend;
	  current = g_utf8_offset_to_pointer(s, i) - s;
	  w += ti.width();

	  if (i == utflen) {
	    w += measureText(font, WString::fromUTF8(utf8.substr(measured)),
			     -1, false).width();
	    measured = utf8.length();
	  }
	}
      }

      if (!attrs[i].is_white)
	end = i + 1;
    }

    delete[] attrs;

    if (maxWidthReached) {
      return WTextItem(WString::fromUTF8(utf8.substr(0, current)), w, nextW);
    } else {
      /*
       * For some reason, the sum of the individual widths is a bit less
       * (for longer stretches of text), so we re-measure it !
       */
      w = measureText(font, WString::fromUTF8(utf8.substr(0, measured)),
		      -1, false).width();
      return WTextItem(text, w);
    }
  } else {
    std::vector<PangoGlyphString *> glyphs;
    int width;

    GList *items = layoutText(font, utf8, glyphs, width);

    double w = pangoUnitsToDouble(width);

    for (unsigned i = 0; i < glyphs.size(); ++i)
      pango_glyph_string_free(glyphs[i]);

    g_list_foreach(items, (GFunc) pango_item_free, nullptr);
    g_list_free(items);

    return WTextItem(text, w);
  }
}

void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   const WTransform& transform, Bitmap& bitmap,
			   WFlags<AlignmentFlag> flags,
			   const WString& text)
{
  PANGO_LOCK;

  enabledFontFormats = enabledFontFormats_;

  PangoMatrix matrix;
  matrix.xx = transform.m11();
  matrix.xy = transform.m21();
  matrix.yx = transform.m12();
  matrix.yy = transform.m22();
  matrix.x0 = transform.dx();
  matrix.y0 = transform.dy();

  std::string utf8 = text.toUTF8();

  std::vector<PangoGlyphString *> glyphs;
  int width;

  pango_context_set_matrix(context_, &matrix);

  /*
   * Oh my god, somebody explain me why we need to do this...
   */
  WFont f = font;
  f.setSize(font.sizeLength().toPixels()
	    / pango_matrix_get_font_scale_factor(&matrix));

  GList *items = layoutText(f, utf8, glyphs, width);
  pango_context_set_matrix(context_, nullptr);

  AlignmentFlag hAlign = flags & AlignHorizontalMask;

  /* FIXME handle bidi ! */

  double x;
  switch (hAlign) {
  case AlignmentFlag::Left:
    x = rect.left();
    break;
  case AlignmentFlag::Right:
    x = rect.right() - pangoUnitsToDouble(width);
    break;
  case AlignmentFlag::Center:
    x = rect.center().x() - pangoUnitsToDouble(width/2);
    break;
  default:
    x = 0;
  }

  AlignmentFlag vAlign = flags & AlignVerticalMask;

  PangoFont *pangoFont = matchFont(font).pangoFont();
  PangoFontMetrics *metrics = pango_font_get_metrics(pangoFont, nullptr);

  double ascent
    = pangoUnitsToDouble(pango_font_metrics_get_ascent(metrics));
  double descent 
    = pangoUnitsToDouble(pango_font_metrics_get_descent(metrics));

  pango_font_metrics_unref(metrics);

  double baseline = ascent;
  double height = ascent + descent;

  double y;
  switch (vAlign) {
  case AlignmentFlag::Top:
    y = rect.top() + baseline;
    break;
  case AlignmentFlag::Middle:
    y = rect.center().y() - height / 2 + baseline;
    break;
  case AlignmentFlag::Bottom:
    y = rect.bottom() - height + baseline;
    break;
  default:
    y = 0;
  }

  FT_Bitmap bmp;
  bmp.buffer = bitmap.buffer();
  bmp.width = bitmap.width();
  bmp.rows = bitmap.height();
  bmp.pitch = bitmap.pitch();
  bmp.pixel_mode = FT_PIXEL_MODE_GRAY;
  bmp.num_grays = 16; // ???

  GList *elem;
  unsigned i = 0;

  for (elem = items; elem; elem = elem->next) {
    PangoItem *item = (PangoItem *)elem->data;
    PangoAnalysis *analysis = &item->analysis;

    PangoGlyphString *gl = glyphs[i++];

    pango_ft2_render_transformed(&bmp, &matrix,
				 analysis->font, gl,
				 pangoUnitsFromDouble(x),
				 pangoUnitsFromDouble(y));

    x += pangoUnitsToDouble(pango_glyph_string_get_width(gl));

    pango_glyph_string_free(gl);
    pango_item_free(item);
  }

  g_list_free(items);
}

}

#endif // DOXYGEN_ONLY
