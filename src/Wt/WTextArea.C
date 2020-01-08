/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStringUtil.h"
#include "Wt/WTextArea.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

const char *WTextArea::INPUT_SIGNAL = "input";

WTextArea::WTextArea()
  : cols_(20),
    rows_(5),
    contentChanged_(false),
    attributesChanged_(false)
{ 
  setInline(true);
  setFormObject(true);
}

WTextArea::WTextArea(const WT_USTRING& text)
  : content_(text),
    cols_(20),
    rows_(5),
    contentChanged_(false),
    attributesChanged_(false)
{ 
  setInline(true);
  setFormObject(true);
}

void WTextArea::setText(const WT_USTRING& text)
{
  content_ = text;
  contentChanged_ = true;
  repaint();

  validate();

  applyEmptyText();
}

void WTextArea::setColumns(int columns)
{
  cols_ = columns;
  attributesChanged_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WTextArea::setRows(int rows)
{
  rows_ = rows;
  attributesChanged_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WTextArea::resetContentChanged()
{
  contentChanged_ = false;
}

void WTextArea::updateDom(DomElement& element, bool all)
{
  if (element.type() == DomElementType::TEXTAREA)
    if (contentChanged_ || all) {
      element.setProperty(Property::Value, content_.toUTF8());
      contentChanged_ = false;
    }

  if (attributesChanged_ || all) {
    element.setAttribute("cols", std::to_string(cols_));
    element.setAttribute("rows", std::to_string(rows_));

    attributesChanged_ = false;
  }

  WFormWidget::updateDom(element, all);
}

void WTextArea::propagateRenderOk(bool deep)
{
  attributesChanged_ = false;
  contentChanged_ = false;
  
  WFormWidget::propagateRenderOk(deep);
}

DomElementType WTextArea::domElementType() const
{
  return DomElementType::TEXTAREA;
}

void WTextArea::setFormData(const FormData& formData)
{
  if (contentChanged_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    std::string value = formData.values[0];

    /*
     * IE puts \r\b for a newline, but then gets confused about this itself
     * when deriving the selection start/end
     */
    Utils::replace(value, '\r', "");
    content_ = WT_USTRING::fromUTF8(value, true);
  }
}

WT_USTRING WTextArea::valueText() const
{
  return text();
}

void WTextArea::setValueText(const WT_USTRING& value)
{
  setText(value);
}

int WTextArea::boxPadding(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.agentIsIE() || env.agentIsOpera())
    return 1;
  else if (env.agentIsChrome())
    return 2;
  else if (env.userAgent().find("Mac OS X") != std::string::npos)
    return 0;
  else if (env.userAgent().find("Windows") != std::string::npos)
    return 0;
  else
    return 1;
}

int WTextArea::boxBorder(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.agentIsIE() || env.agentIsOpera())
    return 2;
  else if (env.agentIsChrome())
    return 1;
  else if (env.userAgent().find("Mac OS X") != std::string::npos)
    return 1;
  else if (env.userAgent().find("Windows") != std::string::npos)
    return 2;
  else
    return 2;
}

int WTextArea::selectionStart() const
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

WT_USTRING WTextArea::selectedText() const
{
  if (selectionStart() != -1) {
    WApplication *app = WApplication::instance();

    std::string result = UTF8Substr(text().toUTF8(), app->selectionStart(),
				    app->selectionEnd() - app->selectionStart());
#ifdef WT_TARGET_JAVA
    return result;
#else
    return WString::fromUTF8(result);
#endif
  } else
    return WString::Empty;
}

bool WTextArea::hasSelectedText() const
{
  return selectionStart() != -1;
}

int WTextArea::cursorPosition() const
{
  WApplication *app = WApplication::instance();

  if (app->focus() == id())
    return app->selectionEnd();
  else
    return -1;
}

EventSignal<>& WTextArea::textInput()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

}
