/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractItemDelegate"

namespace Wt {

WAbstractItemDelegate::WAbstractItemDelegate(WObject *parent)
  : WObject(parent),
    closeEditor_(this)
{ }

WAbstractItemDelegate::~WAbstractItemDelegate()
{ }

void WAbstractItemDelegate::updateModelIndex(WWidget *widget,
					     const WModelIndex& index)
{ }

boost::any WAbstractItemDelegate::editState(WWidget *widget) const
{
  return boost::any();
}

void WAbstractItemDelegate::setEditState(WWidget *widget,
					 const boost::any& value) const
{ }

void WAbstractItemDelegate::setModelData(const boost::any& editState,
					 WAbstractItemModel *model,
					 const WModelIndex& index) const
{ }

WValidator::State WAbstractItemDelegate::validate(const WModelIndex& index,
						  const boost::any& editState)
  const
{
  return WValidator::Valid;
}

}
