/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <cstdio>
#include <sstream>

#include "Wt/WObject"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WStringStream"
#include "Wt/WTheme"

#include "DomElement.h"
#include "WebUtils.h"

namespace {

std::string elementNames_[] =
  { "a", "br", "button", "col",
    "colgroup",
    "div", "fieldset", "form",
    "h1", "h2", "h3", "h4",

    "h5", "h6", "iframe", "img",
    "input", "label", "legend", "li",
    "ol",

    "option", "ul", "script", "select",
    "span", "table", "tbody", "thead",
    "tfoot", "th", "td", "textarea",
    "optgroup",

    "tr", "p", "canvas",
    "map", "area", "style",

    "object", "param",

    "audio", "video", "source",

    "b", "strong", "em", "i", "hr"
  };

bool defaultInline_[] =
  { true, false, true, false,
    false,
    false, false, false,
    false, false, false, false,

    false, false, true, true,
    true, true, true, false,
    false,

    true, false, false, true,
    true, false, false, false,
    false, false, false, true,
    true,

    false, false, true,
    false, true, true,

    false, false,

    false, false, false,

    true, true, true, true, false
  };

std::string cssNames_[] =
  { "position",
    "z-index", "float", "clear",
    "width", "height", "line-height",
    "min-width", "min-height",
    "max-width", "max-height",
    "left", "right", "top", "bottom",
    "vertical-align", "text-align",
    "padding",
    "padding-top", "padding-right",
    "padding-bottom", "padding-left",
    "margin-top", "margin-right",
    "margin-bottom", "margin-left", "cursor",
    "border-top", "border-right",
    "border-bottom", "border-left",
    "border-color-top", "border-color-right",
    "border-color-bottom", "border-color-left",
    "border-width-top", "border-width-right",
    "border-width-bottom", "border-width-left",
    "color", "overflow-x", "overflow-y",
    "opacity",
    "font-family", "font-style", "font-variant",
    "font-weight", "font-size",
    "background-color", "background-image", "background-repeat",
    "background-attachment", "background-position",
    "text-decoration", "white-space",
    "table-layout", "border-spacing",
    "border-collapse",
    "page-break-before", "page-break-after",
    "zoom", "visibility", "display",
    "box-sizing"};

std::string cssCamelNames_[] =
  { "cssText", "width", "position",
    "zIndex", "cssFloat", "clear",
    "width", "height", "lineHeight",
    "minWidth", "minHeight",
    "maxWidth", "maxHeight",
    "left", "right", "top", "bottom",
    "verticalAlign", "textAlign",
    "padding",
    "paddingTop", "paddingRight",
    "paddingBottom", "paddingLeft",
    "marginTop", "marginRight",
    "marginBottom", "marginLeft",
    "cursor",
    "borderTop", "borderRight",
    "borderBottom", "borderLeft",
    "borderColorTop", "borderColorRight",
    "borderColorBottom", "borderColorLeft",
    "borderWidthTop", "borderWidthRight",
    "borderWidthBottom", "borderWidthLeft",
    "color", "overflowX", "overflowY",
    "opacity",
    "fontFamily", "fontStyle", "fontVariant",
    "fontWeight", "fontSize",
    "backgroundColor", "backgroundImage", "backgroundRepeat",
    "backgroundAttachment", "backgroundPosition",
    "textDecoration", "whiteSpace",
    "tableLayout", "borderSpacing",
    "border-collapse",
    "pageBreakBefore", "pageBreakAfter",
    "zoom", "visibility", "display",
    "boxSizing" 
  };

const std::string unsafeChars_ = " $&+,:;=?@'\"<>#%{}|\\^~[]`/";

inline char hexLookup(int n) {
  return "0123456789abcdef"[(n & 0xF)];
}

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
    throw WException("Cannot update widget without id");

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
    removeAllChildren_(-1),
    minMaxSizeProperties_(false),
    unstubbed_(false),
    unwrapped_(false),
    replaced_(0),
    insertBefore_(0),
    type_(type),
    numManipulations_(0),
    timeOut_(-1),
    globalUnfocused_(false)
{ }

DomElement::~DomElement()
{
  for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
    delete childrenToAdd_[i].child;

  for (unsigned i = 0; i < updatedChildren_.size(); ++i)
    delete updatedChildren_[i];

  delete replaced_;
  delete insertBefore_;
}

void DomElement::setDomElementTagName(const std::string& name) {
  this->elementTagName_ = name;
}

#ifndef WT_TARGET_JAVA
#define toChar(b) char(b)
#else
unsigned char toChar(int b) {
  return (unsigned char)b;
}
#endif

std::string DomElement::urlEncodeS(const std::string& url,
                                   const std::string &allowed)
{
  WStringStream result;

#ifdef WT_TARGET_JAVA
  std::vector<unsigned char> bytes;
  try {
    bytes = url.getBytes("UTF-8");
  } catch (UnsupportedEncodingException& e) {
    // eat silly UnsupportedEncodingException
  }
#else
  const std::string& bytes = url;
#endif

  for (unsigned i = 0; i < bytes.size(); ++i) {
    unsigned char c = toChar(bytes[i]);
    if (c <= 31 || c >= 127 || unsafeChars_.find(c) != std::string::npos) {
      if (allowed.find(c) != std::string::npos) {
        result << (char)c;
      } else {
        result << '%';
	result << hexLookup(c >> 4);
        result << hexLookup(c);
      }
    } else
      result << (char)c;
  }

  return result.str();
}

std::string DomElement::urlEncodeS(const std::string& url)
{
  return urlEncodeS(url, std::string());
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

  assert(replaced_ == 0);
  assert(insertBefore_ == 0);

  attributes_.clear();
  removedAttributes_.clear();
  eventHandlers_.clear();

  for (PropertyMap::iterator i = properties_.begin(); i != properties_.end();) {
    if (   i->first == PropertyInnerHTML
	|| i->first == PropertyTarget)
      ++i;
    else
      Utils::eraseAndNext(properties_, i);
  }
}

void DomElement::addChild(DomElement *child)
{
  if (child->mode() == ModeCreate) {
    numManipulations_ += 2; // cannot be short-cutted

    if (wasEmpty_ && canWriteInnerHTML(WApplication::instance())) {
      child->asHTML(childrenHtml_, javaScript_, timeouts_);
      delete child;
    } else {
      childrenToAdd_.push_back(ChildInsertion(-1, child));
    }
  } else
    updatedChildren_.push_back(child);
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
  removedAttributes_.erase(attribute);
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
  ++numManipulations_;
  attributes_.erase(attribute);
  removedAttributes_.insert(attribute);
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

  WStringStream js;
  if (isExposed || anchorClick || !jsCode.empty()) {
    js << "var e=event||window.event,";
    js << "o=this;";

    if (anchorClick)
      js << "if(e.ctrlKey||e.metaKey||(" WT_CLASS ".button(e) > 1))"
	"return true;else{";

    /*
     * This order, first JavaScript and then event propagation is important
     * for WCheckBox where the tristate state is cleared before propagating
     * its value
     */
    js << jsCode;

    if (isExposed)
      js << app->javaScriptClass() << "._p_.update(o,'"
	 << signalName << "',e,true);";

    if (anchorClick)
      js << "}";
  }

  ++numManipulations_;
  eventHandlers_[eventName] = EventHandler(js.str(), signalName);
}

void DomElement::setEvent(const char *eventName, const std::string& jsCode)
{
  eventHandlers_[eventName] = EventHandler(jsCode, std::string());
}

void DomElement::addEvent(const char *eventName, const std::string& jsCode)
{
  eventHandlers_[eventName].jsCode += jsCode;
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
  WStringStream code;

  for (unsigned i = 0; i < actions.size(); ++i) {
    if (!actions[i].jsCondition.empty())
      code << "if(" << actions[i].jsCondition << "){";

    /*
     * This order, first JavaScript and then event propagation is important
     * for WCheckBox where the tristate state is cleared before propagating
     * its value
     */
    code << actions[i].jsCode;

    if (actions[i].exposed)
      code << WApplication::instance()->javaScriptClass()
	   << "._p_.update(o,'" << actions[i].updateCmd << "',e,true);";

    if (!actions[i].jsCondition.empty())
      code << "}";
  }

  setEvent(eventName, code.str(), "");
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
	WStringStream expr;
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
    }
  }
}

