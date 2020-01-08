// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTABLE_ROW_H_
#define WTABLE_ROW_H_

#include <Wt/WObject.h>
#include <Wt/WString.h>

namespace Wt {

class DomElement;
class WTable;
class WTableCell;

/*! \class WTableRow Wt/WTableRow.h Wt/WTableRow.h
 *  \brief A table row.
 *
 * A %WTableRow is returned by WTable::rowAt() and managing various
 * properties of a single row in a table (it is however not a widget).
 *
 * A table row corresponds to the HTML <tt>&lt;tr&gt;</tt> tag.
 *
 * \sa WTable, WTableColumn
 */
class WT_API WTableRow : public WObject 
{
public:
  /*! \brief Creates a new table row.
   *
   * Table rows must be added to a table using WTable::insertRow()
   * before you can access contents in it using elementAt().
   */
  WTableRow();

  /*! \brief Destructor.
   */
  virtual ~WTableRow();

  /*! \brief Returns the table to which this row belongs.
   *
   * \sa WTable::rowAt()
   */
  WTable *table() const { return table_; }

  /*! \brief Access the row element at the given column.
   *
   * Like WTable::elementAt(), if the column is beyond the current
   * table dimensions, then the table is expanded automatically.
   *
   * The row must be inserted within a table first.
   */
  WTableCell *elementAt(int column);

  /*! \brief Returns the row number of this row in the table.
   *
   * Returns -1 if the row is not yet part of a table.
   *
   * \sa WTable::rowAt()
   */
  int rowNum() const;

  /*! \brief Sets the row height.
   *
   * The default row height is WLength::Auto.
   *
   * \sa height(), WWidget::resize()
   */
  void setHeight(const WLength& height);

  /*! \brief Returns the row height.
   *
   * \sa setHeight(const WLength&)
   */
  WLength height() const;

  /*! \brief Sets the CSS style class for this row.
   *
   * The style is inherited by all table cells in this row.
   *
   * \sa styleClass(), WWidget::setStyleClass()
   */
  void setStyleClass(const WT_USTRING& style);

  /*! \brief Returns the CSS style class for this row.
   *
   * \sa styleClass(), WWidget::styleClass()
   */
  const WT_USTRING& styleClass() const { return styleClass_; }

  void addStyleClass(const WT_USTRING& styleClass);
  void removeStyleClass(const WT_USTRING& styleClass);
  void toggleStyleClass(const WT_USTRING& styleClass, bool add);

  /*! \brief Sets whether the row must be hidden.
   *
   * Hide or show the row.
   *
   * The default value is \c false (row is not hidden).
   *
   * \sa hide(), show()
   */
  void setHidden(bool how);

  /*! \brief Returns whether the rows is hidden.
   *
   * \sa setHidden()
   */
  bool isHidden() const { return hidden_; }

  /*! \brief Hides the row.
   *
   * \sa setHidden()
   */
  void hide();

  /*! \brief Shows the row.
   *
   * \sa setHidden()
   */
  void show();

  /*! \brief Sets the CSS Id.
   *
   * Sets a custom Id. Note that the Id must be unique across the whole
   * widget tree, can only be set right after construction and cannot
   * be changed.
   *
   * \sa WObject::id()
   */
  void setId(const std::string& id);

  virtual const std::string id() const override;

protected:
  virtual std::unique_ptr<WTableCell> createCell(int column);

private:
  void expand(int numCells);

  WTable *table_;
  std::vector<std::unique_ptr<WTableCell> > cells_;

  WLength height_;
  std::string id_;
  WT_USTRING styleClass_;
  bool hidden_, hiddenChanged_, wasHidden_;

  void updateDom(DomElement& element, bool all);
  void setTable(WTable *table);
  void insertColumn(int column);
  std::unique_ptr<WTableCell> removeColumn(int column);

  void undoHide();

  friend class WTable;
};

}

#endif // WTABLE_ROW_H_
