/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractItemDelegate"

namespace Wt {

WAbstractItemDelegate::WAbstractItemDelegate(WObject *parent)
  : WObject(parent)
{ }

WAbstractItemDelegate::~WAbstractItemDelegate()
{ }

void WAbstractItemDelegate::updateModelIndex(WWidget *widget,
					     const WModelIndex& index)
{ }

}
