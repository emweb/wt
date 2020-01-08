// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTABLE_H_
#define WTABLE_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WTableCell.h>
#include <Wt/WTableColumn.h>
#include <Wt/WTableRow.h>

namespace Wt {

/*! \class WTable Wt/WTable.h Wt/WTable.h
 *  \brief A container widget which provides layout of children in a table grid.
 *
 * A %WTable arranges its children in a table.
 *
 * To insert or access contents, use elementAt(int row, int column) to
 * access the \link WTableCell cell\endlink at a particular location
 * in the table. The %WTable expands automatically to create the indexed
 * (row, column) as necessary.
 *
 * It is possible to insert and delete entire rows or columns from the
 * table using the insertColumn(int column), insertRow(int row),
 * deleteColumn(int column), or deleteRow(int row) methods.
 *
 * You may indicate a number of rows and columns that act as headers
 * using setHeaderCount(). Header cells are rendered as
 * <tt>&lt;th&gt;</tt> instead of <tt>&lt;td&gt;</tt> elements. By
 * default, no rows or columns are configured as headers.
 *
 * %WTable is displayed as a \link WWidget::setInline(bool) block\endlink.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WTable *table = addWidget(std::make_unique<Wt::WTable>());
 * table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Item @ row 0, column 0"));
 * table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Item @ row 0, column 1"));
 * table->elementAt(1, 0)->addWidget(std::make_unique<Wt::WText>("Item @ row 1, column 0"));
 * table->elementAt(1, 1)->addWidget(std::make_unique<Wt::WText>("Item @ row 1, column 1"));
 *
 * Wt::WTableCell *cell = table->elementAt(2, 0);
 * cell->addWidget(std::make_unique<Wt::WText>("Item @ row 2"));
 * cell->setColumnSpan(2);
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;table&gt;</tt> tag and
 * does not provide styling. It can be styled using inline or external
 * CSS as appropriate.
 *
 * \sa WTableCell, WTableRow, WTableColumn
 */
class WT_API WTable : public WInteractWidget
{
public:
  /*! \brief Creates an empty table.
   */
  WTable();

  /*! \brief Deletes the table and its entire contents.
   */
  virtual ~WTable();

  /*! \brief Accesses the table element at the given row and column.
   *
   * If the row/column is beyond the current table dimensions, then
   * the table is expanded automatically.
   */
  WTableCell *elementAt(int row, int column);

  /*! \brief Returns the row object for the given row.
   *
   * Like with elementAt(), the table expands automatically when the row
   * is beyond the current table dimensions.
   *
   * \sa elementAt(int, int), columnAt(int)
   */
  WTableRow *rowAt(int row);

  /*! \brief Returns the column object for the given column.
   *
   * Like with elementAt(), the table expands automatically when the
   * column is beyond the current table dimensions.
   *
   * \sa elementAt(int, int), rowAt(int)
   */
  WTableColumn *columnAt(int column);

  /*! \brief Deletes a table cell and its contents.
   *
   * The table cell at that position is recreated.
   *
   * \sa removeCell(int, int)
   */
  void removeCell(WTableCell *item);

  /*! \brief Deletes the table cell at the given position.
   *
   * \sa removeCell(WTableCell *)
   */
  virtual void removeCell(int row, int column);

  /*! \brief Inserts a row.
   */
  virtual WTableRow* insertRow
    (int row, std::unique_ptr<WTableRow> tableRow = nullptr);

  /*! \brief Removes a row.
   *
   * Rows below the given row are shifted up. Returns a WTableRow that is
   * not associated with a WTable. Unlinke removeColumn(), the cells in the
   * row will not be deleted, because they are owned by the WTableRow.
   */
  virtual std::unique_ptr<WTableRow> removeRow(int row);

  /*! \brief Inserts an empty column.
   */
  virtual WTableColumn* insertColumn
    (int column, std::unique_ptr<WTableColumn> tableColumn = nullptr);

  /*! \brief Remove a column and all its contents.
   *
   * The contents of the column will be deleted, because a WTableColumn
   * does not own its cells.
   */
  virtual std::unique_ptr<WTableColumn> removeColumn(int column);

  /*! \brief Clears the entire table.
   *
   * This method clears the entire table: all cells and their contents
   * are deleted.
   */
  virtual void clear();

  /*! \brief Returns the number of rows in the table.
   */
  int rowCount() const;

  /*! \brief Returns the number of columns in the table.
   */
  int columnCount() const;

  /*! \brief Sets the number of header rows or columns.
   *
   * The default values are 0.
   *
   * \note This must be set before the initial rendering and cannot
   *       be changed later.
   */
  void setHeaderCount(int count, 
		      Orientation orientation = Orientation::Horizontal);

  /*! \brief Returns the number of header rows or columns.
   *
   * \sa setHeaderCount()
   */
  int headerCount(Orientation orientation = Orientation::Horizontal);

  /*! \brief Move a table row from its original position to a new position.
   *
   * The table expands automatically when the \p to row is beyond the
   * current table dimensions.
   *
   * \sa moveColumn()
   */
  virtual void moveRow(int from, int to);

  /*! \brief Move a table column from its original position to a new position.
   *
   * The table expands automatically when the \p to column is beyond
   * the current table dimensions.
   *
   * \sa moveRow()
   */
  virtual void moveColumn(int from, int to);

private:
  static const int BIT_GRID_CHANGED = 0;
  static const int BIT_COLUMNS_CHANGED = 1;

  std::bitset<3> flags_;

  std::vector<std::unique_ptr<WTableRow> > rows_;
  std::vector<std::unique_ptr<WTableColumn> > columns_;
  std::set<WTableRow *> rowsChanged_;
  unsigned rowsAdded_;
  int headerRowCount_, headerColumnCount_;

  void expand(int row, int column, int rowSpan, int columnSpan);
  WTableCell *itemAt(int row, int column);
  void setItemAt(int row, int column, std::unique_ptr<WTableCell> cell);

  void repaintRow(WTableRow *row);
  void repaintColumn(WTableColumn *col);

  friend class WTableCell;
  friend class WTableColumn;
  friend class WTableRow;

protected:
  /*! \brief Creates a table cell.
   *
   * You may want to override this method if you want your table to
   * contain specialized cells.
   */
  virtual std::unique_ptr<WTableCell> createCell(int row, int column);

  /*! \brief Creates a table row.
   *
   * You may want to override this method if you want your table to
   * contain specialized rows.
   */
  virtual std::unique_ptr<WTableRow> createRow(int row);

  /*! \brief Creates a table column.
   *
   * You may want to override this method if you want your table to
   * contain specialized columns.
   */
  virtual std::unique_ptr<WTableColumn> createColumn(int column);

  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual DomElement *createDomElement(WApplication *app) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual void propagateRenderOk(bool deep) override;

  virtual void iterateChildren(const HandleWidgetMethod &method) const override;

private:
  DomElement *createRowDomElement(int row, bool withIds, WApplication *app);
};

}

#endif // WTABLE_H_
