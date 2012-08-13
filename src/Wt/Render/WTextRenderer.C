/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPainter>
#include <Wt/Render/WTextRenderer>

#include "Block.h"

#include <string>

namespace Wt {

LOGGER("Render.WTextRenderer");

  namespace Render {

WTextRenderer::WTextRenderer()
  : device_(0),
    fontScale_(1)
{ }

WTextRenderer::~WTextRenderer()
{ }

void WTextRenderer::setFontScale(double factor)
{
  fontScale_ = factor;
}

double WTextRenderer::textWidth(int page) const
{
  return pageWidth(page) - margin(Left) - margin(Right);
}

double WTextRenderer::textHeight(int page) const
{
  return pageHeight(page) - margin(Top) - margin(Bottom);
}

double WTextRenderer::render(const WString& text, double y)
{
  std::string xhtml = text.toUTF8();

#ifndef WT_TARGET_JAVA
  unsigned l = xhtml.length();
  boost::scoped_array<char> cxhtml(new char[l + 1]);
  memcpy(cxhtml.get(), xhtml.c_str(), l);
  cxhtml[l] = 0;
#endif

  try {
#ifndef WT_TARGET_JAVA
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_xhtml_entity_translation>(cxhtml.get());
#else
    rapidxml::xml_document<> doc = rapidxml::parseXHTML(xhtml);
#endif

    Block docBlock(&doc, (Block*)0);

    docBlock.determineDisplay();
    docBlock.normalizeWhitespace(false, doc);

    PageState currentPs;
    currentPs.y = y;
    currentPs.page = 0;
    currentPs.minX = 0;
    currentPs.maxX = textWidth(currentPs.page);

    device_ = startPage(currentPs.page);
    painter_ = getPainter(device_);

    WFont defaultFont;
    defaultFont.setFamily(WFont::SansSerif);
    painter_->setFont(defaultFont);

    double collapseMarginBottom = 0;

    double minX = 0;
    double maxX = textWidth(currentPs.page);

    bool tooWide = false;

    for (;;) {
      currentPs.minX = minX;
      currentPs.maxX = maxX;

      collapseMarginBottom 
	= docBlock.layoutBlock(currentPs,
			       false, *this, std::numeric_limits<double>::max(),
			       collapseMarginBottom);

      if (currentPs.maxX > maxX) {
	if (!tooWide) {
	  LOG_WARN("contents too wide for page.");
	  tooWide = true;
	}

	maxX = currentPs.maxX;
      } else {
	Block::clearFloats(currentPs, 
			   maxX - minX);

	break;
      }
    }

    for (int page = 0; page <= currentPs.page; ++page) {
      if (page != 0) {
	device_ = startPage(page);
	painter_ = getPainter(device_);
	painter_->setFont(defaultFont);
      }

      docBlock.render(*this, page);

      endPage(device_);
    }

    return currentPs.y;
  } catch (rapidxml::parse_error& e) {
    throw e;
  }
}

  }
}
