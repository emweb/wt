/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <iostream>

#include "Wt/WTemplate"
#include "DomElement.h"

namespace Wt {

WTemplate::WTemplate(WContainerWidget *parent)
  : WInteractWidget(parent),
    changed_(false)
{
  setInline(false);
}

WTemplate::WTemplate(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    changed_(false)
{
  setInline(false);
  setTemplateText(text);
}

void WTemplate::clear()
{
  setIgnoreChildRemoves(true);
  for (WidgetMap::iterator i = widgets_.begin(); i != widgets_.end(); ++i)
    delete i->second;
  setIgnoreChildRemoves(false);

  widgets_.clear();
  strings_.clear();
}

void WTemplate::bindWidget(const std::string& varName, WWidget *widget)
{
  WidgetMap::iterator i = widgets_.find(varName);
  if (i != widgets_.end())
    delete i->second;

  if (widget)
    widget->setParent(this);
  widgets_[varName] = widget;

  changed_ = true;
  repaint(RepaintInnerHtml);  
}

void WTemplate::bindString(const std::string& varName, const WString& value,
			   TextFormat textFormat)
{
  WString v = value;

  if (textFormat == XHTMLText && text_.literal()) {
    if (!removeScript(v))
      v = escapeText(v, true);
  } else if (textFormat == PlainText)
    v = escapeText(v, true);

  strings_[varName] = v.toUTF8();
}

void WTemplate::bindInt(const std::string& varName, int value)
{
  strings_[varName] = boost::lexical_cast<std::string>(value);
}

void WTemplate::resolveString(const std::string& varName,
			      const std::vector<WString>& args,
			      std::ostream& result)
{
  /*
   * FIXME: have an extra result parameter which indicates whether the
   * widget is view-only. Better to do that in resolveValue() and
   * provide a utility method that converst a widget to XHTML ?
   */

  StringMap::const_iterator i = strings_.find(varName);
  if (i != strings_.end())
    result << i->second;
  else {
    WWidget *w = resolveWidget(varName);
    if (w)
      w->htmlText(result);
    else
      handleUnresolvedVariable(varName, args, result);
  }
}

void WTemplate::handleUnresolvedVariable(const std::string& varName,
                                         const std::vector<WString>& args,
                                         std::ostream& result)
{
  result << "??" << varName << "??";
}

WWidget *WTemplate::resolveWidget(const std::string& varName)
{
  WidgetMap::const_iterator j = widgets_.find(varName);
  if (j != widgets_.end())
    return j->second;
  else
    return 0;
}

void WTemplate::setTemplateText(const WString& text)
{
  text_ = text;

  if (text_.literal())
    if (!removeScript(text_))
      text_ = escapeText(text_, true);

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WTemplate::updateDom(DomElement& element, bool all)
{
  if (changed_ || all) {
    std::stringstream resolved;
    
    renderTemplate(resolved);

    element.setProperty(Wt::PropertyInnerHTML, resolved.str());
    changed_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

void WTemplate::renderTemplate(std::ostream& result)
{
  std::string text = text_.toUTF8();

  std::size_t lastPos = 0;
  for (std::size_t pos = text.find('$'); pos != std::string::npos;
       pos = text.find('$', pos)) {

    result << text.substr(lastPos, pos - lastPos);
    lastPos = pos;

    if (pos + 1 < text.length()) {
      if (text[pos + 1] == '$') { // $$ -> $
	result << '$';
	lastPos += 2;
      } else if (text[pos + 1] == '{') {
	std::size_t startName = pos + 2;
	std::size_t endName = text.find_first_of(" \r\n\t}", startName);
	std::size_t endVar = text.find('}', endName);
	if (endName == std::string::npos || endVar == std::string::npos)
	  throw std::runtime_error("WTemplate syntax error at pos "
				   + boost::lexical_cast<std::string>(pos));

	std::string name = text.substr(startName, endName - startName);
	std::vector<WString> args;
	resolveString(name, args, result);

	lastPos = endVar + 1;
      } else {
	result << '$'; // $. -> $.
	lastPos += 1;
      }
    } else {
      result << '$'; // $ at end of template -> $
      lastPos += 1;
    }

    pos = lastPos;
  }

  result << text.substr(lastPos);
}

void WTemplate::format(std::ostream& result, const std::string& s,
		       TextFormat textFormat)
{
  format(result, WString::fromUTF8(s), textFormat);
}

void WTemplate::format(std::ostream& result, const WString& s,
		       TextFormat textFormat)
{
  WString v = s;

  if (textFormat == XHTMLText) {
    if (!removeScript(v))
      v = escapeText(v, true);
  } else if (textFormat == PlainText)
    v = escapeText(v, true);

  result << v.toUTF8();
}

void WTemplate::propagateRenderOk(bool deep)
{
  changed_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

DomElementType WTemplate::domElementType() const
{
  return isInline() ? DomElement_SPAN : DomElement_DIV;
}

void WTemplate::refresh()
{
  if (text_.refresh()) {
    changed_ = true;
    repaint(RepaintInnerHtml);
  }

  WInteractWidget::refresh();
}

}
