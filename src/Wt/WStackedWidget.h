// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTACKEDWIDGET_H_
#define WSTACKEDWIDGET_H_

#include <Wt/WContainerWidget.h>

namespace Wt {

/*! \class WStackedWidget Wt/WStackedWidget.h Wt/WStackedWidget.h
 *  \brief A container widget that stacks its widgets on top of each
 *         other.
 *
 * This is a container widget which at all times has only one item
 * visible. The widget accomplishes this using setHidden(bool) on the
 * children.
 *
 * Using currentIndex() and setCurrentIndex(int index) you can
 * retrieve or set the visible widget.
 *
 * %WStackedWidget, like WContainerWidget, is by default not inline.
 *
 * <h3>CSS</h3>
 *
 * The widget is rendered using an HTML <tt>&lt;div&gt;</tt> tag and
 * does not provide styling. It can be styled using inline or external
 * CSS as appropriate.
 *
 * \sa WMenu
 */
class WT_API WStackedWidget : public WContainerWidget
{
public:
  /*! \brief Creates a new stack.
   */
  WStackedWidget();

  virtual void addWidget(std::unique_ptr<WWidget> widget) override;

  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  using WWidget::removeWidget;

  virtual std::unique_ptr<WWidget> removeWidget(WWidget* widget) override;

  /*! \brief Returns the index of the widget that is currently shown.
   *
   * \sa setCurrentIndex(), currentWidget()
   */
  int currentIndex() const;

  /*! \brief Returns the widget that is currently shown.
   *
   * \sa setCurrentWidget(), currentIndex()
   */
  WWidget *currentWidget() const;

  /*! \brief Insert a widget at a given index
   */
  virtual void insertWidget(int index, std::unique_ptr<WWidget> widget)
    override;

  /*! \brief Changes the current widget.
   *
   * The widget with index \p index is made visible, while all other
   * widgets are hidden.
   *
   * The change of current widget is done using the animation settings
   * specified by setTransitionAnimation().
   *
   * The default value for current index is 0 (provided thath 
   *
   * \sa currentIndex(), setCurrentWidget()
   */
  void setCurrentIndex(int index);

  /*! \brief Changes the current widget using a custom animation.
   *
   * \sa currentIndex(), setCurrentWidget()
   */
  void setCurrentIndex(int index, const WAnimation& animation,
		       bool autoReverse = true);

  /*! \brief Changes the current widget.
   *
   * The widget \p widget, which must have been added before, is
   * made visible, while all other widgets are hidden.
   *
   * \sa currentWidget(), setCurrentIndex()
   */
  void setCurrentWidget(WWidget *widget);

  /*! \brief Specifies an animation used during transitions.
   *
   * The animation is used to hide the previously current widget and
   * show the next current widget using setCurrentIndex().
   *
   * The initial value for \p animation is WAnimation(), specifying
   * no animation.
   *
   * When \p autoReverse is set to \c true, then the reverse animation
   * is chosen when the new index precedes the current index. This
   * only applies to AnimationEffect::SlideInFromLeft,
   * AnimationEffect::SlideInFromRight, AnimationEffect::SlideInFromTop or
   * AnimationEffect::SlideInFromBottom transition effects.
   *
   * \note If you intend to use a transition animation with a WStackedWidget
   * you should set it before it is first rendered. Otherwise, transition
   * animations caused by setCurrentIndex() may not be correctly performed.
   * If you do want to force this change you can use WApplication::processEvents
   * before calling setCurrentIndex().
   *
   * \note It is also not supported to use a AnimationEffect::Pop animation on a WStackedWidget.
   *
   *
   *
   * \sa setCurrentIndex()
   */
  void setTransitionAnimation(const WAnimation& animation,
			      bool autoReverse = false);

protected:
  virtual DomElement *createDomElement(WApplication *app) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  WAnimation animation_;
  bool autoReverseAnimation_;
  int currentIndex_;
  bool widgetsAdded_, javaScriptDefined_, loadAnimateJS_;

  void defineJavaScript();  
  void loadAnimateJS();
};

}

#endif // WSTACKEDWIDGET_H_
