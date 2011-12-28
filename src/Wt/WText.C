/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WText"
#include "DomElement.h"
#include "WebSession.h"
#include "RefEncoder.h"

namespace Wt {

LOGGER("WText");

WText::WText(WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    padding_(0)
{
  flags_.set(BIT_WORD_WRAP);
  WT_DEBUG(setObjectName("WText"));
}

WText::WText(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    padding_(0)
{
  flags_.set(BIT_WORD_WRAP);
  WT_DEBUG(setObjectName("WText"));
  setText(text);
}

WText::WText(const WString& text, TextFormat format, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(format),
    padding_(0)
{
  flags_.set(BIT_WORD_WRAP);
  WT_DEBUG(setObjectName("WText"));
  setText(text);
}

WText::~WText() 
{
  delete[] padding_;
}

bool WText::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_))
    return true;

  text_ = text;

  bool textok = checkWellFormed();
  if (!textok)
    textFormat_ = PlainText;

  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintInnerHtml);

  return textok;
}

void WText::autoAdjustInline()
{
  if (textFormat_ != PlainText && isInline()) {
    std::string t = text_.toUTF8();
    boost::trim_left(t);
    if (   boost::istarts_with(t, "<div")
	|| boost::istarts_with(t, "<p")
	|| boost::istarts_with(t, "<h"))
      setInline(false);
  }
}

void WText::setWordWrap(bool wordWrap)
{
  if (flags_.test(BIT_WORD_WRAP) != wordWrap) {
    flags_.set(BIT_WORD_WRAP, wordWrap);
    flags_.set(BIT_WORD_WRAP_CHANGED);
    repaint(RepaintPropertyAttribute);
  }
}

void WText::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    std::string text = formattedText();
    if (flags_.test(BIT_TEXT_CHANGED) || !text.empty())
      element.setProperty(Wt::PropertyInnerHTML, formattedText());
    flags_.reset(BIT_TEXT_CHANGED);
  }

  if (flags_.test(BIT_WORD_WRAP_CHANGED) || all) {
    if (!all || !flags_.test(BIT_WORD_WRAP))
      element.setProperty(Wt::PropertyStyleWhiteSpace,
			  flags_.test(BIT_WORD_WRAP) ? "normal" : "nowrap");
    flags_.reset(BIT_WORD_WRAP_CHANGED);
  }

  if (flags_.test(BIT_PADDINGS_CHANGED)
      || (all && padding_ &&
	  !(padding_[0].isAuto() && padding_[1].isAuto()))) {
    
    element.setProperty(PropertyStylePaddingRight, padding_[0].cssText());
    element.setProperty(PropertyStylePaddingLeft, padding_[1].cssText());

    flags_.reset(BIT_PADDINGS_CHANGED);
  }

  WInteractWidget::updateDom(element, all);
}

void WText::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_TEXT_CHANGED);
  flags_.reset(BIT_WORD_WRAP_CHANGED);
  flags_.reset(BIT_PADDINGS_CHANGED);

  WInteractWidget::propagateRenderOk(deep);
}

bool WText::setTextFormat(TextFormat textFormat)
{
  if (textFormat_ != textFormat) {
    TextFormat oldTextFormat = textFormat_;

    textFormat_ = textFormat;
    bool textok = checkWellFormed();

    if (!textok)
      textFormat_ = oldTextFormat;

    return textok;
  } else
    return true;
}

bool WText::checkWellFormed()
{
  if (textFormat_ == XHTMLText && text_.literal()) {
    return removeScript(text_);
  } else
    return true;
}

void WText::setInternalPathEncoding(bool enabled)
{
  if (flags_.test(BIT_ENCODE_INTERNAL_PATHS) != enabled) {
    flags_.set(BIT_ENCODE_INTERNAL_PATHS, enabled);
    flags_.set(BIT_TEXT_CHANGED);
  }
}

void WText::setPadding(const WLength& length, WFlags<Side> sides)
{
  if (!padding_) {
    padding_ = new WLength[2];
#ifdef WT_TARGET_JAVA
    padding_[0] = padding_[1] = WLength::Auto;
#endif // WT_TARGET_JAVA
  }

  if (sides & Right)
    padding_[0] = length;
  if (sides & Left)
    padding_[1] = length;

  if (sides & Top)
    LOG_ERROR("setPadding(..., Top) is not supported.");

  if (sides & Bottom)
    LOG_ERROR("setPadding(..., Bottom) is not supported.");

  flags_.set(BIT_PADDINGS_CHANGED);
  repaint(RepaintPropertyAttribute);
}

WLength WText::padding(Side side) const
{
  if (!padding_)
    return WLength::Auto;

  switch (side) {
  case Top:
    LOG_ERROR("padding(Top) is not supported.");
    return WLength();
  case Right:
    return padding_[1];
  case Bottom:
    LOG_ERROR("padding(Bottom) is not supported.");
  case Left:
    return padding_[3];
  default:
    LOG_ERROR("padding(Side) with invalid side: " << (int)side);
    return WLength();
  }
}

std::string WText::formattedText() const
{
  if (textFormat_ == PlainText)
    return escapeText(text_, true).toUTF8();
  else {
    WApplication *app = WApplication::instance();
    if (flags_.test(BIT_ENCODE_INTERNAL_PATHS)
	|| app->session()->hasSessionIdInUrl()) {
      WFlags<RefEncoderOption> options;
      if (flags_.test(BIT_ENCODE_INTERNAL_PATHS))
	options |= EncodeInternalPaths;
      if (app->session()->hasSessionIdInUrl())
	options |= EncodeRedirectTrampoline;
      WString result = text_;
      EncodeRefs(result, options);
      return result.toUTF8();
    } else
      return text_.toUTF8();
  }
}

DomElementType WText::domElementType() const
{
  return isInline() ? DomElement_SPAN : DomElement_DIV;
}

void WText::render(WFlags<RenderFlag> flags)
{
  if (flags_.test(BIT_TEXT_CHANGED))
    autoAdjustInline();

  WInteractWidget::render(flags);
}

void WText::refresh()
{
  if (text_.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintInnerHtml);
  }

  WInteractWidget::refresh();
}

}
