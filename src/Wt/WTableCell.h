// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTABLECELL_H_
#define WTABLECELL_H_

#include <Wt/WContainerWidget.h>

namespace Wt {

class WTable;
class WTableRow;

/*! \class WTableCell Wt/WTableCell.h Wt/WTableCell.h
 *  \brief A container widget that represents a cell in a table.
 *
 * A WTable provides a table of %WTableCell container widgets. A
 * %WTableCell may overspan more than one grid location in the table,
 * by specifying a \link setRowSpan() rowSpan \endlink and \link
 * setColumnSpan() columnSpan \endlink. Table cells at overspanned
 * positions are hidden. You cannot directly create a %WTableCell,
 * instead, they are created automatically by a table.
 *
 * A %WTableCell acts as any other WContainerWidget, except that both
 * the vertical and horizontal alignment of contents may be specified
 * by setContentAlignment().
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;td&gt;</tt> or
 * <tt>&lt;th&gt;</tt> tag, depending on whether the cell is a plain
 * cell or a header cell. The widget does not provide styling, and can
 * be styled using inline or external CSS as appropriate.
 *
 * \sa WTable
 */
class WT_API WTableCell : public WContainerWidget
{
public:
  /*! \brief Create a table cell.
   */
  WTableCell();

  /*! \brief Sets the row span.
   *
   * The row span indicates how many table rows this WTableCell
   * overspans. By default, a WTableCell has a row span of 1, only
   * occupying its own grid cell. A row span greater than 1 indicates
   * that table cells below this one are overspanned.
   */
  void setRowSpan(int rowSpan);
  
  /*! \brief Returns the row span.
   *
   * \sa setRowSpan(int rowSpan)
   */
  int rowSpan() const { return rowSpan_; }

  /*! \brief Sets the column span.
   *
   * The column span indicates how many table columns this WTableCell
   * overspans. By default, a WTableCell has a column span of 1, only
   * occupying its own grid cell. A column span greater than 1 indicates
   * that table cells to the right of this one are overspanned.
   */
  void setColumnSpan(int colSpan);

  /*! \brief Returns the column span.
   *
   * \sa setColumnSpan(int colSpan)
   */
  int columnSpan() const { return columnSpan_; }

  /*! \brief Returns the row index of this cell.
   */
  int row() const;

  /*! \brief Returns the column index of this cell.
   */
  int column() const { return column_; }

  /*! \brief Returns the table containing this cell.
   */
  WTable *table() const;

  /*! \brief Returns the table row containing this cell.
   */
  WTableRow *tableRow() const;

  /*! \brief Returns the table column containing this cell.
   */
  WTableColumn *tableColumn() const;

  virtual bool isVisible() const override;

private:
  WTableRow *row_;
  int  column_;
  int  rowSpan_, columnSpan_;
  bool spanChanged_;
  bool overSpanned_; // Used during rendering

  friend class WTableRow;
  friend class WTable;

protected:
  virtual void           updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void           propagateRenderOk(bool deep) override;
};

}

#endif // WTEXT_H_
