// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DOMELEMENT_H_
#define DOMELEMENT_H_

#include <map>
#include <vector>
#include <string>

#include "Wt/WWebWidget"
#include "EscapeOStream.h"

namespace Wt {

class WApplication;

typedef EscapeOStream EStream;

/*! \brief Enumeration for a DOM property.
 *
 * This is an internal API, subject to change.
 */
enum Property { PropertyInnerHTML, PropertyAddedInnerHTML,
		PropertyValue, PropertyDisabled,
		PropertyChecked, PropertySelected, PropertySelectedIndex,
		PropertyMultiple, PropertyTarget, PropertyDownload, PropertyIndeterminate,
		PropertySrc,
		PropertyColSpan, PropertyRowSpan, PropertyReadOnly,
		PropertyTabIndex, PropertyLabel,
		PropertyClass,
		PropertyPlaceholder,
		PropertyStyle,
		PropertyStyleWidthExpression,
		PropertyStylePosition,
		PropertyStyleZIndex, PropertyStyleFloat, PropertyStyleClear,
		PropertyStyleWidth, PropertyStyleHeight,
		PropertyStyleLineHeight,
		PropertyStyleMinWidth, PropertyStyleMinHeight,
		PropertyStyleMaxWidth, PropertyStyleMaxHeight,
		PropertyStyleLeft, PropertyStyleRight,
		PropertyStyleTop, PropertyStyleBottom,
		PropertyStyleVerticalAlign, PropertyStyleTextAlign,
		PropertyStylePadding,
		PropertyStylePaddingTop, PropertyStylePaddingRight,
		PropertyStylePaddingBottom, PropertyStylePaddingLeft,
		PropertyStyleMarginTop, PropertyStyleMarginRight,
		PropertyStyleMarginBottom, PropertyStyleMarginLeft,
		PropertyStyleCursor, 
		PropertyStyleBorderTop, PropertyStyleBorderRight,
		PropertyStyleBorderBottom, PropertyStyleBorderLeft,
		PropertyStyleBorderColorTop, PropertyStyleBorderColorRight,
		PropertyStyleBorderColorBottom, PropertyStyleBorderColorLeft,
		PropertyStyleBorderWidthTop, PropertyStyleBorderWidthRight,
		PropertyStyleBorderWidthBottom, PropertyStyleBorderWidthLeft,
		PropertyStyleColor,
		PropertyStyleOverflowX,
		PropertyStyleOverflowY,
		PropertyStyleOpacity,
		PropertyStyleFontFamily,
		PropertyStyleFontStyle,
		PropertyStyleFontVariant,
		PropertyStyleFontWeight,
		PropertyStyleFontSize,
		PropertyStyleBackgroundColor,
		PropertyStyleBackgroundImage,
		PropertyStyleBackgroundRepeat,
		PropertyStyleBackgroundAttachment,
		PropertyStyleBackgroundPosition,
		PropertyStyleTextDecoration, PropertyStyleWhiteSpace,
		PropertyStyleTableLayout, PropertyStyleBorderSpacing,
		PropertyStyleBorderCollapse,
		PropertyStylePageBreakBefore, PropertyStylePageBreakAfter,
		PropertyStyleZoom,
		PropertyStyleVisibility, PropertyStyleDisplay,

		/* CSS 3 */
		PropertyStyleBoxSizing,

		/* Keep as last, e.g. for bitset sizing. Otherwise, unused. */
		PropertyLastPlusOne };

/*! \class DomElement web/DomElement web/DomElement
 *  \brief Class to represent a client-side DOM element (proxy).
 *
 * The DOM element proxy object is used as an intermediate layer to
 * render the creation of new DOM elements or updates to existing DOM
 * elements. A DOM element can be serialized to HTML or to JavaScript
 * manipulations, and therefore is the main abstraction layer to avoid
 * hard-coding JavaScript-based rendering within the library while
 * still allowing fine-grained Ajax updates or large-scale HTML
 * changes.
 *
 * This is an internal API, subject to change.
 */
class WT_API DomElement
{
public:
  /*! \brief Enumeration for the access mode (creation or update) */
  enum Mode { ModeCreate, ModeUpdate };

#ifndef WT_TARGET_JAVA
  /*! \brief A map for property values */
  typedef std::map<Wt::Property, std::string> PropertyMap;
#else
  typedef std::treemap<Wt::Property, std::string> PropertyMap;
#endif

