/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/WText.h"
#include "DomElement.h"
#include "WebSession.h"
#include "RefEncoder.h"

namespace Wt {

LOGGER("WText");

WText::RichText::RichText()
  : format(TextFormat::XHTML)
{ }

bool WText::RichText::setText(const WString& newText)
{
  text = newText;

  bool ok = checkWellFormed();
  if (!ok)
    format = TextFormat::Plain;

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
  if (format == TextFormat::XHTML && 
      (text.literal() || !text.args().empty())) {
    return removeScript(text);
  } else
    return true;
}

std::string WText::RichText::formattedText() const
{
  if (format == TextFormat::Plain)
    return escapeText(text, true).toUTF8();
  else
    return text.toXhtmlUTF8();
}

WText::WText()
  : padding_(nullptr)
{
  flags_.set(BIT_WORD_WRAP);
}

WText::WText(const WString& text)
  : padding_(nullptr)
{
  flags_.set(BIT_WORD_WRAP);
  setText(text);
}

WText::WText(const WString& text, TextFormat format)
  : padding_(nullptr)
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
  bool unChanged = canOptimizeUpdates() && (text == text_.text);

  // Even if the current text is the same, it may be a tr() one which
  // may get other content in a different locale
  bool ok = text_.setText(text);

  if (canOptimizeUpdates() && unChanged)
    return true;

  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintFlag::SizeAffected);

  return ok;
}

void WText::autoAdjustInline()
{
  if (text_.format != TextFormat::Plain && isInline()) {
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
    repaint(RepaintFlag::SizeAffected);
  }
}

void WText::setTextAlignment(AlignmentFlag textAlignment)
{
  flags_.reset(BIT_TEXT_ALIGN_LEFT);
  flags_.reset(BIT_TEXT_ALIGN_CENTER);
  flags_.reset(BIT_TEXT_ALIGN_RIGHT);

  switch (textAlignment) {
  case AlignmentFlag::Left: flags_.set(BIT_TEXT_ALIGN_LEFT); break;
  case AlignmentFlag::Center: flags_.set(BIT_TEXT_ALIGN_CENTER); break;
  case AlignmentFlag::Right: flags_.set(BIT_TEXT_ALIGN_RIGHT); break;
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
    return AlignmentFlag::Center;
  else if (flags_.test(BIT_TEXT_ALIGN_RIGHT))
    return AlignmentFlag::Right;
  else
    return AlignmentFlag::Left; // perhaps take into account RLT setting?
}

void WText::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    std::string text = formattedText();
    if (flags_.test(BIT_TEXT_CHANGED) || !text.empty())
      element.setProperty(Wt::Property::InnerHTML, text);
    flags_.reset(BIT_TEXT_CHANGED);
  }

  if (flags_.test(BIT_WORD_WRAP_CHANGED) || all) {
    if (!all || !flags_.test(BIT_WORD_WRAP))
      element.setProperty(Wt::Property::StyleWhiteSpace,
			  flags_.test(BIT_WORD_WRAP) ? "normal" : "nowrap");
    flags_.reset(BIT_WORD_WRAP_CHANGED);
  }

  if (flags_.test(BIT_PADDINGS_CHANGED)
      || (all && padding_ &&
          !(   padding_[0].isAuto() && padding_[1].isAuto()
            && padding_[2].isAuto() && padding_[3].isAuto()))) {

    if ((padding_[0] == padding_[1]) && (padding_[0] == padding_[2])
        && (padding_[0] == padding_[3]))
      element.setProperty(Property::StylePadding, padding_[0].cssText());
    else {
      WStringStream s;
      for (unsigned i = 0; i < 4; ++i) {
        if (i != 0)
          s << ' ';
        s << (padding_[i].isAuto() ? "0" : padding_[i].cssText());
      }
      element.setProperty(Property::StylePadding, s.str());
    }

    flags_.reset(BIT_PADDINGS_CHANGED);
  }

  if (flags_.test(BIT_TEXT_ALIGN_CHANGED) || all) {
    if (flags_.test(BIT_TEXT_ALIGN_CENTER))
      element.setProperty(Property::StyleTextAlign, "center");
    else if (flags_.test(BIT_TEXT_ALIGN_RIGHT))
      element.setProperty(Property::StyleTextAlign, "right");
    else if (flags_.test(BIT_TEXT_ALIGN_LEFT))
      element.setProperty(Property::StyleTextAlign, "left");
    else if (!all)
      element.setProperty(Property::StyleTextAlign, "");

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
    padding_ = new WLength[4];
#ifdef WT_TARGET_JAVA
    padding_[0] = padding_[1] = padding_[2] = padding_[3] = WLength::Auto;
#endif // WT_TARGET_JAVA
  }

  if (sides.test(Side::Top)) {
    if (isInline()) {
      LOG_WARN("setPadding(..., Side::Top) is not supported for inline WText. "
               "If your WText is not inline, you can call setInline(true) before setPadding(...) "
               "to disable this warning.");
    }
    padding_[0] = length;
  }
  if (sides.test(Side::Right))
    padding_[1] = length;
  if (sides.test(Side::Bottom)) {
    if (isInline()) {
      LOG_WARN("setPadding(..., Side::Bottom) is not supported for inline WText. "
               "If your WText is not inline, you can call setInline(true) before setPadding(...) "
               "to disable this warning.");
    }
    padding_[2] = length;
  }
  if (sides.test(Side::Left))
    padding_[3] = length;

  flags_.set(BIT_PADDINGS_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

WLength WText::padding(Side side) const
{
  if (!padding_)
    return WLength::Auto;

  switch (side) {
  case Side::Top:
    return padding_[0];
  case Side::Right:
    return padding_[1];
  case Side::Bottom:
    return padding_[2];
  case Side::Left:
    return padding_[3];
  default:
    LOG_ERROR("padding(): improper side.");
    return WLength();
  }
}

std::string WText::formattedText() const
{
  if (text_.format == TextFormat::Plain)
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
      return EncodeRefs(text_.text, options).toXhtmlUTF8();
    } else
      return text_.text.toXhtmlUTF8();
  }
}

DomElementType WText::domElementType() const
{
  return isInline() ? DomElementType::SPAN : DomElementType::DIV;
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
    repaint(RepaintFlag::SizeAffected);
  }

  WInteractWidget::refresh();
}

}
