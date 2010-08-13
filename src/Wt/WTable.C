/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "DomElement.h"

namespace Wt {

WTable::WTable(WContainerWidget *parent)
  : WInteractWidget(parent),
    rowsChanged_(0),
    rowsAdded_(0),
    headerRowCount_(0),
    headerColumnCount_(0)
{ 
  setInline(false);
  setIgnoreChildRemoves(true);
}

WTable::~WTable()
{
  for (unsigned i = 0; i < rows_.size(); ++i)
    delete rows_[i];

  for (unsigned i = 0; i < columns_.size(); ++i)
    delete columns_[i];

  delete rowsChanged_;
  rowsChanged_ = 0;
}

WTableCell *WTable::elementAt(int row, int column)
{
  expand(row, column, 1, 1);

  WTableRow::TableData& d = itemAt(row, column);

  return d.cell;
}

WTableRow *WTable::rowAt(int row)
{
  expand(row, 0, 1, 0);

  return rows_[row];
}

WTableColumn *WTable::columnAt(int column)
{
  expand(0, column, 0, 1);

  return columns_[column];
}

void WTable::removeCell(WTableCell *item)
{
  removeCell(item->row(), item->column());
}

void WTable::removeCell(int row, int column)
{
  WTableRow::TableData& d = itemAt(row, column);

  delete d.cell;
  d.cell = new WTableCell(rows_[row], column);
}

void WTable::expand(int row, int column, int rowSpan, int columnSpan)
{
  int newNumRows = row + rowSpan;
  int curNumColumns = columnCount();
  int newNumColumns = std::max(curNumColumns, column + columnSpan);

  if ((newNumRows > rowCount())
      || (newNumColumns > curNumColumns)) {
    if (newNumColumns == curNumColumns && rowCount() >= headerRowCount_)
      rowsAdded_ += newNumRows - rowCount();
    else
      flags_.set(BIT_GRID_CHANGED);

    repaint(RepaintInnerHtml);

    for (int r = rowCount(); r < newNumRows; ++r)
      rows_.push_back(new WTableRow(this, newNumColumns));

    if (newNumColumns > curNumColumns) {
      for (int r = 0; r < rowCount(); ++r) {
	WTableRow *tr = rows_[r];
	tr->expand(newNumColumns);
      }

      for (int c = curNumColumns; c <= column; ++c)
	columns_.push_back(new WTableColumn(this));
    }
  }

  //printDebug();
}

int WTable::rowCount() const
{
  return rows_.size();
}

int WTable::columnCount() const
{
  return columns_.size();
}

WTableRow* WTable::insertRow(int row)
{
  if (row == rowCount())
    return rowAt(row); // trigger a simple expand()
  else {
    WTableRow* tableRow = new WTableRow(this, columnCount());

    rows_.insert(rows_.begin() + row, tableRow);
    flags_.set(BIT_GRID_CHANGED);
    repaint(RepaintInnerHtml);
  
    return tableRow;
  }
}

WTableColumn* WTable::insertColumn(int column)
{
  for (unsigned i = 0; i < rows_.size(); ++i)
    rows_[i]->insertColumn(column);

  WTableColumn* tableColumn = 0;
  if ((unsigned)column <= columns_.size()) {
    tableColumn = new WTableColumn(this);
    columns_.insert(columns_.begin() + column, tableColumn);
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);

  return tableColumn;
}

void WTable::deleteRow(int row)
{
  if (rowsChanged_) {
    rowsChanged_->erase(rows_[row]);
    if (rowsChanged_->empty()) {
      delete rowsChanged_;
      rowsChanged_ = 0;
    }
  }

  for (int i = 0; i < columnCount(); ++i) {
    WTableCell *cell = rows_[row]->cells_[i].cell;
    delete cell;
  }

  if (row >= static_cast<int>(rowCount() - rowsAdded_))
    --rowsAdded_;
  else {
    flags_.set(BIT_GRID_CHANGED);
    repaint(RepaintInnerHtml);
  }

  delete rows_[row];
  rows_.erase(rows_.begin() + row);
}

void WTable::deleteColumn(int column)
{
  for (int i = 0; i < rowCount(); ++i)
    rows_[i]->deleteColumn(column);

  if ((unsigned)column <= columns_.size()) {
    delete columns_[column];
    columns_.erase(columns_.begin() + column);
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);
}

void WTable::repaintRow(WTableRow *row)
{
  if (row->rowNum() >= static_cast<int>(rowCount() - rowsAdded_))
    return;

  if (!rowsChanged_)
    rowsChanged_ = new std::set<WTableRow *>();

  rowsChanged_->insert(row);
  repaint(RepaintInnerHtml);
}

void WTable::repaintColumn(WTableColumn *column)
{
  flags_.set(BIT_COLUMNS_CHANGED);
  repaint(RepaintInnerHtml);
}

void WTable::clear()
{
  while (rowCount() > 0)
    deleteRow(rowCount() - 1);

  while (columnCount() > 0)
    deleteColumn(columnCount() - 1);
}

void WTable::setHeaderCount(int count, Orientation orientation)
{
  if (orientation == Horizontal)
    headerRowCount_ = count;
  else
    headerColumnCount_ = count;
}

int WTable::headerCount(Orientation orientation)
{
  if (orientation == Horizontal)
    return headerRowCount_;
  else
    return headerColumnCount_;
}

void WTable::updateDom(DomElement& element, bool all)
{
  WInteractWidget::updateDom(element, all);
}

void WTable::propagateRenderOk(bool deep)
{
  flags_.reset();
  if (rowsChanged_) {
    delete rowsChanged_;
    rowsChanged_ = 0;
  }

  rowsAdded_ = 0;

  WInteractWidget::propagateRenderOk(deep);
}

DomElementType WTable::domElementType() const
{
  return DomElement_TABLE;
}

DomElement *WTable::createDomElement(WApplication *app)
{
  bool withIds = !app->environment().agentIsSpiderBot();

  DomElement *table = DomElement::createNew(domElementType());
  setId(table, app);

  DomElement *thead = 0;
  if (headerRowCount_ != 0) {
    thead = DomElement::createNew(DomElement_THEAD);
    if (withIds)
      thead->setId(id() + "th");
  }

  DomElement *tbody = DomElement::createNew(DomElement_TBODY);
  if (withIds)
    tbody->setId(id() + "tb");

  for (unsigned col = 0; col < columns_.size(); ++col) {
    DomElement *c = DomElement::createNew(DomElement_COL);
    if (withIds)
      c->setId(columns_[col]->id());
    columns_[col]->updateDom(*c, true);
    table->addChild(c);
  }
  
  flags_.reset(BIT_COLUMNS_CHANGED);

  for (unsigned row = 0; row < (unsigned)rowCount(); ++row)
    for (unsigned col = 0; col < (unsigned)columnCount(); ++col)
      itemAt(row, col).overSpanned = false;
  
  for (unsigned row = 0; row < (unsigned)rowCount(); ++row) {
    DomElement *tr = createRow(row, withIds, app);
    if (row < static_cast<unsigned>(headerRowCount_))
      thead->addChild(tr);
    else
      tbody->addChild(tr);
  }
  rowsAdded_ = 0;

  if (thead)
    table->addChild(thead);
  table->addChild(tbody);

  updateDom(*table, true);

  flags_.reset(BIT_GRID_CHANGED);
  delete rowsChanged_;
  rowsChanged_ = 0;

  return table;
}

DomElement *WTable::createRow(int row, bool withIds, WApplication *app)
{
  DomElement *tr = DomElement::createNew(DomElement_TR);
  if (withIds)
    tr->setId(rows_[row]->id());
  rows_[row]->updateDom(*tr, true);

  // because of the mix of addChild() and insertChildAt()
  tr->setWasEmpty(false);

  for (int col = 0; col < columnCount(); ++col) {
    WTableRow::TableData& d = itemAt(row, col);

    if (!d.overSpanned) {
      DomElement *td = d.cell->createSDomElement(app);

      /*
       * So, IE gets confused when doing appendChild() for TH followed by
       * insertCell(-1) for TD. But, we cannot insertChild() for element 0,
       * so we do TH with appendChild, and insertCell(col).
       */
      if (col < headerColumnCount_ || row < headerRowCount_)
	tr->addChild(td);
      else
	tr->insertChildAt(td, col);

      for (int i = 0; i < d.cell->rowSpan(); ++i)
	for (int j = 0; j < d.cell->columnSpan(); ++j)
	  if (i + j > 0) {
	    itemAt(row + i, col + j).overSpanned = true;
	    itemAt(row + i, col + j).cell->setRendered(false);
	  }
    }
  }

  return tr;
}

void WTable::getDomChanges(std::vector<DomElement *>& result,
			   WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, domElementType());

