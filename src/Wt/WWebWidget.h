// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WWEB_WIDGET_H_
#define WWEB_WIDGET_H_

#include <set>
#include <bitset>

#include <Wt/WString.h>
#include <Wt/WWidget.h>
#include <Wt/WEvent.h>

#ifdef WT_CNOR
#include <Wt/WJavaScript.h>
#endif // WT_CNOR

namespace Wt {

class WStringStream;
class WApplication;

/*! \brief Enumeration for a DOM element type.
 *
 * For internal use only.
 */
enum class DomElementType {
  A, BR, BUTTON, COL,
  COLGROUP,
  DIV, FIELDSET, FORM,
  H1, H2, H3, H4,

  H5, H6, IFRAME, IMG,
  INPUT, LABEL, LEGEND, LI,
  OL,

  OPTION, UL, SCRIPT, SELECT,
  SPAN, TABLE, TBODY, THEAD,
  TFOOT, TH, TD, TEXTAREA,
  OPTGROUP,

  TR, P, CANVAS,
  MAP, AREA, STYLE,

  OBJECT, PARAM,
  
  AUDIO, VIDEO, SOURCE,

  B, STRONG, EM, I, HR,
  UNKNOWN,
  OTHER
};

class WCssDecorationStyle;
class WContainerWidget;
class DomElement;

#ifndef WT_CNOR
template <typename... A> class JSignal;
#endif

#ifdef WT_TARGET_JAVA
struct HandleWidgetMethod {
  HandleWidgetMethod();
  HandleWidgetMethod(void (*)(WWidget*));
  void handle(WWidget *) const;
};
#endif // WT_TARGET_JAVA

/*! \class WWebWidget Wt/WWebWidget.h Wt/WWebWidget.h
 *  \brief A base class for widgets with an HTML counterpart.
 *
 * All descendants of %WWebWidget implement a widget which corresponds
 * almost one-on-one with an HTML element. These widgets provide most
 * capabilities of these HTML elements, but rarely make no attempt to
 * do anything more.
 *
 * \sa WCompositeWidget
 */
class WT_API WWebWidget : public WWidget
{
public:
  /*! \brief Construct a WebWidget.
   */
  WWebWidget();
  virtual ~WWebWidget() override;

  virtual std::vector<WWidget *> children() const override;

  /*! \brief %Signal emitted when children have been added or removed.
   *
   * \sa children()
   */
  Signal<>& childrenChanged();

  virtual void setPositionScheme(PositionScheme scheme) override;
  virtual PositionScheme positionScheme() const override;
  virtual void setOffsets(const WLength& offset,
			  WFlags<Side> sides = AllSides) override;
  virtual WLength offset(Side s) const override;
  virtual void resize(const WLength& width, const WLength& height) override;
  virtual WLength width() const override;
  virtual WLength height() const override;
  virtual void setMinimumSize(const WLength& width, const WLength& height) override;
  virtual WLength minimumWidth() const override;
  virtual WLength minimumHeight() const override;
  virtual void setMaximumSize(const WLength& width, const WLength& height) override;
  virtual WLength maximumWidth() const override;
  virtual WLength maximumHeight() const override;
  virtual void setLineHeight(const WLength& height) override;
  virtual WLength lineHeight() const override;
  virtual void setFloatSide(Side s) override;
  virtual Side floatSide() const override;
  virtual void setClearSides(WFlags<Side> sides) override;
  virtual WFlags<Side> clearSides() const override;
  virtual void setMargin(const WLength& margin, WFlags<Side> sides = AllSides)
    override;
  virtual WLength margin(Side side) const override;
  virtual void setHiddenKeepsGeometry(bool enabled) override;
  virtual bool hiddenKeepsGeometry() const override;
  virtual void setHidden(bool hidden, const WAnimation& animation = WAnimation()) override;
  virtual bool isHidden() const override;
  virtual bool isVisible() const override;
  virtual void setDisabled(bool disabled) override;
  virtual bool isDisabled() const override;
  virtual bool isEnabled() const override;
  virtual void setPopup(bool popup) override;
  virtual bool isPopup() const override;
  virtual void setInline(bool isInline) override;
  virtual bool isInline() const override;
  virtual void setDecorationStyle(const WCssDecorationStyle& style) override;
  virtual WCssDecorationStyle& decorationStyle() override;
  virtual const WCssDecorationStyle& decorationStyle() const override;
  virtual void setStyleClass(const WT_USTRING& styleClass) override;
  void setStyleClass(const char *styleClass);
  virtual WT_USTRING styleClass() const override;
  virtual void addStyleClass(const WT_USTRING& styleClass,
			     bool force = false) override;
  void addStyleClass(const char *styleClass, bool force = false);
  virtual void removeStyleClass(const WT_USTRING& styleClass,
				bool force = false) override;
  void removeStyleClass(const char *styleClass, bool force = false);
  virtual bool hasStyleClass(const WT_USTRING& styleClass) const override;
  virtual void setVerticalAlignment(AlignmentFlag alignment,
				    const WLength& length = WLength()) override;
  virtual AlignmentFlag verticalAlignment() const override;
  virtual WLength verticalAlignmentLength() const override;
  virtual void setToolTip(const WString& text,
			  TextFormat textFormat = TextFormat::Plain)
    override;
  virtual void setDeferredToolTip(bool enable,
                                  TextFormat textFormat = TextFormat::Plain)
    override;
  virtual WString toolTip() const override;
  virtual void refresh() override;
  virtual void setAttributeValue(const std::string& name,
				 const WT_USTRING& value) override;
  virtual WT_USTRING attributeValue(const std::string& name) const override;
  virtual void setJavaScriptMember(const std::string& name,
				   const std::string& value) override;
  virtual std::string javaScriptMember(const std::string& name) const override;
  virtual void callJavaScriptMember(const std::string& name,
				    const std::string& args) override;
  virtual void load() override;
  virtual bool loaded() const override;
  virtual int zIndex() const override;

