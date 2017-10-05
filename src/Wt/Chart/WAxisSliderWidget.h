// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WAXIS_SLIDER_WIDGET_H_
#define CHART_WAXIS_SLIDER_WIDGET_H_

#include <Wt/WBrush.h>
#include <Wt/WJavaScriptHandle.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPen.h>
#include <Wt/WRectF.h>

namespace Wt {

  class WRectF;

  namespace Chart {

    class WDataSeries;

/*! \class WAxisSliderWidget Wt/Chart/WAxisSliderWidget.h Wt/Chart/WAxisSliderWidget.h
 *  \brief A widget for selecting an X axis range to display on an associated WCartesianChart.
 *
 *  \note This widget currently only works with the HtmlCanvas rendering method.
 *
 *  \ingroup charts
 */
class WT_API WAxisSliderWidget : public WPaintedWidget {
public:
  /*! \brief Creates an axis slider widget.
   *
   * Creates an axis slider widget that is not associated with a chart.
   * Before it is used, a chart should be assigned with setChart(),
   * and a series column chosen with setSeriesColumn().
   */
  WAxisSliderWidget();

  /*! \brief Creates an axis slider widget.
   *
   * Creates an axis slider widget associated with the given data series
   * of the given chart.
   */
  WAxisSliderWidget(WDataSeries *series);
  
  /*! \brief Destructor
   */
  virtual ~WAxisSliderWidget();

  void setSeries(WDataSeries *series);

  /*! \brief Set the pen to draw the data series with.
   */
  void setSeriesPen(const WPen& pen);

  /*! \brief Returns the pen to draw the data series with.
   */
  const WPen& seriesPen() const { return seriesPen_; }

  /*! \brief Set the pen to draw the selected part of the data series with.
   *
   * If not set, this defaults to seriesPen().
   */
  void setSelectedSeriesPen(const WPen& pen);

  /*! \brief Returns the pen to draw the selected part of the data series with.
   */
  WPen selectedSeriesPen() const { return *selectedSeriesPen_; }

  /*! \brief Set the brush to draw the handles left and right of the selected area with.
   */
  void setHandleBrush(const WBrush& brush);

  /*! \brief Returns the brush to draw the handles left and right of the selected area with.
   */
  const WBrush& handleBrush() const { return handleBrush_; }

  /*! \brief Set the background brush.
   */
  void setBackground(const WBrush& background);
  
  /*! \brief Returns the background brush.
   */
  const WBrush& background() const { return background_; }

  /*! \brief Set the brush for the selected area.
   */
  void setSelectedAreaBrush(const WBrush& brush);

  /*! \brief Returns the brush for the selected area.
   */
  const WBrush& selectedAreaBrush() const { return selectedAreaBrush_; }

  /*! \brief Sets an internal margin for the selection area.
   *
   * This configures the area (in pixels) around the selection area that
   * is available for the axes and labels, and the handles.
   *
   * Alternatively, you can configure the chart layout to be computed automatically using setAutoLayoutEnabled().
   *
   * \sa setAutoLayoutEnabled()
   */
  void setSelectionAreaPadding(int padding, WFlags<Side> sides = AllSides);

  /*! \brief Returns the internal margin for the selection area.
   *
   * This is either the padding set through setSelectionAreaPadding() or computed using setAutoLayoutEnabled().
   *
   * \sa setPlotAreaPadding()
   */
  int selectionAreaPadding(Side side) const;

  /*! \brief Configures the axis slider layout to be automatic.
   *
   * This configures the selection area so that the space around it is suited for the text that is rendered.
   */
  void setAutoLayoutEnabled(bool enabled = true);

  /*! \brief Returns whether chart layout is computed automatically.
   *
   * \sa setAutoLayoutEnabled()
   */
  bool isAutoLayoutEnabled() const { return autoPadding_; }

  /*! \brief Set whether to draw the X axis tick labels on the slider widget.
   *
   * AxisProperty::Labels are enabled by default.
   */
  void setLabelsEnabled(bool enabled = true);

  /*! \brief Returns whether the X axis tick labels are drawn.
   *
   * \sa setLabelsEnabled()
   */
  bool isLabelsEnabled() const { return labelsEnabled_; }

  /*! \brief Set whether the Y axis of the associated chart should be updated to fit the series.
   *
   * Y axis zoom is enabled by default.
   */
  void setYAxisZoomEnabled(bool enabled = true);

  /*! \brief Returns whether the Y axis of the associated chart should be updated to fit the series.
   *
   * \sa setYAxisZoomEnabled()
   */
  bool isYAxisZoomEnabled() const { return yAxisZoomEnabled_; }

  WDataSeries *series() { return series_; }
  const WDataSeries *series() const { return series_; }

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void paintEvent(WPaintDevice *paintDevice) override;

private:
  void init();
  std::string sObjJsRef() const;
  WRectF hv(const WRectF& rect) const;
  WTransform hv(const WTransform& t) const;

  WCartesianChart *chart();
  const WCartesianChart *chart() const;

  WDataSeries *series_;
  WPen seriesPen_;
  WPen *selectedSeriesPen_;
  WBrush handleBrush_;
  WBrush background_;
  WBrush selectedAreaBrush_;
  bool autoPadding_;
  bool labelsEnabled_;
  bool yAxisZoomEnabled_;
  int padding_[4];

  WJavaScriptHandle<WTransform> transform_;
};

  }
}

#endif // CHART_WAXIS_SLIDER_WIDGET_H_
