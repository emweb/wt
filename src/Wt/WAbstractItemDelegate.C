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

void WAbstractItemDelegate::updateModelIndex(WT_MAYBE_UNUSED WWidget* widget, WT_MAYBE_UNUSED const WModelIndex&)
{ }

cpp17::any WAbstractItemDelegate::editState(WT_MAYBE_UNUSED WWidget* widget, WT_MAYBE_UNUSED const WModelIndex&) const
{
  return cpp17::any();
}

void WAbstractItemDelegate::setEditState(WT_MAYBE_UNUSED WWidget* widget,
                                         WT_MAYBE_UNUSED const WModelIndex&,
                                         WT_MAYBE_UNUSED const cpp17::any& value) const
{ }

void WAbstractItemDelegate::setModelData(WT_MAYBE_UNUSED const cpp17::any& editState,
                                         WT_MAYBE_UNUSED WAbstractItemModel* model,
                                         WT_MAYBE_UNUSED const WModelIndex&) const
{ }

ValidationState WAbstractItemDelegate::validate(WT_MAYBE_UNUSED const WModelIndex&,
                                                WT_MAYBE_UNUSED const cpp17::any& editState)
  const
{
  return ValidationState::Valid;
}

}
