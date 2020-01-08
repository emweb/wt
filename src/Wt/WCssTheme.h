// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WCSS_THEME_H
#define WT_WCSS_THEME_H

#include <Wt/WTheme.h>

namespace Wt {

/*! \class WCssTheme Wt/WCssTheme.h Wt/WCssTheme.h
 *
 * CSS-based theme support. This implements the classic %Wt themes, which
 * were available before theme support was customized with the addition of
 * the WTheme class.
 *
 * The following table shows which style classes are applied by this theme.
 *
 * <table>
 *  <tr>
 *   <td rowspan="12">WAbstractItemView *</td>
 *   <td>.Wt-itemview</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-headerdiv</td>
 *   <td>the header container</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-header</td>
 *   <td>the header</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-header .Wt-label</td>
 *   <td>a header label</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-header .Wt-label</td>
 *   <td>a header label</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-tv-rh</td>
 *   <td>resize handle</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-tv-sh</td>
 *   <td>sort handle</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-tv-sh-none</td>
 *   <td>sort handle, unsorted</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-tv-sh-down</td>
 *   <td>sort handle, descending sort</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-tv-sh-up</td>
 *   <td>sort handle, ascending sort</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-selected</td>
 *   <td>selected item (or row)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-itemview .Wt-spacer</td>
 *   <td>spacer (briefly visible during scrolling)</td>
 *  </tr>
 *  <tr>
 *   <td>WAbstractSpinBox</td>
 *   <td>.Wt-spinbox</td>
 *   <td>(for the HTML4 implementation)</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="11">WCalendar *</td>
 *   <td>.Wt-cal</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal table.d1</td>
 *   <td>the table (single letter days)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal table.d3</td>
 *   <td>the table (three letter days)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal table.dlong</td>
 *   <td>the table (ful day names)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal th.caption</td>
 *   <td>a caption cell (containing month/year navigation)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal th</td>
 *   <td>week day header cell</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal td</td>
 *   <td>day cell</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal-oom</td>
 *   <td>out-of-month day</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal-oom</td>
 *   <td>out-of-range day (ray < bottom or day > top)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal-sel</td>
 *   <td>selected day</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-cal-now</td>
 *   <td>today</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="2">WDateEdit</td>
 *   <td>.Wt-dateedit</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-datepicker</td>
 *   <td>the popup</td>
 *  </tr>
 *  <tr>
 *   <td>WDatePicker</td>
 *   <td>.Wt-datepicker</td>
 *   <td>the popup</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="5">WDialog</td>
 *   <td>.Wt-dialog</td>
 *   <td>the dialog</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-dialog .closeicon</td>
 *   <td>the close icon in the titlebar</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-dialog .titlebar</td>
 *   <td>the titlebar</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-dialog .body</td>
 *   <td>the dialog body</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-dialog .footer</td>
 *   <td>the dialog footer</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="8">WMenuItem</td>
 *   <td>.item</td>
 *   <td>an unselected item</td>
 *  </tr>
 *  <tr>
 *   <td>.itemselected</td>
 *   <td>a selected item</td>
 *  </tr>
 *  <tr>
 *   <td>.item.Wt-closable</td>
 *   <td>a closable item </td>
 *  </tr>
 *  <tr>
 *   <td>.item.Wt-separator</td>
 *   <td>a separator item</td>
 *  </tr>
 *  <tr>
 *   <td>.item.Wt-sectheader</td>
 *   <td>a section header item</td>
 *  </tr>
 *  <tr>
 *   <td>.item .Wt-icon</td>
 *   <td>the item's icon</td>
 *  </tr>
 *  <tr>
 *   <td>.item .Wt-chkbox</td>
 *   <td>the item's checkbox</td>
 *  </tr>
 *  <tr>
 *   <td>.item .closeicon</td>
 *   <td>the item's close icon</td>
 *  </tr>
 *  <tr>
 *   <td>WMessageBox</td>
 *   <td>.Wt-dialog</td>
 *   <td>see supra (%WDialog)</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="3">WPanel</td>
 *   <td>.Wt-panel</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-panel .titlebar</td>
 *   <td>the titlebar</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-panel .body</td>
 *   <td>the body</td>
 *  </tr>
 *  <tr>
 *   <td>WPopupMenu</td>
 *   <td>.Wt-popupmenu</td>
 *   <td>the popup menu; for the items, see supra (%WMenuItem)</td>
 *  </tr>
 *  <tr>
 *   <td>WPopupWidget</td>
 *   <td>.Wt-outset</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td rowspan="3">WProgressBar</td>
 *   <td>.Wt-progressbar</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-progressbar .Wt-pgb-bar</td>
 *   <td>the bar</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-progressbar .Wt-pgb-label</td>
 *   <td>the value label</td>
 *  </tr>
 *  <tr>
 *   <td>WPushButton</td>
 *   <td>.Wt-btn</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td rowspan="6">WSlider *</td>
 *   <td>.Wt-slider-h <i>or</i> .Wt-slider-v</td>
 *   <td>for horizontal or vertical slider</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-slider-[hv] .Wt-slider-bg</td>
 *   <td>background element</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-slider-[hv] .fill</td>
 *   <td>fill to the current value</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-slider-[hv] .handle</td>
 *   <td>the slider handle</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-slider-[hv] .Wt-w</td>
 *   <td>an additional element for styling</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-slider-[hv] .Wt-e</td>
 *   <td>an additional element for styling</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="3">WSuggestionPopup</td>
 *   <td>.Wt-suggest</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-suggest li</td>
 *   <td>an item</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-suggest .active</td>
 *   <td>an active item</td>
 *  </tr>
 *  <tr>
 *   <td>WTabWidget</td>
 *   <td>.Wt-tabs</td>
 *   <td>the header, which is a WMenu</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="3">WTableView *</td>
 *   <td>.Wt-tableview</td>
 *   <td>see supra (%WAbstractItemView)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tableview .Wt-contents</td>
 *   <td>the contents area</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tableview .Wt-contents .Wt-tv-c</td>
 *   <td>a contents cell</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="10">WTreeNode *</td>
 *   <td>.Wt-tree</td>
 *   <td>a tree node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree.Wt-trunk</td>
 *   <td>a trunk node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree.Wt-end</td>
 *   <td>an end node (last node within parent)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree ul</td>
 *   <td>children list</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-ctrl</td>
 *   <td>collapse/expand control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-ctrl.expand</td>
 *   <td>expand control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-ctrl.collapse</td>
 *   <td>collapse control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-ctrl.noexpand</td>
 *   <td>an item that cannot be expanded</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-selected</td>
 *   <td>a selected node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-tree .Wt-label</td>
 *   <td>the label</td>
 *  </tr>
 *  <tr>
 *   <td rowspan="10">WTreeView *</td>
 *   <td>.Wt-treeview</td>
 *   <td>see supra (%WAbstractItemView)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview ul</td>
 *   <td>a node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview ul.Wt-tv-root</td>
 *   <td>the root node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-tv-row</td>
 *   <td>a row of additional cells</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-trunk</td>
 *   <td>a trunk node</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-end</td>
 *   <td>an end node (last node within parent)</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-ctrl</td>
 *   <td>collapse/expand control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-ctrl.expand</td>
 *   <td>expand control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-ctrl.collapse</td>
 *   <td>collapse control</td>
 *  </tr>
 *  <tr>
 *   <td>.Wt-treeview .Wt-ctrl.noexpand</td>
 *   <td>an item that cannot be expanded</td>
 *  </tr>
 * </table>
 *
 * <tt>*</tt> CSS selectors for these widgets are currently still
 * hard-coded in the widget itself.
 *
 * \sa WApplication::setTheme()
 */
class WT_API WCssTheme : public WTheme
{
public:
  /*! \brief Constructor.
   *
   * Creates a classic %Wt theme. %Wt comes with two CSS themes: "polished" and
   * default. For the bootstrap theme, use WBootstrapTheme.
   */
  WCssTheme(const std::string& name);

