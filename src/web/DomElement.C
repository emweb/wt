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

using std::exit;

namespace {

const char *elementNames_[] =
  { "a", "br", "button", "col",
    "div", "fieldset", "form",
    "h1", "h2", "h3", "h4",

    "h5", "h6", "iframe", "img",
    "input", "label", "legend", "li",
    "ol",

    "option", "ul", "script", "select",
    "span", "table", "tbody", "td",
    "textarea",

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
    true,

    false, false, false,
    false, true
  };
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
  return getForUpdate(object->formName(), type);
}

DomElement::DomElement(Mode mode, DomElementType type)
  : mode_(mode),
    wasEmpty_(mode_ == ModeCreate),
    deleted_(false),
    removeAllChildren_(false),
    minMaxSizeProperties_(false),
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

std::string DomElement::urlEncode(const std::string& url)
{
  std::stringstream result;

  static const std::string unsafeChars = "$&+,:;=?@'\"<>#%{}|\\^~[]`";

  for (unsigned i = 0; i < url.length(); ++i) {
    char c = url[i];
    if (c < 31 || c >= 127 || unsafeChars.find(c) != std::string::npos)
      result << '%' << std::hex << (int)c;
    else
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
      properties_.erase(i++);
  }
}

void DomElement::addChild(DomElement *child)
{
  ++numManipulations_;

  javaScript_ += child->javaScriptEvenWhenDeleted_ + child->javaScript_;
  child->javaScriptEvenWhenDeleted_.clear();
  child->javaScript_.clear();

  if (wasEmpty_ && canWriteInnerHTML(WApplication::instance())) {
    if (!childrenHtml_)
      childrenHtml_ = new std::stringstream();
    EscapeOStream sout(*childrenHtml_);
    child->asHTML(sout, timeouts_);

    delete child;
  } else {
    childrenToAdd_.push_back(ChildInsertion(-1, child));
  }
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
    && (strcmp("click", eventName) == 0);

  bool nonEmpty = isExposed || anchorClick || !jsCode.empty();

  std::stringstream js;

  if (nonEmpty)
    if (app->environment().agentIEMobile())
      js << "var e=window.event;";
    else
      js << "var e=event?event:window.event;";

  if (anchorClick)
    js << "if(e.ctrlKey||e.metaKey)return true;else{";

  if (isExposed)
    js << app->javaScriptClass() << "._p_.update(this,'"
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
  std::string code = "var s='';";

  for (unsigned i = 0; i < actions.size(); ++i) {
    if (!actions[i].jsCondition.empty())
      code += "if(" + actions[i].jsCondition + "){";
    code += actions[i].jsCode;
    if (actions[i].exposed) {
      if (actions.size() > 1)
	code += "if(s.length != 0){s += ',';} s +='";
      else
	code += "s='";
      code += actions[i].updateCmd + "';";
    }
    if (!actions[i].jsCondition.empty())
      code += "}";
  }

  code += "if(s.length!=0){"
    + WApplication::instance()->javaScriptClass()
    + "._p_.update(this, s, event, true);}";

  setEvent(eventName, code, "");
}

