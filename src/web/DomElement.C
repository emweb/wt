/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <stdio.h>
#include <sstream>

#include "Wt/WObject"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WebSession.h"
#include "WtException.h"
#include "Utils.h"

namespace {

const char *elementNames_[] =
  { "a", "br", "button", "col",
    "div", "fieldset", "form",
    "h1", "h2", "h3", "h4",

    "h5", "h6", "iframe", "img",
    "input", "label", "legend", "li",
    "ol",

    "option", "ul", "script", "select",
    "span", "table", "tbody", "thead",
    "th", "td", "textarea",

    "tr", "p", "canvas",
    "map", "area"
  };

bool defaultInline_[] =
  { true, true, true, false,
    false, false, false,
    true, false, false, false,

    false, false, true, true,
    true, true, true, false,
    false,

    true, false, false, true,
    true, false, false, false,
    false, false, true,

    false, false, false,
    false, true
  };

  static const std::string unsafeChars_ = "$&+,:;=?@'\"<>#%{}|\\^~[]`";
}

namespace Wt {

int DomElement::nextId_ = 0;

DomElement *DomElement::createNew(DomElementType type)
{
  DomElement *e = new DomElement(ModeCreate, type);
  return e;
}

DomElement *DomElement::getForUpdate(const std::string& id,
				     DomElementType type)
{
  if (id.empty())
    throw WtException("Cannot update widget without id");

  DomElement *e = new DomElement(ModeUpdate, type);
  e->id_ = id;

  return e;
}

DomElement *DomElement::updateGiven(const std::string& var,
				    DomElementType type)
{
  DomElement *e = new DomElement(ModeUpdate, type);
  e->var_ = var;

  return e;
}

DomElement *DomElement::getForUpdate(const WObject *object,
				     DomElementType type)
{
  return getForUpdate(object->id(), type);
}

DomElement::DomElement(Mode mode, DomElementType type)
  : mode_(mode),
    wasEmpty_(mode_ == ModeCreate),
    deleted_(false),
    removeAllChildren_(false),
    minMaxSizeProperties_(false),
    unstubbed_(false),
    replaced_(0),
    insertBefore_(0),
    type_(type),
    numManipulations_(0),
    timeOut_(-1),
    childrenHtml_(0),
    discardWithParent_(true)
{ }

DomElement::~DomElement()
{
  for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
    delete childrenToAdd_[i].child;

  delete replaced_;
  delete insertBefore_;
  delete childrenHtml_;
}

std::string DomElement::urlEncodeS(const std::string& url)
{
  std::stringstream result;

  for (unsigned i = 0; i < url.length(); ++i) {
    char c = url[i];
    if (c < 31 || c >= 127 || unsafeChars_.find(c) != std::string::npos) {
      result << '%';
#ifndef WT_CNOR
      result << std::hex << (int)c;
#else
      result << Utils::toHexString(c);
#endif // WT_CNOR
    } else
      result.put(c);
  }

  return result.str();
}

void DomElement::setType(DomElementType type)
{
  type_ = type;
}

void DomElement::setWasEmpty(bool how)
{
  wasEmpty_ = how;
}

void DomElement::updateInnerHtmlOnly()
{
  mode_ = ModeUpdate;

  assert(deleted_ == false);
  assert(replaced_ == 0);
  assert(insertBefore_ == 0);

  attributes_.clear();
  eventHandlers_.clear();

  for (PropertyMap::iterator i = properties_.begin(); i != properties_.end();) {
    if (   i->first == PropertyInnerHTML
	|| i->first == PropertyText
	|| i->first == PropertyTarget)
      ++i;
    else
#ifndef WT_TARGET_JAVA
      properties_.erase(i++);
#else
      i.remove();
#endif
  }
}

void DomElement::addChild(DomElement *child)
{
  ++numManipulations_;

  if (wasEmpty_ && canWriteInnerHTML(WApplication::instance())) {
    if (!childrenHtml_)
      childrenHtml_ = new std::stringstream();

    std::stringstream js;
    EscapeOStream sout(*childrenHtml_);
    child->asHTML(sout, js, timeouts_);
    javaScript_ += js.str();

    delete child;
  } else {
    childrenToAdd_.push_back(ChildInsertion(-1, child));
  }
}

void DomElement::saveChild(const std::string& id)
{
  childrenToSave_.push_back(id);
}

void DomElement::setAttribute(const std::string& attribute,
			      const std::string& value)
{
  ++numManipulations_;
  attributes_[attribute] = value;
}

std::string DomElement::getAttribute(const std::string& attribute) const
{
  AttributeMap::const_iterator i = attributes_.find(attribute);
  if (i != attributes_.end())
    return i->second;
  else
    return std::string();
}

void DomElement::removeAttribute(const std::string& attribute)
{
  attributes_.erase(attribute);
}

void DomElement::setEventSignal(const char *eventName,
				const EventSignalBase& signal)
{
  setEvent(eventName, signal.javaScript(),
	   signal.encodeCmd(), signal.isExposedSignal());
}

void DomElement::setEvent(const char *eventName,
			  const std::string& jsCode,
			  const std::string& signalName,
			  bool isExposed)
{
  WApplication *app = WApplication::instance();

  bool anchorClick = type() == DomElement_A
    && eventName == WInteractWidget::CLICK_SIGNAL;

  bool nonEmpty = isExposed || anchorClick || !jsCode.empty();

  std::stringstream js;

  if (nonEmpty) {
    if (app->environment().agentIsIEMobile())
      js << "var e=window.event,";
    else
      js << "var e=event||window.event,";
    js << "o=this;";
  }

  if (anchorClick)
    js << "if(e.ctrlKey||e.metaKey)return true;else{";

  if (isExposed)
    js << app->javaScriptClass() << "._p_.update(o,'"
       << signalName << "',e,true);";

  js << jsCode;

  if (anchorClick)
    js << "}";

  ++numManipulations_;
  eventHandlers_[eventName] = EventHandler(js.str(), signalName);
}

void DomElement::setEvent(const char *eventName,
			  const std::string& jsCode)
{
  eventHandlers_[eventName] = EventHandler(jsCode, std::string());
}

void DomElement::setDiscardWithParent(bool discard)
{
  discardWithParent_ = discard;
}

DomElement::EventAction::EventAction(const std::string& aJsCondition,
				     const std::string& aJsCode,
				     const std::string& anUpdateCmd,
				     bool anExposed)
  : jsCondition(aJsCondition),
    jsCode(aJsCode),
    updateCmd(anUpdateCmd),
    exposed(anExposed)
{ }

void DomElement::setEvent(const char *eventName,
			  const std::vector<EventAction>& actions)
{
  std::string code;

  for (unsigned i = 0; i < actions.size(); ++i) {
    if (!actions[i].jsCondition.empty())
      code += "if(" + actions[i].jsCondition + "){";
    if (actions[i].exposed)
      code += WApplication::instance()->javaScriptClass()
	+ "._p_.update(this,'" + actions[i].updateCmd + "',e,true);";
    code += actions[i].jsCode;
    if (!actions[i].jsCondition.empty())
      code += "}";
  }

  setEvent(eventName, code, "");
}

void DomElement::processProperties(WApplication *app) const
{
  if (minMaxSizeProperties_
      && app->environment().agent() == WEnvironment::IE6) {
    DomElement *self = const_cast<DomElement *>(this); 

    PropertyMap::iterator w = self->properties_.find(PropertyStyleWidth);
    PropertyMap::iterator minw = self->properties_.find(PropertyStyleMinWidth);
    PropertyMap::iterator maxw = self->properties_.find(PropertyStyleMaxWidth);

    if (minw != self->properties_.end() || maxw != self->properties_.end()) {
      if (w == self->properties_.end()) {
	std::stringstream expr;
	expr << WT_CLASS ".IEwidth(this,";
	if (minw != self->properties_.end()) {
	  expr << '\'' << minw->second << '\'';
          self->properties_.erase(PropertyStyleMinWidth); // C++: could be minw
	} else
	  expr << "'0px'";
	expr << ',';
	if (maxw != self->properties_.end()) {
	  expr << '\''<< maxw->second << '\'';
	  self->properties_.erase(PropertyStyleMaxWidth); // C++: could be maxw
	} else
	  expr << "'100000px'";
	expr << ")";

	self->properties_.erase(PropertyStyleWidth);
	self->properties_[PropertyStyleWidthExpression] = expr.str();
      }
    }

    PropertyMap::iterator i = self->properties_.find(PropertyStyleMinHeight);

    if (i != self->properties_.end()) {
      self->properties_[PropertyStyleHeight] = i->second;
      self->properties_.erase(PropertyStyleMinHeight); // C++: could be i
    }
  }
}

void DomElement::processEvents(WApplication *app) const
{
  /*
   * when we have a mouseUp event, we also need a mouseDown event
   * to be able to compute dragDX/Y. But when we have a mouseDown event,
   * we need to capture everything after on mouse down.
   */

  DomElement *self = const_cast<DomElement *>(this);

  const char *S_mousedown = WInteractWidget::MOUSE_DOWN_SIGNAL;
  const char *S_mouseup = WInteractWidget::MOUSE_UP_SIGNAL;
  const char *S_keypress = WInteractWidget::KEYPRESS_SIGNAL;

  EventHandlerMap::const_iterator mouseup = eventHandlers_.find(S_mouseup);
  if (mouseup != eventHandlers_.end() && !mouseup->second.jsCode.empty())
    Utils::access(self->eventHandlers_, S_mousedown).jsCode
      = app->javaScriptClass() + "._p_.saveDownPos(event);"
      + Utils::access(self->eventHandlers_, S_mousedown).jsCode;

  EventHandlerMap::const_iterator mousedown = eventHandlers_.find(S_mousedown);
  if (mousedown != eventHandlers_.end() && !mousedown->second.jsCode.empty())
    Utils::access(self->eventHandlers_, S_mousedown).jsCode
      = app->javaScriptClass() + "._p_.capture(this);"
      + Utils::access(self->eventHandlers_, S_mousedown).jsCode;

  EventHandlerMap::const_iterator keypress = eventHandlers_.find(S_keypress);
  if (keypress != eventHandlers_.end() && !keypress->second.jsCode.empty())
    Utils::access(self->eventHandlers_, S_keypress).jsCode
      = "if (" WT_CLASS ".isKeyPress(event)){"
      + Utils::access(self->eventHandlers_, S_keypress).jsCode
      + '}';
}

void DomElement::setTimeout(int msec, bool jsRepeat)
{
  ++numManipulations_;
  timeOut_ = msec;
  timeOutJSRepeat_ = jsRepeat;
}

void DomElement::callJavaScript(const std::string& jsCode,
				bool evenWhenDeleted)
{
  ++numManipulations_;
  if (!evenWhenDeleted)
    javaScript_ += jsCode;
  else
    javaScriptEvenWhenDeleted_ += jsCode;
}

void DomElement::setProperty(Property property, const std::string& value)
{
  ++numManipulations_;
  properties_[property] = value;

  if (property >= PropertyStyleMinWidth && property <= PropertyStyleMaxHeight)
    minMaxSizeProperties_ = true;
}

std::string DomElement::getProperty(Property property) const
{
  PropertyMap::const_iterator i = properties_.find(property);
  
  if (i != properties_.end())
    return i->second;
  else
    return std::string();
}

void DomElement::removeProperty(Property property)
{
  properties_.erase(property);
}

void DomElement::setId(const std::string& id)
{
  ++numManipulations_;
  id_ = id;
}

void DomElement::setName(const std::string& name)
{
  ++numManipulations_;
  id_ = name;
  setAttribute("name", name);
}

void DomElement::insertChildAt(DomElement *child, int pos)
{
  ++numManipulations_;

  childrenToAdd_.push_back(ChildInsertion(pos, child));
}

void DomElement::insertBefore(DomElement *sibling)
{
  ++numManipulations_;
  insertBefore_ = sibling;
}

void DomElement::removeFromParent()
{
  ++numManipulations_;
  deleted_ = true;
}

void DomElement::removeAllChildren()
{
  ++numManipulations_;
  removeAllChildren_ = true;
  wasEmpty_ = true;
}

void DomElement::replaceWith(DomElement *newElement)
{
  ++numManipulations_;
  replaced_ = newElement;
}

void DomElement::unstubWith(DomElement *newElement, bool hideWithDisplay)
{
  replaceWith(newElement);
  unstubbed_ = true;
  hideWithDisplay_ = hideWithDisplay;
}

void DomElement::callMethod(const std::string& method)
{
  ++numManipulations_;
  methodCalls_.push_back(method);
}

void DomElement::jsStringLiteral(std::ostream& out, const std::string& s,
				 char delimiter)
{
  EscapeOStream sout(out);
  jsStringLiteral(sout, s, delimiter);
}

void DomElement::htmlAttributeValue(std::ostream& out, const std::string& s)
{
  EscapeOStream sout(out);
  sout.pushEscape(EscapeOStream::HtmlAttribute);
  sout << s;
}

void DomElement::fastJsStringLiteral(EscapeOStream& outRaw,
				     const EscapeOStream& outEscaped,
				     const std::string& s)
{
  outRaw << '\'';
  outRaw.append(s, outEscaped);
  outRaw << '\'';
}

void DomElement::jsStringLiteral(EscapeOStream& out, const std::string& s,
				 char delimiter)
{
  out << delimiter;

  out.pushEscape(delimiter == '\'' ?
		 EscapeOStream::JsStringLiteralSQuote :
		 EscapeOStream::JsStringLiteralDQuote);
  out << s;
  out.popEscape();

  out << delimiter;
}

void DomElement::fastHtmlAttributeValue(EscapeOStream& outRaw,
					const EscapeOStream& outEscaped,
					const std::string& s)
{
  outRaw << '"';
  outRaw.append(s, outEscaped);
  outRaw << '"';
}

std::string DomElement::cssStyle() const
{
  if (properties_.empty())
    return std::string();

  std::stringstream style;
  const std::string *styleProperty = 0;

  for (PropertyMap::const_iterator j = properties_.begin();
       j != properties_.end(); ++j) {
    if (j->first == PropertyStyle)
      styleProperty = &j->second;
    else if ((j->first >= PropertyStylePosition)
	&& (j->first <= PropertyStyleDisplay)) {
      static std::string cssNames[] =
	{ "position",
	  "z-index", "float", "clear",
	  "width", "height", "line-height",
	  "min-width", "min-height",
	  "max-width", "max-height",
	  "left", "right", "top", "bottom",
	  "vertical-align", "text-align",
	  "padding",
	  "margin-top", "margin-right",
	  "margin-bottom", "margin-left", "cursor",
	  "border-top", "border-right",
	  "border-bottom", "border-left",
	  "color", "overflow", "overflow", // overflox-x/y not well supported
	  "font-family", "font-style", "font-variant",
	  "font-weight", "font-size",
	  "background-color", "background-image", "background-repeat",
	  "background-attachment", "background-position",
	  "text-decoration", "white-space", "table-layout", "border-spacing",
	  "visibility", "display"};

      if ((j->first == PropertyStyleCursor) && (j->second == "pointer")) {
	style << "cursor:pointer;cursor:hand;";	    
      } else {
	style << cssNames[j->first - PropertyStylePosition]
	      << ':' << j->second << ';';
      }
    } else if (j->first == PropertyStyleWidthExpression) {
      style << "width:expression(" << j->second << ");";
    }
  }

  if (styleProperty)
    style << *styleProperty;

  return style.str();
}

void DomElement::setJavaScriptEvent(EscapeOStream& out,
				    const char *eventName,
				    const EventHandler& handler,
				    WApplication *app) const
{
  // events on the dom root container are events received by the whole
  // document when no element has focus
  bool globalUnfocused = (id_ == app->domRoot()->id());
  std::string extra1, extra2;

  if (globalUnfocused) {
    extra1 = 
      "var g = event||window.event; "
      "var t = g.target||g.srcElement;"
      "if ((!t||" WT_CLASS ".hasTag(t,'DIV') "
      ""     "||" WT_CLASS ".hasTag(t,'HTML'))) { "; 
    extra2 =
      "}";
  }

  int fid = nextId_++;

  out << "function f" << fid
      << "(event){ " << extra1 << handler.jsCode << extra2 << "}\n";

  if (globalUnfocused)
    out << "document";
  else {
    declare(out);
    out << var_;
  }

  out << ".on" << eventName << "=f" << fid << ";\n";
}

void DomElement::asHTML(EscapeOStream& out,
			std::ostream& javaScript,
			std::vector<TimeoutEvent>& timeouts,
			bool openingTagOnly) const
{
  if (mode_ != ModeCreate)
    throw WtException("DomElement::asHTML() called with ModeUpdate");

  WApplication *app = WApplication::instance();
  processEvents(app);
  processProperties(app);

  EventHandlerMap::const_iterator clickEvent
    = eventHandlers_.find(WInteractWidget::CLICK_SIGNAL);

  bool needButtonWrap
    = (!(app->environment().ajax())
       && (clickEvent != eventHandlers_.end())
       && (!clickEvent->second.jsCode.empty())
       && (!app->environment().agentIsSpiderBot()));

  bool isSubmit = needButtonWrap;
  DomElementType renderedType = type_;

  if (needButtonWrap) {
    if (type_ == DomElement_BUTTON) {
      /*
       * We don't need to wrap a button: we can just modify the attributes
       * type name and value. This avoid layout problems.
       *
       * Note that IE posts the button text instead of the value. We fix
       * this by encoding the value into the name.
       */
      DomElement *self = const_cast<DomElement *>(this);
      self->setAttribute("type", "submit");
      self->setAttribute("name", "signal=" + clickEvent->second.signalName);

      needButtonWrap = false;
    } else if (type_ == DomElement_IMG) {
      /*
       * We don't need to wrap an image: we can substitute it for an input
       * type image. This avoid layout problems.
       */
      renderedType = DomElement_INPUT;

      DomElement *self = const_cast<DomElement *>(this);
      self->setAttribute("type", "image");
      self->setAttribute("name", "signal=" + clickEvent->second.signalName);
      needButtonWrap = false;
    }
  }

  if (needButtonWrap) {
    PropertyMap::const_iterator i = properties_.find(PropertyStyleDisplay);
    if (i != properties_.end() && i->second == "none")
      return;
  }

  /*
   * We also should not wrap anchors, map area elements and form elements.
   */
  if (needButtonWrap) {
    if (   type_ == DomElement_AREA
	|| type_ == DomElement_INPUT
	|| type_ == DomElement_SELECT)
      needButtonWrap = false;

    if (type_ == DomElement_A) {
      std::string href = getAttribute("href");
      if (app->environment().agentIsIE() || href != "#")
	needButtonWrap = false;
    }
  }

  const bool isIEMobile = app->environment().agentIsIEMobile();
  const bool supportButton = !isIEMobile;

  bool needAnchorWrap = false;

  if (!needButtonWrap) {
    if (isIEMobile && app->environment().ajax()
	&& (clickEvent != eventHandlers_.end())
	&& (!clickEvent->second.jsCode.empty())
	&& (   type_ == DomElement_IMG
	    || type_ == DomElement_SPAN
	    || type_ == DomElement_DIV))
      needAnchorWrap = true;
  }

  if (!supportButton && type_ == DomElement_BUTTON) {
    renderedType = DomElement_INPUT;

    DomElement *self = const_cast<DomElement *>(this);
    if (!isSubmit)
      self->setAttribute("type", "button");
    self->setAttribute("value",
		       properties_.find(PropertyInnerHTML)->second);
    self->setProperty(PropertyInnerHTML, "");
  }

#ifndef WT_TARGET_JAVA
  EscapeOStream attributeValues(out);
#else // WT_TARGET_JAVA
  EscapeOStream attributeValues = out.push();
#endif // WT_TARGET_JAVA
  attributeValues.pushEscape(EscapeOStream::HtmlAttribute);

  std::string style;

  if (needButtonWrap) {
    if (supportButton) {
      out << "<button type=\"submit\" name=\"signal\" value=";
      fastHtmlAttributeValue(out, attributeValues,
			     clickEvent->second.signalName);
      out << " class=\"Wt-wrap ";

      PropertyMap::const_iterator l = properties_.find(PropertyClass);
      if (l != properties_.end())
	out << l->second;

      out << "\"";

      PropertyMap::const_iterator i = properties_.find(PropertyDisabled);
      if ((i != properties_.end()) && (i->second=="true"))
	out << " disabled=\"disabled\"";

      if (app->environment().agent() != WEnvironment::Konqueror
	  && !app->environment().agentIsWebKit())
	style = "margin: -1px -3px -2px -3px;";

      out << "><" << elementNames_[renderedType];
    } else {
      if (type_ == DomElement_IMG)
	out << "<input type=\"image\"";
      else
	out << "<input type=\"submit\"";

      out << " name=";
      fastHtmlAttributeValue(out, attributeValues,
			     "signal=" + clickEvent->second.signalName);
      out << " value=";

      PropertyMap::const_iterator i = properties_.find(PropertyInnerHTML);
      if (i != properties_.end())
	fastHtmlAttributeValue(out, attributeValues, i->second);
      else
	out << "\"\"";
    }
  } else if (needAnchorWrap) {
    out << "<a href=\"#\" class=\"Wt-wrap\" onclick=";
    fastHtmlAttributeValue(out, attributeValues, clickEvent->second.jsCode);
    out << "><" << elementNames_[renderedType];
  } else
    out << "<" << elementNames_[renderedType];

  if (!id_.empty()) {
    out << " id=";
    fastHtmlAttributeValue(out, attributeValues, id_);
  }

  for (AttributeMap::const_iterator i = attributes_.begin();
       i != attributes_.end(); ++i)
    if (!app->environment().agentIsSpiderBot() || i->first != "name") {
      out << " " << i->first << "=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
    }

  if (app->environment().ajax()) {
    for (EventHandlerMap::const_iterator i = eventHandlers_.begin();
	 i != eventHandlers_.end(); ++i) {
      if (!i->second.jsCode.empty()) {
	if (id_ == app->domRoot()->id()) {
	  EscapeOStream eos(javaScript);
	  setJavaScriptEvent(eos, i->first, i->second, app);
	} else {
	  out << " on" << i->first << "=";
	  fastHtmlAttributeValue(out, attributeValues, i->second.jsCode);
	}
      }
    }
  }

  std::string innerHTML = "";

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    switch (i->first) {
    case PropertyText:
    case PropertyInnerHTML:
      innerHTML += i->second; break;
    case PropertyScript:
      innerHTML += "/*<![CDATA[*/\n" + i->second + "\n/* ]]> */"; break;
    case PropertyDisabled:
      if (i->second == "true")
	// following is not XHTML
	// out << " disabled"; 
	// following is not interpreted correctly by all HTML renderers
	//  (like konqueror)
	out << " disabled=\"disabled\"";
      break;
    case PropertyReadOnly:
      if (i->second == "true")
	out << " readonly=\"readonly\"";
      break;
    case PropertyChecked:
      if (i->second == "true")
	// out << " checked";
	out << " checked=\"checked\"";
      break;
    case PropertySelected:
      if (i->second == "true")
	// out << " selected";
	out << " selected=\"selected\"";
      break;
    case PropertyMultiple:
      if (i->second == "true")
	// out << " selected";
	out << " multiple=\"multiple\"";
      break;
    case PropertyTarget:
      out << " target=\"" << i->second << "\"";
      break;
    case PropertyIndeterminate:
      if (i->second == "true") {
	DomElement *self = const_cast<DomElement *>(this);
	self->methodCalls_.push_back("indeterminate=" + i->second);
      }
      break;
    case PropertyValue:
      out << " value=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case PropertySrc:
      out << " src=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case PropertyColSpan:
      out << " colspan=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case PropertyRowSpan:
      out << " rowspan=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case PropertyClass:
      out << " class=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    default:
      break;
    }
  }

