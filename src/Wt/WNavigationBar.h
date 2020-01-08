// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WNAVIGATION_BAR_H_
#define WT_WNAVIGATION_BAR_H_

#include <vector>

#include <Wt/WLink.h>
#include <Wt/WTemplate.h>

namespace Wt {

class WStackedWidget;
class WTable;

/*! \class WNavigationBar Wt/WNavigationBar.h Wt/WNavigationBar.h
 *  \brief A navigation bar
 *
 * \note WNavigationBar is currently only styled in the Bootstrap themes.
 */
class WT_API WNavigationBar : public WTemplate
{
public:
  /*! \brief Constructor.
   */
  WNavigationBar();

  /*! \brief Sets a title.
   *
   * The title may optionally link to a 'homepage'.
   */
  void setTitle(const WString& title, const WLink& link = WLink());

  /*! \brief Sets whether the navigation bar will respond to screen size.
   *
   * For screens that are less wide, the navigation bar can be
   * rendered different (more compact and allowing for vertical menu
   * layouts).
   */
  void setResponsive(bool responsive);

  /*! \brief Adds a menu to the navigation bar.
   *
   * Typically, a navigation bar will contain at least one menu which
   * implements the top-level navigation options allowed by the
   * navigation bar.
   *
   * The menu may be aligned to the left or to the right of the
   * navigation bar.
   */
  WMenu *addMenu(std::unique_ptr<WMenu> menu,
               AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a form field to the navigation bar.
   *
   * In some cases, one may want to add a few form fields to the navigation
   * bar (e.g. for a compact login option).
   */
  void addFormField(std::unique_ptr<WWidget> widget,
		    AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a search widget to the navigation bar.
   *
   * This is not so different from addFormField(), except that the
   * form field may be styled differently to indicate a search
   * function.
   */
  void addSearch(std::unique_ptr<WLineEdit> field,
		 AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a widget to the navigation bar.
   *
   * Any other widget may be added to the navigation bar, although they may
   * require special CSS style to blend well with the navigation bar style.
   */
  void addWidget(std::unique_ptr<WWidget> widget,
		 AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a widget to the navigation bar, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Widget *result = widget.get();
   * addWidget(std::unique_ptr<WWidget>(std::move(widget)));
   * return result;
   * \endcode
   */
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

protected:
  std::unique_ptr<WInteractWidget> createCollapseButton();
  std::unique_ptr<WInteractWidget> createExpandButton();

private:
  void expandContents();
  void collapseContents();
  void undoExpandContents();

  void addWrapped(std::unique_ptr<WWidget> widget, AlignmentFlag alignment,
		  const char *wrapClass);
  void addWrapped(std::unique_ptr<WWidget> widget, WWidget* parent, int role,
                  AlignmentFlag alignment);
  void align(WWidget *widget, AlignmentFlag alignment);

  bool animatedResponsive() const;
};

}

#endif // WT_WNAVIGATION_BAR_H_
