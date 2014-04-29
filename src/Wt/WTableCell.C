/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WTableCell"
#include "Wt/WTable"
#include "DomElement.h"

namespace Wt {

WTableCell::WTableCell()
  : WContainerWidget(0),
    row_(0),
    column_(0),
    rowSpan_(1),
    columnSpan_(1),
    spanChanged_(false)
{
  contentAlignment_ = AlignLeft | AlignTop;
}

WTableCell::WTableCell(WTableRow *row, int column)
  : WContainerWidget(0),
    row_(row),
    column_(column),
    rowSpan_(1),
    columnSpan_(1),
    spanChanged_(false)
{
  contentAlignment_ = AlignLeft | AlignTop;
  setParentWidget(row->table());
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
    table()->repaint(RepaintSizeAffected);
  }
}

void WTableCell::setColumnSpan(int colSpan)
{
  if (columnSpan_ != colSpan) {
    columnSpan_ = colSpan;
    row_->table()->expand(row(), column_, rowSpan_, columnSpan_);
    spanChanged_ = true;
    
    table()->flags_.set(WTable::BIT_GRID_CHANGED);
    table()->repaint(RepaintSizeAffected);
  }
}

DomElementType WTableCell::domElementType() const
{
  if (column_ < table()->headerCount(Vertical)
      || row() < table()->headerCount(Horizontal))
    return DomElement_TH;
  else
    return DomElement_TD;
}

void WTableCell::updateDom(DomElement& element, bool all)
{
  if ((all && rowSpan_ != 1) || spanChanged_)
    element.setProperty(PropertyRowSpan,
			boost::lexical_cast<std::string>(rowSpan_));

  if ((all && columnSpan_ != 1) || spanChanged_)
    element.setProperty(PropertyColSpan,
			boost::lexical_cast<std::string>(columnSpan_));

  if (row() < table()->headerCount(Horizontal))
    element.setAttribute("scope", "col");
  else if (column_ < table()->headerCount(Vertical))
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
