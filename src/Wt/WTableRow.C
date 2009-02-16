/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "DomElement.h"
#include "Utils.h"

#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WTableRow"

namespace Wt {

WTableRow::WTableRow(WTable *table, int numCells)
  : table_(table),
    height_(0),
    hidden_(false)
{ 
  expand(numCells);

  implementStateless(&WTableRow::hide, &WTableRow::undoHide);
  implementStateless(&WTableRow::show, &WTableRow::undoHide);
}

WTableRow::~WTableRow()
{
  delete height_;
}

WTableRow::TableData::TableData()
  : cell(0),
    overSpanned(false)
{ }

void WTableRow::expand(int numCells)
{
  int cursize = cells_.size();

  for (int col = cursize; col < numCells; ++col) {
    cells_.push_back(TableData());
    cells_.back().cell = new WTableCell(this, col);
  }
}

void WTableRow::insertColumn(int column)
{
  cells_.insert(cells_.begin() + column, TableData());
  cells_[column].cell = new WTableCell(this, column);

  for (unsigned i = column; i < cells_.size(); ++i)
    cells_[i].cell->column_ = i;
}

void WTableRow::deleteColumn(int column)
{
  delete cells_[column].cell;
  cells_.erase(cells_.begin() + column);

  for (unsigned i = column; i < cells_.size(); ++i)
    cells_[i].cell->column_ = i;
}

int WTableRow::rowNum() const
{
  return Utils::indexOf(table_->rows_, const_cast<WTableRow *>(this));
}

void WTableRow::setHeight(const WLength& height)
{
  if (!height_)
    height_ = new WLength(height);
  else
    *height_ = height;

  table_->repaintRow(this);
}

void WTableRow::setStyleClass(const WString& style)
{
  styleClass_ = style;

  table_->repaintRow(this);
}

WLength WTableRow::height() const
{
  return height_ ? *height_ : WLength();
}

void WTableRow::hide()
{
  setHidden(true);
}

void WTableRow::show()
{
  setHidden(false);
}

void WTableRow::undoHide()
{
  setHidden(wasHidden_);
}

void WTableRow::setHidden(bool how)
{
  if (WWebWidget::canOptimizeUpdates() && hidden_ == how)
    return;

  wasHidden_ = hidden_;
  hidden_ = how;
  hiddenChanged_ = true;

  table_->repaintRow(this);
}

void WTableRow::updateDom(DomElement& element, bool all)
{
  if (height_)
    element.setProperty(PropertyStyleHeight, height_->cssText());

  if (!all || !styleClass_.empty())
    element.setAttribute("class", styleClass_.toUTF8());

  if (all || hiddenChanged_) {
    element.setProperty(PropertyStyleDisplay, hidden_ ? "none" : "");
    hiddenChanged_ = false;
  }
}

}