  virtual ~WCssTheme();

  virtual std::string name() const override;

  /*! \brief Returns the stylesheets.
   *
   * Returns wt.css, plus on IE wt_ie.css, plus on IE6 wt_ie6.css. The
   * style sheets are located in the theme directory in the resources
   * folder.
   */
  virtual std::vector<WLinkedCssStyleSheet> styleSheets() const override;

  virtual void apply(WWidget *widget, WWidget *child, int widgetRole) const override;
  virtual void apply(WWidget *widget, DomElement& element, int elementRole)
    const override;

  /*! \brief Returns a generic CSS class name for a disabled element.
   *
   * The CSS class Wt-disabled is applied to disabled classes.
   */
  virtual std::string disabledClass() const override;

  /*! \brief Returns a generic CSS class name for an active element.
   *
   * The CSS class Wt-selected is applied to active classes.
   */
  virtual std::string activeClass() const override;

  virtual std::string utilityCssClass(int utilityCssClassRole) const override;

  /*! \brief Returns whether the theme allows for an anchor to be styled
   *         as a button.
   *
   * Returns false.
   */
  virtual bool canStyleAnchorAsButton() const override;

  virtual void applyValidationStyle(WWidget *widget,
                                    const Wt::WValidator::Result& validation,
                                    WFlags<ValidationStyleFlag> styles) const override;

  virtual bool canBorderBoxElement(const DomElement& element) const override;

private:
  std::string name_;
};

}

#endif // WT_WCSS_THEME_H