  virtual void setId(const std::string& id) override;
  virtual WWidget *find(const std::string& name) override;
  virtual WWidget *findById(const std::string& id) override;
  virtual void setSelectable(bool selectable) override;
  virtual void doJavaScript(const std::string& javascript) override;
  virtual const std::string id() const override;

#ifdef WT_TARGET_JAVA
  /*! \brief Create DOM element for widget
   *
   * This is an internal function, and should not be called directly,
   * or be overridden!
   */
#endif
  virtual DomElement *createDomElement(WApplication *app);
#ifdef WT_TARGET_JAVA
  /*! \brief Get DOM changes for this widget
   *
   * This is an internal function, and should not be called directly,
   * or be overridden!
   */
#endif
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app);
  virtual DomElementType domElementType() const = 0;

  DomElement *createStubElement(WApplication *app);
  DomElement *createActualElement(WWidget *self, WApplication *app);

  /*! \brief Change the way the widget is loaded when invisible.
   *
   * By default, invisible widgets are loaded only after visible content.
   * For tiny widgets this may lead to a performance loss, instead of the
   * expected increase, because they require many more DOM manipulations
   * to render, reducing the overall responsiveness of the application.
   *
   * Therefore, this is disabled for some widgets like WImage, or
   * empty WContainerWidgets.
   *
   * You may also want to disable deferred loading when JavaScript event
   * handling expects the widget to be loaded.
   *
   * Usually the default settings are fine, but you may want to change
   * the behaviour.
   *
   * \sa WApplication::setTwoPhaseRenderingThreshold()
   */
  void setLoadLaterWhenInvisible(bool);
  
  /*!
   * \brief returns the current html tag name
   * 
   * \sa setHtmlTagName()
   */
  std::string htmlTagName() const;

  /*!
   * \brief set the custom HTML tag name
   * 
   * The custom tag will replace the actual tag. 
   * The tag is not tested to see if
   * it is a valid one and a closing tag will always be added.
   *
   * \sa htmlTagName()
   */
  void setHtmlTagName(const std::string & tag);

  static WString escapeText(const WString& text, bool newlinesToo = false);
  static std::string& escapeText(std::string& text, bool newlinestoo = false);
  static std::string& unescapeText(std::string& text);
  static bool removeScript(WString& text);

  /*! \brief Turn a CharEncoding::UTF8 encoded string into a JavaScript string literal
   *
   * The \p delimiter may be a single or double quote.
   */
  static std::string jsStringLiteral(const std::string& v,
				     char delimiter = '\'');
  static std::string jsStringLiteral(const WString& v,
				     char delimiter = '\'');

