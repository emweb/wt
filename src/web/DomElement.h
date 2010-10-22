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
#include <sstream>
#include <string>

#include "Wt/WWebWidget"
#include "EscapeOStream.h"

namespace Wt {

class WApplication;
class EscapeOStream;
class EventSignalBase;
class WObject;

enum Property { PropertyInnerHTML, PropertyAddedInnerHTML,
		PropertyValue, PropertyDisabled,
		PropertyChecked, PropertySelected, PropertySelectedIndex,
		PropertyMultiple, PropertyTarget, PropertyIndeterminate,
		PropertySrc, PropertyText, PropertyScript,
		PropertyColSpan, PropertyRowSpan, PropertyReadOnly,
		PropertyTabIndex,
		PropertyClass,
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
		PropertyStylePaddingRight, PropertyStylePaddingLeft,
		PropertyStyleMarginTop, PropertyStyleMarginRight,
		PropertyStyleMarginBottom, PropertyStyleMarginLeft,
		PropertyStyleCursor, 
		PropertyStyleBorderTop, PropertyStyleBorderRight,
		PropertyStyleBorderBottom, PropertyStyleBorderLeft,
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
		PropertyStyleZoom,
		PropertyStyleVisibility, PropertyStyleDisplay };

class WT_API DomElement
{
public:
  enum Mode { ModeCreate, ModeUpdate };
  typedef std::map<Wt::Property, std::string> PropertyMap;

  DomElement(Mode mode, DomElementType type);
  ~DomElement();

  static std::string urlEncodeS(const std::string& url);

  Mode mode() const { return mode_; }

  void setType(DomElementType type);
  DomElementType type() const { return type_; }

  static DomElement *createNew(DomElementType type);
  static DomElement *getForUpdate(const std::string& id, DomElementType type);
  static DomElement *getForUpdate(const WObject *object, DomElementType type);
  static DomElement *updateGiven(const std::string& el, DomElementType type);

  std::string var() { return var_; }

  /*
   * General methods (for both createnew and update modes)
   */
  void setWasEmpty(bool how); // allows optimisation of addChild()
  void addChild(DomElement *child);
  void insertChildAt(DomElement *child, int pos);
  void saveChild(const std::string& id);

  void setAttribute(const std::string& attribute, const std::string& value);

  std::string getAttribute(const std::string& attribute) const;
  void removeAttribute(const std::string& attribute);

  void setProperty(Wt::Property property, const std::string& value);
  std::string getProperty(Wt::Property property) const;
  void removeProperty(Wt::Property property);
  void setProperties(const PropertyMap& properties);
  const PropertyMap& properties() const { return properties_; }
  void clearProperties();

  void setEventSignal(const char *eventName, const EventSignalBase& signal);

  void setEvent(const char *eventName,
		const std::string& jsCode,
		const std::string& signalName,
		bool isExposed = false);
  void setEvent(const char *eventName, const std::string& jsCode);

  struct EventAction
  {
    std::string jsCondition;
    std::string jsCode;
    std::string updateCmd;
    bool        exposed;

    EventAction(const std::string& jsCondition, const std::string& jsCode,
		const std::string& updateCmd, bool exposed);
  };

  void setEvent(const char * eventName,
		const std::vector<EventAction>& actions);

  void setId(const std::string& id);
  void setName(const std::string& name);
  void setTimeout(int msec, bool jsRepeat);
  void callMethod(const std::string& method);
  void callJavaScript(const std::string& javascript,
		      bool evenWhenDeleted = false);

  const std::string& id() const { return id_; }

  /*
   * only for ModeUpdate
   */
  void removeAllChildren(int firstChild = 0);
  void removeFromParent();
  void replaceWith(DomElement *newElement);
  void unstubWith(DomElement *newElement, bool hideWithDisplay);
  void insertBefore(DomElement *sibling);
  void unwrap();

  void setDiscardWithParent(bool discard);
  bool discardWithParent() const { return discardWithParent_; }

  enum Priority { Delete, Create, Update };

  struct TimeoutEvent {
    int msec;
    std::string event;
    bool repeat;

    TimeoutEvent() { }
    TimeoutEvent(int m, const std::string& e, bool r)
      : msec(m), event(e), repeat(r) { }
  };

  typedef std::vector<TimeoutEvent> TimeoutList;

  void asJavaScript(std::ostream& out);
  std::string asJavaScript(EscapeOStream& out, Priority priority) const;

  void asHTML(EscapeOStream& out, EscapeOStream& javaScript,
	      TimeoutList& timeouts, bool openingTagOnly = false) const;
  static void createTimeoutJs(std::ostream& out, const TimeoutList& timeouts,
			      WApplication *app);

  bool isDefaultInline() const;
  void declare(EscapeOStream& out) const;

  std::string cssStyle() const;

  static void fastJsStringLiteral(EscapeOStream& outRaw,
				  const EscapeOStream& outEscaped,
				  const std::string& s);
  static void jsStringLiteral(EscapeOStream& out, const std::string& s,
			      char delimiter);
  static void jsStringLiteral(std::ostream& out, const std::string& s,
			      char delimiter);
  static void fastHtmlAttributeValue(EscapeOStream& outRaw,
				     const EscapeOStream& outEscaped,
				     const std::string& s);
  static void htmlAttributeValue(std::ostream& out, const std::string& s);
  static bool isSelfClosingTag(const std::string& tag);
  static bool isSelfClosingTag(DomElementType element);

  std::string javaScript() const { return javaScript_.str(); }

  void updateInnerHtmlOnly();

  std::string addToParent(std::ostream& out, const std::string& parentVar,
			  int pos, WApplication *app);

  void createElement(std::ostream& out, WApplication *app,
		     const std::string& domInsertJS);

  std::string createVar() const;

private:
  struct EventHandler {
    std::string jsCode;
    std::string signalName;

    EventHandler() { }
    EventHandler(const std::string& j, const std::string& sn)
      : jsCode(j), signalName(sn) { }
  };

  typedef std::map<std::string, std::string> AttributeMap;
  typedef std::map<const char *, EventHandler> EventHandlerMap;

  bool canWriteInnerHTML(WApplication *app) const;
  bool containsElement(DomElementType type) const;
  void processEvents(WApplication *app) const;
  void processProperties(WApplication *app) const;
  void setJavaScriptProperties(EscapeOStream& out, WApplication *app) const;
  void setJavaScriptAttributes(EscapeOStream& out) const;
  void setJavaScriptEvent(EscapeOStream& out, const char *eventName,
			  const EventHandler& handler, WApplication *app) const;
  void createElement(EscapeOStream& out, WApplication *app,
		     const std::string& domInsertJS);
  std::string addToParent(EscapeOStream& out, const std::string& parentVar,
			  int pos, WApplication *app);
  std::string createAsJavaScript(EscapeOStream& out,
				 const std::string& parentVar, int pos,
				 WApplication *app);
  void renderInnerHtmlJS(EscapeOStream& out, WApplication *app) const;

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
  std::vector<std::string> methodCalls_;
  int          timeOut_;
  bool         timeOutJSRepeat_;
  EscapeOStream javaScript_;
  std::string  javaScriptEvenWhenDeleted_;
  mutable std::string var_;

  AttributeMap    attributes_;
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
  EscapeOStream childrenHtml_;
  TimeoutList timeouts_;

  bool discardWithParent_;

  static int nextId_;

  friend class WCssDecorationStyle;
};

}

#endif // DOMELEMENT_H_
