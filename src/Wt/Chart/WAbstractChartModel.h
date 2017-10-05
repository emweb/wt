// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_CHART_MODEL_H_
#define WABSTRACT_CHART_MODEL_H_

#include "Wt/WObject.h"
#include "Wt/WColor.h"
#include "Wt/WModelIndex.h"
#include "Wt/WSignal.h"
#include "Wt/WString.h"

namespace Wt {
  namespace Chart {

/*! \class WAbstractChartModel Wt/Chart/WAbstractChartModel.h Wt/Chart/WAbstractChartModel.h
 *  \brief An abstract model for use with %Wt's charts.
 *
 * This abstract model is used by WAbstractChart as data model.
 *
 * \ingroup charts
 */
class WT_API WAbstractChartModel : public WObject
{
public:
  /*! \brief Creates a new chart model.
   */
  WAbstractChartModel();

  virtual ~WAbstractChartModel();

  /*! \brief Returns data at a given row and column.
   *
   * This value determines the position of a data point
   * on the chart.
   */
  virtual double data(int row, int column) const = 0;

  /*! \brief Returns display data at a given row and column.
   *
   * This value should be a textual representation of the
   * value returned by data(). This defaults to the string
   * representation of the double returned by data().
   */
  virtual WString displayData(int row, int column) const;

  /*! \brief Returns the given column's header data.
   *
   * This is used as the name in the legend for a data series.
   *
   * Defaults to an empty string.
   */
  virtual WString headerData(int column) const;

  /*! \brief Returns the tooltip text to use on a given row and column.
   *
   * Defaults to an empty string, signifying that no tooltip
   * should be shown.
   */
  virtual WString toolTip(int row, int column) const;

  /*! \brief Returns the item flags for the given row and column.
   *
   * Only the ItemIsXHTMLText and ItemHasDeferredTooltip flags
   * are supported for charts.
   *
   * ItemIsXHTMLText determines whether the tooltip text should be
   * rendered as XHTML or as plain text, and ItemHasDeferredTooltip
   * makes it so that tooltips are only loaded on demand.
   *
   * \note An XHTML text tooltip will be forced to be deferred. Non-deferred
   *       XHTML tooltips are not supported.
   *
   * \note When not using deferred tooltips, the HTML <area> tag will be
   *       used. If there are many tooltips and the chart is interactive
   *       this may cause client-side performance issues. If deferred tooltips
   *       are used, this will cause some load on the server, as it calculates
   *       server-side what marker or bar the user is hovering over.
   */
  virtual WFlags<ItemFlag> flags(int row, int column) const;

  /*! \brief Returns the link for a given row and column.
   *
   * Defaults to an empty link, signifying that no link should be shown.
   */
  virtual WLink *link(int row, int column) const;

  /*!\brief Returns the marker pen color to use for a given row and column.
   *
   * This is used as the color of the outline of markers when drawing
   * a SeriesType::Point. The default is null, indicating that the default color,
   * as determined by WDataSeries::markerPen(), should be used.
   *
   * \sa WDataSeries::setMarkerPen()
   */
  virtual const WColor *markerPenColor(int row, int column) const;

  /*!\brief Returns the marker brush color to use for a given row and column.
   *
   * This is used as the color of the brush used when drawing a
   * SeriesType::Point. The default is null, indicating that the default color,
   * as determined by WDataSeries::markerBrush(), should be used.
   *
   * \sa WDataSeries::setMarkerBrush()
   */
  virtual const WColor *markerBrushColor(int row, int column) const;

  /*!\brief Returns the bar pen color to use for a given row and column.
   *
   * This is used as the color of the outline of bars when drawing a
   * SeriesType::Bar. The default is null, indicating that the default color,
   * as determined by WDataSeries::pen(), should be used.
   *
   * \sa WDataSeries::setPen()
   */
  virtual const WColor *barPenColor(int row, int column) const;

  /*!\brief Returns the bar brush color to use for a given row and column.
   *
   * This is used as the color of the brush used when drawing a
   * SeriesType::Bar. The default is null, indicating that the default color,
   * as determined by WDataSeries::brush(), should be used.
   *
   * \sa WDataSeries::setBrush()
   */
  virtual const WColor *barBrushColor(int row, int column) const;

  /*!\brief Returns the marker scale factor to use for a given row and column.
   *
   * This is used to scale the size of the marker when drawing a
   * SeriesType::Point. The default is null, indicating that the default scale should be used.
   */
  virtual const double *markerScaleFactor(int row, int column) const;

  /*! \brief Returns the number of columns.
   *
   * \sa rowCount()
   */
  virtual int columnCount() const = 0;

  /*! \brief Returns the number of rows.
   *
   * \sa columnCount()
   */
  virtual int rowCount() const = 0;

  /*! \brief A signal that notifies of any change to the model.
   *
   * Implementations should trigger this signal in order to update the chart.
   */
  virtual Signal<>& changed() { return changed_; }

private:
  Signal<> changed_;
};

  }
}

#endif // WABSTRACT_CHART_MODEL_H_
