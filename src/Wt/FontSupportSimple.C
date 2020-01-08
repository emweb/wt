/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFont.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WLogger.h"
#include "Wt/FontSupport.h"

#include "WebUtils.h"
#include "FileUtils.h"

#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED

#include <boost/algorithm/string.hpp>

#include <cassert>
#include <vector>

namespace {
  // Maps TrueType file names to font names
  std::map<std::string, std::string> fontRegistry_;
}

namespace Wt {

LOGGER("FontSupportSimple");
	      
#ifndef WT_TARGET_JAVA
FontSupport::Bitmap::Bitmap(int width, int height)
{ }

FontSupport::Bitmap::~Bitmap()
{ }
#endif

FontSupport::FontMatch::FontMatch()
  : quality_(0.0)
{ }

FontSupport::FontMatch::FontMatch(const std::string& fileName, double quality)
  : file_(fileName),
    quality_(quality)
{ }

// FontSupportSimple only supports TrueType fonts, so can ignore enabledFontFormats
FontSupport::FontSupport(WPaintDevice *device, EnabledFontFormats /* enabledFontFormats */)
  : device_(device),
    font_(nullptr)
{ 
  for (int i = 0; i < 5; ++i)
    cache_.push_back(Matched());
}

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
  WFontMetrics fm = device_->fontMetrics();
  font_ = nullptr;
  return fm;
}

WTextItem FontSupport::measureText(const WFont& font, const WString& text,
				   double maxWidth, bool wordWrap)
{
  font_ = &font;
  WTextItem ti = device_->measureText(text, maxWidth, wordWrap);
  font_ = nullptr;
  return ti;
}

void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   WFlags<AlignmentFlag> flags, const WString& text)
{
  font_ = &font;
  device_->drawText(rect, flags, TextFlag::SingleLine, text, 0);
  font_ = nullptr;
}

#ifndef WT_TARGET_JAVA
void FontSupport::drawText(const WFont& font, const WRectF& rect,
			   const WTransform& transform, Bitmap& bitmap,
			   WFlags<AlignmentFlag> flags,
			   const WString& text)
{
  assert(false);
}
#endif

FontSupport::FontMatch FontSupport::matchFont(const WFont& font) const
{
  for (MatchCache::iterator i = cache_.begin(); i != cache_.end(); ++i) {
    if (i->font.genericFamily() == font.genericFamily() &&
	i->font.specificFamilies() == font.specificFamilies() &&
	i->font.weight() == font.weight() &&
	i->font.style() == font.style()) {
      cache_.splice(cache_.begin(), cache_, i); // implement LRU
      return cache_.front().match;
    }
  }

  FontMatch match;

  for (unsigned i = 0; i < fontCollections_.size(); ++i) {
    FontMatch m = matchFont(font, fontCollections_[i].directory,
			    fontCollections_[i].recursive);

    if (m.quality() > match.quality())
      match = m;
  }

  // implement LRU
  cache_.pop_back();
  cache_.push_front(Matched());
  cache_.front().font = font;
  cache_.front().match = match;

  return match;
}

FontSupport::FontMatch FontSupport::matchFont(const WFont& font,
					      const std::string& directory,
					      bool recursive) const
{
  if (!FileUtils::exists(directory)
      || !FileUtils::isDirectory(directory)) {
    LOG_ERROR("cannot read directory '" << directory << "'");
    return FontMatch();
  }

  std::vector<std::string> fontNames;
  std::string families = font.specificFamilies().toUTF8();

  boost::split(fontNames, families, boost::is_any_of(","));
  for (unsigned i = 0; i < fontNames.size(); ++i) {
    std::string s = Utils::lowerCase(fontNames[i]); // UTF-8 !
    boost::trim_if(s, boost::is_any_of("\"'"));
    s = Utils::replace(s, ' ', std::string());
    fontNames[i] = s;
  }

  switch (font.genericFamily()) {
  case FontFamily::Serif:
    fontNames.push_back("times");
    fontNames.push_back("timesnewroman");
    break;
  case FontFamily::SansSerif:
    fontNames.push_back("helvetica");
    fontNames.push_back("arialunicode");
    fontNames.push_back("arial");
    fontNames.push_back("verdana");
    break;
  case FontFamily::Cursive:
    fontNames.push_back("zapfchancery");
    break;
  case FontFamily::Fantasy:
    fontNames.push_back("western");
    break;
  case FontFamily::Monospace:
    fontNames.push_back("courier");
    fontNames.push_back("mscouriernew");
    break;
  default:
    ;
  }

  FontMatch match;
  matchFont(font, fontNames, directory, recursive, match);

  return match;
}

void FontSupport::matchFont(const WFont& font,
			    const std::vector<std::string>& fontNames,
			    const std::string& path,
			    bool recursive,
			    FontMatch& match) const
{
  std::vector<std::string> files;
  FileUtils::listFiles(path, files);
  
  for (unsigned i = 0; i < files.size(); ++i) {
    std::string f = files[i];
    if (FileUtils::isDirectory(f)) {
      if (recursive) {
	matchFont(font, fontNames, f, recursive, match);
	if (match.quality() == 1.0)
	  return;
      }
    } else {
      matchFont(font, fontNames, f, match);
      if (match.quality() == 1.0)
	return;
    }
  }
}

void FontSupport::matchFont(const WFont& font,
			    const std::vector<std::string>& fontNames,
			    const std::string& path,
			    FontMatch& match) const
{
  if (boost::ends_with(path, ".ttf")
      || boost::ends_with(path, ".ttc")) {
    std::string name = Utils::lowerCase(FileUtils::leaf(path));
    name = name.substr(0, name.length() - 4);
    Utils::replace(name, ' ', std::string());

    std::vector<const char*> weightVariants, styleVariants;

    if (font.weight() == FontWeight::Bold) {
      weightVariants.push_back("bold");
      weightVariants.push_back("bf");
    } else {
      weightVariants.push_back("");
    }

    switch (font.style()) {
    case FontStyle::Normal: 
      styleVariants.push_back("regular");
      styleVariants.push_back(""); 
      break;
    case FontStyle::Italic:  
      styleVariants.push_back("italic");
      styleVariants.push_back("oblique");  
      break;
    case FontStyle::Oblique: 
      styleVariants.push_back("oblique"); 
      break;
    }

    for (unsigned i = 0; i < fontNames.size(); ++i) {
      double q = 1.0 - 0.1 * i;

      if (q <= match.quality())
	return;
      
      for (unsigned w = 0; w < weightVariants.size(); ++w)
	for (unsigned s = 0; s < styleVariants.size(); ++s) {
	  std::string fn = fontNames[i] + weightVariants[w] + styleVariants[s];
	  if (fn == name) {
	    match = FontMatch(path, q);
	    return;
	  }
	}
    }
  }
}

}
