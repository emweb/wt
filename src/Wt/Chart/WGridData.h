// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WGRIDDATA_H
#define CHART_WGRIDDATA_H

#include <Wt/Chart/WAbstractGridData.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/WPen.h>

namespace Wt {
class WAbstractItemModel;
class WMemoryResource;

  namespace Chart {
  class WCartesian3DChart;

/*! \class WGridData
 *  \brief Class representing grid-based data for a 3D chart.
 *
 * General information can be found at WAbstractDataSeries3D. The model for 
 * this dataseries is structured as a table. One of the columns (by default 
 * index 0) contains the x-axis values, one of the rows (by default index 0) 
 * contains the y-axis values. All other values in the table contain the 
 * z-value corresponding to the x- and y-values with the same column- and 
 * row-index.
 *
 * \ingroup charts
 */
class WT_API WGridData : public WAbstractGridData {
public:
  /*! \brief Constructor
   */
  WGridData(std::shared_ptr<WAbstractItemModel> model);

  virtual ~WGridData();

  virtual double minimum(Axis axis) const override;

  virtual double maximum(Axis axis) const override;
  
  /*! \brief Set which column in the model is used as x-axis.
   *
   * The default column that is used has index 0.
   */
  void setXSeriesColumn(int modelColumn);

  /*! \brief Returns which column in the model is used as x-axis.
   *
   * \sa setXSeriesColumn()
   */
  int XSeriesColumn() const { return XAbscisColumn_; }

  /*! \brief Set which row in the model is used as y-axis.
   *
   * The default row that is used has index 0.
   */
  void setYSeriesRow(int modelRow);

  /*! \brief Returns which row in the model is used as y-axis.
   *
   * \sa setYSeriesRow()
   */
  int YSeriesRow() const { return YAbscisRow_; }
  
  // below = internal API
  virtual int nbXPoints() const override;
  virtual int nbYPoints() const override;
  virtual WString axisLabel(int u, Axis axis) const override;
  virtual cpp17::any data(int i, int j) const override;

protected:
  virtual int countSimpleData() const override;
  virtual void pointDataFromModel(FloatBuffer& simplePtsArray,
				  FloatBuffer& simplePtsSize,
				  FloatBuffer& coloredPtsArray,
				  FloatBuffer& coloredPtsSize,
				  FloatBuffer& coloredPtsColor) const override;
  virtual void surfaceDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const override;
  virtual void barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const override;
  virtual void barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays,
				std::vector<FloatBuffer>& coloredPtsArrays,
				std::vector<FloatBuffer>& coloredPtsColors) const override;

private:
  void findRange() const;

  int XAbscisColumn_;
  int YAbscisRow_;
};

  }
}

#endif