void DomElement::processEvents(WApplication *app) const
{
  DomElement *self = const_cast<DomElement *>(this);

  const char *S_keypress = WInteractWidget::KEYPRESS_SIGNAL;

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
    javaScript_ << jsCode << '\n';
  else
    javaScriptEvenWhenDeleted_ += jsCode;
}

void DomElement::setProperties(const PropertyMap& properties)
{
  for (PropertyMap::const_iterator i = properties.begin();
       i != properties.end(); ++i)
    setProperty(i->first, i->second);
}

void DomElement::clearProperties()
{
  numManipulations_ -= properties_.size();
  properties_.clear();
}

void DomElement::setProperty(Property property, const std::string& value)
{
  ++numManipulations_;
  properties_[property] = value;

  if (property >= PropertyStyleMinWidth && property <= PropertyStyleMaxHeight)
    minMaxSizeProperties_ = true;
}

void DomElement::addPropertyWord(Property property, const std::string& value)
{
  PropertyMap::const_iterator i = properties_.find(property);
  
  if (i != properties_.end()) {
    Utils::SplitSet words;
    Utils::split(words, i->second, " ", true);
    if (words.find(value) != words.end())
      return;
  }

  setProperty(property, Utils::addWord(getProperty(property), value));
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
  callJavaScript(WT_CLASS ".remove('" + id() + "');", true);
}