  static std::string resolveRelativeUrl(const std::string& url);

  void setFormObject(bool how);
  static bool canOptimizeUpdates();
  void setZIndex(int zIndex);

  bool isRendered() const;

  virtual void setCanReceiveFocus(bool enabled) override;
  virtual bool canReceiveFocus() const override;
  virtual bool setFirstFocus() override;
  virtual void setFocus(bool focus) override;
  virtual bool hasFocus() const override;
  virtual void setTabIndex(int index) override;
  virtual int tabIndex() const override;

  /*! \brief %Signal emitted when the widget lost focus.
   *
   * This signals is only emitted for a widget that canReceiveFocus()
   */
  EventSignal<>& blurred();

  /*! \brief %Signal emitted when the widget recieved focus.
   *
   * This signals is only emitted for a widget that canReceiveFocus()
   */
  EventSignal<>& focussed();

#ifndef WT_TARGET_JAVA
  using WWidget::setFocus;
#endif

  virtual bool scrollVisibilityEnabled() const final override;
  virtual void setScrollVisibilityEnabled(bool enabled) final override;
  virtual int scrollVisibilityMargin() const final override;
  virtual void setScrollVisibilityMargin(int margin) final override;
  virtual Signal<bool> &scrollVisibilityChanged() final override;
  virtual bool isScrollVisible() const final override;

  virtual void setThemeStyleEnabled(bool enabled) final override;
  virtual bool isThemeStyleEnabled() const final override;

  virtual void setObjectName(const std::string& name) override;

  virtual int baseZIndex() const final override;
  void setBaseZIndex(int zIndex);

protected:
  typedef std::map<std::string, WObject *> FormObjectsMap;
#ifndef WT_TARGET_JAVA
  typedef std::function<void (WWidget *)> HandleWidgetMethod;
#endif

  void repaint(WFlags<RepaintFlag> flags = None);

  virtual void iterateChildren(const HandleWidgetMethod& method) const;
  virtual void getFormObjects(FormObjectsMap& formObjects);
  virtual void doneRerender();
  virtual void updateDom(DomElement& element, bool all);
  virtual bool domCanBeSaved() const;
  virtual void propagateRenderOk(bool deep = true);
  virtual std::string renderRemoveJs(bool recursive);

  virtual void propagateSetEnabled(bool enabled) override;
  virtual void propagateSetVisible(bool visible) override;
  virtual bool isStubbed() const override;
  virtual void enableAjax() override;

  virtual void setHideWithOffsets(bool how = true) override;
  virtual WStatelessSlot *getStateless(Method method) override;

  WWidget *selfWidget();

  void doLoad(WWidget *w);
  void widgetAdded(WWidget *child);
  void widgetRemoved(WWidget *child, bool renderRemove);

#ifndef WT_TARGET_JAVA
  template <class Widget>
  std::unique_ptr<WWidget> manageWidget(std::unique_ptr<Widget>& managed,
					std::unique_ptr<Widget> w)
#else // WT_TARGET_JAVA
  template <class Widget>
  std::unique_ptr<WWidget> manageWidget(std::unique_ptr<WWidget> managed, std::unique_ptr<Widget> w);
  std::unique_ptr<WWidget> manageWidgetImpl(std::unique_ptr<WWidget> managed,
                                            std::unique_ptr<WWidget> w)
#endif // WT_TARGET_JAVA
  {
    if (managed)
      widgetRemoved(managed.get(), true);
    std::unique_ptr<WWidget> result = std::move(managed);
    managed = std::move(w);
    if (managed)
      widgetAdded(managed.get());
    return result;
  }

  virtual void render(WFlags<RenderFlag> flags) override;

  virtual void signalConnectionsChanged() override;

