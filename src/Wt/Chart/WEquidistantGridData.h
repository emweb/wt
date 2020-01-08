// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WEQUIDISTANTGRIDDATA_H
#define CHART_WEQUIDISTANTGRIDDATA_H

#include <Wt/Chart/WAbstractGridData.h>

namespace Wt {
  namespace Chart {

/*! \class WEquidistantGridData
 *  \brief Class representing grid-based data for on a 3D chart.
 *
 * General information can be found at WAbstractDataSeries3D. The model for 
 * this dataseries does not contain an x- and y-axis. Instead the class 
 * derives the x- and y-values from the minimum and delta provided in the 
 * constructor. The size of the model determines the size of the grid. The 
 * model itself is structured as a table, where every value represents the 
 * z-value of a data-point. The corresponding x- and y-values are calculated 
 * by adding delta times the row/column-index to the axis-minimum.
 *
 * \ingroup charts
 */
class WT_API WEquidistantGridData : public WAbstractGridData {
public:
  /*! \brief Constructor
   */
  WEquidistantGridData(std::shared_ptr<WAbstractItemModel> model,
                       double XMin,
                       double deltaX,
                       double YMin,
                       double deltaY);

  virtual double minimum(Axis axis) const override;

  virtual double maximum(Axis axis) const override;
  
  /*! \brief Sets the minimum and delta for the X-axis.
   */
  void setXAbscis(double XMinimum, double deltaX);

  /*! \brief Returns the minimum value of the X-axis.
   *
   * \sa setXAbscis()
   */
  double XMinimum() const { return XMinimum_; }

  /*! \brief Returns the delta value of the X-axis.
   *
   * The delta is the interval between subsequent values on the axis.
   *
   * \sa setXAbscis()
   */
  double deltaX() const {return deltaX_; } 
  
  /*! \brief Sets the minimum and delta for the Y-axis.
   */
  void setYAbscis(double YMinimum, double deltaY);

  /*! \brief Returns the minimum value of the Y-axis.
   *
   * \sa setYAbscis()
   */
  double YMinimum() const { return YMinimum_; }

  /*! \brief Returns the delta value of the Y-axis.
   *
   * The delta is the interval between subsequent values on the axis.
   *
   * \sa setYAbscis()
   */
  double deltaY() const {return deltaY_; } 

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

  double XMinimum_;
  double deltaX_;
  double YMinimum_;
  double deltaY_;
};
    
  }
}

#endif
