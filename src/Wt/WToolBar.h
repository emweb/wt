// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WTOOLBAR_H_
#define WT_WTOOLBAR_H_

#include <vector>

#include <Wt/WCompositeWidget.h>

namespace Wt {

/*! \class WToolBar Wt/WToolBar.h Wt/WToolBar.h
 *  \brief A toolbar
 *
 * By default, a toolbar is rendered as "compact" leaving no margin
 * between buttons. By adding a separator or a split button, the
 * toolbar also supports separation between buttons.
 */
class WT_API WToolBar : public WCompositeWidget
{
public:
  /*! \brief Constructor.
   */
  WToolBar();

  /*! \brief Set vertical or horizontal orientation
   *
   * Use bootstrap btn-group-vertical style for vertical orientation.
   */
  void setOrientation(Orientation orientation);

  /*! \brief Adds a button.
   */
  void addButton(std::unique_ptr<WPushButton> button,
                 AlignmentFlag alignmentFlag = AlignmentFlag::Left);

  /*! \brief Adds a split button.
   *
   * When adding a split button, the toolbar automatically becomes
   * non-compact, since otherwise the split button functionality
   * cannot be distinguished from other buttons.
   *
   * \sa setCompact()
   */
  void addButton(std::unique_ptr<WSplitButton> button,
                 AlignmentFlag alignmentFlag = AlignmentFlag::Left);

  /*! \brief Adds a widget.
   *
   * The toolbar automatically becomes non-compact.
   */
  void addWidget(std::unique_ptr<WWidget> widget,
		 AlignmentFlag alignmentFlag = AlignmentFlag::Left);

  using WWidget::removeWidget;
  virtual std::unique_ptr<WWidget> removeWidget(WWidget *widget) override;

  /*! \brief Adds a separator.
   *
   * The toolbar automatically becomes non-compact.
   *
   * \sa setCompact()
   */
  void addSeparator();

  /*! \brief Returns the number of buttons.
   *
   * \sa widget()
   */
  int count() const;

  /*! \brief Returns a button.
   *
   * The returned widget is a WPushButton or WSplitButton added by addButton()
   * or a widget added by addWidget().
   */
  WWidget *widget(int index) const;

  /*! \brief Sets the toolbar to be rendered compact.
   *
   * The default value is \c true, but <tt>setCompact(true)</tt> is
   * called automatically when calling addButton(WSplitButton *) or
   * addSeparator().
   */
  void setCompact(bool compact);

  /*! \brief Returns whether the toolbar was rendered compact.
   *
   * \sa setCompact()
   */
  bool isCompact() const { return compact_; }

private:
  bool compact_;
  WContainerWidget *impl_;
  WContainerWidget *lastGroup_;

  WContainerWidget *lastGroup();
};

}

#endif // WT_WTOOLBAR_H_
