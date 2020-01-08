/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WTableCell.h"
#include "Wt/WTable.h"
#include "DomElement.h"

namespace Wt {

WTableCell::WTableCell()
  : row_(nullptr),
    column_(0),
    rowSpan_(1),
    columnSpan_(1),
    spanChanged_(false),
    overSpanned_(false)
{
  contentAlignment_ = AlignmentFlag::Left | AlignmentFlag::Top;
}

int WTableCell::row() const
{
  return row_->rowNum();
}

WTable *WTableCell::table() const
{
  return row_->table();
}

WTableRow *WTableCell::tableRow() const
{
  return row_;
}

WTableColumn *WTableCell::tableColumn() const
{
  return table()->columnAt(column());
}

void WTableCell::setRowSpan(int rowSpan)
{
  if (rowSpan_ != rowSpan) {
    rowSpan_ = rowSpan;
    row_->table()->expand(row(), column_, rowSpan_, columnSpan_);
    spanChanged_ = true;
    
    table()->flags_.set(WTable::BIT_GRID_CHANGED);
    table()->repaint(RepaintFlag::SizeAffected);
  }
}

void WTableCell::setColumnSpan(int colSpan)
{
  if (columnSpan_ != colSpan) {
    columnSpan_ = colSpan;
    row_->table()->expand(row(), column_, rowSpan_, columnSpan_);
    spanChanged_ = true;
    
    table()->flags_.set(WTable::BIT_GRID_CHANGED);
    table()->repaint(RepaintFlag::SizeAffected);
  }
}

DomElementType WTableCell::domElementType() const
{
  if (column_ < table()->headerCount(Orientation::Vertical)
      || row() < table()->headerCount(Orientation::Horizontal))
    return DomElementType::TH;
  else
    return DomElementType::TD;
}

void WTableCell::updateDom(DomElement& element, bool all)
{
  if ((all && rowSpan_ != 1) || spanChanged_)
    element.setProperty(Property::RowSpan, std::to_string(rowSpan_));

  if ((all && columnSpan_ != 1) || spanChanged_)
    element.setProperty(Property::ColSpan, std::to_string(columnSpan_));

  if (row() < table()->headerCount(Orientation::Horizontal))
    element.setAttribute("scope", "col");
  else if (column_ < table()->headerCount(Orientation::Vertical))
    element.setAttribute("scope", "row");

  spanChanged_ = false;

  WContainerWidget::updateDom(element, all);
}

void WTableCell::propagateRenderOk(bool deep)
{
  spanChanged_ = false;

  WContainerWidget::propagateRenderOk(deep);
}

bool WTableCell::isVisible() const
{
  if (row_)
    if (row_->isHidden())
      return false;

  return WContainerWidget::isVisible();
}

}
