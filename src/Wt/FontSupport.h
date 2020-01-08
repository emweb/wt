// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FONT_SUPPORT_H_
#define FONT_SUPPORT_H_

#include <list>

#include "Wt/WDllDefs.h"

#ifdef WT_TARGET_JAVA
#define WT_FONTSUPPORT_SIMPLE
#endif // WT_TARGET_JAVA
#ifdef WT_FONTSUPPORT_SIMPLE
#include <string>
#endif // WT_FONTSUPPORT_SIMPLE
#ifdef WT_FONTSUPPORT_PANGO
#include <pango/pango.h>
#endif // WT_FONTSUPPORT_PANGO
#ifdef WT_FONTSUPPORT_DIRECTWRITE
#include <dwrite.h>
#endif // WT_FONTSUPPORT_DIRECTWRITE

#include "Wt/WFont.h"
#include "Wt/WPaintDevice.h"

namespace Wt {

/*
 * A private utility class that provides improved font matching.
 *
 * This utility class uses libpango for font selection and layout if
 * available, otherwise uses a simple font matching which choses a
 * single font and does not take into account its supported glyphs.
 */
class FontSupport
{
public:
#ifndef WT_TARGET_JAVA
  class Bitmap {
  public:
    Bitmap(int width, int height);
    ~Bitmap();

    int width() const { return width_; }
    int height() const { return height_; }
    int pitch() const { return pitch_; }

    unsigned char value(int x, int y) const { return buffer_[y * pitch_ + x]; }

    unsigned char *buffer() { return buffer_; }

  private:
    int width_, height_, pitch_;

    unsigned char *buffer_;
  };
#endif

  /*  For limiting font formats in FontMatch constructor
   */
  enum EnabledFontFormats { AnyFont,      // Any available font format
                            TrueTypeOnly  // Only .ttf or .ttc fonts
  };

  class FontMatch {
  public:
#ifdef WT_FONTSUPPORT_SIMPLE

    FontMatch();
    FontMatch(const std::string& fileName, double quality);

    bool matched() const { return quality_ > 0; }

    std::string fileName() const { return file_; }
    void setFileName(const std::string &file) { file_ = file; }
    double quality() const { return quality_; }
    void setQuality(double quality) { quality_ = quality; }

#endif // WT_FONTSUPPORT_SIMPLE

#ifdef WT_FONTSUPPORT_PANGO

    FontMatch(PangoFont *font, PangoFontDescription *desc);

    bool matched() const { return true; }
    std::string fileName() const;
    PangoFont *pangoFont() const { return font_; }
    PangoFontDescription *pangoFontDescription() const { return desc_; }

#endif // WT_FONTSUPPORT_PANGO

#ifdef WT_FONTSUPPORT_DIRECTWRITE
    FontMatch();
    FontMatch(IDWriteTextFormat *textFormat,
      IDWriteFontFamily *fontFamily,
      IDWriteFont *font,
      std::string fontFamilyFileName);
    ~FontMatch();

    FontMatch(const FontMatch &other);
    FontMatch &operator=(const FontMatch &other);
#ifdef WT_CXX11
    FontMatch(FontMatch &&other);
    FontMatch &operator=(FontMatch &&other);
#endif // WT_CXX11

    bool matched() const;
    std::string fileName() const;

    std::wstring fontFamilyName() const;

    // For use in WRasterImage-d2d1.C
    IDWriteTextFormat *textFormat() { return textFormat_; }
    IDWriteFontFamily *fontFamily() { return fontFamily_; }
    IDWriteFont *font() { return font_; }
#endif // WT_FONTSUPPORT_DIRECTWRITE
  private:

#ifdef WT_FONTSUPPORT_SIMPLE
    std::string file_;
    double quality_;
#endif // WT_FONTSUPPORT_SIMPLE
#ifdef WT_FONTSUPPORT_PANGO
    mutable PangoFont *font_;
    PangoFontDescription *desc_;
#endif // WT_FONTSUPPORT_PANGO
#ifdef WT_FONTSUPPORT_DIRECTWRITE
    IDWriteTextFormat *textFormat_;
    IDWriteFontFamily *fontFamily_;
    IDWriteFont *font_;
    mutable std::string fontFamilyFileName_;
#endif // WT_FONTSUPPORT_DIRECTWRITE
  };

  FontSupport(WPaintDevice *device, EnabledFontFormats enabledFontFormats=AnyFont);
  ~FontSupport();

  void setDevice(WPaintDevice *device);

  /*
   * Returns the best matching true type font.
   */
  FontMatch matchFont(const WFont& f) const;

  /*
   * Returns font metrics
   */
  WFontMetrics fontMetrics(const WFont& f);

