/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WLineEdit"

#include "DomElement.h"
#include "CgiParser.h"

namespace Wt {

WLineEdit::WLineEdit(WContainerWidget *parent)
  : WFormWidget(parent),
    textSize_(10),
    maxLength_(-1),
    echoMode_(Normal)
{ 
  setInline(true);
  setFormObject(true);
}

WLineEdit::WLineEdit(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    content_(text),
    textSize_(10),
    maxLength_(-1),
    echoMode_(Normal)
{ 
  setInline(true);
  setFormObject(true);
}

void WLineEdit::setText(const WString& text)
{
  if (content_ != text) {
    content_ = text;
    flags_.set(BIT_CONTENT_CHANGED);
    repaint(RepaintPropertyIEMobile);

    if (validator())
      setStyleClass(validate() == WValidator::Valid ? "" : "Wt-invalid");
  }
}

void WLineEdit::setTextSize(int chars)
{
  if (textSize_ != chars) {
    textSize_ = chars;
    flags_.set(BIT_TEXT_SIZE_CHANGED);
    repaint(RepaintPropertyAttribute);
  }
}

void WLineEdit::setMaxLength(int chars)
{
  if (maxLength_ != chars) {
    maxLength_ = chars;
    flags_.set(BIT_MAX_LENGTH_CHANGED);
    // IMPROVE: could be RepaintPropertyIEMobile if we would use property
    repaint(RepaintPropertyAttribute);
  }
}

void WLineEdit::setEchoMode(EchoMode echoMode)
{
  if (echoMode_ != echoMode) {
    echoMode_ = echoMode;
    flags_.set(BIT_ECHO_MODE_CHANGED);
    repaint(RepaintPropertyAttribute);
  }
}

void WLineEdit::updateDom(DomElement& element, bool all)
{
  if (all || flags_.test(BIT_CONTENT_CHANGED)) {
    element.setProperty(Wt::PropertyValue, content_.toUTF8());
    flags_.reset(BIT_CONTENT_CHANGED);
  }

  if (all || flags_.test(BIT_ECHO_MODE_CHANGED)) {
    element.setAttribute("type", echoMode_ == Normal ? "text" : "password");
    flags_.reset(BIT_ECHO_MODE_CHANGED);
  }

  if (all || flags_.test(BIT_TEXT_SIZE_CHANGED)) {
    element.setAttribute("size",
			 boost::lexical_cast<std::string>(textSize_));
    flags_.reset(BIT_TEXT_SIZE_CHANGED);
  }

  if (all || flags_.test(BIT_MAX_LENGTH_CHANGED)) {
    if (!all || maxLength_ > 0)
      element.setAttribute("maxlength",
			   boost::lexical_cast<std::string>(maxLength_));

    flags_.reset(BIT_MAX_LENGTH_CHANGED);
  }

  WFormWidget::updateDom(element, all);
}

DomElementType WLineEdit::domElementType() const
{
  return DomElement_INPUT;
}

void WLineEdit::setFormData(CgiEntry *entry)
{
  content_ = WString(entry->value(), UTF8);
}

WValidator::State WLineEdit::validate()
{
  if (validator()) {
    int pos;

    return validator()->validate(content_, pos);
  } else
    return WValidator::Valid;
}

}
