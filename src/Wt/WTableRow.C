/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DomElement.h"
#include "WebUtils.h"
#include "StringUtils.h"

#include <boost/algorithm/string.hpp>

#include "Wt/WTable.h"
#include "Wt/WTableCell.h"
#include "Wt/WTableRow.h"

namespace Wt {

WTableRow::WTableRow()
  : table_(nullptr)
{
  implementStateless(&WTableRow::hide, &WTableRow::undoHide);
  implementStateless(&WTableRow::show, &WTableRow::undoHide);
}

WTableRow::~WTableRow()
{ }

std::unique_ptr<WTableCell> WTableRow::createCell(int column)
{
  if (table_)
    return table_->createCell(rowNum(), column);
  else
    return std::unique_ptr<WTableCell>(new WTableCell());
}

void WTableRow::expand(int numCells)
{
  int cursize = cells_.size();
  for (int col = cursize; col < numCells; ++col) {
    cells_.push_back(createCell(col));
    WTableCell *cell = cells_.back().get();
    if (table_)
      table_->widgetAdded(cell);
    cell->row_ = this;
    cell->column_ = col;
  }
}

void WTableRow::insertColumn(int column)
{
  cells_.insert(cells_.begin() + column, createCell(column));
  WTableCell *cell = cells_[column].get();
  if (table_)
    table_->widgetAdded(cell);
  cell->row_ = this;
  cell->column_ = column;

  for (unsigned i = column; i < cells_.size(); ++i)
    cells_[i]->column_ = i;
}

std::unique_ptr<WTableCell> WTableRow::removeColumn(int column)
{
  auto result = std::move(cells_[column]);
  cells_.erase(cells_.begin() + column);

  for (unsigned i = column; i < cells_.size(); ++i)
    cells_[i]->column_ = i;

  if (table_)
    table_->widgetRemoved(result.get(), false);

  return result;
}

WTableCell *WTableRow::elementAt(int column)
{
  if (table_)
    return table_->elementAt(rowNum(), column);
  else {
    expand(column + 1);
    return cells_[column].get();
  }
}

int WTableRow::rowNum() const
{
  if (table_)
    for (unsigned i = 0; i < table_->rows_.size(); ++i)
      if (table_->rows_[i].get() == this)
        return i;

  return -1;
}

void WTableRow::setHeight(const WLength& height)
{
  height_ = height;
  if (table_)
    table_->repaintRow(this);
}

void WTableRow::setStyleClass(const WT_USTRING& style)
{
  if (WWebWidget::canOptimizeUpdates() && (style == styleClass_))
    return;

  styleClass_ = style;
  if (table_)
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
    if (table_)
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
    if (table_)
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
  return height_;
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
  setHidden(flags_.test(BIT_WAS_HIDDEN));
}

void WTableRow::setHidden(bool how)
{
  if (WWebWidget::canOptimizeUpdates() &&
      flags_.test(BIT_HIDDEN) == how)
    return;

  flags_.set(BIT_WAS_HIDDEN, flags_.test(BIT_HIDDEN));
  flags_.set(BIT_HIDDEN, how);

  flags_.set(BIT_HIDDEN_CHANGED);

  if (table_)
    table_->repaintRow(this);
}

void WTableRow::setId(const std::string& id)
{
  id_ = id;
}

const std::string WTableRow::id() const
{
  if (!id_.empty())
    return id_;
  else
    return WObject::id();
}

void WTableRow::setObjectName(const std::string& name)
{
  if (objectName() != name) {
    WObject::setObjectName(name);
    flags_.set(BIT_OBJECT_NAME_CHANGED);
    if (table_) {
      table_->repaintRow(this);
    }
  }
}

void WTableRow::updateDom(DomElement& element, bool all)
{
  if (!height_.isAuto())
    element.setProperty(Property::StyleHeight, height_.cssText());

  if (!all || !styleClass_.empty())
    element.setProperty(Property::Class, styleClass_.toUTF8());

  if ((all && flags_.test(BIT_HIDDEN)) ||
      (!all && flags_.test(BIT_HIDDEN_CHANGED))) {
    element.setProperty(Property::StyleDisplay, flags_.test(BIT_HIDDEN) ? "none" : "");
    flags_.reset(BIT_HIDDEN_CHANGED);
  }

  if (all || flags_.test(BIT_OBJECT_NAME_CHANGED)) {
    if (!objectName().empty()) {
      element.setAttribute("data-object-name", objectName());
    } else if (!all) {
      element.removeAttribute("data-object-name");
    }
    flags_.reset(BIT_OBJECT_NAME_CHANGED);
  }
}

void WTableRow::setTable(WTable *table)
{
  table_ = table;
}

}
