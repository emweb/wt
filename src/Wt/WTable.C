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
    columns_(0),
    rowsChanged_(0),
    rowsAdded_(0)
{ 
  setInline(false);
  setIgnoreChildRemoves(true);
}

WTable::~WTable()
{
  for (unsigned i = 0; i < rows_.size(); ++i)
    delete rows_[i];

  if (columns_) {
    for (unsigned i = 0; i < columns_->size(); ++i)
      delete (*columns_)[i];

    delete columns_;
  }
  delete rowsChanged_;
  rowsChanged_ = 0;
}

void WTable::printDebug()
{
  std::cerr << "Table: "
	    << formName() << " " << rowCount() << "x" << columnCount()
	    << std::endl;
  
  for (int i = 0; i < rowCount(); ++i) {
    for (int j = 0; j < columnCount(); ++j) {
      std::cerr << "(" << i << "," << j << "): "
		<< itemAt(i, j).cell << std::endl;
    }
  }
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

  if (!columns_)
    columns_ = new std::vector<WTableColumn *>();

  if (columns_->size() <= (unsigned)column) {
    for (unsigned col = columns_->size(); col <= (unsigned)column; ++col)
      columns_->push_back(new WTableColumn(this));
  }

  return (*columns_)[column];
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
    if (newNumColumns == curNumColumns)
      rowsAdded_ += newNumRows - rowCount();
    else
      flags_.set(BIT_GRID_CHANGED);

    repaint(RepaintInnerHtml);

    for (int row = rowCount(); row < newNumRows; ++row) {
      rows_.push_back(new WTableRow(this, newNumColumns));
    }

    if (newNumColumns > curNumColumns) {
      for (int row = 0; row < rowCount(); ++row) {
	WTableRow *tr = rows_[row];
	tr->expand(newNumColumns);
      }
    }
  }

  //printDebug();
}

int WTable::numRows() const
{
  return rowCount();
}

int WTable::rowCount() const
{
  return rows_.size();
}

int WTable::numColumns() const
{
  return columnCount();
}

int WTable::columnCount() const
{
  return rows_.size() > 0 ? rows_[0]->cells_.size() : 0;
}

void WTable::insertRow(int row)
{
  rows_.insert(rows_.begin() + row, new WTableRow(this, columnCount()));
  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);
}

void WTable::insertColumn(int column)
{
  for (unsigned i = 0; i < rows_.size(); ++i) {
    rows_[i]->insertColumn(column);
  }

  if (columns_ && (unsigned)column <= columns_->size())
    columns_->insert(columns_->begin() + column, new WTableColumn(this));

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);
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

  for (int i = 0; i < columnCount(); ++i)
    delete rows_[row]->cells_[i].cell;

  delete rows_[row];
  rows_.erase(rows_.begin() + row);

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);
}

void WTable::deleteColumn(int column)
{
  for (int i = 0; i < rowCount(); ++i)
    rows_[i]->deleteColumn(column);

  if (columns_ && (unsigned)column <= columns_->size()) {
    delete (*columns_)[column];
    columns_->erase(columns_->begin() + column);
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintInnerHtml);
}

void WTable::repaintRow(WTableRow *row)
{
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
}

void WTable::updateDom(DomElement& element, bool all)
{
  WInteractWidget::updateDom(element, all);
}

DomElementType WTable::domElementType() const
{
  return DomElement_TABLE;
}

DomElement *WTable::createDomElement(WApplication *app)
{
  DomElement *table = DomElement::createNew(domElementType());
  setId(table, app);
  DomElement *tbody = DomElement::createNew(DomElement_TBODY);
  tbody->setId(formName() + "tb");

  bool withIds = !app->environment().agentIsSpiderBot();

  if (columns_) {
    for (unsigned col = 0; col < columns_->size(); ++col) {
      DomElement *c = DomElement::createNew(DomElement_COL);
      if (withIds)
	c->setId((*columns_)[col]);
      (*columns_)[col]->updateDom(*c, true);
      tbody->addChild(c);
    }

    flags_.reset(BIT_COLUMNS_CHANGED);
  }

  for (unsigned row = 0; row < (unsigned)rowCount(); ++row)
    for (unsigned col = 0; col < (unsigned)columnCount(); ++col)
      itemAt(row, col).overSpanned = false;
  
  for (unsigned row = 0; row < (unsigned)rowCount(); ++row) {
    DomElement *tr = createRow(row, withIds, app);
    tbody->addChild(tr);
  }
  rowsAdded_ = 0;

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
    tr->setId(rows_[row]);
  rows_[row]->updateDom(*tr, true);

  for (unsigned col = 0; col < (unsigned)columnCount(); ++col) {
    WTableRow::TableData& d = itemAt(row, col);

    if (!d.overSpanned) {
      DomElement *td = d.cell->createSDomElement(app);
      tr->addChild(td);

      for (int i = 0; i < d.cell->rowSpan(); ++i)
	for (int j = 0; j < d.cell->columnSpan(); ++j)
	  if (i + j > 0)
	    itemAt(row + i, col + j).overSpanned = true;
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
    e->replaceWith(newE, true);
  } else {
    if (rowsChanged_) {
      for (std::set<WTableRow *>::iterator i = rowsChanged_->begin();
	   i != rowsChanged_->end(); ++i) {
	DomElement *e = DomElement::getForUpdate(*i, DomElement_TR);
	(*i)->updateDom(*e, false);
	result.push_back(e);
      }

      delete rowsChanged_;
      rowsChanged_ = 0;
    }

    if (rowsAdded_) {
      DomElement *etb = DomElement::getForUpdate(formName() + "tb",
						 DomElement_TBODY);
      for (unsigned i = 0; i < rowsAdded_; ++i) {
	DomElement *tr = createRow(rowCount() - rowsAdded_ + i, true, app);
	etb->addChild(tr);
      }

      result.push_back(etb);

      rowsAdded_ = 0;
    }

    if (flags_.test(BIT_COLUMNS_CHANGED)) {
      if (columns_) {
	for (unsigned i = 0; i < columns_->size(); ++i) {
	  DomElement *e
	    = DomElement::getForUpdate((*columns_)[i], DomElement_COL);
	  (*columns_)[i]->updateDom(*e, false);
	  result.push_back(e);
	}
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
