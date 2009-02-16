/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractProxyModel"

namespace Wt {

WAbstractProxyModel::WAbstractProxyModel(WObject *parent)
  : WAbstractItemModel(parent),
    sourceModel_(0)
{ }

void WAbstractProxyModel::setSourceModel(WAbstractItemModel *sourceModel)
{
  sourceModel_ = sourceModel;
}

boost::any WAbstractProxyModel::data(const WModelIndex& index, int role) const
{
  return sourceModel_->data(mapToSource(index), role);
}

bool WAbstractProxyModel::setData(const WModelIndex& index,
				    const boost::any& value, int role)
{
  return sourceModel_->setData(mapToSource(index), value, role);
}

int WAbstractProxyModel::flags(const WModelIndex& index) const
{
  return sourceModel_->flags(mapToSource(index));
}

bool WAbstractProxyModel::setHeaderData(int section, Orientation orientation,
					  const boost::any& value, int role)
{
  if (rowCount() > 0)
    if (orientation == Vertical)
      section = mapToSource(index(section, 0)).row();
    else
      section = mapToSource(index(0, section)).column();

  return sourceModel_->setHeaderData(section, orientation, value, role);
}

boost::any WAbstractProxyModel::headerData(int section,
					   Orientation orientation, int role)
  const
{
  if (rowCount() > 0)
    if (orientation == Vertical)
      section = mapToSource(index(section, 0)).row();
    else
      section = mapToSource(index(0, section)).column();

  return sourceModel_->headerData(section, orientation, role);
}

bool WAbstractProxyModel::insertColumns(int column, int count,
					  const WModelIndex& parent)
{
  return sourceModel_->insertColumns(column, count, parent);
}

bool WAbstractProxyModel::insertRows(int row, int count,
				       const WModelIndex& parent)
{
  int sourceRow = mapToSource(index(row, 0, parent)).row();

  return sourceModel_->insertRows(sourceRow, count, mapToSource(parent));
}

bool WAbstractProxyModel::removeColumns(int column, int count,
					  const WModelIndex& parent)
{
  return sourceModel_->removeColumns(column, count, parent);
}

bool WAbstractProxyModel::removeRows(int row, int count,
				       const WModelIndex& parent)
{
  int sourceRow = mapToSource(index(row, 0, parent)).row();

  return sourceModel_->removeRows(sourceRow, count, mapToSource(parent));
}

std::string WAbstractProxyModel::mimeType() const
{
  return sourceModel_->mimeType();
}

std::vector<std::string> WAbstractProxyModel::acceptDropMimeTypes() const
{
  return sourceModel_->acceptDropMimeTypes();
}

void WAbstractProxyModel::dropEvent(const WDropEvent& e, DropAction action,
				      int row, int column,
				      const WModelIndex& parent)
{
  WModelIndex sourceParent = mapToSource(parent);

  int sourceRow = row;
  int sourceColumn = column;

  if (sourceRow != -1)
    sourceRow = mapToSource(index(row, 0, parent)).row();

  sourceModel_->dropEvent(e, action, sourceRow, sourceColumn, sourceParent);
}

void *WAbstractProxyModel::toRawIndex(const WModelIndex& index) const
{
  return sourceModel_->toRawIndex(mapToSource(index));
}

WModelIndex WAbstractProxyModel::fromRawIndex(void *rawIndex) const
{
  return mapFromSource(sourceModel_->fromRawIndex(rawIndex));
}


}