  /*
   * Measures the text, taking into account actual font choices.
   *
   * When there is a device_, the actual measurements are delegated to the
   * device. Otherwise, pango is used for measurements.
   */
  WTextItem measureText(const WFont& f, const WString& text, double maxWidth,
			bool wordWrap);

  /*
   * Draws the text, using pango for font choices, but delegating the actual
   * drawing to the device
   *
   * Precondition: device_ != nullptr
   */
  void drawText(const WFont& f,
		const WRectF& rect,
		WFlags<AlignmentFlag> alignmentFlags, const WString& text);

  /*
   * Returns true while in measureText() or drawText()
   */
  bool busy() const;

  /*
   * Returns the currently matched font path (while busy()).
   */
  std::string drawingFontPath() const;

  /*
   * Returns whether the implementation can actually render (which is only
   * the case for pango
   */
  bool canRender() const;

#ifndef WT_TARGET_JAVA
  /*
   * Draws the text, using pango and libfreetype to do the actual rendering
   *
   * Precondition: device_ == nullptr
   */
  void drawText(const WFont& f,
		const WRectF& rect,
		const WTransform& transform,
		Bitmap& bitmap,
		WFlags<AlignmentFlag> alignmentFlags, const WString& text);
#endif

  /*
   * If libpango support is available, this is a no-op.
   */
  void addFontCollection(const std::string& directory, bool recursive = true);

#ifdef WT_FONTSUPPORT_DIRECTWRITE
  /*
   * Direct access to IDWriteFactory for WRasterImage-d2d1
   */
  IDWriteFactory *writeFactory() { return writeFactory_; }
#endif // WT_FONTSUPPORT_DIRECTWRITE

private:
  WPaintDevice *device_;

#ifdef WT_FONTSUPPORT_SIMPLE

  struct FontCollection {
    std::string directory;
    bool recursive;
  };

  std::vector<FontCollection> fontCollections_;

  struct Matched {
    WFont font;
    FontMatch match;

    Matched() : font(), match() { }
  };

  typedef std::list<Matched> MatchCache;
  mutable MatchCache cache_;

  const WFont *font_;

  FontMatch matchFont(const WFont& font, const std::string& directory,
		      bool recursive) const;
  void matchFont(const WFont& font,
		 const std::vector<std::string>& fontNames,
		 const std::string& path,
		 bool recursive,
		 FontMatch& match) const;
  void matchFont(const WFont& font,
		 const std::vector<std::string>& fontNames,
		 const std::string& path,
		 FontMatch& match) const;

#endif // WT_FONTSUPPORT_SIMPLE

#ifdef WT_FONTSUPPORT_PANGO

  PangoContext *context_;
  PangoFont *currentFont_;
  EnabledFontFormats enabledFontFormats_;

  struct Matched {
    WFont font;
    PangoFont *match;
    PangoFontDescription *desc;

    Matched() : font(), match(nullptr), desc(nullptr) { }
    Matched(const WFont& f, PangoFont *m, PangoFontDescription *d) : font(f), match(m), desc(d) { }
  };

  typedef std::list<Matched> MatchCache;
  mutable MatchCache cache_;

  PangoFontDescription *createFontDescription(const WFont& f) const;
  static std::string fontPath(PangoFont *font);
  GList *layoutText(const WFont& font, const std::string& utf8,
		    std::vector<PangoGlyphString *>& glyphs, int& width);

  friend class FontMatch;

#endif // WT_FONTSUPPORT_PANGO

#ifdef WT_FONTSUPPORT_DIRECTWRITE
  EnabledFontFormats enabledFontFormats_;

  IDWriteFactory *writeFactory_;

  const WFont *font_;
  std::string drawingFontPath_;

  struct Matched {
    WFont font;
    FontMatch match;

    Matched()
      : font(), match()
    { }

    Matched(const WFont &f,
            FontMatch &&m)
      : font(f), match(m)
    { }
  };

  typedef std::list<Matched> MatchCache;
  mutable MatchCache cache_;

  struct TextFragment {
    TextFragment(std::string fontPath,
      std::size_t start,
      std::size_t length,
      double width)
      : fontPath(fontPath),
        start(start),
        length(length),
        width(width)
    { }

    std::string fontPath;
    std::size_t start;
    std::size_t length;
    double width;
  };
  class TextFragmentRenderer;

  void layoutText(const WFont &f, std::vector<TextFragment> &v, const std::wstring &s, double &width, double &nextWidth, double maxWidth, bool wordWrap);
#endif // WT_FONTSUPPORT_DIRECTWRITE
};

}

#endif // FONT_SUPPORT_H_