void DomElement::processProperties(WApplication *app) const
{
  if (minMaxSizeProperties_ && app->environment().agentIE()) {
    DomElement *self = const_cast<DomElement *>(this); 

    PropertyMap::iterator w = self->properties_.find(PropertyStyleWidth);
    PropertyMap::iterator minw = self->properties_.find(PropertyStyleMinWidth);
    PropertyMap::iterator maxw = self->properties_.find(PropertyStyleMaxWidth);

    if (minw != self->properties_.end() || maxw != self->properties_.end()) {
      std::stringstream style;
      if (w == self->properties_.end()) {
	style << "expression(" WT_CLASS ".IEwidth(this,";
	if (minw != self->properties_.end()) {
	  style << '\'' << minw->second << '\'';
          self->properties_.erase(PropertyStyleMinWidth); // C++: could be minw
	} else
	  style << "'0px'";
	style << ',';
	if (maxw != self->properties_.end()) {
	  style << '\''<< maxw->second << '\'';
	  self->properties_.erase(PropertyStyleMaxWidth); // C++: could be maxw
	} else
	  style << "'100000px'";
	style << "));";
      }

      self->properties_[PropertyStyleWidth] = style.str();
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

  EventHandlerMap::const_iterator mouseup = eventHandlers_.find("mouseup");
  if (mouseup != eventHandlers_.end() && !mouseup->second.jsCode.empty())
    Utils::access(self->eventHandlers_, "mousedown").jsCode
      = app->javaScriptClass() + "._p_.saveDownPos(event);"
      + Utils::access(self->eventHandlers_, "mousedown").jsCode;

  EventHandlerMap::const_iterator mousedown = eventHandlers_.find("mousedown");
  if (mousedown != eventHandlers_.end() && !mousedown->second.jsCode.empty())
    Utils::access(self->eventHandlers_, "mousedown").jsCode
      = app->javaScriptClass() + "._p_.capture(this);"
      + Utils::access(self->eventHandlers_, "mousedown").jsCode;

  EventHandlerMap::const_iterator keypress = eventHandlers_.find("keypress");
  if (keypress != eventHandlers_.end() && !keypress->second.jsCode.empty())
    Utils::access(self->eventHandlers_, "keypress").jsCode
      = "if (" WT_CLASS ".isKeyPress(event)){"
      + Utils::access(self->eventHandlers_, "keypress").jsCode
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

void DomElement::setProperty(Wt::Property property, const std::string& value)
{
  ++numManipulations_;
  properties_[property] = value;

  if (property >= PropertyStyleMinWidth && property <= PropertyStyleMaxHeight)
    minMaxSizeProperties_ = true;
}

std::string DomElement::getProperty(Wt::Property property) const
{
  PropertyMap::const_iterator i = properties_.find(property);
  
  if (i != properties_.end())
    return i->second;
  else
    return std::string();
}

void DomElement::removeProperty(Wt::Property property)
{
  properties_.erase(property);
}

void DomElement::setId(const std::string& id, bool andName)
{
  ++numManipulations_;
  id_ = id;
  if (andName)
    setAttribute("name", id);
}

void DomElement::setId(const WObject *object, bool andName)
{
  setId(object->formName(), andName);
}

void DomElement::insertChildAt(DomElement *child, int pos)
{
  ++numManipulations_;

  javaScript_ += child->javaScriptEvenWhenDeleted_ + child->javaScript_;
  child->javaScriptEvenWhenDeleted_.clear();
  child->javaScript_.clear();

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

void DomElement::replaceWith(DomElement *newElement, bool hideWithDisplay)
{
  ++numManipulations_;
  replaced_ = newElement;
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
  sout.flush();
}

void DomElement::fastJsStringLiteral(EscapeOStream& outRaw,
				     EscapeOStream& outEscaped,
				     const std::string& s)
{
  outRaw << '\'';
  outRaw.flush();
  outEscaped << s;
  outEscaped.flush();
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
					EscapeOStream& outEscaped,
					const std::string& s)
{
  outRaw << '"';
  outRaw.flush();
  outEscaped << s;
  outEscaped.flush();
  outRaw << '"';
}

std::string DomElement::cssStyle() const
{
  AttributeMap::const_iterator i = attributes_.find("style");

  if (properties_.empty())
    return (i != attributes_.end() ? i->second : std::string());

  std::stringstream style;

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    if ((i->first >= Wt::PropertyStylePosition)
	&& (i->first <= Wt::PropertyStyleDisplay)) {
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

      if ((i->first == Wt::PropertyStyleCursor) && (i->second == "pointer")) {
	style << "cursor:pointer;cursor:hand;";	    
      } else {
	style << cssNames[i->first - Wt::PropertyStylePosition]
	      << ':' << i->second << ';';
      }
    }
  }

  if (i != attributes_.end())
    style << i->second;

  return style.str();
}

void DomElement::asHTML(EscapeOStream& out,
			std::vector<TimeoutEvent>& timeouts) const
{
  if (mode_ != ModeCreate)
    throw WtException("DomElement::asHTML() called with ModeUpdate");

  WApplication *app = WApplication::instance();
  processEvents(app);
  processProperties(app);

  EventHandlerMap::const_iterator clickEvent = eventHandlers_.find("click");

  bool needButtonWrap
    = (!(app->environment().ajax())
       && (clickEvent != eventHandlers_.end())
       && (!clickEvent->second.jsCode.empty())
       && (!app->environment().agentIsSpiderBot()));

  if (needButtonWrap) {
    PropertyMap::const_iterator i = properties_.find(Wt::PropertyStyleDisplay);
    if (i != properties_.end() && i->second == "none")
      return;
  }

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
      self->setAttribute("name", "e0signal=" + clickEvent->second.signalName);

      needButtonWrap = false;
    } else if (type_ == DomElement_IMG) {
      /*
       * We don't need to wrap an image: we can substitute it for an input
       * type image. This avoid layout problems.
       */
      renderedType = DomElement_INPUT;

      DomElement *self = const_cast<DomElement *>(this);
      self->setAttribute("type", "image");
      self->setAttribute("name", "e0signal=" + clickEvent->second.signalName);
      needButtonWrap = false;
    }
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
      if (href != "#")
	needButtonWrap = false;
    }
  }

  const bool isIEMobile = app->environment().agentIEMobile();
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
		       properties_.find(Wt::PropertyInnerHTML)->second);
    self->setProperty(Wt::PropertyInnerHTML, "");
  }

  EscapeOStream attributeValues(out);
  attributeValues.pushEscape(EscapeOStream::HtmlAttribute);

  std::string style;

  if (needButtonWrap) {
    if (supportButton) {
      out << "<button type=\"submit\" name=\"e0signal\" value=";
      fastHtmlAttributeValue(out, attributeValues,
			     clickEvent->second.signalName);
      out << " class=\"Wt-wrap ";

      AttributeMap::const_iterator l = attributes_.find("class");
      if (l != attributes_.end())
	out << l->second;

      out << "\"";

      PropertyMap::const_iterator i = properties_.find(Wt::PropertyDisabled);
      if ((i != properties_.end()) && (i->second=="true"))
	out << " disabled=\"disabled\"";

      if (!app->environment().agentKonqueror()
	  && !app->environment().agentWebKit())
	style = "margin: -1px -3px -2px -3px;";

      out << "><" << elementNames_[renderedType];
    } else {
      if (type_ == DomElement_IMG)
	out << "<input type=\"image\"";
      else
	out << "<input type=\"submit\"";

      out << " name=";
      fastHtmlAttributeValue(out, attributeValues,
			     "e0signal=" + clickEvent->second.signalName);
      out << " value=";

      PropertyMap::const_iterator i = properties_.find(Wt::PropertyInnerHTML);
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
    if (i->first != "style"
	&& (!app->environment().agentIsSpiderBot() || i->first != "name")) {
      out << " " << i->first << "=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
    }

  if (app->environment().ajax()) {
    for (EventHandlerMap::const_iterator i = eventHandlers_.begin();
	 i != eventHandlers_.end(); ++i) {
      if (!i->second.jsCode.empty()) {
	out << " on" << i->first << "=";
	fastHtmlAttributeValue(out, attributeValues, i->second.jsCode);
      }
    }
  }

  std::string innerHTML = "";

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    switch (i->first) {
    case Wt::PropertyText:
    case Wt::PropertyInnerHTML:
      innerHTML += i->second; break;
    case Wt::PropertyScript:
      innerHTML += "/*<![CDATA[*/\n" + i->second + "\n/* ]]> */"; break;
    case Wt::PropertyDisabled:
      if (i->second == "true")
	// following is not XHTML
	// out << " disabled"; 
	// following is not interpreted correctly by all HTML renderers
	//  (like konqueror)
	out << " disabled=\"disabled\"";
      break;
    case Wt::PropertyChecked:
      if (i->second == "true")
	// out << " checked";
	out << " checked=\"checked\"";
      break;
    case Wt::PropertySelected:
      if (i->second == "true")
	// out << " selected";
	out << " selected=\"selected\"";
      break;
    case Wt::PropertyMultiple:
      if (i->second == "true")
	// out << " selected";
	out << " multiple=\"multiple\"";
      break;
    case Wt::PropertyTarget:
      out << " target=\"" << i->second << "\"";
      break;
    case Wt::PropertyValue:
      out << " value=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case Wt::PropertySrc:
      out << " src=";
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
    /*
     * http://www.w3.org/TR/html/#guidelines
     * XHTML recommendation, back-wards compatibility with HTML: C.2, C.3:
     * do not use minimized forms when content is empty like <p />, and use
     * minimized forms for certain elements like <br />
     */
    if (!isSelfClosingTag(renderedType)) {
      out << ">" << innerHTML;
      for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	childrenToAdd_[i].child->asHTML(out, timeouts);

      if (childrenHtml_)
	out << childrenHtml_->str();
      out << "</" << elementNames_[renderedType] << ">";
    } else
      out << " />";

    if (needButtonWrap && supportButton)
      out << "</button>";
    else if (needAnchorWrap)
      out << "</a>";
  }

  for (unsigned i = 0; i < methodCalls_.size(); ++i)
    app->doJavaScript(WT_CLASS ".getElement('" + id_ + "')."
		      + methodCalls_[i] + ';');

  if (timeOut_ != -1)
    timeouts.push_back(TimeoutEvent(timeOut_, id_, timeOutJSRepeat_));

#ifndef JAVA
  timeouts.insert(timeouts.end(), timeouts_.begin(), timeouts_.end());
#endif // FIXME JAVA
}

void DomElement::createReference(EscapeOStream& out) const
{
  if (mode_ == ModeCreate)
    out << "document.createElement('" << elementNames_[type_] << "')";
  else
    out << WT_CLASS ".getElement('" << id_ << "')";
}

std::string DomElement::createReference() const
{
  static const std::string CREATE("document.createElement('"); 

  if (mode_ == ModeCreate)
    return CREATE + elementNames_[type_] + "')";
  else
    return WT_CLASS ".getElement('" + id_ + "')";
}

void DomElement::declare(EscapeOStream& out) const
{
  if (var_.empty()) {
#ifndef JAVA
    char buf[20];
    sprintf(buf, "j%d", nextId_++);
    var_ = buf;
#else // !JAVA
    var_ = "j" + boost::lexical_cast<std::string>(nextId_++);
#endif // !JAVA

    out << "var " << var_ << "=";
    createReference(out);
    out << ';';
  }
}

bool DomElement::canWriteInnerHTML(WApplication *app) const
{
  if (app->environment().agentIEMobile())
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
  if ((app->environment().agentIE()
       || app->environment().agentKonqueror())
      && (   type_ == DomElement_TBODY
	  || type_ == DomElement_TABLE
          || type_ == DomElement_TR
	  || type_ == DomElement_SELECT))
    return false;

  return true;
}

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

void DomElement::asJavaScript(std::ostream& out)
{
  mode_ = ModeUpdate;

  EscapeOStream eout(out);

  declare(eout);
  eout << var_ << ".setAttribute('id', '" << id_ << "');";

  mode_ = ModeCreate;

  setJavaScriptProperties(eout);
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

std::string DomElement::createAsJavaScript(EscapeOStream& out,
					   const std::string& parentVar,
					   int pos)
{
#ifndef WT_JAVA
  char buf[20];
  sprintf(buf, "j%d", nextId_++);
  var_ = buf;
#else // !WT_JAVA
  var_ = "j" + boost::lexical_cast<std::string>(nextId_++);
#endif // !WT_JAVA

  out << "var " << var_ << "=";

  if (type_ == DomElement_TD)
    out << parentVar << ".insertCell(" << pos << ");";
  else if (type_ == DomElement_TR)
    out << parentVar << ".parentNode.insertRow(" << pos << ");";
  else {
    out << "document.createElement('" << elementNames_[type_] << "');";
    if (pos != -1)
      out << parentVar << ".insertBefore(" << var_ << ","
	  << parentVar << ".childNodes[" << pos << "]);";
    else
      out << parentVar << ".appendChild(" << var_ << ");";
  }

  return asJavaScript(out, Create);
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
	    << var_ << ");";
      } else  if (removeAllChildren_) {
	out << var_ << ".innerHTML='';";
      }
    }

    return var_;

    break;
  case Create:
    if (mode_ == ModeCreate) {
      declare(out);

      if (!id_.empty())
	out << var_ << ".setAttribute('id', '" << id_ << "');";

      setJavaScriptProperties(out);
      setJavaScriptAttributes(out);
    }

    return var_;

    break;
  case Update:
  {
    if (deleted_)
      break;

    WApplication *app = WApplication::instance();

    /*
     * short-cut for frequent short manipulations
     */
    if (mode_ == ModeUpdate && numManipulations_ == 1) {
      if (properties_.find(Wt::PropertyStyleDisplay) != properties_.end()) {
	std::string style = properties_.find(Wt::PropertyStyleDisplay)->second;
	if (style == "none") {
	  out << WT_CLASS ".hide('" << id_ << "');";
	  return var_;
	} else if (style.empty()) {
	  out << WT_CLASS ".show('" << id_ << "');";
	  return var_;
	} else if (style == "inline") {
	  out << WT_CLASS ".inline('" + id_ + "');";
	  return var_;
	} else if (style == "block") {
	  out << WT_CLASS ".block('" + id_ + "');";
	  return var_;
	}
      }
    }

    processEvents(app);

    if (replaced_) {
      declare(out);

      std::string varr = replaced_->asJavaScript(out, Create);
      out << var_ << ".parentNode.replaceChild("
	  << varr << ',' << var_ << ");";

      replaced_->asJavaScript(out, Update);
      if (hideWithDisplay_)
	out << varr << ".style.display = " << var_ << ".style.display;";
      else
	out << WT_CLASS ".copyhide(" << var_ << ',' << varr << ");";

      return var_;
    } else if (insertBefore_) {
      declare(out);

      std::string varr = insertBefore_->asJavaScript(out, Create);
      out << var_ << ".parentNode.insertBefore(" << varr << ","
	  << var_ + ");";

      insertBefore_->asJavaScript(out, Update);

      return var_;
    }

    if (mode_ != ModeCreate) {
      setJavaScriptAttributes(out);
      setJavaScriptProperties(out);
    }

    for (EventHandlerMap::const_iterator i = eventHandlers_.begin();
	 i != eventHandlers_.end(); ++i) {
      if ((mode_ == ModeUpdate) || (!i->second.jsCode.empty())) {
	declare(out);

	int fid = nextId_++;

	out << "function f" << fid
	    << "(event){" << i->second.jsCode << "}";

	if (i->first.substr(0, 3) == "key" && id_ == app->root()->formName())
	  out << "document";
	else
	  out << var_;

	out << ".on" << i->first << "=f" << fid << ";";
      }
    }

    if (wasEmpty_ && canWriteInnerHTML(app)) {
      if (!childrenToAdd_.empty() || childrenHtml_) {
	declare(out);

	out << WT_CLASS ".setHtml(" << var_ << ",'";

	out.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	out << childrenHtml_->str();

	TimeoutList timeouts;
	for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	  childrenToAdd_[i].child->asHTML(out, timeouts);
	out.popEscape();

	out << "');";

#ifndef JAVA
	timeouts.insert(timeouts.end(), timeouts_.begin(), timeouts_.end());
#endif // FIXME JAVA

	for (unsigned i = 0; i < timeouts.size(); ++i)
	  out << app->javaScriptClass()
	      << "._p_.addTimerEvent('" << timeouts[i].event << "', " 
	      << timeouts[i].msec << ","
	      << (timeouts[i].repeat ? "true" : "false") << ");\n";
      }
    } else {
      for (unsigned i = 0; i < childrenToAdd_.size(); ++i) {
	declare(out);

	DomElement *child = childrenToAdd_[i].child;

	std::string cvar = child->createAsJavaScript(out, var_,
						     childrenToAdd_[i].pos);

	child->asJavaScript(out, Update);
      }
    }

    for (unsigned i = 0; i < methodCalls_.size(); ++i) {
      declare(out);
      out << var_ << "." << methodCalls_[i] << ';';
    }

    if (!javaScriptEvenWhenDeleted_.empty()) {
      declare(out);
      out << javaScriptEvenWhenDeleted_;
    }

    if (!javaScript_.empty()) {
      declare(out);
      out << javaScript_;
    }

    if (timeOut_ != -1)
      out << app->javaScriptClass() << "._p_.addTimerEvent('"
	  << id_ << "', " << timeOut_ << ","
	  << (timeOutJSRepeat_ ? "true" : "false") << ");\n";

    return var_;
  }
  }

  return var_;
}

