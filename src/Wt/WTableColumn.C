/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DomElement.h"

#include "Wt/WTable.h"
#include "Wt/WTableColumn.h"

namespace Wt {

WTableColumn::WTableColumn()
{ }

WTableColumn::~WTableColumn()
{ }

WTableCell *WTableColumn::elementAt(int row)
{
  return table_->elementAt(row, columnNum());
}

int WTableColumn::columnNum() const
{
  for (unsigned i =0; i < table_->columns_.size(); i++) 
    if (table_->columns_[i].get() == this)
      return i;

  return -1;
}

void WTableColumn::setWidth(const WLength& width)
{
#ifndef WT_TARGET_JAVA
  if (!width_)
    width_.reset(new WLength(width));
  else
#endif
    *width_ = width;

  table_->repaintColumn(this);
}

void WTableColumn::setStyleClass(const WT_USTRING& style)
{
  if (WWebWidget::canOptimizeUpdates() && (style == styleClass_))
    return;

  styleClass_ = style;
  table_->repaintColumn(this);
}

WLength WTableColumn::width() const
{
  return width_ ? *width_ : WLength::Auto;
}

void WTableColumn::setId(const std::string& id)
{
  if (!id_)
    id_.reset(new std::string());

  *id_ = id;
}

const std::string WTableColumn::id() const
{
  if (id_)
    return *id_;
  else
    return WObject::id();
}

void WTableColumn::updateDom(DomElement& element, bool all)
{
  if (width_)
    element.setProperty(Property::StyleWidth, width_->cssText());

  if (!all || !styleClass_.empty())
    element.setProperty(Property::Class, styleClass_.toUTF8());
}

void WTableColumn::setTable(WTable *table)
{
  table_ = table;
}

}
