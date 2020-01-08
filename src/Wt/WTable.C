/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WTable.h"
#include "Wt/WTableCell.h"
#include "Wt/WTableRow.h"
#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WTable");

WTable::WTable()
  : rowsAdded_(0),
    headerRowCount_(0),
    headerColumnCount_(0)
{ 
  setInline(false);
}

WTable::~WTable()
{
  beingDeleted();
  clear();
}

WTableCell *WTable::elementAt(int row, int column)
{
  expand(row, column, 1, 1);

  return itemAt(row, column);
}

WTableRow *WTable::rowAt(int row)
{
  expand(row, 0, 1, 0);

  return rows_[row].get();
}

WTableColumn *WTable::columnAt(int column)
{
  expand(0, column, 0, 1);

  return columns_[column].get();
}

void WTable::removeCell(WTableCell *item)
{
  removeCell(item->row(), item->column());
}

void WTable::removeCell(int row, int column)
{
  setItemAt(row, column, rows_[row]->createCell(column));
}

void WTable::expand(int row, int column, int rowSpan, int columnSpan)
{
  int curNumRows = rowCount();
  int curNumColumns = columnCount();

  int newNumRows = row + rowSpan;
  int newNumColumns = std::max(curNumColumns, column + columnSpan);

  for (int r = curNumRows; r < newNumRows; ++r)
    insertRow(r);

  for (int c = curNumColumns; c < newNumColumns; ++c)
    insertColumn(c);

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

WTableRow* WTable::insertRow(int row, std::unique_ptr<WTableRow> tableRow)
{
  if (row == rowCount() && rowCount() >= headerRowCount_)
    ++rowsAdded_;
  else
    flags_.set(BIT_GRID_CHANGED);

  if (!tableRow)
    tableRow = createRow(row);

  tableRow->setTable(this);
  for (auto &cell : tableRow->cells_) {
    widgetAdded(cell.get());
  }
  rows_.insert(rows_.begin() + row, std::move(tableRow));
  rows_[row].get()->expand(columnCount());
  repaint(RepaintFlag::SizeAffected);

  return rows_[row].get();
}

WTableColumn* WTable::insertColumn(int column,
				   std::unique_ptr<WTableColumn> tableColumn)
{
  for (unsigned i = 0; i < rows_.size(); ++i)
    rows_[i]->insertColumn(column);

  if ((unsigned)column <= columns_.size()) {
    if (!tableColumn){
      tableColumn = createColumn(column);
      tableColumn->setTable(this);
    }

    columns_.insert(columns_.begin() + column, std::move(tableColumn));
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintFlag::SizeAffected);

  return columns_[column].get();
}

std::unique_ptr<WTableRow> WTable::removeRow(int row)
{
  rowsChanged_.erase(rows_[row].get());

  if (row >= static_cast<int>(rowCount() - rowsAdded_))
    --rowsAdded_;
  else {
    flags_.set(BIT_GRID_CHANGED);
    repaint(RepaintFlag::SizeAffected);
  }

  std::unique_ptr<WTableRow> result = std::move(rows_[row]);
  rows_.erase(rows_.begin() + row);
  result->setTable(nullptr);

  for (auto &cell : result->cells_)
    widgetRemoved(cell.get(), false);

  return result;
}

std::unique_ptr<WTableColumn> WTable::removeColumn(int column)
{
  for (int i = 0; i < rowCount(); ++i)
    rows_[i]->removeColumn(column);

  std::unique_ptr<WTableColumn> result = std::move(columns_[column]);
  columns_.erase(columns_.begin() + column);
  result->setTable(nullptr);

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintFlag::SizeAffected);

  return result;
}

void WTable::repaintRow(WTableRow *row)
{
  if (row->rowNum() >= static_cast<int>(rowCount() - rowsAdded_))
    return;

  rowsChanged_.insert(row);
  repaint(RepaintFlag::SizeAffected);
}

