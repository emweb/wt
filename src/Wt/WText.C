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

WText::RichText::RichText()
  : format(XHTMLText)
{ }

bool WText::RichText::setText(const WString& newText)
{
  text = newText;

  bool ok = checkWellFormed();
  if (!ok)
    format = PlainText;

  return ok;
}

bool WText::RichText::setFormat(TextFormat newFormat)
{
  if (format != newFormat) {
    TextFormat oldFormat = format;
    format = newFormat;
    bool ok = checkWellFormed();

    if (!ok)
      format = oldFormat;

    return ok;
  } else
    return true;
}

bool WText::RichText::checkWellFormed()
{
  if (format == XHTMLText && text.literal()) {
    return removeScript(text);
  } else
    return true;
}

std::string WText::RichText::formattedText() const
{
  if (format == PlainText)
    return escapeText(text, true).toUTF8();
  else
    return text.toUTF8();
}

WText::WText(WContainerWidget *parent)
  : WInteractWidget(parent),
    padding_(0)
{
  flags_.set(BIT_WORD_WRAP);
}

WText::WText(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    padding_(0)
{
  flags_.set(BIT_WORD_WRAP);
  setText(text);
}

WText::WText(const WString& text, TextFormat format, WContainerWidget *parent)
  : WInteractWidget(parent),
    padding_(0)
{
  text_.format = format;
  flags_.set(BIT_WORD_WRAP);
  setText(text);
}

WText::~WText() 
{
  delete[] padding_;
}

bool WText::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_.text))
    return true;

  bool ok = text_.setText(text);

  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintSizeAffected);

  return ok;
}

void WText::autoAdjustInline()
{
  if (text_.format != PlainText && isInline()) {
    std::string t = text_.text.toUTF8();
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
    repaint(RepaintSizeAffected);
  }
}

void WText::setTextAlignment(AlignmentFlag textAlignment)
{
  flags_.reset(BIT_TEXT_ALIGN_LEFT);
  flags_.reset(BIT_TEXT_ALIGN_CENTER);
  flags_.reset(BIT_TEXT_ALIGN_RIGHT);

  switch (textAlignment) {
  case AlignLeft: flags_.set(BIT_TEXT_ALIGN_LEFT); break;
  case AlignCenter: flags_.set(BIT_TEXT_ALIGN_CENTER); break;
  case AlignRight: flags_.set(BIT_TEXT_ALIGN_RIGHT); break;
  default:
    LOG_ERROR("setTextAlignment(): illegal value for textAlignment");
    return;
  }

  flags_.set(BIT_TEXT_ALIGN_CHANGED);
  repaint();
}

AlignmentFlag WText::textAlignment() const
{
  if (flags_.test(BIT_TEXT_ALIGN_CENTER))
    return AlignCenter;
  else if (flags_.test(BIT_TEXT_ALIGN_RIGHT))
    return AlignRight;
  else
    return AlignLeft; // perhaps take into account RLT setting?
}

void WText::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    std::string text = formattedText();
    if (flags_.test(BIT_TEXT_CHANGED) || !text.empty())
      element.setProperty(Wt::PropertyInnerHTML, text);
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

  if (flags_.test(BIT_TEXT_ALIGN_CHANGED) || all) {
    if (flags_.test(BIT_TEXT_ALIGN_CENTER))
      element.setProperty(PropertyStyleTextAlign, "center");
    else if (flags_.test(BIT_TEXT_ALIGN_RIGHT))
      element.setProperty(PropertyStyleTextAlign, "right");
    else if (flags_.test(BIT_TEXT_ALIGN_LEFT))
      element.setProperty(PropertyStyleTextAlign, "left");
    else if (!all)
      element.setProperty(PropertyStyleTextAlign, "");

    flags_.reset(BIT_TEXT_ALIGN_CHANGED);
  }

  WInteractWidget::updateDom(element, all);
}

void WText::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_TEXT_CHANGED);
  flags_.reset(BIT_WORD_WRAP_CHANGED);
  flags_.reset(BIT_PADDINGS_CHANGED);
  flags_.reset(BIT_TEXT_ALIGN_CHANGED);

  WInteractWidget::propagateRenderOk(deep);
}

bool WText::setTextFormat(TextFormat textFormat)
{
  return text_.setFormat(textFormat);
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
  repaint(RepaintSizeAffected);
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
  if (text_.format == PlainText)
    return escapeText(text_.text, true).toUTF8();
  else {
    WApplication *app = WApplication::instance();
    if (flags_.test(BIT_ENCODE_INTERNAL_PATHS)
	|| app->session()->hasSessionIdInUrl()) {
      WFlags<RefEncoderOption> options;
      if (flags_.test(BIT_ENCODE_INTERNAL_PATHS))
	options |= EncodeInternalPaths;
      if (app->session()->hasSessionIdInUrl())
	options |= EncodeRedirectTrampoline;
      WString result = text_.text;
      EncodeRefs(result, options);
      return result.toUTF8();
    } else
      return text_.text.toUTF8();
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
  if (text_.text.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintSizeAffected);
  }

  WInteractWidget::refresh();
}

}
