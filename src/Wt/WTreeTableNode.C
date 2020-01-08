/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget.h"
#include "Wt/WIconPair.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "Wt/WTreeTable.h"
#include "Wt/WTreeTableNode.h"

namespace Wt {

WTreeTableNode::WTreeTableNode(const WString& labelText,
			       std::unique_ptr<WIconPair> labelIcon)
  : WTreeNode(labelText, std::move(labelIcon)),
    table_(nullptr),
    row_(nullptr)
{ }

void WTreeTableNode::insertChildNode(int index,
				     std::unique_ptr<WTreeNode> node)
{
  if (table_)
    (dynamic_cast<WTreeTableNode *>(node.get()))->setTable(table_);

  WTreeNode::insertChildNode(index, std::move(node));
}

void WTreeTableNode::setColumnWidget(int column,
				     std::unique_ptr<WWidget> widget)
{
  --column;

  createExtraColumns(column);

  if (column < (int)columnWidgets_.size()) {
    if (columnWidgets_[column].widget)
      columnWidgets_[column].widget->removeFromParent();
    columnWidgets_[column] = ColumnWidget(widget.get(), true);
  } else {
    columnWidgets_.push_back(ColumnWidget(widget.get(), true));
  }

  widget->setInline(false);
  widget->setFloatSide(Side::Left);
  widget->resize(columnWidth(column + 1), WLength::Auto);
  widget->setMinimumSize(WLength::Auto, 1);
  if (column == static_cast<int>(columnWidgets_.size()) - 1)
    row_->addWidget(std::move(widget));
  else
    row_->insertBefore(std::move(widget), columnWidgets_[column + 1].widget);
}

void WTreeTableNode::createExtraColumns(int numColumns)
{
  if (!row_) {
    row_ = impl()->bindWidget("cols-row", cpp14::make_unique<WContainerWidget>());
    row_->addStyleClass("cols-row");
  }

  while (static_cast<int>(columnWidgets_.size()) < numColumns) {
    std::unique_ptr<WText> w(new WText(WString::fromUTF8(" ")));
    w->setInline(false);
    columnWidgets_.push_back(ColumnWidget(w.get(), false));
    w->setFloatSide(Side::Left);
    w->resize(columnWidth(columnWidgets_.size()), 1);
    row_->addWidget(std::move(w));
  }
}

WWidget *WTreeTableNode::columnWidget(int column)
{
  --column;

  return
    (column < static_cast<int>(columnWidgets_.size())
     && columnWidgets_[column].isSet)
    ? columnWidgets_[column].widget
    : nullptr;
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