  style += cssStyle();

  if (!style.empty()) {
    out << " style=";
    fastHtmlAttributeValue(out, attributeValues, style);
  }

  if (needButtonWrap && !supportButton)
    out << " />";
  else {
    if (openingTagOnly) {
      out << ">";
      if (!innerHTML.empty()) {
	DomElement *self = const_cast<DomElement *>(this);
	if (!self->childrenHtml_)
	  self->childrenHtml_ = new std::stringstream();
	*self->childrenHtml_ << innerHTML;
      }
      return;
    }

    /*
     * http://www.w3.org/TR/html/#guidelines
     * XHTML recommendation, back-wards compatibility with HTML: C.2, C.3:
     * do not use minimized forms when content is empty like <p />, and use
     * minimized forms for certain elements like <br />
     */
    if (!isSelfClosingTag(renderedType)) {
      out << ">";
      for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	childrenToAdd_[i].child->asHTML(out, javaScript, timeouts);

      out << innerHTML; // for WPushButton must be after childrenToAdd_

      if (childrenHtml_)
	out << childrenHtml_->str();

      // IE6 will incorrectly set the height of empty divs
      if (renderedType == DomElement_DIV
	  && app->environment().agent() == WEnvironment::IE6
	  && innerHTML.empty()
	  && childrenToAdd_.empty()
	  && !childrenHtml_)
	out << "&nbsp;";

      out << "</" << elementNames_[renderedType] << ">";
    } else
      out << " />";

    if (needButtonWrap && supportButton)
      out << "</button>";
    else if (needAnchorWrap)
      out << "</a>";
  }