  /*! \brief Constructor.
   *
   * This constructs a DomElement reference, with a given mode and
   * element type. Note that even when updating an existing element,
   * the type is taken into account for information on what kind of
   * operations are allowed (workarounds for IE deficiencies for
   * examples) or to infer some basic CSS defaults for it (whether it
   * is inline or a block element).
   *
   * Typically, elements are created using one of the 'named'
   * constructors: createNew(), getForUpdate() or updateGiven().
   */
  DomElement(Mode mode, DomElementType type);

  /*! \brief Destructor.
   */
  ~DomElement();

  /*! \brief set dom element custom tag name 
   */
  void setDomElementTagName(const std::string& name);

  /*! \brief Low-level URL encoding function.
   */
  static std::string urlEncodeS(const std::string& url);

  /*! \brief Low-level URL encoding function.
   *
   * This variant allows the exclusion of certain characters from URL
   * encoding.
   */
  static std::string urlEncodeS(const std::string& url,
                                const std::string& allowed);

  /*! \brief Returns the mode.
   */
  Mode mode() const { return mode_; }
  
  /*! \brief Sets the element type.
   */
  void setType(DomElementType type);

  /*! \brief Returns the element type.
   */
  DomElementType type() const { return type_; }

  /*! \brief Creates a reference to a new element.
   */
  static DomElement *createNew(DomElementType type);

  /*! \brief Creates a reference to an existing element, using its ID.
   */
  static DomElement *getForUpdate(const std::string& id, DomElementType type);

  /*! \brief Creates a reference to an existing element, deriving the ID from
   *         an object.
   *
   * This uses object->id() as the id.
   */
  static DomElement *getForUpdate(const WObject *object, DomElementType type);

  /*! \brief Creates a reference to an existing element, using an expression
   *         to access the element.
   */
  static DomElement *updateGiven(const std::string& el, DomElementType type);

  /*! \brief Returns the JavaScript variable name.
   *
   * This variable name is only defined when the element is being
   * rendered using JavaScript, after declare() has been called.
   */
  std::string var() { return var_; }

  /*! \brief Sets whether the element was initially empty.
   *
   * Knowing that an element was empty allows optimization of
   * addChild()
   */
  void setWasEmpty(bool how);

  /*! \brief Adds a child.
   *
   * Ownership of the child is transferred to this element, and the
   * child should not be manipulated after the call, since it could be
   * that it gets directly converted into HTML and deleted.
   */
  void addChild(DomElement *child);

  /*! \brief Inserts a child.
   *
   * Ownership of the child is transferred to this element, and the child
   * should not be manipulated after the call.
   */
  void insertChildAt(DomElement *child, int pos);

  /*! \brief Saves an existing child.
   *
   * This detaches the child from the parent, allowing the
   * manipulation of the innerHTML without deleting the child. Stubs
   * in the the new HTML that reference the same id will be replaced
   * with the saved child.
   */
  void saveChild(const std::string& id);

  /*! \brief Sets an attribute value.
   */
  void setAttribute(const std::string& attribute, const std::string& value);

  /*! \brief Returns an attribute value set.
   *
   * \sa setAttribute()
   */
  std::string getAttribute(const std::string& attribute) const;

  /*! \brief Removes an attribute.
   */
  void removeAttribute(const std::string& attribute);

  /*! \brief Sets a property.
   */
  void setProperty(Wt::Property property, const std::string& value);

  /*! \brief Adds a 'word' to a property.
   *
   * This adds a word (delimited by a space) to an existing property value.
   */
  void addPropertyWord(Wt::Property property, const std::string& value);

  /*! \brief Returns a property value set.
   *
   * \sa setProperty()
   */
  std::string getProperty(Wt::Property property) const;

  /*! \brief Removes a property.
   */
  void removeProperty(Wt::Property property);

  /*! \brief Sets a whole map of properties.
   */
  void setProperties(const PropertyMap& properties);

  /*! \brief Returns all properties currently set.
   */
  const PropertyMap& properties() const { return properties_; }

  /*! \brief Clears all properties.
   */
  void clearProperties();

  /*! \brief Sets an event handler based on a signal's connections.
   */
  void setEventSignal(const char *eventName, const EventSignalBase& signal);

  /*! \brief Sets an event handler.
   *
   * This sets an event handler by a combination of client-side
   * JavaScript code and a server-side signal to emit.
   */
  void setEvent(const char *eventName,
		const std::string& jsCode,
		const std::string& signalName,
		bool isExposed = false);

  /*! \brief Sets an event handler.
   *
   * This sets a JavaScript event handler.
   */
  void setEvent(const char *eventName, const std::string& jsCode);

  /*! \brief This adds more JavaScript to an event handler.
   */
  void addEvent(const char *eventName, const std::string& jsCode);