void DomElement::removeAllChildren(int firstChild)
{
  ++numManipulations_;
  removeAllChildren_ = firstChild;
  wasEmpty_ = firstChild == 0;
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

void DomElement::unwrap()
{
  ++numManipulations_;
  unwrapped_ = true;
}

void DomElement::callMethod(const std::string& method)
{
  ++numManipulations_;

  if (var_.empty())
    javaScript_ << WT_CLASS << ".$('" << id_ << "').";
  else
    javaScript_ << var_ << '.';

  javaScript_ << method << ";\n";
}

void DomElement::jsStringLiteral(WStringStream& out, const std::string& s,
				 char delimiter)
{
  EscapeOStream sout(out);
  jsStringLiteral(sout, s, delimiter);
}

void DomElement::htmlAttributeValue(WStringStream& out, const std::string& s)
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

  EscapeOStream style;
  const std::string *styleProperty = 0;

  for (PropertyMap::const_iterator j = properties_.begin();
       j != properties_.end(); ++j) {
    if (j->first == PropertyStyle)
      styleProperty = &j->second;
    else if ((j->first >= PropertyStylePosition)
	&& (j->first <= PropertyStyleBoxSizing)) {
      if ((j->first == PropertyStyleCursor) && (j->second == "pointer")) {
	style << "cursor:pointer;cursor:hand;";	    
      } else {
	if (!j->second.empty()) {
	  style << cssNames_[j->first - PropertyStylePosition]
		<< ':' << j->second << ';';
	  if (j->first >= PropertyStyleBoxSizing) {
	    WApplication *app = WApplication::instance(); // XXX

	    if (app->environment().agentIsGecko())
	      style << "-moz-";
	    else if (app->environment().agentIsWebKit())
	      style << "-webkit-";

	    style << cssNames_[j->first - PropertyStylePosition]
		  << ':' << j->second << ';';
	  }
	}
      }
    } else if (j->first == PropertyStyleWidthExpression) {
      style << "width:expression(" << j->second << ");";
    }
  }

  if (styleProperty)
    style << *styleProperty;

  return style.c_str();
}

