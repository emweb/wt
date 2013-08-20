// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FONT_SUPPORT_H_
#define FONT_SUPPORT_H_

#include <list>

#ifndef HAVE_PANGO
#include <string>
#else
#include <pango/pango.h>
#endif // HAVE_PANGO

#include <Wt/WPaintDevice>

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

  class FontMatch {
  public:
#ifdef HAVE_PANGO

    FontMatch(PangoFont *font);

    bool matched() const { return true; }
    std::string fileName() const;
    PangoFont *pangoFont() const { return font_; }

#else

    FontMatch();
    FontMatch(const std::string& fileName, double quality);

    bool matched() const { return quality_ > 0; }

    std::string fileName() const { return file_; }
    void setFileName(const std::string &file) { file_ = file; }
    double quality() const { return quality_; }
    void setQuality(double quality) { quality_ = quality; }

#endif

  private:

#ifdef HAVE_PANGO
    mutable PangoFont *font_;
#else
    std::string file_;
    double quality_;
#endif
  };

  FontSupport(WPaintDevice *device);
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
   * Precondition: device_ != 0
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
   * Precondition: device_ == 0
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

private:
  WPaintDevice *device_;

#ifdef HAVE_PANGO

  PangoContext *context_;
  PangoFont *currentFont_;

  struct Matched {
    WFont font;
    PangoFont *match;

    Matched() : font(), match(0) { }
    Matched(const WFont& f, PangoFont *m) : font(f), match(m) { }
  };

  typedef std::list<Matched> MatchCache;
  mutable MatchCache cache_;

  PangoFontDescription *createFontDescription(const WFont& f) const;
  static std::string fontPath(PangoFont *font);
  GList *layoutText(const WFont& font, const std::string& utf8,
		    std::vector<PangoGlyphString *>& glyphs, int& width);

  friend class FontMatch;

#else 

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

#endif // HAVE_PANGO
};

}

#endif // FONT_SUPPORT_H_
