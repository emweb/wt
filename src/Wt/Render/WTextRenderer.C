/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "web/FileUtils.h"
#include <Wt/WLogger.h>
#include <Wt/WPainter.h>
#include <Wt/Render/WTextRenderer.h>
#include <Wt/Render/CssParser.h>
#include <WebUtils.h>

#include "Block.h"

#include <fstream>
#include <string>

namespace {
  const double EPSILON = 1e-4;
  bool isEpsilonMore(double x, double limit) {
    return x - EPSILON > limit;
  }
}

namespace Wt {

LOGGER("Render.WTextRenderer");

  namespace Render {

class CombinedStyleSheet final : public StyleSheet
{
public:
  CombinedStyleSheet() { }
  virtual ~CombinedStyleSheet() { }

  void use(StyleSheet *sh) {
    sheets_.push_back(sh);
  }

  void use(std::unique_ptr<StyleSheet> sh) {
    use(sh.get());
    sheets_owned_.push_back(std::move(sh));
  }

  virtual unsigned int rulesetSize() const override
  {
    unsigned int result = 0;
    for (unsigned i = 0; i < sheets_.size(); ++i)
      result += sheets_[i]->rulesetSize();
    return result;
  }

  virtual const Ruleset& rulesetAt(int j) const override
  {
    for (unsigned i = 0; i < sheets_.size(); ++i) {
      if ((unsigned)j < sheets_[i]->rulesetSize())
	return sheets_[i]->rulesetAt(j);
      j -= sheets_[i]->rulesetSize();
    }

    return sheets_[0]->rulesetAt(0);
  }

private:
  std::vector<StyleSheet *> sheets_;
  std::vector<std::unique_ptr<StyleSheet> > sheets_owned_;
};

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
  return lb_.x + renderer_.margin(Side::Left);
}

double WTextRenderer::Node::y() const
{
  return lb_.y + renderer_.margin(Side::Top);
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
  : device_(nullptr),
    fontScale_(1),
    styleSheet_(nullptr)
{ }

WTextRenderer::~WTextRenderer()
{ }

void WTextRenderer::setFontScale(double factor)
{
  fontScale_ = factor;
}

double WTextRenderer::textWidth(int page) const
{
  return pageWidth(page) - margin(Side::Left) - margin(Side::Right);
}

double WTextRenderer::textHeight(int page) const
{
  return pageHeight(page) - margin(Side::Top) - margin(Side::Bottom);
}

double WTextRenderer::render(const WString& text, double y)
{
#ifndef WT_TARGET_JAVA
  std::string xhtml = text.toXhtmlUTF8();
#else
  std::string xhtml = WString(text).toXhtmlUTF8();
#endif

#ifndef WT_TARGET_JAVA
  unsigned l = xhtml.length();
  std::unique_ptr<char[]> cxhtml(new char[l + 1]);
  memcpy(cxhtml.get(), xhtml.c_str(), l);
  cxhtml[l] = 0;
#endif

  try {
#ifndef WT_TARGET_JAVA
    Wt::rapidxml::xml_document<> doc;
    doc.parse<Wt::rapidxml::parse_xhtml_entity_translation>(cxhtml.get());
#else
    Wt::rapidxml::xml_document<> doc = Wt::rapidxml::parseXHTML(xhtml);
#endif

    Block docBlock(&doc, nullptr);

    CombinedStyleSheet styles;
    if (styleSheet_)
      styles.use(styleSheet_.get());

    WStringStream ss;
    docBlock.collectStyles(ss);

    if (!ss.empty()) {
      CssParser parser;
      std::unique_ptr<Wt::Render::StyleSheet> docStyles = parser.parse(ss.str());
      if (docStyles)
	styles.use(std::move(docStyles));
      else
	LOG_ERROR("Error parsing style sheet: " << parser.getLastError());
    }

    docBlock.setStyleSheet(&styles);
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
    defaultFont.setFamily(FontFamily::SansSerif);
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
  } catch (Wt::rapidxml::parse_error& e) {
    throw e;
  }
}

bool WTextRenderer::useStyleSheet(const WString& filename)
{
  std::string* contents = FileUtils::fileToString(filename.toUTF8());
  if(!contents)
    return false;

  bool b = setStyleSheetText(styleSheetText() + "\n" + *contents);
  delete contents;
  return b;
}

void WTextRenderer::clearStyleSheet()
{
  setStyleSheetText("");
}

bool WTextRenderer::setStyleSheetText(const WString& styleSheetContents)
{
  if (styleSheetContents.empty()) {
    styleSheetText_ = WString();
    styleSheet_.reset();
    error_ = "";
    return true;
  } else {
    CssParser parser;
    std::unique_ptr<Wt::Render::StyleSheet> styleSheet = parser.parse(styleSheetContents);
    if (!styleSheet) {
      error_ = parser.getLastError();
      return false;
    }

    error_ = "";
    styleSheetText_ = styleSheetContents;
    styleSheet_ = std::move(styleSheet);
    return true;
  }
}

WString WTextRenderer::styleSheetText() const
{
  return styleSheetText_;
}

std::string WTextRenderer::getStyleSheetParseErrors() const
{
  return error_;
}

void WTextRenderer::paintNode(WPainter& painter, const Node& node)
{
  node.block().actualRender(*this, painter, node.lb());
}

  }
}