void DomElement::setJavaScriptEvent(EscapeOStream& out,
				    const char *eventName,
				    const EventHandler& handler,
				    WApplication *app) const
{
  // events on the dom root container are events received by the whole
  // document when no element has focus

  int fid = nextId_++;

  out << "function f" << fid << "(event) { ";

  out << handler.jsCode;

  out << "}\n";

  if (globalUnfocused_) {
    out << app->javaScriptClass() 
      <<  "._p_.bindGlobal('" << std::string(eventName) <<"', '" << id_ << "', f" << fid 
      << ")\n";
    return;
  } else {
    declare(out);
    out << var_;
  }

  if (eventName == WInteractWidget::WHEEL_SIGNAL
      && app->environment().agentIsIE() && app->environment().agent() >= WEnvironment::IE9)
    out << ".addEventListener('wheel', f" << fid << ", false);\n";
  else
    out << ".on" << const_cast<char *>(eventName) << "=f" << fid << ";\n";
}

void DomElement::asHTML(EscapeOStream& out,
			EscapeOStream& javaScript,
			std::vector<TimeoutEvent>& timeouts,
			bool openingTagOnly) const
{
  if (mode_ != ModeCreate)
    throw WException("DomElement::asHTML() called with ModeUpdate");

  WApplication *app = WApplication::instance();
  processEvents(app);
  processProperties(app);

  EventHandlerMap::const_iterator clickEvent
    = eventHandlers_.find(WInteractWidget::CLICK_SIGNAL);

  bool needButtonWrap
    = (!app->environment().ajax()
       && (clickEvent != eventHandlers_.end())
       && (!clickEvent->second.signalName.empty())
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
       *
       * IE6 hell: IE will post all submit buttons, not just the one clicked.
       * We should therefore really be using input
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

      /*
       * If we're IE7/8 or there is a real URL, then we don't wrap
       */
      if (app->environment().agent() == WEnvironment::IE7 ||
	  app->environment().agent() == WEnvironment::IE8 ||
	  href.length() > 1)
	needButtonWrap = false;
      else if (app->theme()->canStyleAnchorAsButton()) {
	DomElement *self = const_cast<DomElement *>(this);
	self->setAttribute("href", app->url(app->internalPath())
			   + "&signal=" + clickEvent->second.signalName);
	needButtonWrap = false;
      }
    } else if (type_ == DomElement_AREA) {
      DomElement *self = const_cast<DomElement *>(this);
      self->setAttribute("href", app->url(app->internalPath())
			 + "&signal=" + clickEvent->second.signalName);
    }
  }

  const bool supportButton = true;

  bool needAnchorWrap = false;

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
      out << "<button type=\"submit\" name=\"signal=";
      out.append(clickEvent->second.signalName, attributeValues);
      out << "\" class=\"Wt-wrap ";

      PropertyMap::const_iterator l = properties_.find(PropertyClass);
      if (l != properties_.end()) {
	out << l->second;
	PropertyMap& map = const_cast<PropertyMap&>(properties_);
	map.erase(PropertyClass);
      }

      out << '"';

      std::string wrapStyle = cssStyle();
      if (!isDefaultInline())
	wrapStyle += "display: block;";

      if (!wrapStyle.empty()) {
	out << " style=";
	fastHtmlAttributeValue(out, attributeValues, wrapStyle);
      }

      PropertyMap::const_iterator i = properties_.find(PropertyDisabled);
      if ((i != properties_.end()) && (i->second=="true"))
	out << " disabled=\"disabled\"";

      for (AttributeMap::const_iterator j = attributes_.begin();
	   j != attributes_.end(); ++j)
	if (j->first == "title") {
	  out << ' ' << j->first << '=';
	  fastHtmlAttributeValue(out, attributeValues, j->second);
	}

      if (app->environment().agent() != WEnvironment::Konqueror
	  && !app->environment().agentIsWebKit()
	  && !app->environment().agentIsIE())
	style = "margin: 0px -3px -2px -3px;";

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
  } else if (renderedType == DomElement_OTHER)  // Custom tag name
	out << '<' << elementTagName_;
  else
    out << '<' << elementNames_[renderedType];

  if (!id_.empty()) {
    out << " id=";
    fastHtmlAttributeValue(out, attributeValues, id_);
  }

  for (AttributeMap::const_iterator i = attributes_.begin();
       i != attributes_.end(); ++i)
    if (!app->environment().agentIsSpiderBot() || i->first != "name") {
      out << ' ' << i->first << '=';
      fastHtmlAttributeValue(out, attributeValues, i->second);
    }

  if (app->environment().ajax()) {
    for (EventHandlerMap::const_iterator i = eventHandlers_.begin();
	 i != eventHandlers_.end(); ++i) {
      if (!i->second.jsCode.empty()) {
	if (globalUnfocused_ 
	    || (i->first == WInteractWidget::WHEEL_SIGNAL
		&& app->environment().agentIsIE() && app->environment().agent() >= WEnvironment::IE9))
	  setJavaScriptEvent(javaScript, i->first, i->second, app);
	else {
	  out << " on" << const_cast<char *>(i->first) << '=';
	  fastHtmlAttributeValue(out, attributeValues, i->second.jsCode);
	}
      }
    }
  }

  std::string innerHTML = "";

  for (PropertyMap::const_iterator i = properties_.begin();
       i != properties_.end(); ++i) {
    switch (i->first) {
    case PropertyInnerHTML:
      innerHTML += i->second; break;
    case PropertyDisabled:
      if (i->second == "true")
	out << " disabled=\"disabled\"";
      break;
    case PropertyReadOnly:
      if (i->second == "true")
	out << " readonly=\"readonly\"";
      break;
    case PropertyTabIndex:
      out << " tabindex=\"" << i->second << '"';
      break;
    case PropertyChecked:
      if (i->second == "true")
	out << " checked=\"checked\"";
      break;
    case PropertySelected:
      if (i->second == "true")
	out << " selected=\"selected\"";
      break;
    case PropertySelectedIndex:
      if (i->second == "-1") {
	DomElement *self = const_cast<DomElement *>(this);
	self->callMethod("selectedIndex=-1");
      }
      break;
    case PropertyMultiple:
      if (i->second == "true")
	out << " multiple=\"multiple\"";
      break;
    case PropertyTarget:
      out << " target=\"" << i->second << "\"";
      break;
	case PropertyDownload:
	  out << " download=\"" << i->second << "\"";
	  break;
    case PropertyIndeterminate:
      if (i->second == "true") {
	DomElement *self = const_cast<DomElement *>(this);
	self->callMethod("indeterminate=" + i->second);
      }
      break;
    case PropertyValue:
      if (type_ != DomElement_TEXTAREA) {
	out << " value=";
	fastHtmlAttributeValue(out, attributeValues, i->second);
      } else {
	std::string v = i->second;
  innerHTML += WWebWidget::escapeText(v, false);
      }
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
    case PropertyLabel:
      out << " label=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    case PropertyPlaceholder:
      out << " placeholder=";
      fastHtmlAttributeValue(out, attributeValues, i->second);
      break;
    default:
      break;
    }
  }

  if (!needButtonWrap)
    style += cssStyle();

  if (!style.empty()) {
    out << " style=";
    fastHtmlAttributeValue(out, attributeValues, style);
  }

  if (needButtonWrap && !supportButton)
    out << " />";
  else {
    if (openingTagOnly) {
      out << '>';
      return;
    }

    /*
     * http://www.w3.org/TR/html/#guidelines
     * XHTML recommendation, back-wards compatibility with HTML: C.2, C.3:
     * do not use minimized forms when content is empty like <p />, and use
     * minimized forms for certain elements like <br />
     */
    if (!isSelfClosingTag(renderedType)) {
      out << '>';
      for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	childrenToAdd_[i].child->asHTML(out, javaScript, timeouts);

      out << innerHTML; // for WPushButton must be after childrenToAdd_

      out << childrenHtml_.str();

      // IE6 will incorrectly set the height of empty divs
      if (renderedType == DomElement_DIV
	  && app->environment().agent() == WEnvironment::IE6
	  && innerHTML.empty()
	  && childrenToAdd_.empty()
	  && childrenHtml_.empty())
	out << "&nbsp;";
	  if( renderedType  == DomElement_OTHER) // Custom tag name
		out << "</" << elementTagName_ << ">";
	  else
		out << "</" << elementNames_[renderedType] << ">";
    } else
      out << " />";

    if (needButtonWrap && supportButton)
      out << "</button>";
    else if (needAnchorWrap)
      out << "</a>";
  }

  javaScript << javaScriptEvenWhenDeleted_ << javaScript_;

  if (timeOut_ != -1)
    timeouts.push_back(TimeoutEvent(timeOut_, id_, timeOutJSRepeat_));

  Utils::insert(timeouts, timeouts_);
}

