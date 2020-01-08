// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WIDENTITY_PROXY_MODEL_H_
#define WIDENTITY_PROXY_MODEL_H_

#include <vector>

#include <Wt/WAbstractProxyModel.h>

namespace Wt {

/*! \class WIdentityProxyModel Wt/WIdentityProxyModel.h Wt/WIdentityProxyModel.h
 * \brief A proxy model that proxies its source model unmodified.
 *
 * A WIdentityProxyModel simply forwards the structure of the source model,
 * without any transformation. WIdentityProxyModel can be used as a base class
 * for implementing proxy models that reimplement data(), but retain all
 * other characteristics of the source model.
 */
class WT_API WIdentityProxyModel : public WAbstractProxyModel
{
public:
  /*! \brief Constructor.
   */
  WIdentityProxyModel();

  virtual ~WIdentityProxyModel();

  /*! \brief Returns the number of columns.
   *
   * Translates the parent index to the source model,
   * and returns the number of columns of the source model.
   */
  virtual int columnCount(const WModelIndex &parent = WModelIndex()) const override;

  /*! \brief Returns the number of rows.
   *
   * Translates the parent index to the source model,
   * and returns the number of rows of the source model.
   */
  virtual int rowCount(const WModelIndex &parent = WModelIndex()) const override;

  /*! \brief Returns the parent for a model index.
   *
   * Translates the child index to the source model,
   * and translates its parent back to this proxy model.
   */
  virtual WModelIndex parent(const WModelIndex &child) const override;

  /*! \brief Returns the child index for the given row and column.
   *
   */
  virtual WModelIndex index(int row, int column, const WModelIndex &parent = WModelIndex()) const override;

  /*! \brief Maps a proxy model index to the source model.
   *
   * Returns a model index with the same row and column as
   * the source index. The parent index is mapped recursively.
   */
  virtual WModelIndex mapFromSource(const WModelIndex &sourceIndex) const override;

  /*! \brief Maps a source model index to the proxy model.
   *
   * Returns a model index with the same row and column as
   * the proxy index. The parent index is mapped recursively.
   */
  virtual WModelIndex mapToSource(const WModelIndex &proxyIndex) const override;

  /*! \brief Sets the source model.
   *
   * The source model provides the actual data for the proxy
   * model.
   *
   * Ownership of the source model is <i>not</i> transferred.
   *
   * All signals of the source model are forwarded to the
   * proxy model.
   */
  virtual void setSourceModel(const std::shared_ptr<WAbstractItemModel>& sourceModel) override;

  /*! \brief Inserts one or more columns.
   *
   * Inserts \c count columns at column \c column in the
   * source model.
   *
   * Forwards the result indicating success from the source model.
   */
   virtual bool insertColumns(int column, int count,
		     const WModelIndex &parent = WModelIndex()) override;

  /*! \brief Inserts one or more rows.
   *
   * Inserts \c count rows at row \c row in the source model.
   *
   * Forwards the result indicating success from the source model.
   */
  virtual bool insertRows(int row, int count,
		  const WModelIndex &parent = WModelIndex()) override;

  /*! \brief Removes columns.
   *
   * Removes \c count columns at column \c column in the source model.
   *
   * Forwards the result indicating success from the source model.
   */
  virtual bool removeColumns(int column, int count,
		     const WModelIndex &parent = WModelIndex()) override;

  /*! \brief Removes rows.
   *
   * Removes \c count rows at row \c row in the source model.
   *
   * Forwards the result indicating success from the source model.
   */
  virtual bool removeRows(int row, int count,
		  const WModelIndex &parent = WModelIndex()) override;

  using WAbstractProxyModel::setHeaderData;

  /*! \brief Set header data for a column or row.
   *
   * Sets the header data for a column or row in the source model.
   *
   * Forwards the result indicating success from the source model.
   */
  bool setHeaderData(int section, Orientation orientation,
		     const cpp17::any& value, ItemDataRole role = ItemDataRole::Edit);

private:
  std::vector<Wt::Signals::connection> modelConnections_;

  void sourceColumnsAboutToBeInserted(const WModelIndex &parent,
				      int start, int end);
  void sourceColumnsAboutToBeRemoved(const WModelIndex &parent,
				     int start, int end);
  void sourceColumnsInserted(const WModelIndex &parent, int start, int end);
  void sourceColumnsRemoved(const WModelIndex &parent, int start, int end);
  void sourceRowsAboutToBeInserted(const WModelIndex &parent,
				   int start, int end);
  void sourceRowsAboutToBeRemoved(const WModelIndex &parent,
				  int start, int end);
  void sourceRowsInserted(const WModelIndex &parent, int start, int end);
  void sourceRowsRemoved(const WModelIndex &parent, int start, int end);
  void sourceDataChanged(const WModelIndex &topLeft,
			 const WModelIndex &bottomRight);
  void sourceHeaderDataChanged(Orientation orientation, int start, int end);
  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();
  void sourceModelReset();
};

} // namespace Wt

#endif // #ifndef WIDENTITY_PROXY_MODEL_H_
