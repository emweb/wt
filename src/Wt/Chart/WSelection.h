// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WSELECTION_H_
#define CHART_WSELECTION_H_

#include <limits>

#include <Wt/WModelIndex.h>

namespace Wt {
  namespace Chart {

/*! \class WSelection Wt/Chart/WSelection.h
 *  \brief Represents a selection on a chart.
 *
 *  \ingroup charts
 */
class WT_API WSelection {
public:
  double distance; //!< The distance from the look point to the selected point.

  WSelection()
    : distance(std::numeric_limits<double>::infinity())
  {}

  WSelection(double distance)
    : distance(distance)
  {}
};

/*! \class WPointSelection Wt/Chart/WSelection.h
 *  \brief Represents a single point selection on a WScatterData
 */
class WT_API WPointSelection : public WSelection {
public:
  int rowNumber; //!< The row number of the WAbstractDataModel that the selected point is defined in.

  WPointSelection()
    : WSelection(), rowNumber(-1)
  {}

  WPointSelection(double distance, int rowNumber)
    : WSelection(distance), rowNumber(rowNumber)
  {}
};

/*! \class WSurfaceSelection Wt/Chart/WSelection.h
    \brief Represents a selection on a surface plot.
 */
class WT_API WSurfaceSelection : public WSelection {
public:
  double x; //!< The x coordinate in the coordinate system of the WAbstractDataModel
  double y; //!< The y coordinate in the coordinate system of the WAbstractDataModel
  double z; //!< The z coordinate in the coordinate system of the WAbstractDataModel

  WSurfaceSelection()
    : WSelection(), x(0.0), y(0.0), z(0.0)
  {}

  WSurfaceSelection(double distance, double x, double y, double z)
    : WSelection(distance), x(x), y(y), z(z)
  {}
};

/*! \class WBarSelection Wt/Chart/WSelection.h
 *  \brief Represents a selection of a bar.
 */
class WT_API WBarSelection : public WSelection {
public:
  WModelIndex index; //!< The index that corresponds to the selected bar in the WAbstractDataModel

  WBarSelection()
    : WSelection(), index(WModelIndex())
  {}

  WBarSelection(double distance, WModelIndex index)
    : WSelection(distance), index(index)
  {}
};

  }
}

#endif // CHART_WSELECTION_H_
