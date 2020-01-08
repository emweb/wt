/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WIdentityProxyModel.h>
#include <Wt/WReadOnlyProxyModel.h>
#include <Wt/WStandardItemModel.h>

namespace Wt {

WIdentityProxyModel::WIdentityProxyModel()
{ }

WIdentityProxyModel::~WIdentityProxyModel()
{ }

int WIdentityProxyModel::columnCount(const WModelIndex &parent) const
{
  return sourceModel()->columnCount(mapToSource(parent));
}

int WIdentityProxyModel::rowCount(const WModelIndex &parent) const
{
  return sourceModel()->rowCount(mapToSource(parent));
}

WModelIndex WIdentityProxyModel::parent(const WModelIndex &child) const
{
  const WModelIndex sourceIndex = mapToSource(child);
  const WModelIndex sourceParent = sourceIndex.parent();
  return mapFromSource(sourceParent);
}

WModelIndex WIdentityProxyModel
::index(int row, int column, const WModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return WModelIndex();
  const WModelIndex sourceParent = mapToSource(parent);
  const WModelIndex sourceIndex
    = sourceModel()->index(row, column, sourceParent);
  return mapFromSource(sourceIndex);
}

WModelIndex WIdentityProxyModel
::mapFromSource(const WModelIndex &sourceIndex) const
{
  if (!sourceIndex.isValid())
    return WModelIndex();

  return createIndex(sourceIndex.row(), sourceIndex.column(),
		     sourceIndex.internalPointer());
}

WModelIndex WIdentityProxyModel
::mapToSource(const WModelIndex &proxyIndex) const
{
  if (!sourceModel() || !proxyIndex.isValid())
    return WModelIndex();
  return createSourceIndex(proxyIndex.row(), proxyIndex.column(),
			   proxyIndex.internalPointer());
}

void WIdentityProxyModel
::setSourceModel(const std::shared_ptr<WAbstractItemModel>& newSourceModel)
{
  if (sourceModel()) {
    for (unsigned int i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  WAbstractProxyModel::setSourceModel(newSourceModel);

  if (newSourceModel) {
    modelConnections_.push_back(newSourceModel->rowsAboutToBeInserted().connect
      (this, &WIdentityProxyModel::sourceRowsAboutToBeInserted));
    modelConnections_.push_back(newSourceModel->rowsInserted().connect
      (this, &WIdentityProxyModel::sourceRowsInserted));
    modelConnections_.push_back(newSourceModel->rowsAboutToBeRemoved().connect
      (this, &WIdentityProxyModel::sourceRowsAboutToBeRemoved));
    modelConnections_.push_back(newSourceModel->rowsRemoved().connect
      (this, &WIdentityProxyModel::sourceRowsRemoved));
    modelConnections_.push_back(newSourceModel->columnsAboutToBeInserted().connect
      (this, &WIdentityProxyModel::sourceColumnsAboutToBeInserted));
    modelConnections_.push_back(newSourceModel->columnsInserted().connect
      (this, &WIdentityProxyModel::sourceColumnsInserted));
    modelConnections_.push_back(newSourceModel->columnsAboutToBeRemoved().connect
      (this, &WIdentityProxyModel::sourceColumnsAboutToBeRemoved));
    modelConnections_.push_back(newSourceModel->columnsRemoved().connect
      (this, &WIdentityProxyModel::sourceColumnsRemoved));
    modelConnections_.push_back(newSourceModel->modelReset().connect
      (this, &WIdentityProxyModel::sourceModelReset));
    modelConnections_.push_back(newSourceModel->dataChanged().connect
      (this, &WIdentityProxyModel::sourceDataChanged));
    modelConnections_.push_back(newSourceModel->headerDataChanged().connect
      (this, &WIdentityProxyModel::sourceHeaderDataChanged));
    modelConnections_.push_back(newSourceModel->layoutAboutToBeChanged().connect
      (this, &WIdentityProxyModel::sourceLayoutAboutToBeChanged));
    modelConnections_.push_back(newSourceModel->layoutChanged().connect
      (this, &WIdentityProxyModel::sourceLayoutChanged));
  }
}

bool WIdentityProxyModel::insertColumns(int column, int count,
					const WModelIndex &parent)
{
  return sourceModel()->insertColumns(column, count, mapToSource(parent));
}

bool WIdentityProxyModel::insertRows(int row, int count,
				     const WModelIndex &parent)
{
  return sourceModel()->insertRows(row, count, mapToSource(parent));
}

bool WIdentityProxyModel::removeColumns(int column, int count,
					const WModelIndex &parent)
{
  return sourceModel()->removeColumns(column, count, mapToSource(parent));
}

bool WIdentityProxyModel::removeRows(int row, int count,
				     const WModelIndex &parent)
{
  return sourceModel()->removeRows(row, count, mapToSource(parent));
}

bool WIdentityProxyModel::setHeaderData(int section,
					Orientation orientation,
					const cpp17::any& value, ItemDataRole role)
{
  return sourceModel()->setHeaderData(section, orientation, value, role);
}

void WIdentityProxyModel
::sourceColumnsAboutToBeInserted(const WModelIndex &parent, int start, int end)
{
  beginInsertColumns(mapFromSource(parent), start, end);
}

void WIdentityProxyModel
::sourceColumnsInserted(const WModelIndex &parent, int start, int end)
{
  endInsertColumns();
}

void WIdentityProxyModel
::sourceColumnsAboutToBeRemoved(const WModelIndex &parent, int start, int end)
{
  beginRemoveColumns(mapFromSource(parent), start, end);
}

void WIdentityProxyModel
::sourceColumnsRemoved(const WModelIndex &parent, int start, int end)
{
  endRemoveColumns();
}

void WIdentityProxyModel
::sourceRowsAboutToBeInserted(const WModelIndex &parent, int start, int end)
{
  beginInsertRows(mapFromSource(parent), start, end);
}

void WIdentityProxyModel
::sourceRowsInserted(const WModelIndex &parent, int start, int end)
{
  endInsertRows();
}

void WIdentityProxyModel
::sourceRowsAboutToBeRemoved(const WModelIndex &parent, int start, int end)
{
  beginRemoveRows(mapFromSource(parent), start, end);
}

void WIdentityProxyModel
::sourceRowsRemoved(const WModelIndex &parent, int start, int end)
{
  endRemoveRows();
}

void WIdentityProxyModel
::sourceDataChanged(const WModelIndex &topLeft, const WModelIndex &bottomRight)
{
  dataChanged().emit(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void WIdentityProxyModel
::sourceHeaderDataChanged(Orientation orientation, int start, int end)
{
  headerDataChanged().emit(orientation, start, end);
}

void WIdentityProxyModel::sourceLayoutAboutToBeChanged()
{
  layoutAboutToBeChanged().emit();
}

void WIdentityProxyModel::sourceLayoutChanged()
{
  layoutChanged().emit();
}

void WIdentityProxyModel::sourceModelReset()
{
  modelReset().emit();
}

} // namespace Wt
