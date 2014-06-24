/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cctype>
#include <exception>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WLogger"
#include "Wt/WTemplate"

#include "EscapeOStream.h"
#include "WebUtils.h"
#include "DomElement.h"
#include "RefEncoder.h"
#include "WebSession.h"

namespace Wt {
LOGGER("WTemplate");

bool WTemplate::_tr(const std::vector<WString>& args,
		    std::ostream& result)
{
  if (args.size() >= 1) {
    WString s = WString::tr(args[0].toUTF8());
    for (unsigned j = 1; j < args.size(); ++j)
      s.arg(args[j]);
    result << s.toUTF8(); // FIXME formatting / escaping ?
    return true;
  } else {
    LOG_ERROR("Functions::tr(): expects at least one argument");
    return false;
  }
}

bool WTemplate::_block(const std::vector<WString>& args,
                              std::ostream& result)
{
  if (args.size() < 1)
    return false;

  WString tblock = WString::tr(args[0].toUTF8());
  for (unsigned i = 1; i < args.size(); ++i)
    tblock.arg(args[i]);

  this->renderTemplateText(result, tblock);

  return true;
}

bool WTemplate::_while(const std::vector<WString>& args,
			      std::ostream& result)
{
  if (args.size() < 2)
    return false;

  WString tblock = WString::tr(args[1].toUTF8());
  while (conditionValue(args[0].toUTF8()))
    this->renderTemplateText(result, tblock);

  return true;
}

bool WTemplate::_id(const std::vector<WString>& args,
		    std::ostream& result)
{
  if (args.size() == 1) {
    WWidget *w = this->resolveWidget(args[0].toUTF8());
    if (w) {
      result << w->id();
      return true;
    } else
      return false;
  } else {
    LOG_ERROR("Functions::tr(): expects exactly one argument");
    return false;
  }
}

#ifndef WT_TARGET_JAVA
bool WTemplate::Functions::tr(WTemplate *t, const std::vector<WString>& args,
			      std::ostream& result)
{
  return t->_tr(args, result);
}

bool WTemplate::Functions::block(WTemplate *t, const std::vector<WString>& args,
                              std::ostream& result)
{
  return t->_block(args, result);
}

bool WTemplate::Functions::while_f(WTemplate *t, const std::vector<WString>& args,
			      std::ostream& result)
{
  return t->_while(args, result);
}

bool WTemplate::Functions::id(WTemplate *t, const std::vector<WString>& args,
			      std::ostream& result)
{
  return t->_id(args, result);
}

#else

bool WTemplate::TrFunction::evaluate(WTemplate *t, 
				     const std::vector<WString>& args,
				     std::ostream& result) const
{
  try {
    return t->_tr(args, result);
  } catch (std::io_exception ioe) {
    return false;
  } 
}

bool WTemplate::BlockFunction::evaluate(WTemplate *t,
                                     const std::vector<WString>& args,
                                     std::ostream& result) const
{
  try {
    return t->_block(args, result);
  } catch (std::io_exception ioe) {
    return false;
  }
}

bool WTemplate::WhileFunction::evaluate(WTemplate *t,
				    const std::vector<WString>& args,
				    std::ostream& result) const
{
  try {
    return t->_while(args, result);
  } catch (std::io_exception ioe) {
    return false;
  }
}

bool WTemplate::IdFunction::evaluate(WTemplate *t, 
				     const std::vector<WString>& args,
				     std::ostream& result) const
{
  try {
  return t->_id(args, result);
  } catch (std::io_exception ioe) {
    return false;
  }
}

#endif

WTemplate::WTemplate(WContainerWidget *parent)
  : WInteractWidget(parent),
    previouslyRendered_(0),
    newlyRendered_(0),
    encodeInternalPaths_(false),
    changed_(false)
{
  plainTextNewLineEscStream_ = new EscapeOStream();
  plainTextNewLineEscStream_->pushEscape(EscapeOStream::PlainTextNewLines);
  setInline(false);
}

WTemplate::WTemplate(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    previouslyRendered_(0),
    newlyRendered_(0),
    encodeInternalPaths_(false),
    changed_(false)
{
  plainTextNewLineEscStream_ = new EscapeOStream();
  plainTextNewLineEscStream_->pushEscape(EscapeOStream::PlainTextNewLines);
  setInline(false);
  setTemplateText(text);
}

WTemplate::~WTemplate()
{
  delete plainTextNewLineEscStream_;
}

void WTemplate::clear()
{
  setIgnoreChildRemoves(true);
  /*
   * We need to copy them first so that removeChild() will not be confused
   * by it.
   */
  WidgetMap toDelete = widgets_;
  widgets_ = WidgetMap();
  for (WidgetMap::iterator i = toDelete.begin(); i != toDelete.end(); ++i)
    delete i->second;
  setIgnoreChildRemoves(false);

  strings_.clear();
  conditions_.clear();

  changed_ = true;
  repaint(RepaintSizeAffected);  
}

#ifndef WT_TARGET_JAVA
void WTemplate::addFunction(const std::string& name, const Function& function)
{
  functions_[name] = function;
}
#else
void WTemplate::addFunction(const std::string& name, const Function *function)
{
  functions_[name] = *function;
}
#endif

void WTemplate::setCondition(const std::string& name, bool value)
{
  if (conditionValue(name) != value) {
    if (value)
      conditions_.insert(name);
    else
      conditions_.erase(name);

    changed_ = true;
    repaint(RepaintSizeAffected);
  }
}

bool WTemplate::conditionValue(const std::string& name) const
{
  return conditions_.find(name) != conditions_.end();
}

void WTemplate::bindWidget(const std::string& varName, WWidget *widget)
{
  WidgetMap::iterator i = widgets_.find(varName);
  if (i != widgets_.end()) {
    if (i->second == widget)
      return;
    else {
      WWidget *toDelete = i->second;
#ifndef WT_TARGET_JAVA
      widgets_.erase(i);
#else
      widgets_.erase(varName);
#endif
      delete toDelete;
    }
  }

  if (widget) {
    widget->setParentWidget(this);
    widgets_[varName] = widget;
    strings_.erase(varName);
  } else {
    StringMap::const_iterator j = strings_.find(varName);
    if (j != strings_.end() && j->second.empty())
      return;
    strings_[varName] = std::string();
  }

  changed_ = true;
  repaint(RepaintSizeAffected);  
}

WWidget *WTemplate::takeWidget(const std::string& varName)
{
  WidgetMap::iterator i = widgets_.find(varName);

  if (i != widgets_.end()) {
    WWidget *result = i->second;
    result->setParentWidget(0);
    return result;
  } else
    return 0;
}

void WTemplate::bindEmpty(const std::string& varName)
{
  bindWidget(varName, 0);
}

void WTemplate::bindString(const std::string& varName, const WString& value,
			   TextFormat textFormat)
{
  WWidget *w = resolveWidget(varName);
  if (w)
    bindWidget(varName, 0);

  WString v = value;

  if (textFormat == XHTMLText && v.literal()) {
    if (!removeScript(v))
      v = escapeText(v, true);
  } else if (textFormat == PlainText)
    v = escapeText(v, true);

  StringMap::const_iterator i = strings_.find(varName);

  if (i == strings_.end() || i->second != v.toUTF8()) {
    strings_[varName] = v.toUTF8();

    changed_ = true;
    repaint(RepaintSizeAffected);  
  }
}

void WTemplate::bindInt(const std::string& varName, int value)
{
  bindString(varName, boost::lexical_cast<std::string>(value), XHTMLUnsafeText);
}

bool WTemplate::resolveFunction(const std::string& name,
				const std::vector<WString>& args,
				std::ostream& result)
{
  FunctionMap::const_iterator i = functions_.find(name);

  if (i != functions_.end()) {
#ifndef WT_TARGET_JAVA
    bool ok = i->second(this, args, result);
#else
    bool ok = i->second.evaluate(this, args, result);
#endif // WT_TARGET_JAVA

    if (!ok)
      result << "??" << name << ":??";

    return true;
  }

  return false;
}

void WTemplate::resolveString(const std::string& varName,
			      const std::vector<WString>& args,
			      std::ostream& result)
{
  /*
   * FIXME: have an extra result parameter which indicates whether the
   * widget is view-only. Better to do that in resolveValue() and
   * provide a utility method that converts a widget to XHTML ?
   */

  StringMap::const_iterator i = strings_.find(varName);
  if (i != strings_.end())
    result << i->second;
  else {
    WWidget *w = resolveWidget(varName);
    if (w) {
      w->setParentWidget(this);

      if (previouslyRendered_
	  && previouslyRendered_->find(w) != previouslyRendered_->end()) {
	result << "<span id=\"" << w->id() << "\"> </span>";
      } else {
	applyArguments(w, args);
	w->htmlText(result);
      }

      if (newlyRendered_)
        newlyRendered_->push_back(w);
    } else
      handleUnresolvedVariable(varName, args, result);
  }
}

void WTemplate::applyArguments(WWidget *w, const std::vector<WString>& args)
{
  for (unsigned i = 0; i < args.size(); ++i) {
    std::string s = args[i].toUTF8();
    if (boost::starts_with(s, "class="))
      w->addStyleClass(WString::fromUTF8(s.substr(6)));
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

void WTemplate::setTemplateText(const WString& text, TextFormat textFormat)
{
  text_ = text;

  if (textFormat == XHTMLText && text_.literal()) {
    if (!removeScript(text_))
      text_ = escapeText(text_, true);
  } else if (textFormat == PlainText)
    text_ = escapeText(text_, true);

  changed_ = true;
  repaint(RepaintSizeAffected);
}

void WTemplate::updateDom(DomElement& element, bool all)
{
  if (changed_ || all) {
    std::set<WWidget *> previouslyRendered;
    std::vector<WWidget *> newlyRendered;

    for (WidgetMap::const_iterator i = widgets_.begin(); i != widgets_.end();
	 ++i) {
      WWidget *w = i->second;
      if (w->isRendered() && w->webWidget()->domCanBeSaved()) {
	previouslyRendered.insert(w);
      }
    }

    bool saveWidgets = element.mode() == DomElement::ModeUpdate;

    previouslyRendered_ = saveWidgets ? &previouslyRendered : 0;
    newlyRendered_ = &newlyRendered;

    std::stringstream html;
    renderTemplate(html);

    previouslyRendered_ = 0;
    newlyRendered_ = 0;

    for (unsigned i = 0; i < newlyRendered.size(); ++i) {
      WWidget *w = newlyRendered[i];
      if (previouslyRendered.find(w) != previouslyRendered.end()) {
	if (saveWidgets)
	  element.saveChild(w->id());
	previouslyRendered.erase(w);
      }
    }

    element.setProperty(Wt::PropertyInnerHTML, html.str());
    changed_ = false;

    for (std::set<WWidget *>::const_iterator i = previouslyRendered.begin();
	 i != previouslyRendered.end(); ++i) {
      WWidget *w = *i;
      // it could be that the widget was removed/deleted in the mean time
      // as a side-effect of rendering some of the widgets; thus we check
      // that the widget is still a child
      for (WidgetMap::const_iterator j = widgets_.begin();
	   j != widgets_.end(); ++j) {
	if (j->second == w) {
	  w->webWidget()->setRendered(false);
	  break;
	}
      }
    }

    WApplication::instance()->session()->renderer()
      .updateFormObjects(this, true);
  }

  WInteractWidget::updateDom(element, all);
}

void WTemplate::renderTemplate(std::ostream& result)
{
  renderTemplateText(result, templateText());
}

bool WTemplate::renderTemplateText(std::ostream& result, const WString& templateText)
{
  errorText_ = "";
  std::string text;

  WApplication *app = WApplication::instance();

  if (app && (encodeInternalPaths_ || app->session()->hasSessionIdInUrl())) {
    WFlags<RefEncoderOption> options;
    if (encodeInternalPaths_)
      options |= EncodeInternalPaths;
    if (app->session()->hasSessionIdInUrl())
      options |= EncodeRedirectTrampoline;
    WString t = templateText;
    EncodeRefs(t, options);
    text = t.toUTF8();
  } else
    text = templateText.toUTF8();

  std::size_t lastPos = 0;
  std::vector<WString> args;
  std::vector<std::string> conditions;
  int suppressing = 0;

  for (std::size_t pos = text.find('$'); pos != std::string::npos;
       pos = text.find('$', pos)) {

    if (!suppressing)
      result << text.substr(lastPos, pos - lastPos);

    lastPos = pos;

    if (pos + 1 < text.length()) {
      if (text[pos + 1] == '$') { // $$ -> $
	if (!suppressing)
	  result << '$';

	lastPos += 2;
      } else if (text[pos + 1] == '{') {
	std::size_t startName = pos + 2;
	std::size_t endName = text.find_first_of(" \r\n\t}", startName);

        args.clear();
        std::size_t endVar = parseArgs(text, endName, args);

        if (endVar == std::string::npos) {
          std::stringstream errorStream;
          errorStream << "variable syntax error near \"" << text.substr(pos)
                      << "\"";
          errorText_ = errorStream.str();
          LOG_ERROR(errorText_);
          return false;
        }

        std::string name = text.substr(startName, endName - startName);
	std::size_t nl = name.length();

	if (nl > 2 && name[0] == '<' && name[nl - 1] == '>') {
	  if (name[1] != '/') {
	    std::string cond = name.substr(1, nl - 2);
	    conditions.push_back(cond);
	    if (suppressing || !conditionValue(cond))
	      ++suppressing;
	  } else {
	    std::string cond = name.substr(2, nl - 3);
	    if (conditions.empty() || conditions.back() != cond) {
              std::stringstream errorStream;
              errorStream << "mismatching condition block end: " << cond;
              errorText_ = errorStream.str();
              LOG_ERROR(errorText_);
              return false;
	    }
	    conditions.pop_back();

	    if (suppressing)
	      --suppressing;
	  }
	} else {
	  if (!suppressing) {
	    std::size_t colonPos = name.find(':');

	    bool handled = false;
	    if (colonPos != std::string::npos) {
	      std::string fname = name.substr(0, colonPos);
	      std::string arg0 = name.substr(colonPos + 1);
	      args.insert(args.begin(), WString::fromUTF8(arg0));
	      if (resolveFunction(fname, args, result))
		handled = true;
	      else
		args.erase(args.begin());
	    }

	    if (!handled)
	      resolveString(name, args, result);
	  }
	}

	lastPos = endVar + 1;
      } else {
	if (!suppressing)
	  result << '$'; // $. -> $.
	lastPos += 1;
      }
    } else {
      if (!suppressing)
	result << '$'; // $ at end of template -> $
      lastPos += 1;
    }

    pos = lastPos;
  }

  result << text.substr(lastPos);
  return true;
}

std::size_t WTemplate::parseArgs(const std::string& text,
				 std::size_t pos,
				 std::vector<WString>& result)
{
  std::size_t Error = std::string::npos;

  if (pos == std::string::npos)
    return Error;

  enum { Next, Name, Value, SValue, DValue } state = Next;

  WStringStream v;

  for (; pos < text.length(); ++pos) {
    char c = text[pos];
    switch (state) {
    case Next:
      if (!std::isspace(c)) {
	if (c == '}')
	  return pos;
	else if (std::isalpha(c) || c == '_') {
	  state = Name;
	  v.clear();
	  v << c;
	} else if (c == '\'') {
	  state = SValue;
	  v.clear();
	} else if (c == '"') {
	  state = DValue;
	  v.clear();
	} else
	  return Error;
      }
      break;

    case Name:
      if (c == '=') {
	state = Value;
	v << '=';
      } else if (std::isspace(c)) {
	result.push_back(WString::fromUTF8(v.str()));
	state = Next;
      } else if (c == '}') {
	result.push_back(WString::fromUTF8(v.str()));
	return pos;
      } else if (std::isalnum(c) || c == '_' || c == '-' || c == '.')
	v << c;
      else
	return Error;
      break;

    case Value:
      if (c == '\'')
	state = SValue;
      else if (c == '"')
	state = DValue;
      else
	return Error;
      break;

    case SValue:
    case DValue:
      char quote = state == SValue ? '\'' : '"';

      std::size_t end = text.find(quote, pos);
      if (end == std::string::npos)
	return Error;
      if (text[end - 1] == '\\')
	v << text.substr(pos, end - pos - 1) << quote;
      else {
	v << text.substr(pos, end - pos);
	result.push_back(WString::fromUTF8(v.str()));
	state = Next;
      }

      pos = end;
    }
  }

  return pos == text.length() ? std::string::npos : pos;
}

void WTemplate::format(std::ostream& result, const std::string& s,
		       TextFormat textFormat)
{
  format(result, WString::fromUTF8(s), textFormat);
}

void WTemplate::format(std::ostream& result, const WString& s,
		       TextFormat textFormat)
{
  if (textFormat == XHTMLText) {
    WString v = s;
    if (removeScript(v)) {
      result << v.toUTF8();
      return;
    } else {
      EscapeOStream sout(result);
      sout.append(v.toUTF8(), *plainTextNewLineEscStream_);
      return;
    }
  } else if (textFormat == PlainText) {
    EscapeOStream sout(result);
    sout.append(s.toUTF8(), *plainTextNewLineEscStream_);
    return;
  }

  result << s.toUTF8();
}

void WTemplate::removeChild(WWidget *child)
{
  for (WidgetMap::iterator i = widgets_.begin(); i != widgets_.end(); ++i) {
    if (i->second == child) {
      Utils::eraseAndNext(widgets_, i);

      changed_ = true;
      repaint(RepaintSizeAffected);

      break;
    }
  }

  WInteractWidget::removeChild(child);
}

void WTemplate::propagateRenderOk(bool deep)
{
  changed_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

void WTemplate::enableAjax()
{
  WInteractWidget::enableAjax();
}

DomElementType WTemplate::domElementType() const
{
  DomElementType type = isInline() ? DomElement_SPAN : DomElement_DIV;

  WContainerWidget *p = dynamic_cast<WContainerWidget *>(parentWebWidget());
  if (p && p->isList())
    type = DomElement_LI;

  return type;
}

void WTemplate::setInternalPathEncoding(bool enabled)
{
  if (encodeInternalPaths_ != enabled) {
    encodeInternalPaths_ = enabled;
    changed_ = true;
    repaint();
  }
}

void WTemplate::refresh()
{
  if (text_.refresh()) {
    changed_ = true;
    repaint(RepaintSizeAffected);
  }

  WInteractWidget::refresh();
}

}