  javaScript << javaScriptEvenWhenDeleted_ << javaScript_;

  for (unsigned i = 0; i < methodCalls_.size(); ++i)
    javaScript << "$('#" << id_ << "').get(0)." << methodCalls_[i] << ';';

  if (timeOut_ != -1)
    timeouts.push_back(TimeoutEvent(timeOut_, id_, timeOutJSRepeat_));

  Utils::insert(timeouts, timeouts_);
}

std::string DomElement::createVar() const
{
#ifndef WT_TARGET_JAVA
  char buf[20];
  sprintf(buf, "j%d", nextId_++);
  var_ = buf;
#else // !WT_TARGET_JAVA
  var_ = "j" + boost::lexical_cast<std::string>(nextId_++);
#endif // !WT_TARGET_JAVA

  return var_;
}

void DomElement::declare(EscapeOStream& out) const
{
  if (var_.empty())
    out << "var " << createVar() << "=$('#" << id_ << "').get(0);\n";
}

bool DomElement::canWriteInnerHTML(WApplication *app) const
{
  if (app->environment().agentIsIEMobile())
    return true;

  /*
   * http://lists.apple.com/archives/web-dev/2004/Apr/msg00122.html
   * "The problem is not that innerHTML doesn't work (it works fine),
   *  but that Safari can't handle writing the innerHTML of a <tbody> tag.
   *  If you write the entire table (including <table> and <tbody>) in the
   *  innerHTML string it works fine.
   */
  /* http://msdn.microsoft.com/workshop/author/tables/buildtables.asp
   * Note When using Dynamic HTML (DHTML) to create a document, you can 
   * create objects and set the innerText or innerHTML property of the object.
   * However, because of the specific structure required by tables,
   * the innerText and innerHTML properties of the table and tr objects are
   * read-only.
   */
  /* http://support.microsoft.com/kb/276228
   * BUG: Internet Explorer Fails to Set the innerHTML Property of the
   * Select Object. Seems to affect at least up to IE6.0
   */
  if ((app->environment().agentIsIE()
       || app->environment().agent() == WEnvironment::Konqueror)
      && (   type_ == DomElement_TBODY
	  || type_ == DomElement_THEAD
	  || type_ == DomElement_TABLE
	  || type_ == DomElement_TR
	  || type_ == DomElement_SELECT
	  || type_ == DomElement_TD))
    return false;

  return true;
}