  /*! \brief A data-structure for an aggregated event handler. */ 
  struct EventAction
  {
    std::string jsCondition;
    std::string jsCode;
    std::string updateCmd;
    bool        exposed;

    EventAction(const std::string& jsCondition, const std::string& jsCode,
		const std::string& updateCmd, bool exposed);
  };

  /*! \brief Sets an aggregated event handler. */
  void setEvent(const char * eventName,
		const std::vector<EventAction>& actions);

  /*! \brief Sets the DOM element id.
   */
  void setId(const std::string& id);

  /*! \brief Sets a DOM element name.
   */
  void setName(const std::string& name);

  /*! \brief Configures the DOM element as a source for timed events.
   */
  void setTimeout(int msec, bool jsRepeat);

  /*! \brief Calls a JavaScript method on the DOM element.
   */
  void callMethod(const std::string& method);

  /*! \brief Calls JavaScript (related to the DOM element).
   */
  void callJavaScript(const std::string& javascript,
		      bool evenWhenDeleted = false);

  /*! \brief Returns the id.
   */
  const std::string& id() const { return id_; }

  /*! \brief Removes all children.
   *
   * If firstChild != 0, then only children starting from firstChild
   * are removed.
   */
  void removeAllChildren(int firstChild = 0);

  /*! \brief Removes the element.
   */
  void removeFromParent();

  /*! \brief Replaces the element by another element.
   */
  void replaceWith(DomElement *newElement);

  /*! \brief Unstubs an element by another element.
   *
   * Stubs are used to render hidden elements initially and update
   * them in the background. This is almost the same as replaceWith()
   * except that some style properties are copied over (most
   * importantly its visibility).
   */
  void unstubWith(DomElement *newElement, bool hideWithDisplay);

  /*! \brief Inserts the element in the DOM as a new sibling.
   */
  void insertBefore(DomElement *sibling);

  /*! \brief Unwraps an element to progress to Ajax support.
   *
   * In plain HTML mode, some elements are rendered wrapped in or as
   * another element, to provide more interactivity in the absense of
   * JavaScript.
   */
  void unwrap();

  /*! \brief Enumeration for an update rendering phase.
   */
  enum Priority { Delete, Create, Update };

  /*! \brief Structure for keeping track of timers attached to this element.
   */
  struct TimeoutEvent {
    int msec;
    std::string event;
    bool repeat;

    TimeoutEvent() { }
    TimeoutEvent(int m, const std::string& e, bool r)
      : msec(m), event(e), repeat(r) { }
  };

  /*! \brief A list of timeouts.
   */
  typedef std::vector<TimeoutEvent> TimeoutList;

  /*! \brief Renders the element as JavaScript.
   */
  void asJavaScript(WStringStream& out);

  /*! \brief Renders the element as JavaScript, by phase.
   *
   * To avoid temporarily having dupliate IDs as elements move around
   * in the page, rendering is ordered in a number of phases : first
   * deleting existing elements, then creating new elements, and
   * finally updates to existing elements.
   */
  std::string asJavaScript(EStream& out, Priority priority) const;

  /*! \brief Renders the element as HTML.
   *
   * Anything that cannot be rendered as HTML is rendered as
   * javaScript as a by-product.
   */
  void asHTML(EStream& out, EStream& javaScript,
	      TimeoutList& timeouts, bool openingTagOnly = false) const;

  /*! \brief Creates the JavaScript statements for timer rendering.
   */
  static void createTimeoutJs(WStringStream& out, const TimeoutList& timeouts,
			      WApplication *app);

  /*! \brief Returns the default display property for this element.
   *
   * This returns whether the element is by default an inline or block
   * element.
   */
  bool isDefaultInline() const;

  /*! \brief Declares the element.
   *
   * Only after the element has been declared, var() returns a useful
   * JavaScript reference.
   */
  void declare(EStream& out) const;

  /*! \brief Renders properties and attributes into CSS.
   */
  std::string cssStyle() const;

  /*! \brief Utility for rapid rendering of JavaScript strings.
   *
   * It uses pre-computed mixing rules for escaping of the string.
   */
  static void fastJsStringLiteral(EStream& outRaw,
				  const EStream& outEscaped,
				  const std::string& s);

  /*! \brief Utility that renders a string as JavaScript literal.
   */
  static void jsStringLiteral(EStream& out, const std::string& s,
			      char delimiter);

  /*! \brief Utility that renders a string as JavaScript literal.
   */
  static void jsStringLiteral(WStringStream& out, const std::string& s,
			      char delimiter);

  /*! \brief Utility for rapid rendering of HTML attribute values.
   *
   * It uses pre-computed mixing rules for escaping of the attribute
   * value.
   */
  static void fastHtmlAttributeValue(EStream& outRaw,
				     const EStream& outEscaped,
				     const std::string& s);

