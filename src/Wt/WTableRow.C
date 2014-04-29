/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DomElement.h"
#include "WebUtils.h"

#include <boost/algorithm/string.hpp>

#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WTableRow"

namespace Wt {

WTableRow::WTableRow()
  : height_(0),
    id_(0),
    hidden_(false),
    hiddenChanged_(false)
{ 
  implementStateless(&WTableRow::hide, &WTableRow::undoHide);
  implementStateless(&WTableRow::show, &WTableRow::undoHide);
}

WTableRow::~WTableRow()
{
  delete height_;
  delete id_;
}

WTableRow::TableData::TableData()
  : cell(0),
    overSpanned(false)
{ }

WTableCell *WTableRow::createCell(int column)
{
  return table_->createCell(rowNum(), column);
}

void WTableRow::expand(int numCells)
{
  int cursize = cells_.size();

  for (int col = cursize; col < numCells; ++col) {
    cells_.push_back(TableData());
    WTableCell *cell = createCell(col);
    cell->row_ = this;
    cell->column_ = col;
    cell->setParentWidget(table_);
    cells_.back().cell = cell;
  }
}

void WTableRow::insertColumn(int column)
{
  cells_.insert(cells_.begin() + column, TableData());
  WTableCell *cell = createCell(column);
  cell->row_ = this;
  cell->column_ = column;
  cell->setParentWidget(table_);
  cells_.back().cell = cell;

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

WTableCell *WTableRow::elementAt(int column)
{
  return table_->elementAt(rowNum(), column);
}

int WTableRow::rowNum() const
{
  return Utils::indexOf(table_->rows_, const_cast<WTableRow *>(this));
}

void WTableRow::setHeight(const WLength& height)
{
#ifndef WT_TARGET_JAVA
  if (!height_)
    height_ = new WLength(height);
  else
#endif
    *height_ = height;

  table_->repaintRow(this);
}

void WTableRow::setStyleClass(const WT_USTRING& style)
{
  if (WWebWidget::canOptimizeUpdates() && (style == styleClass_))
    return;

  styleClass_ = style;
  table_->repaintRow(this);
}

void WTableRow::addStyleClass(const WT_USTRING& style)
{
  std::string currentClass = styleClass_.toUTF8();
  Utils::SplitSet classes;
  Utils::split(classes, currentClass, " ", true);
  
  if (classes.find(style.toUTF8()) == classes.end()) {
    styleClass_ = WT_USTRING::fromUTF8(Utils::addWord(styleClass_.toUTF8(),
						      style.toUTF8()));
    table_->repaintRow(this);
  }
}

void WTableRow::removeStyleClass(const WT_USTRING& style)
{
  std::string currentClass = styleClass_.toUTF8();
  Utils::SplitSet classes;
  Utils::split(classes, currentClass, " ", true);

  if (classes.find(style.toUTF8()) != classes.end()) {
    styleClass_ = WT_USTRING::fromUTF8(Utils::eraseWord(styleClass_.toUTF8(),
							style.toUTF8()));
    table_->repaintRow(this);
  }
}

void WTableRow::toggleStyleClass(const WT_USTRING& style, bool add)
{
  if (add)
    addStyleClass(style);
  else
    removeStyleClass(style);
}

WLength WTableRow::height() const
{
  return height_ ? *height_ : WLength::Auto;
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

void WTableRow::setId(const std::string& id)
{
  if (!id_)
    id_ = new std::string();

  *id_ = id;
}

const std::string WTableRow::id() const
{
  if (id_)
    return *id_;
  else
    return WObject::id();
}

void WTableRow::updateDom(DomElement& element, bool all)
{
  if (height_)
    element.setProperty(PropertyStyleHeight, height_->cssText());

  if (!all || !styleClass_.empty())
    element.setProperty(PropertyClass, styleClass_.toUTF8());

  if ((all && hidden_) || (!all && hiddenChanged_)) {
    element.setProperty(PropertyStyleDisplay, hidden_ ? "none" : "");
    hiddenChanged_ = false;
  }
}

}
