/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget"
#include "Wt/WIconPair"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WTreeTable"
#include "Wt/WTreeTableNode"

namespace Wt {

WTreeTableNode::WTreeTableNode(const WString& labelText,
			       WIconPair *labelIcon,
			       WTreeTableNode *parentNode)
  : WTreeNode(labelText, labelIcon),
    table_(0),
    row_(0)
{ 
  if (parentNode)
    parentNode->addChildNode(this);
}

void WTreeTableNode::addChildNode(WTreeNode *node)
{
  WTreeNode::addChildNode(node);

  if (table_)
    (dynamic_cast<WTreeTableNode *>(node))->setTable(table_);
}

void WTreeTableNode::setColumnWidget(int column, WWidget *widget)
{
  --column;

  createExtraColumns(column);

  if (column < (int)columnWidgets_.size()) {
    delete columnWidgets_[column].first;
    columnWidgets_[column] = std::make_pair(widget, true);
  } else {
    columnWidgets_.push_back(std::make_pair(widget, true));
  }

  widget->setInline(false);
  widget->setFloatSide(Left);
  widget->resize(columnWidth(column + 1), WLength());
  if (column == static_cast<int>(columnWidgets_.size()) - 1)
    row_->addWidget(widget);
  else
    row_->insertBefore(widget, columnWidgets_[column + 1].first);
}

void WTreeTableNode::createExtraColumns(int numColumns)
{
  if (!row_) {
    row_ = new WContainerWidget();
    labelArea()->insertBefore(row_, labelArea()->children()[0]);
    row_->setFloatSide(Right);
    labelArea()->resize(WLength(100, WLength::Percentage), WLength());
    labelArea()->table()->resize(WLength(100, WLength::Percentage), WLength());
  }

  while (static_cast<int>(columnWidgets_.size()) < numColumns) {
    WText *w = new WText(false, "&nbsp;", row_);
    columnWidgets_.push_back(std::make_pair(w, false));
    w->setFloatSide(Left);
    w->resize(columnWidth(columnWidgets_.size()), WLength());
  }
}

WWidget *WTreeTableNode::columnWidget(int column)
{
  --column;

  return
    (column < static_cast<int>(columnWidgets_.size())
     && columnWidgets_[column].second)
    ? columnWidgets_[column].first
    : 0;
}

WLength WTreeTableNode::columnWidth(int column)
{
  if (table_)
    return table_->columnWidth(column);
  else
    return WLength();
}

void WTreeTableNode::setTable(WTreeTable *table)
{
  if (table_ != table) {
    table_ = table;

    for (unsigned i = 0; i < childNodes().size(); ++i)
      dynamic_cast<WTreeTableNode *>(childNodes()[i])->setTable(table);

    createExtraColumns(table->numColumns() - 1);

    for (unsigned i = 0; i < columnWidgets_.size(); ++i)
      columnWidgets_[i].first->resize(columnWidth(i + 1), WLength());
  }
}

}
