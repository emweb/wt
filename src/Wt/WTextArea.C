/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WTextArea"

#include "DomElement.h"
#include "CgiParser.h"

namespace Wt {

WTextArea::WTextArea(WContainerWidget *parent)
  : WFormWidget(parent),
    cols_(20),
    rows_(5),
    contentChanged_(false),
    colsRowsChanged_(false)
{ 
  setInline(true);
  setFormObject(true);
}

WTextArea::WTextArea(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    content_(text),
    cols_(20),
    rows_(5),
    contentChanged_(false),
    colsRowsChanged_(false)
{ 
  setInline(true);
  setFormObject(true);
}

void WTextArea::setText(const WString& text)
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
  colsRowsChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WTextArea::setRows(int rows)
{
  rows_ = rows;
  colsRowsChanged_ = true;
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

  if (colsRowsChanged_ || all) {
    element.setAttribute("cols",
			 boost::lexical_cast<std::string>(cols_));
    element.setAttribute("rows",
			 boost::lexical_cast<std::string>(rows_));
    colsRowsChanged_ = false;
  }

  WFormWidget::updateDom(element, all);
}

DomElementType WTextArea::domElementType() const
{
  return DomElement_TEXTAREA;
}

void WTextArea::setFormData(CgiEntry *entry)
{
  content_ = WString(entry->value(), UTF8);
}

WValidator::State WTextArea::validate()
{
  if (validator()) {
    int pos;

    return validator()->validate(content_, pos);
  } else
    return WValidator::Valid;
}

}
