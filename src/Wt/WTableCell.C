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

WTableCell::WTableCell(WTableRow *row, int column)
  : WContainerWidget(0),
    row_(row),
    column_(column),
    rowSpan_(1),
    columnSpan_(1),
    spanChanged_(false)
{
  setParent(row->table());
}

int WTableCell::row() const
{
  return row_->rowNum();
}

WTable *WTableCell::table() const
{
  return row_->table();
}

void WTableCell::setRowSpan(int rowSpan)
{
  if (rowSpan_ != rowSpan) {
    rowSpan_ = rowSpan;
    row_->table()->expand(row(), column_, rowSpan_, columnSpan_);
    spanChanged_ = true;
    repaint(RepaintPropertyAttribute);
  }
}

void WTableCell::setColumnSpan(int colSpan)
{
  if (columnSpan_ != colSpan) {
    columnSpan_ = colSpan;
    row_->table()->expand(row(), column_, rowSpan_, columnSpan_);
    spanChanged_ = true;
    repaint(RepaintPropertyAttribute);
  }
}

void WTableCell::setContentAlignment(int contentAlignment)
{
  HorizontalAlignment h = (HorizontalAlignment)(contentAlignment & 0x0F);
  VerticalAlignment v = (VerticalAlignment)(contentAlignment & 0xF0);
  WContainerWidget::setContentAlignment(h);
  WContainerWidget::setVerticalAlignment(v);
}

int WTableCell::contentAlignment() const
{
  return WContainerWidget::contentAlignment() |
    verticalAlignment();
}

DomElementType WTableCell::domElementType() const
{
  return DomElement_TD;
}

void WTableCell::updateDom(DomElement& element, bool all)
{
  if ((all && rowSpan_ != 1) || spanChanged_)
    element.setAttribute("rowspan",
			 boost::lexical_cast<std::string>(rowSpan_));

  if ((all && columnSpan_ != 1) || spanChanged_)
    element.setAttribute("colspan",
			 boost::lexical_cast<std::string>(columnSpan_));

  spanChanged_ = false;

  WContainerWidget::updateDom(element, all);
}

}
