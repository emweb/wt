// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCOMPOSITE_WIDGET_H_
#define WCOMPOSITE_WIDGET_H_

#include <Wt/WWidget.h>

namespace Wt {

/*! \class WCompositeWidget Wt/WCompositeWidget.h Wt/WCompositeWidget.h
 *  \brief A widget that hides the implementation of composite widgets.
 *
 * Composite widgets, built on top of the WebWidgets, should derive
 * from this class, and use setImplementation() to set the widget that
 * implements the composite widget (which is typically a WContainerWidget
 * or a WTable, or another widget that allows composition, including perhaps
 * another %WCompositeWidget).
 *
 * Using this class you can completely hide the implementation of your
 * composite widget, and provide access to only the standard WWidget
 * methods.
 *
 * \if cpp
 * Usage example:
 * \code
 * class MyWidget : public Wt::WCompositeWidget
 * {
 * public:
 *   MyWidget()
 *     : WCompositeWidget()
 *       // initialize members ...
 *   {
 *     impl_ = setImplementation(std::make_unique<Wt::WContainerWidget>());
 *
 *     // further initialization code ...
 *   }
 *
 * private:
 *   Wt::WContainerWidget *impl_;
 * };
 * \endcode
 * \endif
 */
class WT_API WCompositeWidget : public WWidget
{
public:
  /*! \brief Creates a %WCompositeWidget.
   *
   * You need to set an implemetation using setImplementation() directly
   * after construction.
   */
  WCompositeWidget();

  /*! \brief Creates a %WCompositeWidget with given implementation.
   *
   * \sa setImplementation()
   */
  WCompositeWidget(std::unique_ptr<WWidget> implementation);

  ~WCompositeWidget();

  virtual std::vector<WWidget *> children() const override;
  using WWidget::removeWidget;
  virtual std::unique_ptr<WWidget> removeWidget(WWidget *widget) override;
  virtual void setObjectName(const std::string& name) override;
  virtual std::string objectName() const override;
  virtual const std::string id() const override;

  virtual void setPositionScheme(PositionScheme scheme) override;
  virtual PositionScheme positionScheme() const override;
  virtual void setOffsets(const WLength& offset, WFlags<Side> sides = AllSides)
    override;
  virtual WLength offset(Side s) const override;
  virtual void resize(const WLength& width, const WLength& height) override;
  virtual WLength width() const override;
  virtual WLength height() const override;
  virtual void setMinimumSize(const WLength& width, const WLength& height)
    override;
  virtual WLength minimumWidth() const override;
  virtual WLength minimumHeight() const override;
  virtual void setMaximumSize(const WLength& width, const WLength& height)
    override;
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
  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation()) override;
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
  virtual void addStyleClass(const WT_USTRING& styleClass, bool force = false)
    override;
  void addStyleClass(const char *styleClass, bool force = false);
  virtual void removeStyleClass(const WT_USTRING& styleClass,
				bool force = false) override;
  void removeStyleClass(const char *styleClass, bool force = false);
  virtual bool hasStyleClass(const WT_USTRING& styleClass) const override;
  virtual void setVerticalAlignment(AlignmentFlag alignment,
				    const WLength& length = WLength::Auto)
    override;
  virtual AlignmentFlag verticalAlignment() const override;
  virtual WLength verticalAlignmentLength() const override;
  virtual WWebWidget *webWidget() override;
  virtual void setToolTip(const WString& text,
			  TextFormat textFormat = TextFormat::Plain)
    override;
  virtual WString toolTip() const override;
  virtual void setDeferredToolTip(bool enable,
                                  TextFormat textFormat = TextFormat::Plain)
    override;
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
  virtual void setCanReceiveFocus(bool enabled) override;
  virtual bool canReceiveFocus() const override;
  virtual void setFocus(bool focus) override;
  virtual bool setFirstFocus() override;
  virtual bool hasFocus() const override;
  virtual void setTabIndex(int index) override;
  virtual int tabIndex() const override;
  virtual int zIndex() const override;
  virtual void setId(const std::string& id) override;
  virtual WWidget *find(const std::string& name) override;
  virtual WWidget *findById(const std::string& name) override;
  virtual void setSelectable(bool selectable) override;
  virtual void doJavaScript(const std::string& js) override;
  virtual void propagateSetEnabled(bool enabled) override;
  virtual void propagateSetVisible(bool visible) override;

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

  virtual int baseZIndex() const final override;

protected:
  virtual void setHideWithOffsets(bool how) override;

  virtual bool isStubbed() const override;
  virtual void enableAjax() override;

  /*! \brief Set the implementation widget
   *
   * This sets the widget that implements this compositeWidget. Ownership
   * of the widget is completely transferred (including deletion).
   *
   * \note You cannot change the implementation of a composite widget after
   *       it has been rendered.
   */
  void setImplementation(std::unique_ptr<WWidget> widget);

  template <typename Widget>
    Widget *setImplementation(std::unique_ptr<Widget> widget)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    setImplementation(std::unique_ptr<WWidget>(std::move(widget)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  template <typename W, typename... Args>
  W *setNewImplementation(Args&&... args)
  {
    std::unique_ptr<W> w(new W(std::forward<Args>(args)...));
    W *result = w.get();
    setImplementation(std::move(w));
    return result;
  }
#else // WT_TARGET_JAVA
  template <typename W>
  W *setNewImplementation();
  template <typename W, typename Arg1>
  W *setNewImplementation(Arg1);
  template <typename W, typename Arg1, typename Arg2>
  W *setNewImplementation(Arg1, Arg2);
  template <typename W, typename Arg1, typename Arg2, typename Arg3>
  W *setNewImplementation(Arg1, Arg2, Arg3);
#endif // WT_TARGET_JAVA
 
  /*! \brief Get the implementation widget
   *
   * This returns the widget that implements this compositeWidget.
   */
  WWidget* implementation() { return impl_.get(); }

  std::unique_ptr<WWidget> takeImplementation();

  virtual void getSDomChanges(std::vector<DomElement *>& result,
			      WApplication *app) override;
  virtual bool needsToBeRendered() const override;

  virtual int boxPadding(Orientation orientation) const override;
  virtual int boxBorder(Orientation orientation) const override;
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  std::unique_ptr<WWidget> impl_;
};

}

#endif // WCOMPOSITE_WIDGET_H_
