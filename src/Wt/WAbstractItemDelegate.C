/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractItemDelegate.h"

namespace Wt {

WAbstractItemDelegate::WAbstractItemDelegate()
{ }

WAbstractItemDelegate::~WAbstractItemDelegate()
{ }

void WAbstractItemDelegate::updateModelIndex(WWidget *widget,
					     const WModelIndex& index)
{ }

cpp17::any WAbstractItemDelegate::editState(WWidget *widget,
					 const WModelIndex& index) const
{
  return cpp17::any();
}

void WAbstractItemDelegate::setEditState(WWidget *widget,
					 const WModelIndex& index,
					 const cpp17::any& value) const
{ }

void WAbstractItemDelegate::setModelData(const cpp17::any& editState,
					 WAbstractItemModel *model,
					 const WModelIndex& index) const
{ }

ValidationState WAbstractItemDelegate::validate(const WModelIndex& index,
						const cpp17::any& editState)
  const
{
  return ValidationState::Valid;
}

}