  if (!isStubbed() && flags_.test(BIT_GRID_CHANGED)) {
    DomElement *newE = createDomElement(app);
    e->replaceWith(newE);
  } else {
    if (rowsChanged_) {
      for (std::set<WTableRow *>::iterator i = rowsChanged_->begin();
	   i != rowsChanged_->end(); ++i) {
	DomElement *e2 = DomElement::getForUpdate(*i, DomElement_TR);
	(*i)->updateDom(*e2, false);
	result.push_back(e2);
      }

      delete rowsChanged_;
      rowsChanged_ = 0;
    }

    if (rowsAdded_) {
      DomElement *etb = DomElement::getForUpdate(id() + "tb",
						 DomElement_TBODY);
      for (unsigned i = 0; i < static_cast<unsigned>(rowsAdded_); ++i) {
	DomElement *tr = createRow(rowCount() - rowsAdded_ + i, true, app);
	etb->addChild(tr);
      }

      result.push_back(etb);

      rowsAdded_ = 0;
    }

    if (flags_.test(BIT_COLUMNS_CHANGED)) {
	for (unsigned i = 0; i < columns_.size(); ++i) {
	  DomElement *e2
	    = DomElement::getForUpdate(columns_[i], DomElement_COL);
	  columns_[i]->updateDom(*e2, false);
	  result.push_back(e2);
	}

      flags_.reset(BIT_COLUMNS_CHANGED);
    }

    updateDom(*e, false);
  }

  result.push_back(e);
}

WTableRow::TableData& WTable::itemAt(int row, int column)
{
  return rows_[row]->cells_[column];
}

}
