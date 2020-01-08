// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef GIT_MODEL_H_
#define GIT_MODEL_H_

#include <Wt/WAbstractItemModel.h>

#include "Git.h"

using namespace Wt;

/**
 * @addtogroup gitmodelexample
 */
/*@{*/

/*! \class GitModel
 *  \brief A model that retrieves revision trees from a git repository.
 *
 * In its present form, it presents only a single column of data: the
 * file names. Additional data could be easily added. Git "tree" objects
 * correspond to folders, and "blob" objects to files.
 *
 * The model is read-only, does not support sorting (that could be
 * provided by using a WSortFilterProxyModel).
 *
 * The model loads only minimal information in memory: to create model indexes
 * for folders. These cannot be uniquely identified by their SHA1 id, since
 * two identical folders at different locations would have the same SHA1 id.
 *
 * The internal id of model indexes created by the model uniquely identify
 * a containing folder for a particular file.
 */
class GitModel : public Wt::WAbstractItemModel
{
public:
  /*! \brief The role which may be used on a file to retrieve its contents.
   */
  static const ItemDataRole ContentsRole;
  static const ItemDataRole FilePathRole;

  /*! \brief Constructor.
   */
  GitModel();

  /*! \brief Set the repository and load its 'master' revision.
   */
  void setRepositoryPath(const std::string& repositoryPath);

  /*! \brief Load a particular revision.
   *
   * The revision name may be any revision accepted by git, by
   * git-rev-parse(1).
   */
  void loadRevision(const std::string& revName);

  /*! \brief Returns the parent index.
   *
   * Consults the internal data structure to find the parent index.
   */
  virtual WModelIndex parent(const WModelIndex& index) const;

  /*! \brief Returns the column count.
   *
   * Returns 1.
   */
  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const;

  /*! \brief Returns the row count.
   *
   * Returns 0 unless the item represents a folder, in which case it returns
   * the number of items in the tree object that corresponds to the folder.
   */
  virtual int rowCount(const WModelIndex& parent = WModelIndex()) const;

  /*! \brief Returns a child index.
   *
   * Consults the internal data structure to create a child index. If
   * necessary, the internal data structure is expanded by adding an
   * entry for using the <i>parent</i> index as a parent index.
   */
  virtual WModelIndex
  index(int row, int column, const WModelIndex& parent = WModelIndex())
    const;

  /*! \brief Returns data.
   *
   * Returns only data corresponding to DisplayRole and ContentsRole.
   */
  virtual cpp17::any
  data(const WModelIndex& index, ItemDataRole role = ItemDataRole::Display) const;
  
  /*! \brief Returns header data.
   */
  virtual cpp17::any
  headerData(int section, Orientation orientation = Orientation::Horizontal,
             ItemDataRole role = ItemDataRole::Display) const;

  using WAbstractItemModel::data;

private:
  /*! \brief The git repository. */
  Git git_;

  /*! \class ChildIndex
   *  \brief Index usable as a key to a map, that identifies a child/row
   *         within a tree.
   */
  struct ChildIndex {
    int parentId;
    int index;

    ChildIndex(int aParent, int anIndex)
      : parentId(aParent), index(anIndex) { }

    bool operator< (const ChildIndex& other) const {
      if (parentId < other.parentId)
	return true;
      else if (parentId > other.parentId)
	return false;
      else return index < other.index;
    }
  };

  /*! \class Tree
   *  \brief Used to uniquely locate a folder within the folder hierarchy.
   */
  class Tree {
  public:
    /*! \brief Constructor.
     */
    Tree(int parentId, int index, const Git::ObjectId& object,
	 int rowCount)
      : index_(parentId, index),
	treeObject_(object),
	rowCount_(rowCount)
    { }

    /*! \brief Returns the parent id.
     *
     * Index of the parent folder within the treeData_ vector.
     */
    int parentId() const { return index_.parentId; }

    /*! \brief Returns the child index within the parent folder.
     *
     * Index of this folder within the file list of the parent folder.
     */
    int index() const { return index_.index; }

    /*! \brief Returns the SHA1 id for the git tree object.
     */
    const Git::ObjectId& treeObject() const { return treeObject_; }

    /*! \brief Returns the (cached) row count.
     */
    int rowCount() const { return rowCount_; }

  private:
    ChildIndex    index_;
    Git::ObjectId treeObject_;
    int           rowCount_;
  };

  typedef std::map<ChildIndex, int> ChildPointerMap;

  /*! \brief List of folder objects.
   *
   * This list contains folders for which a model index has been allocated.
   *
   * Model indexes have an internal id that are indexes into this vector,
   * identifying the folder that contains a particular file.
   *
   * Note: only for folders this is needed, since files will never be used
   * as a 'parent' index.
   *
   * It is populated on-the-fly, as the user navigates the model.
   */
  mutable std::vector<Tree> treeData_;

  /*! \brief Maps child indexes to tree indexes.
   *
   * This map provides a way to lookup data in treeData_. It has an entry
   * corresponding to every entry in treeData_: it maps child indexes for
   * folders to indexes in the treeData_ vector.
   *
   * It is populated on-the-fly, as the user navigates the model.
   */
  mutable ChildPointerMap   childPointer_;

  /*! \brief Get or allocate an id for a folder.
   *
   * The folder is identified by a given childIndex within a parent
   * folder. This method adds data to the treeData_ (and
   * childPointer_) data structures.
   */
  int getTreeId(int parentId, int childIndex) const;

  /*! \brief Get the Git::Object that corresponds to an index.
   */
  Git::Object getObject(const WModelIndex& index) const;
};

/*@}*/

#endif // GIT_MODEL_H_