#if 0
bool DomElement::containsElement(DomElementType type) const
{
  for (unsigned i = 0; i < childrenToAdd_.size(); ++i) {
    if (childrenToAdd_[i].child->type_ == type)
      return true;
    if (childrenToAdd_[i].child->containsElement(type))
      return true;
  }

  return false;
}
#endif

void DomElement::asJavaScript(std::ostream& out)
{
  mode_ = ModeUpdate;

  EscapeOStream eout(out);

  declare(eout);
  eout << var_ << ".setAttribute('id', '" << id_ << "');\n";

  mode_ = ModeCreate;

  setJavaScriptProperties(eout, WApplication::instance());
  setJavaScriptAttributes(eout);
  asJavaScript(eout, Update);
}

void DomElement::createTimeoutJs(std::ostream& out, const TimeoutList& timeouts,
				 WApplication *app)
{
  for (unsigned i = 0; i < timeouts.size(); ++i)
    out << app->javaScriptClass()
	<< "._p_.addTimerEvent('" << timeouts[i].event << "', " 
	<< timeouts[i].msec << ","
	<< (timeouts[i].repeat ? "true" : "false") << ");\n";
}

void DomElement::createElement(std::ostream& out, WApplication *app,
			       const std::string& domInsertJS)
{
  EscapeOStream sout(out);
  createElement(sout, app, domInsertJS);
}

