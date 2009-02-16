/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "DomElement.h"
#include "Utils.h"

#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WTableRow"

namespace Wt {

WTableColumn::WTableColumn(WTable *table)
  : table_(table),
    width_(0)
{ }

WTableColumn::~WTableColumn()
{
  delete width_;
}

int WTableColumn::columnNum() const
{
  return Utils::indexOf(*table_->columns_, const_cast<WTableColumn *>(this));
}

void WTableColumn::setWidth(const WLength& width)
{
  if (!width_)
    width_ = new WLength(width);
  else
    *width_ = width;

  table_->repaintColumn(this);
}

void WTableColumn::setStyleClass(const WString& style)
{
  styleClass_ = style;
  table_->repaintColumn(this);
}

WLength WTableColumn::width() const
{
  return width_ ? *width_ : WLength();
}

void WTableColumn::updateDom(DomElement& element, bool all)
{
  if (width_)
    element.setProperty(PropertyStyleWidth, width_->cssText());

  if (!all || !styleClass_.empty())
    element.setAttribute("class", styleClass_.toUTF8());
}

}
