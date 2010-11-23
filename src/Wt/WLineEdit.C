/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WLineEdit"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WStringUtil"

#include "DomElement.h"
#include "Utils.h"

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

WLineEdit::WLineEdit(const WT_USTRING& text, WContainerWidget *parent)
  : WFormWidget(parent),
    content_(text),
    textSize_(10),
    maxLength_(-1),
    echoMode_(Normal)
{ 
  setInline(true);
  setFormObject(true);
}

void WLineEdit::setText(const WT_USTRING& text)
{
  if (content_ != text) {
    content_ = text;
    flags_.set(BIT_CONTENT_CHANGED);
    repaint(RepaintPropertyIEMobile);

    validate();

    updateEmptyText();
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
      element.setAttribute("maxLength",
			   boost::lexical_cast<std::string>(maxLength_));

    flags_.reset(BIT_MAX_LENGTH_CHANGED);
  }

  WFormWidget::updateDom(element, all);
}

void WLineEdit::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  if (app->environment().agentIsIE() && flags_.test(BIT_ECHO_MODE_CHANGED)) {
    DomElement *e = DomElement::getForUpdate(this, domElementType());
    DomElement *d = createDomElement(app);
    e->replaceWith(d);
    result.push_back(e);
  } else
    WFormWidget::getDomChanges(result, app);
}

void WLineEdit::propagateRenderOk(bool deep)
{
  flags_.reset();

  WFormWidget::propagateRenderOk(deep);
}

DomElementType WLineEdit::domElementType() const
{
  return DomElement_INPUT;
}

void WLineEdit::setFormData(const FormData& formData)
{
  // if the value was updated through the API, then ignore the update from
  // the browser, this happens when an action generated multiple events,
  // and we do not want to revert the changes made through the API
  if (flags_.test(BIT_CONTENT_CHANGED))
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];
    content_ = WT_USTRING::fromUTF8(value, true);
  }
}

WValidator::State WLineEdit::validate()
{
  if (validator()) {
    WValidator::State result = validator()->validate(content_);
    if (result == WValidator::Valid)
      removeStyleClass("Wt-invalid", true);
    else
      addStyleClass("Wt-invalid", true);

    return result;
  } else
    return WValidator::Valid;
}


int WLineEdit::boxPadding(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.agentIsIE() || env.agentIsOpera())
    return 1;
  else if (env.agent() == WEnvironment::Arora)
    return 0;
  else if (env.userAgent().find("Mac OS X") != std::string::npos)
    return 1;
  else if (env.userAgent().find("Windows") != std::string::npos
	   && !env.agentIsGecko())
    return 0;
  else
    return 1;
}

int WLineEdit::boxBorder(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.userAgent().find("Mac OS X") != std::string::npos
      && env.agentIsGecko())
    return 3;
  else if (env.agent() == WEnvironment::Arora)
    return 0;
  else
    return 2;
}

int WLineEdit::selectionStart() const
{
  WApplication *app = WApplication::instance();

  if (app->focus() == id()) {
    if (app->selectionStart() != -1
	&& app->selectionEnd() != app->selectionStart()) {
      return app->selectionStart();
    } else
      return -1;
  } else
    return -1;
}

WT_USTRING WLineEdit::selectedText() const
{
  if (selectionStart() != -1) {
    WApplication *app = WApplication::instance();

    return WString::fromUTF8(UTF8Substr(text().toUTF8(), app->selectionStart(),
		    app->selectionEnd() - app->selectionStart()));
  } else
    return WString::Empty;
}

bool WLineEdit::hasSelectedText() const
{
  return selectionStart() != -1;
}

int WLineEdit::cursorPosition() const
{
  WApplication *app = WApplication::instance();

  if (app->focus() == id())
    return app->selectionEnd();
  else
    return -1;
}

}