std::string DomElement::createVar() const
{
#ifndef WT_TARGET_JAVA
  char buf[20];
  std::sprintf(buf, "j%d", nextId_++);
  var_ = buf;
#else // !WT_TARGET_JAVA
  var_ = "j" + boost::lexical_cast<std::string>(nextId_++);
#endif // !WT_TARGET_JAVA

  return var_;
}

void DomElement::declare(EscapeOStream& out) const
{
  if (var_.empty())
    out << "var " << createVar() << "=" WT_CLASS ".$('" << id_ << "');\n";
}

bool DomElement::canWriteInnerHTML(WApplication *app) const
{
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
	  || type_ == DomElement_COLGROUP
	  || type_ == DomElement_TR
	  || type_ == DomElement_SELECT
	  || type_ == DomElement_TD
	  || type_ == DomElement_OPTGROUP))
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

void DomElement::asJavaScript(WStringStream& out)
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

void DomElement::createTimeoutJs(WStringStream& out,
				 const TimeoutList& timeouts, WApplication *app)
{
  for (unsigned i = 0; i < timeouts.size(); ++i)
    out << app->javaScriptClass()
	<< "._p_.addTimerEvent('" << timeouts[i].event << "', " 
	<< timeouts[i].msec << ","
	<< timeouts[i].repeat << ");\n";
}