  void beingDeleted();

private:
  /*
   * Booleans packed in a bitset.
   */
  static const int BIT_INLINE = 0;
  static const int BIT_HIDDEN = 1;
  static const int BIT_LOADED = 2;
  static const int BIT_RENDERED = 3;
  static const int BIT_STUBBED = 4;
  static const int BIT_FORM_OBJECT = 5;
  static const int BIT_FLEX_BOX = 6;
  static const int BIT_FLEX_BOX_CHANGED = 7;
  static const int BIT_GEOMETRY_CHANGED = 8;
  static const int BIT_HIDE_WITH_OFFSETS = 9;
  static const int BIT_BEING_DELETED = 10;
  static const int BIT_DONOT_STUB = 11;
  static const int BIT_FLOAT_SIDE_CHANGED = 12;
  static const int BIT_REPAINT_TO_AJAX = 13;
  static const int BIT_HIDE_WITH_VISIBILITY = 14;
  static const int BIT_HIDDEN_CHANGED = 15;
  static const int BIT_ENABLED = 16; // caches isEnabled() for WInteractWidget
  static const int BIT_TOOLTIP_CHANGED = 17;
  static const int BIT_MARGINS_CHANGED = 18;
  static const int BIT_STYLECLASS_CHANGED = 19;
  static const int BIT_SET_UNSELECTABLE = 20;
  static const int BIT_SET_SELECTABLE = 21;
  static const int BIT_SELECTABLE_CHANGED = 22;
  static const int BIT_WIDTH_CHANGED = 23;
  static const int BIT_HEIGHT_CHANGED = 24;
  static const int BIT_DISABLED = 25;
  static const int BIT_DISABLED_CHANGED = 26;
  static const int BIT_CONTAINS_LAYOUT = 27;
  static const int BIT_ZINDEX_CHANGED = 28;
  static const int BIT_TOOLTIP_DEFERRED = 29;
  static const int BIT_GOT_FOCUS        = 30;
  static const int BIT_TABINDEX_CHANGED = 31;
  static const int BIT_SCROLL_VISIBILITY_ENABLED = 32;
  // BIT_SCROLL_VISIBILITY_LOADED makes sure that scrollVisibility.remove is never
  // called for widgets that never had scroll visibility enabled
  static const int BIT_SCROLL_VISIBILITY_LOADED = 33;
  static const int BIT_IS_SCROLL_VISIBLE = 34; // tracks whether the widget is currently "scroll visible"
  // Tracks whether scroll visibility is enabled/disabled, and whether the
  // scroll visibility margin has been modified.
  static const int BIT_SCROLL_VISIBILITY_CHANGED = 35;
  static const int BIT_THEME_STYLE_DISABLED = 36;
  static const int BIT_OBJECT_NAME_CHANGED = 37;

  static const char *FOCUS_SIGNAL;
  static const char *BLUR_SIGNAL;

  static const int DEFAULT_BASE_Z_INDEX;

  std::string elementTagName_;

#ifndef WT_TARGET_JAVA
  static const std::bitset<38> AllChangeFlags;
#endif // WT_TARGET_JAVA

  void loadToolTip();

  /*
   * Frequently used attributes.
   */
  std::bitset<38> flags_;
  std::unique_ptr<WLength> width_;
  std::unique_ptr<WLength> height_;

  /*
   * Data only stored transiently, during event handling.
   */
  struct TransientImpl {
    std::vector<std::string> childRemoveChanges_;
    std::vector<WT_USTRING> addedStyleClasses_, removedStyleClasses_;
    std::vector<std::string> attributesSet_;

    int addedChildren_;
    bool specialChildRemove_;
    WAnimation animation_;

    TransientImpl();
    ~TransientImpl();
  };

  std::unique_ptr<TransientImpl> transientImpl_;

  struct LayoutImpl {
    PositionScheme positionScheme_;
    Side floatSide_;
    WFlags<Side> clearSides_;
    WLength offsets_[4]; // left, right, top, bottom
    WLength minimumWidth_, minimumHeight_, maximumWidth_, maximumHeight_;
    int baseZIndex_;
    int	zIndex_; // -1 = wants popup
    AlignmentFlag verticalAlignment_;
    WLength verticalAlignmentLength_, margin_[4], lineHeight_;

    LayoutImpl();
  };

  std::unique_ptr<LayoutImpl> layoutImpl_;

  struct LookImpl {
    std::unique_ptr<WCssDecorationStyle> decorationStyle_;
    WT_USTRING styleClass_;
    std::unique_ptr<WString> toolTip_;
    TextFormat toolTipTextFormat_;
    JSignal<> loadToolTip_;

    LookImpl(WWebWidget *w);
    ~LookImpl();
  };

  mutable std::unique_ptr<LookImpl> lookImpl_;

  struct DropMimeType {
    WT_USTRING hoverStyleClass;

