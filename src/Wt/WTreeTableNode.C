/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget"
#include "Wt/WTemplate"
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

void WTreeTableNode::insertChildNode(int index, WTreeNode *node)
{
  WTreeNode::insertChildNode(index, node);

  if (table_)
    (dynamic_cast<WTreeTableNode *>(node))->setTable(table_);
}

void WTreeTableNode::setColumnWidget(int column, WWidget *widget)
{
  --column;

  createExtraColumns(column);

  if (column < (int)columnWidgets_.size()) {
    delete columnWidgets_[column].widget;
    columnWidgets_[column] = ColumnWidget(widget, true);
  } else {
    columnWidgets_.push_back(ColumnWidget(widget, true));
  }

  widget->setInline(false);
  widget->setFloatSide(Left);
  widget->resize(columnWidth(column + 1), WLength::Auto);
  if (column == static_cast<int>(columnWidgets_.size()) - 1)
    row_->addWidget(widget);
  else
    row_->insertBefore(widget, columnWidgets_[column + 1].widget);
}

void WTreeTableNode::createExtraColumns(int numColumns)
{
  if (!row_) {
    row_ = new WContainerWidget();
    row_->addStyleClass("cols-row");
    impl()->bindWidget("cols-row", row_);
  }

  while (static_cast<int>(columnWidgets_.size()) < numColumns) {
    WText *w = new WText(WString::fromUTF8(" "), row_);
    w->setInline(false);
    columnWidgets_.push_back(ColumnWidget(w, false));
    w->setFloatSide(Left);
    w->resize(columnWidth(columnWidgets_.size()), 1);
  }
}

WWidget *WTreeTableNode::columnWidget(int column)
{
  --column;

  return
    (column < static_cast<int>(columnWidgets_.size())
     && columnWidgets_[column].isSet)
    ? columnWidgets_[column].widget
    : 0;
}

WLength WTreeTableNode::columnWidth(int column)
{
  if (table_)
    return table_->columnWidth(column);
  else
    return WLength::Auto;
}

void WTreeTableNode::setTable(WTreeTable *table)
{
  if (table_ != table) {
    table_ = table;

    for (unsigned i = 0; i < childNodes().size(); ++i)
      dynamic_cast<WTreeTableNode *>(childNodes()[i])->setTable(table);

    createExtraColumns(table->columnCount() - 1);

    for (unsigned i = 0; i < columnWidgets_.size(); ++i) {
      WWidget *w = columnWidgets_[i].widget;
      w->resize(columnWidth(i + 1), w->height());
    }
  }
}

}