void DomElement::createElement(WStringStream& out, WApplication *app,
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

  if (app->environment().agentIsIE()
      && app->environment().agent() <= WEnvironment::IE8
      && type_ != DomElement_TEXTAREA) {
    /*
     * IE pre 9 can create the entire opening tag at once.
     * This rocks because it results in fewer JavaScript statements.
     * It also avoids problems with changing certain attributes not
     * working in IE.
     *
     * However, we cannot do it for TEXTAREA since there are inconsistencies
     * with setting its value
     */
    out << "document.createElement('";
    out.pushEscape(EscapeOStream::JsStringLiteralSQuote);
    TimeoutList timeouts;
    EscapeOStream dummy;
    asHTML(out, dummy, timeouts, true);
    out.popEscape();
    out << "');";
    out << domInsertJS;
    renderInnerHtmlJS(out, app);
    renderDeferredJavaScript(out);
  } else {
    out << "document.createElement('" << elementNames_[type_] << "');";
    out << domInsertJS;
    asJavaScript(out, Create);
    asJavaScript(out, Update);
  }
}

std::string DomElement::addToParent(WStringStream& out,
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
    WStringStream insertJS;
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
    if (!javaScriptEvenWhenDeleted_.empty() || (removeAllChildren_ >= 0)) {
      out << javaScriptEvenWhenDeleted_;
      if (removeAllChildren_ >= 0) {
	declare(out);
	if (removeAllChildren_ == 0)
	  out << WT_CLASS << ".setHtml(" << var_ << ", '');\n";
	else {
	  out << "$(" << var_ << ").children(':gt(" << (removeAllChildren_ - 1)
	      << ")').remove();";
	}
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
    WApplication *app = WApplication::instance();

    bool childrenUpdated = false;

    /*
     * short-cut for frequent short manipulations
     */
    if (mode_ == ModeUpdate && numManipulations_ == 1) {
      for (unsigned i = 0; i < updatedChildren_.size(); ++i) {
	DomElement *child = updatedChildren_[i];
	child->asJavaScript(out, Update);
      }

      childrenUpdated = true;

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
      } else if (!javaScript_.empty()) {
	out << javaScript_;
	return var_;
      }
    }

    if (unwrapped_)
      out << WT_CLASS ".unwrap('" << id_ << "');\n";      

    processEvents(app);
    processProperties(app);

    if (replaced_) {
      declare(out);

      std::string varr = replaced_->createVar();
      WStringStream insertJs;
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
      WStringStream insertJs;
      insertJs << var_ << ".parentNode.insertBefore(" << varr << ","
	       << var_ + ");\n";
      insertBefore_->createElement(out, app, insertJs.str());

      return var_;
    }

    // FIXME optimize with subselect

    if (!childrenToSave_.empty()) {
      declare(out);
      out << WT_CLASS << ".saveReparented(" << var_ << ");";
    }

    for (unsigned i = 0; i < childrenToSave_.size(); ++i) {
      out << "var c" << var_ << (int)i << '='
	  << "$('#" << childrenToSave_[i] << "')";
      // In IE, contents is deleted by setting innerHTML
      if (app->environment().agentIsIE())
	out << ".detach()";
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
      out << WT_CLASS ".replaceWith('" << childrenToSave_[i] << "',c"
	  << var_ << (int)i << ");";

    // Fix for http://redmine.emweb.be/issues/1847: custom JS
    // won't find objects that still have to be moved in place
    renderDeferredJavaScript(out);

    if (!childrenUpdated)
      for (unsigned i = 0; i < updatedChildren_.size(); ++i) {
	DomElement *child = updatedChildren_[i];
	child->asJavaScript(out, Update);
      }

    return var_;
  }
  }

  return var_;
}