void DomElement::createElement(EscapeOStream& out, WApplication *app,
			       const std::string& domInsertJS)
{
  if (var_.empty())
    createVar();

  out << "var " << var_ << "=";

  if (app->environment().agentIsIE()) {
    /*
     * IE can do create the entire opening tag at once.
     * This rocks because it results in fewer JavaScript statements.
     * It also avoids problems with changing certain attributes not
     * working in IE.
     */
    std::stringstream dummy;
    out << "document.createElement('";
    out.pushEscape(EscapeOStream::JsStringLiteralSQuote);
    TimeoutList timeouts;
    asHTML(out, dummy, timeouts, true);
    out.popEscape();
    out << "');";
    out << domInsertJS;
    renderInnerHtmlJS(out, app);
  } else {
    out << "document.createElement('" << elementNames_[type_] << "');";
    out << domInsertJS;
    asJavaScript(out, Create);
    asJavaScript(out, Update);
  }
}

std::string DomElement::addToParent(std::ostream& out,
				    const std::string& parentVar,
				    int pos, WApplication *app)
{
  EscapeOStream sout(out);
  return addToParent(sout, parentVar, pos, app);
}

std::string DomElement::addToParent(EscapeOStream& out,
				    const std::string& parentVar,
				    int pos, WApplication *app)
{
  createVar();

  if (type_ == DomElement_TD || type_ == DomElement_TR) {
    out << "var " << var_ << "=";

    if (type_ == DomElement_TD)
      out << parentVar << ".insertCell(" << pos << ");\n";
    else
      out << parentVar << ".insertRow(" << pos << ");\n";

    asJavaScript(out, Create);
    asJavaScript(out, Update);
  } else {
    std::stringstream insertJS;
    if (pos != -1)
      insertJS << WT_CLASS ".insertAt(" << parentVar << "," << var_
	       << "," << pos << ");";
    else
      insertJS << parentVar << ".appendChild(" << var_ << ");\n";

    createElement(out, app, insertJS.str());
  }

  return var_;
}

