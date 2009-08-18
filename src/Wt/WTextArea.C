/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WTextArea"

#include "DomElement.h"

namespace Wt {

WTextArea::WTextArea(WContainerWidget *parent)
  : WFormWidget(parent),
    cols_(20),
    rows_(5),
    contentChanged_(false),
    attributesChanged_(false)
{ 
  setInline(true);
  setFormObject(true);
}

WTextArea::WTextArea(const WT_USTRING& text, WContainerWidget *parent)
  : WFormWidget(parent),
    content_(text),
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
  repaint(RepaintInnerHtml);

  if (validator())
    setStyleClass(validate() == WValidator::Valid ? "" : "Wt-invalid");
}

void WTextArea::setColumns(int columns)
{
  cols_ = columns;
  attributesChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WTextArea::setRows(int rows)
{
  rows_ = rows;
  attributesChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WTextArea::resetContentChanged()
{
  contentChanged_ = false;
}

void WTextArea::updateDom(DomElement& element, bool all)
{
  if (element.type() == DomElement_TEXTAREA)
    if (contentChanged_ || all) {
      if (all)
	element.setProperty(Wt::PropertyInnerHTML,
			    escapeText(content_).toUTF8());
      else
	element.setProperty(Wt::PropertyValue, content_.toUTF8());
      contentChanged_ = false;
    }

  if (attributesChanged_ || all) {
    element.setAttribute("cols",
			 boost::lexical_cast<std::string>(cols_));
    element.setAttribute("rows",
			 boost::lexical_cast<std::string>(rows_));

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
  return DomElement_TEXTAREA;
}

void WTextArea::setFormData(const FormData& formData)
{
  if (contentChanged_)
    return;

  if (!formData.values.empty()) {
    const std::string& value = formData.values[0];
    content_ = WT_USTRING::fromUTF8(value);
  }
}

WValidator::State WTextArea::validate()
{
  if (validator()) {
    return validator()->validate(content_);
  } else
    return WValidator::Valid;
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


}