bool DomElement::willRenderInnerHtmlJS(WApplication *app) const
{
  /*
   * Returns whether we will (or at least can) write the
   * innerHTML with setHtml(), combining children and literal innerHTML
   */
  return !childrenHtml_.empty() || (wasEmpty_ && canWriteInnerHTML(app));
}

void DomElement::renderInnerHtmlJS(EscapeOStream& out, WApplication *app) const
{
  if (willRenderInnerHtmlJS(app)) {
    std::string innerHTML;

    if (!properties_.empty()) {
      PropertyMap::const_iterator i = properties_.find(PropertyInnerHTML);
      if (i != properties_.end()) {
	innerHTML += i->second;
      }
      i = properties_.find(PropertyAddedInnerHTML);
      if (i != properties_.end()) {
	innerHTML += i->second;
      }
    }

    /*
     * Do we actually have anything to render ?
     *   first condition: for IE6: write &nbsp; inside a empty <div></div>
     */
    if ((type_ == DomElement_DIV
	 && app->environment().agent() == WEnvironment::IE6)
	|| !childrenToAdd_.empty() || !childrenHtml_.empty()
	|| !innerHTML.empty()) {
      declare(out);

      out << WT_CLASS ".setHtml(" << var_ << ",'";

      out.pushEscape(EscapeOStream::JsStringLiteralSQuote);
      TimeoutList timeouts;
      EscapeOStream js;

      for (unsigned i = 0; i < childrenToAdd_.size(); ++i)
	childrenToAdd_[i].child->asHTML(out, js, timeouts);

      out << innerHTML;

      out << childrenHtml_.str();

      if (type_ == DomElement_DIV
	  && app->environment().agent() == WEnvironment::IE6
	  && childrenToAdd_.empty()
	  && innerHTML.empty()
	  && childrenHtml_.empty())
	out << "&nbsp;";

      out.popEscape();

      out << "');\n";

      Utils::insert(timeouts, timeouts_);

      for (unsigned i = 0; i < timeouts.size(); ++i) {
	out << app->javaScriptClass()
	    << "._p_.addTimerEvent('" << timeouts[i].event << "', " 
	    << timeouts[i].msec << ','
	    << timeouts[i].repeat << ");\n";
      }

      out << js;
    }
  } else {
    for (unsigned i = 0; i < childrenToAdd_.size(); ++i) {
      declare(out);
      DomElement *child = childrenToAdd_[i].child;
      child->addToParent(out, var_, childrenToAdd_[i].pos, app);
    }
  }

  if (timeOut_ != -1) {
    out << app->javaScriptClass() << "._p_.addTimerEvent('"
	<< id_ << "', " << timeOut_ << ','
	<< timeOutJSRepeat_ << ");\n";
  }
}

