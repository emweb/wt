// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTANDARD_CHART_PROXY_MODEL_H_
#define WSTANDARD_CHART_PROXY_MODEL_H_

#include "Wt/Chart/WAbstractChartModel.h"
#include "Wt/WLink.h"

namespace Wt {

class WAbstractItemModel;

  namespace Chart {

/*! \class WStandardChartProxyModel Wt/Chart/WStandardChartProxyModel.h Wt/Chart/WStandardChartProxyModel.h
 *  \brief A WAbstractChartModel implementation that wraps a WAbstractItemModel
 *
 * This model delegates all functions to an underlying WAbstractItemModel,
 * using the appropriate roles.
 *
 * This model also triggers the changed() signal whenever the underlying WAbstractItemModel
 * is changed.
 *
 * \ingroup charts
 */
class WT_API WStandardChartProxyModel : public WAbstractChartModel 
{
public:
  /*! \brief Creates a new WStandardChartProxyModel that wraps the given source model.
   */
  WStandardChartProxyModel(const std::shared_ptr<WAbstractItemModel>&
			   sourceModel);

  virtual ~WStandardChartProxyModel();

  /*! \brief Returns data at a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::Display\endlink as a double.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual double data(int row, int column) const override;

  /*! \brief Returns display data at a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::Display\endlink as a WString.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual WString displayData(int row, int column) const override;

  /*! \brief Returns the given column's header data.
   *
   * Returns the result of WAbstractItemModel::headerData() for the
   * given column with the \link ItemDataRole ItemDataRole::Display\endlink as a WString.
   *
   * \sa WAbstractItemModel::headerData()
   */
  virtual WString headerData(int column) const override;

  /*! \brief Returns the tooltip text to use on a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::ToolTip\endlink as a WString.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual WString toolTip(int row, int column) const override;

  /*! \brief Returns the item flags for the given row and column.
   *
   * Returns the result of WAbstractItemModel::index(row, column).flags()
   * for the given row and column.
   *
   * \sa WModelIndex::flags()
   */
  virtual WFlags<ItemFlag> flags(int row, int column) const override;

  /*! \brief Returns the link to use on a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole LinkRole\endlink as a WLink.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual WLink *link(int row, int column) const override;

  /*!\brief Returns the marker pen color to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::MarkerPenColor\endlink, or null if no color
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const WColor *markerPenColor(int row, int column) const override;

  /*!\brief Returns the marker brush color to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::MarkerBrushColor\endlink, or null if no color
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const WColor *markerBrushColor(int row, int column) const override;

  /*!\brief Returns the marker type to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole MarkerTypeRole\endlink, or null if no marker type
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const MarkerType *markerType(int row, int column) const override;

  /*!\brief Returns the bar pen color to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::BarPenColor\endlink, or null if no color
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const WColor *barPenColor(int row, int column) const override;

  /*!\brief Returns the bar brush color to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::BarBrushColor\endlink, or null if no color
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const WColor *barBrushColor(int row, int column) const override;

  /*!\brief Returns the marker scale factor to use for a given row and column.
   *
   * Returns the result of WAbstractItemModel::data() for the given
   * row and column with the \link ItemDataRole ItemDataRole::MarkerScaleFactor\endlink, or null if no color
   * is defined.
   *
   * \sa WAbstractItemModel::data()
   */
  virtual const double *markerScaleFactor(int row, int column) const override;

  /*! \brief Returns the number of columns.
   *
   * \sa WAbstractItemModel::columnCount()
   */
  virtual int columnCount() const override;

  /*! \brief Returns the number of rows.
   *
   * \sa WAbstractItemModel::rowCount()
   */
  virtual int rowCount() const override;

  /*! \brief Returns the wrapped source model.
   */
  std::shared_ptr<WAbstractItemModel> sourceModel() const 
  { return sourceModel_; }

private:
  std::shared_ptr<WAbstractItemModel> sourceModel_;
#ifndef WT_TARGET_JAVA
  mutable WColor color_;
  mutable WLink link_;
  mutable MarkerType markerType_;
  mutable double markerScaleFactor_;
#endif

  void sourceModelModified();
  const WColor *color(int row, int column, ItemDataRole colorRole) const;
};

  }
}

#endif // WSTANDARD_CHART_PROXY_MODEL_H_
