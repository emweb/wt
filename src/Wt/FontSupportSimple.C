/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFont"
#include "Wt/WFontMetrics"
#include "Wt/FontSupport.h"

#include "Utils.h"
#include "WtException.h"

#ifdef WT_THREADED
#include <boost/thread.hpp>
#endif // WT_THREADED

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace {
#ifdef WT_THREADED
  boost::mutex fontRegistryMutex_;
#endif // WT_THREADED

  // Maps TrueType file names to font names
  std::map<std::string, std::string> fontRegistry_;
}

namespace Wt {

FontSupport::Bitmap::Bitmap(int width, int height)
{ }

FontSupport::Bitmap::~Bitmap()
{ }

FontSupport::FontMatch::FontMatch()
  : quality_(0.0)
{ }

FontSupport::FontMatch::FontMatch(const std::string& fileName, double quality)
  : file_(fileName),
    quality_(quality)
{ }

FontSupport::FontSupport(WPaintDevice *device)
  : device_(device),
    font_(0)
{ }

FontSupport::~FontSupport()
{ }

void FontSupport::addFontCollection(const std::string& directory, bool recursive)
{
  FontCollection c;
  c.directory = directory;
  c.recursive = recursive;

  fontCollections_.push_back(c);
}

bool FontSupport::canRender() const
{
  return false;
}

bool FontSupport::busy() const
{
  return font_;
}

std::string FontSupport::drawingFontPath() const
{
  return matchFont(*font_).fileName();
}

void FontSupport::setDevice(WPaintDevice *device)
{
  assert(false);
}

WFontMetrics FontSupport::fontMetrics(const WFont& font)
{
  font_ = &font;
  return device_->fontMetrics();
  font_ = 0;
}

WTextItem FontSupport::measureText(const WFont& font, const WString& text,
				   double maxWidth, bool wordWrap)
{
  font_ = &font;
  return device_->measureText(text, maxWidth, wordWrap);
  font_ = 0;
}

void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   WFlags<AlignmentFlag> flags, const WString& text)
{
  font_ = &font;
  device_->drawText(rect, flags, TextSingleLine, text);
  font_ = 0;
}

void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   const WTransform& transform, Bitmap& bitmap,
			   WFlags<AlignmentFlag> flags,
			   const WString& text)
{
  assert(false);
}

FontSupport::FontMatch FontSupport::matchFont(const WFont& font) const
{
  FontMatch match;

  for (unsigned i = 0; i < fontCollections_.size(); ++i) {
    FontMatch m = matchFont(font, fontCollections_[i].directory,
			    fontCollections_[i].recursive);

    if (m.quality() > match.quality())
      match = m;
  }

  return match;
}

FontSupport::FontMatch FontSupport::matchFont(const WFont& font,
					      const std::string& directory,
					      bool recursive) const
{
  boost::filesystem::path path(directory);

  if (!boost::filesystem::exists(path)
      || !boost::filesystem::is_directory(path))
    throw WtException("FontSupport: cannot read directory '" + directory + "'");

  std::vector<std::string> fontNames;
  std::string families = font.specificFamilies().toUTF8();

  boost::split(fontNames, families, boost::is_any_of(","));
  for (unsigned i = 0; i < fontNames.size(); ++i) {
    std::string s = Utils::lowerCase(fontNames[i]); // UTF-8 !
    Utils::replace(s, ' ', std::string());
    boost::trim_if(s, boost::is_any_of("\"'"));
    fontNames[i] = s;
  }

  switch (font.genericFamily()) {
  case WFont::Serif:
    fontNames.push_back("times");
    fontNames.push_back("timesnewroman");
    break;
  case WFont::SansSerif:
    fontNames.push_back("helvetica");
    fontNames.push_back("arialunicode");
    fontNames.push_back("arial");
    fontNames.push_back("verdana");
    break;
  case WFont::Cursive:
    fontNames.push_back("zapfchancery");
    break;
  case WFont::Fantasy:
    fontNames.push_back("western");
    break;
  case WFont::Monospace:
    fontNames.push_back("courier");
    fontNames.push_back("mscouriernew");
    break;
  default:
    ;
  }

  FontMatch match;
  matchFont(font, fontNames, path, recursive, match);

  return match;
}

void FontSupport::matchFont(const WFont& font,
			    const std::vector<std::string>& fontNames,
			    const boost::filesystem::path& path,
			    bool recursive,
			    FontMatch& match) const
{
  boost::filesystem::directory_iterator end_itr;

  for (boost::filesystem::directory_iterator i(path); i != end_itr; ++i) {
    if (boost::filesystem::is_directory(*i)) {
      if (recursive) {
	matchFont(font, fontNames, *i, recursive, match);
	if (match.quality() == 1.0)
	  return;
      }
    } else {
      matchFont(font, fontNames, *i, match);
      if (match.quality() == 1.0)
	return;
    }
  }
}

void FontSupport::matchFont(const WFont& font,
			    const std::vector<std::string>& fontNames,
			    const boost::filesystem::path& path,
			    FontMatch& match) const
{
  std::string f = Utils::lowerCase(path.leaf());

  if (boost::ends_with(f, ".ttf")
      || boost::ends_with(f, ".ttc")) {
    std::string name = f.substr(0, f.length() - 4);
    Utils::replace(name, ' ', std::string());

    char const *const boldVariants[] = { "bold", "bf", 0 };
    char const *const normalWeightVariants[] = { "", 0 };

    char const *const regularVariants[] = { "regular", "", 0 };
    char const *const italicVariants[] = { "italic", "oblique", 0 };
    char const *const obliqueVariants[] = { "oblique", 0 };

    char const *const *weightVariants, *const *styleVariants;

    if (font.weight() == WFont::Bold)
      weightVariants = boldVariants;
    else
      weightVariants = normalWeightVariants;

    switch (font.style()) {
    case WFont::NormalStyle: styleVariants = regularVariants; break;
    case WFont::Italic:  styleVariants = italicVariants;  break;
    case WFont::Oblique: styleVariants = obliqueVariants; break;
    }

    for (unsigned i = 0; i < fontNames.size(); ++i) {
      double q = 1.0 - 0.1 * i;

      if (q <= match.quality())
	return;

      for (char const *const *w = weightVariants; *w; ++w)
	for (char const *const *s = styleVariants; *s; ++s)
	  if (fontNames[i] + *w + *s == name) {
	    match = FontMatch(path.string(), q);
	    return;
	  }
    }
  }
}

}