std::string DomElement::asJavaScript(EscapeOStream& out,
				     Priority priority) const
{
  switch(priority) {
  case Delete:
    if (deleted_ || removeAllChildren_) {
      declare(out);

      if (deleted_) {
	out << javaScriptEvenWhenDeleted_
	    << var_ << ".parentNode.removeChild("
	    << var_ << ");\n";
      } else  if (removeAllChildren_) {
	out << var_ << ".innerHTML='';\n";
      }
    }

    return var_;
  case Create:
    if (mode_ == ModeCreate) {
      if (!id_.empty())
	out << var_ << ".setAttribute('id', '" << id_ << "');\n";

      setJavaScriptAttributes(out);
      setJavaScriptProperties(out, WApplication::instance());
    }

    return var_;
  case Update:
  {
    if (deleted_)
      break;

    WApplication *app = WApplication::instance();

    /*
     * short-cut for frequent short manipulations
     */
    if (mode_ == ModeUpdate && numManipulations_ == 1) {
      if (properties_.find(PropertyStyleDisplay) != properties_.end()) {
	std::string style = properties_.find(PropertyStyleDisplay)->second;
	if (style == "none") {
	  out << WT_CLASS ".hide('" << id_ << "');\n";
	  return var_;
	} else if (style.empty()) {
	  out << WT_CLASS ".show('" << id_ << "');\n";
	  return var_;
	} else if (style == "inline") {
	  out << WT_CLASS ".inline('" + id_ + "');\n";
	  return var_;
	} else if (style == "block") {
	  out << WT_CLASS ".block('" + id_ + "');\n";
	  return var_;
	}
      }
    }

    processEvents(app);
    processProperties(app);

    if (replaced_) {
      declare(out);

      std::string varr = replaced_->createVar();
      std::stringstream insertJs;
      insertJs << var_ << ".parentNode.replaceChild("
	       << varr << ',' << var_ << ");\n";
      replaced_->createElement(out, app, insertJs.str());
      if (unstubbed_)
	out << WT_CLASS ".unstub(" << var_ << ',' << varr << ','
	    << (hideWithDisplay_ ? 1 : 0) << ");\n";

      return var_;
    } else if (insertBefore_) {
      declare(out);

      std::string varr = insertBefore_->createVar();
      std::stringstream insertJs;
      insertJs << var_ << ".parentNode.insertBefore(" << varr << ","
	       << var_ + ");\n";
      insertBefore_->createElement(out, app, insertJs.str());

      return var_;
    }

    // FIXME optimize with subselect

    for (unsigned i = 0; i < childrenToSave_.size(); ++i) {
      declare(out);

      out << "var c" << var_ << (int)i << '='
	  << "$('#" << childrenToSave_[i] << "')";
      // In IE, contents is deleted by setting innerHTML
      if (app->environment().agentIsIE())
	out << ".remove()";
      out << ";";
    }

    if (mode_ != ModeCreate) {
      setJavaScriptProperties(out, app);
      setJavaScriptAttributes(out);
    }

    for (EventHandlerMap::const_iterator i = eventHandlers_.begin();
	 i != eventHandlers_.end(); ++i)
      if ((mode_ == ModeUpdate) || !i->second.jsCode.empty())
	setJavaScriptEvent(out, i->first, i->second, app);

    renderInnerHtmlJS(out, app);

    for (unsigned i = 0; i < childrenToSave_.size(); ++i)
      out << "$('#" << childrenToSave_[i] << "').replaceWith(c"
	  << var_ << (int)i << ");";

    return var_;
  }
  }

  return var_;
}