void DomElement::setJavaScriptProperties(EscapeOStream& out) const
{
  EscapeOStream escaped(out);

  bool pushed = false;

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    declare(out);

    switch(i->first) {
    case Wt::PropertyInnerHTML:
      out << WT_CLASS ".setHtml(" << var_ << ',';
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ");";
      break;
    case Wt::PropertyScript:
      out << var_ << ".innerHTML=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped,
			  "/*<![CDATA[*/\n" + i->second + "\n/* ]]> */");
      out << ';';
      break;
    case Wt::PropertyValue:
      out << var_ << ".value=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    case Wt::PropertyTarget:
      out << var_ << ".target='" << i->second << "';";
      break;
    case Wt::PropertyDisabled:
      out << var_ << ".disabled=" << i->second << ';';
      break;
    case Wt::PropertyChecked:
      out << var_ << ".checked=" << i->second << ';';
      break;
    case Wt::PropertySelected:
      out << var_ << ".selected=" << i->second << ';';
      break;
    case Wt::PropertySelectedIndex:
      out << var_ << ".selectedIndex=" << i->second << ';';
      break;
    case Wt::PropertyMultiple:
      out << var_ << ".multiple=" << i->second << ';';
      break;
    case Wt::PropertySrc:
      out << var_ << ".src='" << i->second << "\';";
      break;
    case Wt::PropertyText:
      out << var_ << ".text=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    default:
      if ((i->first >= Wt::PropertyStylePosition)
	  && (i->first <= Wt::PropertyStyleDisplay)) {
	static std::string cssCamelNames[] =
	  { "position",
	    "zIndex", "float", "clear",
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
	    << cssCamelNames[i->first - Wt::PropertyStylePosition]
	    << "=\'" << i->second << "\';";
      }
    }
  }
}

void DomElement::setJavaScriptAttributes(EscapeOStream& out) const
{
  for (AttributeMap::const_iterator i = attributes_.begin();
       i != attributes_.end(); ++i) {
    declare(out);

    if (i->first == "class") {
      out << var_ << ".className = ";
      jsStringLiteral(out, i->second, '\'');
      out << ";";
    } else if (i->first == "style") {
      out << var_ << ".style.cssText = ";
      jsStringLiteral(out, i->second, '\'');
      out << ";";
    } else {
      out << var_ << ".setAttribute('" << i->first << "',";
      jsStringLiteral(out, i->second, '\'');
      out << ");";
    }
  }
}

std::string DomElement::asJavaScript(std::stringstream& js, bool create) const
{
  EscapeOStream sout(js);

  if (create)
    asJavaScript(sout, Create);
  else
    asJavaScript(sout, Update);

  return var_;
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
