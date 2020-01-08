/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "GitModel.h"

const ItemDataRole GitModel::ContentsRole = Wt::ItemDataRole::User;
const ItemDataRole GitModel::FilePathRole = Wt::ItemDataRole::User + 1;

GitModel::GitModel()
  : WAbstractItemModel()
{ }

void GitModel::setRepositoryPath(const std::string& gitRepositoryPath)
{
  git_.setRepositoryPath(gitRepositoryPath);
  loadRevision("master");
}

void GitModel::loadRevision(const std::string& revName)
{
  Git::ObjectId treeRoot = git_.getCommitTree(revName);

  // You need to call this method before invalidating all existing
  // model indexes. Anyone listening for this event could temporarily
  // convert some model indexes to a raw index pointer, but this model
  // does not reimplement these methods.
  layoutAboutToBeChanged().emit();

  treeData_.clear();
  childPointer_.clear();

  // Store the tree root as treeData_[0]
  treeData_.push_back(Tree(-1, -1, treeRoot, git_.treeSize(treeRoot)));

  layoutChanged().emit();
}

WModelIndex GitModel::parent(const WModelIndex& index) const
{
  // treeData_[0] indicates the top-level parent.
  if (!index.isValid() || index.internalId() == 0)
    return WModelIndex();
  else {
    // get the item that corresponds to the parent ...
    const Tree& item = treeData_[index.internalId()];

    // ... and construct that identifies the parent:
    //   row = child index in the grand parent
    //   internalId = id of the grand parent
    return createIndex(item.index(), 0, item.parentId()); 
  }
}

WModelIndex GitModel::index(int row, int column,
			    const WModelIndex& parent) const
{
  int parentId;

  // the top-level parent has id=0.
  if (!parent.isValid())
    parentId = 0;
  else {
    // the internal id of the parent identifies the grand parent
    int grandParentId = parent.internalId();

    // lookup the parent id for the parent himself, based on grand parent
    // and child-index (=row) within the grand parent
    parentId = getTreeId(grandParentId, parent.row());
  }

  return createIndex(row, column, parentId);
}

int GitModel::getTreeId(int parentId, int childIndex) const
{
  ChildIndex index(parentId, childIndex);

  ChildPointerMap::const_iterator i = childPointer_.find(index);
  if (i == childPointer_.end()) {
    // no tree object was already allocated, so do that now.

    // lookup the git SHA1 object Id (within the parent)
    const Tree& parentItem = treeData_[parentId];
    Git::Object o = git_.treeGetObject(parentItem.treeObject(), childIndex);

    // and add to treeData_ and childPointer_ data structures
    treeData_.push_back(Tree(parentId, childIndex, o.id, git_.treeSize(o.id)));
    int result = treeData_.size() - 1;
    childPointer_[index] = result;
    return result;
  } else
    return i->second;
}

int GitModel::columnCount(const WModelIndex& index) const
{
  // currently only one column
  return 1;
}

int GitModel::rowCount(const WModelIndex& index) const
{
  // we are looking for the git SHA1 id of a tree object (since only folders
  // may contain children).
  Git::ObjectId objectId;
  int treeId;

  if (index.isValid()) {
    // only column 0 items may contain children
    if (index.column() != 0)
      return 0;

    Git::Object o = getObject(index);
    if (o.type == Git::Tree) {
      objectId = o.id;
      treeId = getTreeId(index.internalId(), index.row());
    } else
      // not a folder: no children
      return 0;
  } else {
    treeId = 0;
    // the index corresponds to the root object
    if (treeData_.empty())
      // model not yet loaded !
      return 0;
    else
      objectId = treeData_[0].treeObject();
  }

  return treeData_[treeId].rowCount();
}

cpp17::any GitModel::data(const WModelIndex& index, ItemDataRole role) const
{
  if (!index.isValid())
    return cpp17::any();

  /* Only 3 data roles on column 0 data are supported:
   * - DisplayRole: the file name
   * - DecorationRole: an icon (folder or file)
   * - ContentsRole: the file contents
   */
  if (index.column() == 0) {
    Git::Object object = getObject(index);
    if (role == ItemDataRole::Display) {
      if (object.type == Git::Tree)
	return object.name + '/';
      else
	return object.name;
    } else if (role == ItemDataRole::Decoration) {
      if (object.type == Git::Blob)
	return static_cast<const char*>("icons/git-blob.png");
      else if (object.type == Git::Tree)
	return static_cast<const char*>("icons/git-tree.png");
    } else if (role == ContentsRole) {
      if (object.type == Git::Blob)
        return git_.catFile(object.id);
    } else if (role == FilePathRole) {
      return cpp17::any();
    }
  }

  return cpp17::any();
}

cpp17::any GitModel::headerData(int section, Orientation orientation,
                                ItemDataRole role) const
{
  if (orientation == Orientation::Horizontal && role == ItemDataRole::Display)
    return static_cast<const char*>("File");
  else
    return cpp17::any();
}

Git::Object GitModel::getObject(const WModelIndex& index) const
{
  int parentId = index.internalId();
  const Tree& parentItem = treeData_[parentId];
  return git_.treeGetObject(parentItem.treeObject(), index.row());
}