void DomElement::renderInnerHtmlJS(EscapeOStream& out, WApplication *app) const
{
  if (wasEmpty_ && canWriteInnerHTML(app)) {
    if ((type_ == DomElement_DIV
	 && app->environment().agent() == WEnvironment::IE6)
	|| !childrenToAdd_.empty() || childrenHtml_) {
      declare(out);

      out << WT_CLASS ".setHtml(" << var_ << ",'";

      out.pushEscape(EscapeOStream::JsStringLiteralSQuote);
      if (childrenHtml_)
	out << childrenHtml_->str();

      TimeoutList timeouts;
      std::stringstream js;

      for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	childrenToAdd_[i].child->asHTML(out, js, timeouts);

      if (type_ == DomElement_DIV
	  && app->environment().agent() == WEnvironment::IE6
	  && childrenToAdd_.empty()
	  && !childrenHtml_)
	out << "&nbsp;";

      out.popEscape();

      out << "');\n";

      Utils::insert(timeouts, timeouts_);

      for (unsigned i = 0; i < timeouts.size(); ++i)
	out << app->javaScriptClass()
	    << "._p_.addTimerEvent('" << timeouts[i].event << "', " 
	    << timeouts[i].msec << ","
	    << (timeouts[i].repeat ? "true" : "false") << ");\n";

      out << js.str();
    }
  } else {
    for (unsigned i = 0; i < childrenToAdd_.size(); ++i) {
      declare(out);
      DomElement *child = childrenToAdd_[i].child;
      child->addToParent(out, var_, childrenToAdd_[i].pos, app);
    }
  }

  for (unsigned i = 0; i < methodCalls_.size(); ++i) {
    declare(out);
    out << var_ << "." << methodCalls_[i] << ';' << '\n';
  }

  if (!javaScriptEvenWhenDeleted_.empty()) {
    declare(out);
    out << javaScriptEvenWhenDeleted_ << '\n';
  }

  if (!javaScript_.empty()) {
    declare(out);
    out << javaScript_ << '\n';
  }

  if (timeOut_ != -1)
    out << app->javaScriptClass() << "._p_.addTimerEvent('"
	<< id_ << "', " << timeOut_ << ","
	<< (timeOutJSRepeat_ ? "true" : "false") << ");\n";
}

