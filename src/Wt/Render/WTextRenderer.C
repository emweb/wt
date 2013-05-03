/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPainter>
#include <Wt/Render/WTextRenderer>
#include <WebUtils.h>

#include "Block.h"

#include <string>
#include <boost/lexical_cast.hpp>

namespace {
  const double EPSILON = 1e-4;
  bool isEpsilonMore(double x, double limit) {
    return x - EPSILON > limit;
  }
}

namespace Wt {

LOGGER("Render.WTextRenderer");

  namespace Render {

WTextRenderer::Node::Node(Block& block, LayoutBox& lb,
                          WTextRenderer& renderer):
  renderer_(renderer),
  block_(block),
  lb_(lb)
{ }

DomElementType WTextRenderer::Node::type() const
{
  return block_.type();
}

std::string WTextRenderer::Node::attributeValue(const std::string& attribute)
const
{
  const char *a = attribute.c_str();
  std::string ans = block_.attributeValue(a);
  return ans;
}

int WTextRenderer::Node::page() const
{
  return lb_.page;
}

double WTextRenderer::Node::x() const
{
  return lb_.x + renderer_.margin(Left);
}

double WTextRenderer::Node::y() const
{
  return lb_.y + renderer_.margin(Top);
}

double WTextRenderer::Node::width() const
{
  return lb_.width;
}

double WTextRenderer::Node::height() const
{
  return lb_.height;
}

int WTextRenderer::Node::fragment() const
{
  if (!block_.blockLayout.empty()){
    for(unsigned i = 0; i < block_.blockLayout.size(); ++i)
      if(&lb_ == &block_.blockLayout[i])
        return i;

    return -1;
  }else{
    for(unsigned i = 0; i < block_.inlineLayout.size(); ++i)
      if(&lb_ == &block_.inlineLayout[i])
        return i;

    return -1;
  }
}

int WTextRenderer::Node::fragmentCount() const
{
  return block_.blockLayout.size() + block_.inlineLayout.size();
}

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

    for (int i = 0; i < 2; ++i) {
      currentPs.y = y;
      currentPs.page = 0;
      currentPs.minX = minX;
      currentPs.maxX = maxX;

      collapseMarginBottom 
	= docBlock.layoutBlock(currentPs, false, *this,
			       std::numeric_limits<double>::max(),
			       collapseMarginBottom);

      if (isEpsilonMore(currentPs.maxX, maxX)) {
	if (!tooWide) {
	  LOG_WARN("contents too wide for page. ("
		   << currentPs.maxX << " > " << maxX << ")");
	  tooWide = true;
	}

	maxX = currentPs.maxX;
      } else {
	Block::clearFloats(currentPs, maxX - minX);

	break;
      }
    }

    for (int page = 0; page <= currentPs.page; ++page) {
      if (page != 0) {
	device_ = startPage(page);
	painter_ = getPainter(device_);
	painter_->setFont(defaultFont);
      }

      docBlock.render(*this, *painter_, page);

      endPage(device_);
    }

    return currentPs.y;
  } catch (rapidxml::parse_error& e) {
    throw e;
  }
}

void WTextRenderer::paintNode(WPainter& painter, const Node& node)
{
  node.block().actualRender(*this, painter, node.lb());
}

  }
}