  /*! \brief Utility that renders a string as HTML attribute.
   */
  static void htmlAttributeValue(WStringStream& out, const std::string& s);

  /*! \brief Returns whether a tag is self-closing in HTML.
   */
  static bool isSelfClosingTag(const std::string& tag);

  /*! \brief Returns whether a tag is self-closing in HTML.
   */
  static bool isSelfClosingTag(DomElementType element);

  /*! \brief Parses a tag name to a DOMElement type.
   */
  static DomElementType parseTagName(const std::string& tag);

  /*! \brief Returns the tag name for a DOMElement type.
   */
  static std::string tagName(DomElementType type);

  /*! \brief Returns the name for a CSS property, as a string.
   */
  static const std::string& cssName(Property property);

  /*! \brief Returns whether a paritcular element is by default inline.
   */
  static bool isDefaultInline(DomElementType type);

  /*! \brief Returns all custom JavaScript collected in this element.
   */
  std::string javaScript() const { return javaScript_.str(); }

  /*! \brief Something to do with broken IE Mobile 5 browsers...
   */
  void updateInnerHtmlOnly();

  /*! \brief Adds an element to a parent, using suitable methods.
   *
   * Depending on the type, different DOM methods are needed. In
   * particular for table cells, some browsers require dedicated API
   * instead of generic insertAt() or appendChild() functions.
   */
  std::string addToParent(WStringStream& out, const std::string& parentVar,
			  int pos, WApplication *app);

  /*! \brief Renders the element as JavaScript, and inserts it in the DOM.
   */
  void createElement(WStringStream& out, WApplication *app,
		     const std::string& domInsertJS);

  /*! \brief Allocates a JavaScript variable.
   */
  std::string createVar() const;

  void setGlobalUnfocused(bool b);

private:
  struct EventHandler {
    std::string jsCode;
    std::string signalName;

    EventHandler() { }
    EventHandler(const std::string& j, const std::string& sn)
      : jsCode(j), signalName(sn) { }
  };

  typedef std::map<std::string, std::string> AttributeMap;
  typedef std::set<std::string> AttributeSet;
  typedef std::map<const char *, EventHandler> EventHandlerMap;

  bool willRenderInnerHtmlJS(WApplication *app) const;
  bool canWriteInnerHTML(WApplication *app) const;
  bool containsElement(DomElementType type) const;
  void processEvents(WApplication *app) const;
  void processProperties(WApplication *app) const;
  void setJavaScriptProperties(EStream& out, WApplication *app) const;
  void setJavaScriptAttributes(EStream& out) const;
  void setJavaScriptEvent(EStream& out, const char *eventName,
			  const EventHandler& handler, WApplication *app) const;
  void createElement(EStream& out, WApplication *app,
		     const std::string& domInsertJS);
  std::string addToParent(EStream& out, const std::string& parentVar,
			  int pos, WApplication *app);
  std::string createAsJavaScript(EStream& out,
				 const std::string& parentVar, int pos,
				 WApplication *app);
  void renderInnerHtmlJS(EStream& out, WApplication *app) const;
  void renderDeferredJavaScript(EStream& out) const;

  Mode         mode_;
  bool         wasEmpty_;
  int	       removeAllChildren_;
  bool         hideWithDisplay_;
  bool         minMaxSizeProperties_;
  bool         unstubbed_;
  bool         unwrapped_;
  DomElement  *replaced_;        // when replaceWith() is called
  DomElement  *insertBefore_;
  DomElementType type_;
  std::string  id_;
  int          numManipulations_;
  int          timeOut_;
  bool         timeOutJSRepeat_;
  EStream      javaScript_;
  std::string  javaScriptEvenWhenDeleted_;
  mutable std::string var_;
  mutable bool declared_;
  bool globalUnfocused_;

  AttributeMap    attributes_;
  AttributeSet    removedAttributes_;
  PropertyMap     properties_;
  EventHandlerMap eventHandlers_;

  struct ChildInsertion {
    int pos;
    DomElement *child;

    ChildInsertion() : pos(0), child(0) { }
    ChildInsertion(int p, DomElement *c) : pos(p), child(c) { }
  };

  std::vector<ChildInsertion> childrenToAdd_;
  std::vector<std::string> childrenToSave_;
  std::vector<DomElement *> updatedChildren_;
  EStream childrenHtml_;
  TimeoutList timeouts_;
  std::string elementTagName_;

  static int nextId_;

  friend class WCssDecorationStyle;
};

}

#endif // DOMELEMENT_H_