void DomElement::setJavaScriptProperties(EscapeOStream& out,
					 WApplication *app) const
{
#ifndef WT_TARGET_JAVA
  EscapeOStream escaped(out);
#else
  EscapeOStream escaped = out.push();
#endif // WT_TARGET_JAVA

  bool pushed = false;

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    declare(out);

    switch(i->first) {
    case PropertyInnerHTML:
    case PropertyAddedInnerHTML:
      out << WT_CLASS ".setHtml(" << var_ << ',';
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << (i->first == PropertyInnerHTML ? ",false" : ",true")
	  << ");";
      break;
    case PropertyScript:
      out << var_ << ".innerHTML=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped,
			  "/*<![CDATA[*/\n" + i->second + "\n/* ]]> */");
      out << ';';
      break;
    case PropertyValue:
      out << var_ << ".value=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    case PropertyTarget:
      out << var_ << ".target='" << i->second << "';";
      break;
    case PropertyIndeterminate:
      out << var_ << ".indeterminate=" << i->second << ";";
      break;
    case PropertyDisabled:
      out << var_ << ".disabled=" << i->second << ';';
      break;
    case PropertyReadOnly:
      out << var_ << ".readOnly=" << i->second << ';';
      break;
    case PropertyChecked:
      out << var_ << ".checked=" << i->second << ';';
      break;
    case PropertySelected:
      out << var_ << ".selected=" << i->second << ';';
      break;
    case PropertySelectedIndex:
      out << var_ << ".selectedIndex=" << i->second << ';';
      break;
    case PropertyMultiple:
      out << var_ << ".multiple=" << i->second << ';';
      break;
    case PropertySrc:
      out << var_ << ".src='" << i->second << "\';";
      break;
    case PropertyColSpan:
      out << var_ << ".colSpan=" << i->second << ";";
      break;
    case PropertyRowSpan:
      out << var_ << ".rowSpan=" << i->second << ";";
      break;
    case PropertyClass:
      out << var_ << ".className=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    case PropertyText:
      out << var_ << ".text=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    case PropertyStyleFloat:
      out << var_ << ".style."
	  << (app->environment().agentIsIE() ? "styleFloat" : "cssFloat")
	  << "=\'" << i->second << "\';";
      break;
    case Wt::PropertyStyleWidthExpression:
      out << var_ << ".style.setExpression('width',";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ");";
      break;
    default:
      if ((i->first >= PropertyStyle)
	  && (i->first <= PropertyStyleDisplay)) {
	static std::string cssCamelNames[] =
	  { "cssText", "width", "position",
	    "zIndex", "cssFloat", "clear",
	    "width", "height", "lineHeight",
	    "minWidth", "minHeight",
	    "maxWidth", "maxHeight",
	    "left", "right", "top", "bottom",
	    "verticalAlign", "textAlign",
	    "padding",
	    "marginTop", "marginRight",
	    "marginBottom", "marginLeft",
	    "cursor", 	    
	    "borderTop", "borderRight",
	    "borderBottom", "borderLeft",
	    "color", "overflow", "overflow",
	    "fontFamily", "fontStyle", "fontVariant",
	    "fontWeight", "fontSize",
	    "backgroundColor", "backgroundImage", "backgroundRepeat",
	    "backgroundAttachment", "backgroundPosition",
	    "textDecoration", "whiteSpace", "tableLayout", "borderSpacing",
	    "visibility", "display" };
	out << var_ << ".style."
	    << cssCamelNames[i->first - PropertyStyle]
	    << "='" << i->second << "';";
      }
    }

    out << '\n';
  }
}

void DomElement::setJavaScriptAttributes(EscapeOStream& out) const
{
  for (AttributeMap::const_iterator i = attributes_.begin();
       i != attributes_.end(); ++i) {
    declare(out);

    if (i->first == "style") {
      out << var_ << ".style.cssText = ";
      jsStringLiteral(out, i->second, '\'');
      out << ';' << '\n';
    } else {
      out << var_ << ".setAttribute('" << i->first << "',";
      jsStringLiteral(out, i->second, '\'');
      out << ");\n";
    }
  }
}

bool DomElement::isDefaultInline() const
{
  return defaultInline_[type_];
}

bool DomElement::isSelfClosingTag(const std::string& tag)
{
  return (   (tag == "br")
	  || (tag == "hr")
	  || (tag == "img")
	  || (tag == "area")
	  || (tag == "col")
	  || (tag == "input"));
}

bool DomElement::isSelfClosingTag(DomElementType element)
{
  return ((   element == DomElement_BR)
       /* || (element == DomElement_HR) */
	  || (element == DomElement_IMG)
	  || (element == DomElement_AREA)
	  || (element == DomElement_COL)
	  || (element == DomElement_INPUT));
}

}
