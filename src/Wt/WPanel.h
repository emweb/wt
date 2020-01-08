// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPANEL_H_
#define WPANEL_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {

  class WContainerWidget;
  class WIconPair;
  class WTemplate;
  class WText;

/*! \class WPanel Wt/WPanel.h Wt/WPanel.h
 *  \brief A %WPanel provides a container with a title bar.
 *
 * The panel provides a container with an optional title bar, and an
 * optional collapse icon.
 *
 * \if cpp
 * Usage example:
 * \code
 * auto panel = std::make_unique<Wt::WPanel>();
 * panel->setTitle("A panel");
 * panel->setCentralWidget(std::make_unique<Wt::WText>("This is the panel contents"));
 * panel->setCollapsible(true);
 * \endcode
 * \endif
 *
 * \image html WPanel-default-1.png "Two panels: one collapsed and one expanded (default theme)"
 * \image html WPanel-polished-1.png "Two panels: one collapsed and one expanded (polished theme)"
 */
class WT_API WPanel : public WCompositeWidget
{
public:
  /*! \brief Creates a panel.
   */
  WPanel();

  /*! \brief Sets a title.
   *
   * The panel title is set in the title bar. This method also makes
   * the title bar visible by calling setTitleBar(true).
   *
   * The default value is "" (no title).
   *
   * \sa title(), setTitleBar(bool)
   */
  void setTitle(const WString& title);

  /*! \brief Returns the title.
   *
   * \sa setTitle(const WString&)
   */
  WString title() const;

  /*! \brief Shows or hides the title bar for the panel.
   *
   * The title bar appears at the top of the panel.
   *
   * The default value is \c false: the title bar is not shown unless a
   * title is set or the panel is made collapsible.
   *
   * \sa setTitle(const WString&), setCollapsible(bool)
   */
  void setTitleBar(bool enable);

  /*! \brief Returns if a title bar is set.
   *
   * \sa setTitleBar(bool)
   */
  bool titleBar() const;

  /*! \brief Returns the title bar widget.
   *
   * The title bar widget contains the collapse/expand icon (if the
   * panel isCollapsible()), and the title text (if a title was set
   * using setTitle()). You can access the title bar widget to customize
   * the contents of the title.
   *
   * The method returns \c 0 if titleBar() is \p false. You need to call
   * setTitleBar() first.
   *
   * \sa setTitleBar()
   */
  WContainerWidget *titleBarWidget() const;

  /*! \brief Makes the panel collapsible.
   *
   * When \p on is \c true, a collapse/expand icon is added to the
   * title bar. This also calls setTitleBar(true) to enable the
   * title bar.
   *
   * The default value is \c false.
   *
   * \sa setTitleBar(bool), setCollapsed(bool), isCollapsed()
   */
  void setCollapsible(bool on);

  /*! \brief Returns if the panel can be collapsed by the user.
   *
   * \sa setCollapsible(bool)
   */
  bool isCollapsible() const { return collapseIcon_ != nullptr; } 

  /*! \brief Sets the panel expanded or collapsed.
   *
   * When \p on is \c true, equivalent to collapse(), otherwise to
   * expand().
   *
   * The default value is \c false.
   *
   * \sa setCollapsible(bool)
   */
  void setCollapsed(bool on);

  /*! \brief Returns if the panel is collapsed.
   *
   * \sa setCollapsed(bool)
   * \sa collapsed(), expanded()
   */
  bool isCollapsed() const;

  /*! \brief Collapses the panel.
   *
   * When isCollapsible() is true, the panel is collapsed to minimize
   * screen real-estate.
   *
   * \sa setCollapsible(bool), expand()
   */
  void collapse();

  /*! \brief Collapses the panel.
   *
   * When isCollapsible() is true, the panel is expanded to its original
   * state.
   *
   * \sa setCollapsible(bool), expand()
   */
  void expand();

  /*! \brief Sets an animation.
   *
   * The animation is used when collapsing or expanding the panel.
   */
  void setAnimation(const WAnimation& transition);

  /*! \brief Sets the central widget.
   *
   * Sets the widget that is the contents of the panel.
   *
   * The default value is \c 0 (no widget set).
   */
  void setCentralWidget(std::unique_ptr<WWidget> widget);

  template <typename Widget>
    Widget *setCentralWidget(std::unique_ptr<Widget> widget)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    setCentralWidget(std::unique_ptr<WWidget>(std::move(widget)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  /*! \brief Returns the central widget.
   *
   * \sa setCentralWidget()
   */
  WWidget *centralWidget() const { return centralWidget_; }

  /*! \brief %Signal emitted when the panel is collapsed.
   *
   * %Signal emitted when the panel is collapsed. The signal is only
   * emitted when the panel is collapsed by the user using the
   * collapse icon in the tible bar, not when calling
   * setCollapsed(bool).
   *
   * \sa expanded()
   */
  Signal<>& collapsed() { return collapsed_; }

  /*! \brief %Signal emitted when the panel is expanded.
   *
   * %Signal emitted when the panel is expanded. The signal is only
   * emitted when the panel is expanded by the user using the expand
   * icon in the title bar, not when calling setCollapsed(bool).
   *
   * \sa collapsed()
   */
  Signal<>& expanded() { return expanded_; }

  Signal<bool>& collapsedSS() { return collapsedSS_; }
  Signal<bool>& expandedSS() { return expandedSS_; }

  WIconPair *collapseIcon() const { return collapseIcon_; }

private:
  WIconPair *collapseIcon_;
  WText *title_;

  WTemplate *impl_;
  WWidget *centralWidget_;
  WAnimation animation_;

  Signal<> collapsed_, expanded_;
  Signal<bool> collapsedSS_, expandedSS_;

  bool wasCollapsed_;

  void setJsSize();
  void toggleCollapse();
  void doExpand();
  void doCollapse();
  void undoExpand();
  void undoCollapse();

  virtual void onExpand();
  virtual void onCollapse();

  WContainerWidget *centralArea() const;
};

}

#endif // WPANEL_H_