    DropMimeType();
    DropMimeType(const WT_USTRING& hoverStyleClass);
  };

  enum class JavaScriptStatementType { 
    SetMember, 
    CallMethod,
    Statement
  };

  struct OtherImpl {
    struct Member {
      std::string name;
      std::string value;
    };

    struct JavaScriptStatement {
      JavaScriptStatement(JavaScriptStatementType type,
			  const std::string& data);

      JavaScriptStatementType type;
      std::string data;
    };

    std::unique_ptr<std::string> elementTagName_;
    std::unique_ptr<std::string> id_;
    std::unique_ptr<std::map<std::string, WT_USTRING> > attributes_;
    std::unique_ptr<std::vector<Member> > jsMembers_;
    std::unique_ptr<std::vector<JavaScriptStatement> > jsStatements_;
    std::unique_ptr<JSignal<int, int> > resized_;
    int tabIndex_;

    // drag source id, drag mime type
    std::unique_ptr<JSignal<std::string, std::string, WMouseEvent> > dropSignal_;
    std::unique_ptr<JSignal<std::string, std::string, WTouchEvent> > dropSignal2_;

    typedef std::map<std::string, DropMimeType> MimeTypesMap;
    std::unique_ptr<MimeTypesMap> acceptedDropMimeTypes_;
    Signal<> childrenChanged_;

    int scrollVisibilityMargin_;
    Signal<bool> scrollVisibilityChanged_;
    JSignal<bool> jsScrollVisibilityChanged_;

    OtherImpl(WWebWidget *const self);
    ~OtherImpl();
  };

  std::unique_ptr<OtherImpl> otherImpl_;

  void renderOk();
  void calcZIndex();

  virtual bool needsToBeRendered() const override;
  virtual void getSDomChanges(std::vector<DomElement *>& result,
			      WApplication *app) override;
  void getSFormObjects(FormObjectsMap& formObjects);

  WWebWidget *parentWebWidget() const;

  /*
   * Drag & drop stuff.
   */
  bool setAcceptDropsImpl(const std::string& mimeType,
			  bool accept,
			  const WT_USTRING& hoverStyleClass);
  void setImplementLayoutSizeAware(bool aware);
  JSignal<int, int>& resized();

  void addJavaScriptStatement(JavaScriptStatementType type,
			      const std::string& data);
  int indexOfJavaScriptMember(const std::string& name) const;
  void declareJavaScriptMember(DomElement& element,
			       const std::string& name,
			       const std::string& value);
  WString storedToolTip() const;
  void undoSetFocus();

  void jsScrollVisibilityChanged(bool visible);

  void emitChildrenChanged();

protected:
  virtual void setParentWidget(WWidget *parent) override;
  void setRendered(bool rendered);

  void setId(DomElement *element, WApplication *app);
  virtual WWebWidget *webWidget() override { return this; }

  EventSignal<> *voidEventSignal(const char *name, bool create);
  EventSignal<WKeyEvent> *keyEventSignal(const char *name, bool create);
  EventSignal<WMouseEvent> *mouseEventSignal(const char *name, bool create);
  EventSignal<WScrollEvent> *scrollEventSignal(const char *name, bool create);
  EventSignal<WTouchEvent> *touchEventSignal(const char *name, bool create);
  EventSignal<WGestureEvent> *gestureEventSignal(const char *name, bool create);

  void updateSignalConnection(DomElement& element, EventSignalBase& signal,
			      const char *name, bool all);

  virtual void parentResized(WWidget *parent, WFlags<Orientation> directions);
  void containsLayout();
  void setFlexBox(bool enabled);

  /*
   * WWebWidget ended up with more friends than me...
   */
  friend class WebRenderer;
  friend class WebSession;

  friend class WApplication;
  friend class WCompositeWidget;
  friend class WContainerWidget;
  friend class WCssDecorationStyle;
  friend class WCssTemplateRule;
  friend class WDialog;
  friend class WFont;
  friend class WGLWidget;
  friend class WInteractWidget;
  friend class WLeafletMap;
  friend class JSlot;
  friend class WTable;
  friend class WViewWidget;
  friend class WWidget;
  friend class WTemplate;
  friend class WWidgetItem;
  friend class FlexLayoutImpl;
};

}

#endif // WWEB_WIDGET_H_