void WTable::repaintColumn(WTableColumn *column)
{
  flags_.set(BIT_COLUMNS_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

void WTable::clear()
{
  while (rowCount() > 0)
    removeRow(rowCount() - 1);

  while (columnCount() > 0)
    removeColumn(columnCount() - 1);
}

void WTable::setHeaderCount(int count, Orientation orientation)
{
  if (orientation == Orientation::Horizontal)
    headerRowCount_ = count;
  else
    headerColumnCount_ = count;
}

int WTable::headerCount(Orientation orientation)
{
  if (orientation == Orientation::Horizontal)
    return headerRowCount_;
  else
    return headerColumnCount_;
}

std::unique_ptr<WTableCell> WTable::createCell(int row, int column)
{
  return std::unique_ptr<WTableCell>(new WTableCell());
}

std::unique_ptr<WTableRow> WTable::createRow(int row)
{
  return std::unique_ptr<WTableRow>(new WTableRow());
}

std::unique_ptr<WTableColumn> WTable::createColumn(int column)
{
  return std::unique_ptr<WTableColumn>(new WTableColumn());
}

void WTable::updateDom(DomElement& element, bool all)
{
  WInteractWidget::updateDom(element, all);
}

void WTable::propagateRenderOk(bool deep)
{
  flags_.reset();
  rowsChanged_.clear();
  rowsAdded_ = 0;

  WInteractWidget::propagateRenderOk(deep);
}

DomElementType WTable::domElementType() const
{
  return DomElementType::TABLE;
}

DomElement *WTable::createDomElement(WApplication *app)
{
  bool withIds = !app->environment().agentIsSpiderBot();

  DomElement *table = DomElement::createNew(domElementType());
  setId(table, app);

  DomElement *thead = nullptr;
  if (headerRowCount_ != 0) {
    thead = DomElement::createNew(DomElementType::THEAD);
    if (withIds)
      thead->setId(id() + "th");
  }

  DomElement *tbody = DomElement::createNew(DomElementType::TBODY);
  if (withIds)
    tbody->setId(id() + "tb");

  DomElement *colgroup = DomElement::createNew(DomElementType::COLGROUP);

  for (unsigned col = 0; col < columns_.size(); ++col) {
    DomElement *c = DomElement::createNew(DomElementType::COL);
    if (withIds)
      c->setId(columns_[col]->id());
    columns_[col]->updateDom(*c, true);
    colgroup->addChild(c);
  }

  table->addChild(colgroup);
  
  flags_.reset(BIT_COLUMNS_CHANGED);

  for (unsigned row = 0; row < (unsigned)rowCount(); ++row)
    for (unsigned col = 0; col < (unsigned)columnCount(); ++col)
      itemAt(row, col)->overSpanned_ = false;
  
  for (unsigned row = 0; row < (unsigned)rowCount(); ++row) {
    DomElement *tr = createRowDomElement(row, withIds, app);
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
  rowsChanged_.clear();

  return table;
}

DomElement *WTable::createRowDomElement(int row, bool withIds, WApplication *app)
{
  DomElement *tr = DomElement::createNew(DomElementType::TR);
  if (withIds)
    tr->setId(rows_[row]->id());
  rows_[row]->updateDom(*tr, true);

  // because of the mix of addChild() and insertChildAt()
  tr->setWasEmpty(false);
  int spanCounter = 0;

  for (int col = 0; col < columnCount(); ++col) {
    auto cell = itemAt(row, col);

    if (!cell->overSpanned_) {
      DomElement *td = cell->createSDomElement(app);

      /*
       * So, IE gets confused when doing appendChild() for TH followed by
       * insertCell(-1) for TD. But, we cannot insertChild() for element 0,
       * so we do TH with appendChild, and insertCell(col).
       */
      if (col < headerColumnCount_ || row < headerRowCount_)
	tr->addChild(td);
      else
	tr->insertChildAt(td, col - spanCounter);

      for (int i = 0; i < cell->rowSpan(); ++i)
	for (int j = 0; j < cell->columnSpan(); ++j)
	  if (i + j > 0) {
	    itemAt(row + i, col + j)->overSpanned_ = true;
	    itemAt(row + i, col + j)->setRendered(false);
	  }
    } else {
      spanCounter++;
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
    for (std::set<WTableRow *>::iterator i = rowsChanged_.begin();
	 i != rowsChanged_.end(); ++i) {
      DomElement *e2 = DomElement::getForUpdate(*i, DomElementType::TR);
      (*i)->updateDom(*e2, false);
      result.push_back(e2);
    }

    rowsChanged_.clear();

    if (rowsAdded_) {
      DomElement *etb = DomElement::getForUpdate(id() + "tb",
						 DomElementType::TBODY);
      for (unsigned i = 0; i < static_cast<unsigned>(rowsAdded_); ++i) {
        DomElement *tr = createRowDomElement(rowCount() - rowsAdded_ + i,
					     true, app);
	etb->addChild(tr);
      }

      result.push_back(etb);

      rowsAdded_ = 0;
    }

    if (flags_.test(BIT_COLUMNS_CHANGED)) {
	for (unsigned i = 0; i < columns_.size(); ++i) {
	  DomElement *e2
	    = DomElement::getForUpdate(columns_[i].get(), DomElementType::COL);
	  columns_[i]->updateDom(*e2, false);
	  result.push_back(e2);
	}

      flags_.reset(BIT_COLUMNS_CHANGED);
    }

    updateDom(*e, false);
  }

  result.push_back(e);
}

WTableCell *WTable::itemAt(int row, int column)
{
  return rows_[row]->cells_[column].get();
}

void WTable::setItemAt(int row, int column, std::unique_ptr<WTableCell> cell)
{
  rows_[row]->cells_[column] = std::move(cell);
}

void WTable::moveRow(int from, int to)
{
  if (from < 0 || from >= (int)rows_.size()) {
    LOG_ERROR("moveRow: the from index is not a valid row index.");
    return;
  }

  auto from_tr = Utils::take(rows_, rowAt(from));
  if (to > (int)rows_.size())
    rowAt(to);
  rows_.insert(rows_.begin() + to, std::move(from_tr));

  // make sure spans don't cause segmentation faults during rendering
  auto& cells = rows_[to]->cells_;
  for (unsigned i = 0; i < cells.size(); ++i) {
    if (cells[i]->rowSpan() > 1)
      rowAt(to + cells[i]->rowSpan() - 1);
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

void WTable::moveColumn(int from, int to)
{
  if (from < 0 || from >= (int)columns_.size()) {
    LOG_ERROR("moveColumn: the from index is not a valid column index.");
    return;
  }

  auto from_tc = Utils::take(columns_, columnAt(from));
  if (to > (int)columns_.size())
    columnAt(to);
  columns_.insert(columns_.begin() + to, std::move(from_tc));

  for (unsigned i = 0; i < rows_.size(); i++) {
    auto& cells = rows_[i]->cells_;
    auto cell = std::move(cells[from]);
    cells.erase(cells.begin() + from);
    cells.insert(cells.begin() + to, std::move(cell));

    // make sure spans don't cause segmentation faults during rendering
    int colSpan = cells[to]->columnSpan();
    if (colSpan > 1)
      columnAt(to + colSpan - 1);

    for (unsigned j = std::min(from, to); j < cells.size(); ++j)
      cells[j]->column_ = j;
  }

  flags_.set(BIT_GRID_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

void WTable::iterateChildren(const HandleWidgetMethod &method) const
{
  for (const auto &row : rows_) {
    for (const auto &cell : row->cells_) {
#ifndef WT_TARGET_JAVA
      method(cell.get());
#else
      method.handle(cell.get());
#endif
    }
  }
}

}
