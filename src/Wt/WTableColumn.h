// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTABLE_COLUMN_H_
#define WTABLE_COLUMN_H_

#include <Wt/WObject.h>
#include <Wt/WString.h>

namespace Wt {

class DomElement;
class WLength;
class WTable;

/*! \class WTableColumn Wt/WTableColumn.h Wt/WTableColumn.h
 *  \brief A table column.
 *
 * A %WTableColumn is returned by WTable::columnAt() and managing
 * various properties of a single column in a table (it is however not
 * a widget).
 *
 * A table column corresponds to the HTML <tt>&lt;col&gt;</tt> tag.
 *
 * \sa WTable, WTableRow
 */
class WT_API WTableColumn : public WObject 
{
public:
  /*! \brief Creates a new table column.
   *
   * Table columns must be added to a table using WTable::insertColumn()
   * before you can access contents in it using elementAt().
   */
  WTableColumn();

  /*! \brief Destructor.
   */
  ~WTableColumn();

  /*! \brief Returns the table to which this column belongs.
   *
   * \sa WTable::rowAt()
   */
  WTable *table() const { return table_; }

  /*! \brief Access the column element at the given row.
   *
   * Like WTable::elementAt(), if the row is beyond the current table
   * dimensions, then the table is expanded automatically.
   *
   * The column must be inserted within a table first.
   */
  WTableCell *elementAt(int row);

  /*! \brief Returns the column number of this column in the table.
   *
   * Returns -1 if the column is not yet part of a table.
   *
   * \sa WTable::columnAt()
   */
  int columnNum() const;

   /*! \brief Sets the column width.
   *
   * The default column width is WLength::Auto.
   *
   * \sa width(), WWidget::resize()
   */
  void setWidth(const WLength& width);

  /*! \brief Returns the column width.
   *
   * \sa setWidth(const WLength&)
   */
  WLength width() const;

  /*! \brief Sets the CSS style class for this column.
   *
   * The style is inherited by all table cells in this column.
   *
   * \sa styleClass(), WWidget::setStyleClass()
   */
  void setStyleClass(const WT_USTRING& style);

  /*! \brief Returns the CSS style class for this column.
   *
   * \sa styleClass(), WWidget::styleClass()
   */
  const WT_USTRING& styleClass() const { return styleClass_; }

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

private:
  WTable *table_;

  std::unique_ptr<WLength> width_;
  std::unique_ptr<std::string> id_;
  WT_USTRING styleClass_;

  void setTable(WTable *table);
  void updateDom(DomElement& element, bool all);

  friend class WTable;
};

}

#endif // WTABLE_COLUMN_H_
