// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WREADONLY_PROXY_MODEL_H_
#define WREADONLY_PROXY_MODEL_H_

#include <Wt/WAbstractProxyModel.h>

namespace Wt {

/*! \class WReadOnlyProxyModel Wt/WReadOnlyProxyModel.h
 *  \brief A read-only wrapper for a source model
 *
 * This is a simple proxy model which provides a read-only view on a
 * source model. This is convenient for situations where you want to
 * share a common read-only source model between different sessions.
 *
 * \ingroup modelview
 */
class WT_API WReadOnlyProxyModel : public WAbstractProxyModel
{
public:
  /*! \brief Constructor.
   */
  WReadOnlyProxyModel();

  /*! \brief Maps a source model index to the proxy model.
   *
   * Returns the sourceIndex unmodified.
   */
  virtual WModelIndex mapFromSource(const WModelIndex& sourceIndex)
    const override;

  /*! \brief Maps a proxy model index to the source model.
   *
   * Returns the proxyIndex unmodified.
   */
  virtual WModelIndex mapToSource(const WModelIndex& proxyIndex)
    const override;

  /*! \brief Returns the number of columns.
   *
   * This returns the column count of the source model.
   */
  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const override;

  /*! \brief Returns the number of rows.
   *
   * This returns the row count of the source model.
   */
  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  /*! \brief Returns the parent for a model index.
   *
   * Returns the parent of the given index in the source model.
   */
  virtual WModelIndex parent(const WModelIndex& index) const override;

  /*! \brief Returns the child index for the given row and column.
   *
   * Returns the index in the source model.
   */
  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const override;

  using WAbstractProxyModel::setData;

  /*! \brief Always returns \c false and has no effect.
   *
   */
  virtual bool setData(const WModelIndex& index,
                       const cpp17::any& value, ItemDataRole role = ItemDataRole::Edit) override;

  /*! \brief Always returns \c false and has no effect.
   *
   */
  virtual bool setItemData(const WModelIndex& index, const DataMap& values)
    override;

  using WAbstractProxyModel::setHeaderData;

  /*! \brief Always returns \c false and has no effect.
   *
   */
  virtual bool setHeaderData(int section, Orientation orientation,
                             const cpp17::any& value, ItemDataRole role = ItemDataRole::Edit)
    override;

  /*! \brief Always returns \c false and has no effect.
   *
   */
  virtual bool insertColumns(int column, int count, const WModelIndex& parent)
    override;

  /*! \brief Always returns \c false and has no effect.
   *
   */
  virtual bool removeColumns(int column, int count, const WModelIndex& parent)
    override;

  /*! \brief Has no effect.
   *
   */
  virtual void dropEvent(const WDropEvent& e, DropAction action,
			 int row, int column, const WModelIndex& parent)
    override;
};

}

#endif // WREADONLY_PROXY_MODEL_H_
