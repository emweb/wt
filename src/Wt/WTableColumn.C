/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DomElement.h"

#include "Wt/WTable"
#include "Wt/WTableColumn"

namespace Wt {

WTableColumn::WTableColumn()
  : width_(0),
    id_(0)
{ }

WTableColumn::~WTableColumn()
{
  delete width_;
  delete id_;
}

WTableCell *WTableColumn::elementAt(int row)
{
  return table_->elementAt(row, columnNum());
}

int WTableColumn::columnNum() const
{
  for (unsigned i =0; i < table_->columns_.size(); i++) 
    if (table_->columns_[i] == this)
      return i;

  return -1;
}

void WTableColumn::setWidth(const WLength& width)
{
#ifndef WT_TARGET_JAVA
  if (!width_)
    width_ = new WLength(width);
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
    id_ = new std::string();

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
    element.setProperty(PropertyStyleWidth, width_->cssText());

  if (!all || !styleClass_.empty())
    element.setProperty(PropertyClass, styleClass_.toUTF8());
}

}
