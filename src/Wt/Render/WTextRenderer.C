/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPainter>
#include <Wt/Render/WTextRenderer>

#include "Block.h"

#include <string.h>

namespace Wt {
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
  unsigned l = xhtml.length();

  boost::scoped_array<char> cxhtml(new char[l + 1]);
  memcpy(cxhtml.get(), xhtml.c_str(), l);
  cxhtml[l] = 0;

  try {
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_xhtml_entity_translation>(cxhtml.get());

    Block docBlock(&doc, 0);

    docBlock.determineDisplay();
    docBlock.normalizeWhitespace(false, doc);

    double currentY = y;
    int currentPage = 0;

    BlockList floats;

    device_ = startPage(currentPage);
    painter_ = getPainter(device_);

    WFont defaultFont;
    defaultFont.setFamily(WFont::SansSerif);
    painter_->setFont(defaultFont);

    double collapseMarginBottom;

    double minX = 0;
    double maxX = textWidth(currentPage);
    bool tooWide = false;

    for (;;) {
      try {
	docBlock.layoutBlock(currentY, currentPage, floats, minX, maxX,
			     false, *this, std::numeric_limits<double>::max(),
			     collapseMarginBottom);

	Block::clearFloats(currentY, currentPage, floats, minX, maxX,
			   maxX - minX);

	break;
      } catch (PleaseWiden& w) {
	if (!tooWide) {
	  std::cerr << "Warning: contents too wide for page." << std::endl;
	  tooWide = true;
	}

	maxX += w.width;
      }
    }

    for (int page = 0; page <= currentPage; ++page) {
      if (page != 0) {
	device_ = startPage(page);
	painter_ = getPainter(device_);
	painter_->setFont(defaultFont);
      }

      docBlock.render(*this, page);

      endPage(device_);
    }

    return currentY;
  } catch (rapidxml::parse_error& e) {
    throw e;
  }
}

  }
}
