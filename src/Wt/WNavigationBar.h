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
 *
 * \note When using WBootstrap5Theme, no color schemes are applied by default.
 * You will have to add a "bg-" style class, and "navbar-light" or "navbar-dark"
 * yourself. See the
 * [Bootstrap documentation on navbar color schemes](https://getbootstrap.com/docs/5.1/components/navbar/#color-schemes)
 * for more info.
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
   *
   * \note When using WBootstrap5Theme the navigation bar is responsive by
   * default. Setting this to false has no effect. You can change the collapsing behavior
   * by setting one of the ".navbar-expand-" style classes, see the
   * [Bootstrap documentation on navbars](https://getbootstrap.com/docs/5.1/components/navbar/) for more info.
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
   *
   * \note WBootstrap5Theme ignores alignment. Use classes like "me-auto" and "ms-auto" for alignment instead.
   */
  WMenu *addMenu(std::unique_ptr<WMenu> menu,
               AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a form field to the navigation bar.
   *
   * In some cases, one may want to add a few form fields to the navigation
   * bar (e.g. for a compact login option).
   *
   * \note WBootstrap5Theme ignores alignment. Use classes like "me-auto" and "ms-auto" for alignment instead.
   */
  void addFormField(std::unique_ptr<WWidget> widget,
                    AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a search widget to the navigation bar.
   *
   * This is not so different from addFormField(), except that the
   * form field may be styled differently to indicate a search
   * function.
   *
   * \note WBootstrap5Theme ignores alignment. Use classes like "me-auto" and "ms-auto" for alignment instead.
   */
  void addSearch(std::unique_ptr<WLineEdit> field,
                 AlignmentFlag alignment = AlignmentFlag::Left);

  /*! \brief Adds a widget to the navigation bar.
   *
   * Any other widget may be added to the navigation bar, although they may
   * require special CSS style to blend well with the navigation bar style.
   *
   * \note WBootstrap5Theme ignores alignment. Use classes like "me-auto" and "ms-auto" for alignment instead.
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
  void toggleContents();
  void expandContents();
  void collapseContents();
  void undoExpandContents();

  void addWrapped(std::unique_ptr<WWidget> widget, AlignmentFlag alignment, int role);
  void align(WWidget *widget, AlignmentFlag alignment);

  bool animatedResponsive() const;
};

}

#endif // WT_WNAVIGATION_BAR_H_