void DomElement::renderDeferredJavaScript(EscapeOStream& out) const
{
  if (!javaScript_.empty()) {
    declare(out);
    out << javaScript_ << '\n';
  }
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
      /*
       * In all cases, setJavaScriptProperties() is followed by
       * renderInnerHtmlJS() which also considers children.
       *
       * When there's 'AddedInnerHTML' then willRenderInnerHtmlJS() should
       * return false, and that's necessary since then we need to pass 'true'
       * as last argument to setHtml()
       */
      if (willRenderInnerHtmlJS(app))
	break;

      out << WT_CLASS ".setHtml(" << var_ << ',';
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      if (i->first == PropertyInnerHTML)
	out << ",false";
      else
	out << ",true";

      out << ");";

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
      if (type_ == DomElement_A) {
	if (i->second == "true")
	  out << var_ << ".setAttribute('disabled', 'disabled');";
	else
	  out << var_ << ".removeAttribute('disabled', 'disabled');";
      } else
	out << var_ << ".disabled=" << i->second << ';';
      break;
    case PropertyReadOnly:
      out << var_ << ".readOnly=" << i->second << ';';
      break;
    case PropertyTabIndex:
      out << var_ << ".tabIndex=" << i->second << ';';
      break;
    case PropertyChecked:
      out << var_ << ".checked=" << i->second << ';';
      break;
    case PropertySelected:
      out << var_ << ".selected=" << i->second << ';';
      break;
    case PropertySelectedIndex:
      out << "setTimeout(function() { "
	  << var_ << ".selectedIndex=" << i->second << ";}, 0);";
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
    case PropertyLabel:
      out << var_ << ".label=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
      break;
    case PropertyPlaceholder:
      out << var_ << ".placeholder=";
      if (!pushed) {
	escaped.pushEscape(EscapeOStream::JsStringLiteralSQuote);
	pushed = true;
      }
      fastJsStringLiteral(out, escaped, i->second);
      out << ';';
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
    case PropertyStyleFloat:
      out << var_ << ".style.";
      if (app->environment().agentIsIE())
	out << "styleFloat";
      else
	out << "cssFloat";
      out << "=\'" << i->second << "\';";
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
      if (i->first >= PropertyStyle && i->first <= PropertyStyleBoxSizing) {
	if (app->environment().agent() == WEnvironment::IE6) {
	  /*
	   * Unsupported properties, like min-height, would otherwise be
	   * ignored, but we want this information client-side. (Still, really ?)
	   */
	  out << var_ << ".style['"
	      << cssNames_[i->first - PropertyStylePosition]
	      << "']='" << i->second << "';";
	} else {
	  out << var_ << ".style."
	      << cssCamelNames_[i->first - PropertyStyle]
	      << "='" << i->second << "';";
	}
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

  for (AttributeSet::const_iterator i = removedAttributes_.begin();
      i != removedAttributes_.end(); ++i) {
    declare(out);

    out << var_ << ".removeAttribute('" << *i << "');\n";
  }
}

bool DomElement::isDefaultInline() const
{
  return isDefaultInline(type_);
}

bool DomElement::isDefaultInline(DomElementType type)
{
  return defaultInline_[type];
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

DomElementType DomElement::parseTagName(const std::string& tag)
{
  for (unsigned i = 0; i < DomElement_UNKNOWN; ++i)
    if (tag == elementNames_[i])
      return (DomElementType)i;

  return DomElement_UNKNOWN;
}

std::string DomElement::tagName(DomElementType type)
{
  return elementNames_[type];
}

const std::string& DomElement::cssName(Property property)
{
  return cssNames_[property - PropertyStylePosition];
}

void DomElement::setGlobalUnfocused(bool b)
{
  globalUnfocused_ = b;
}



}
