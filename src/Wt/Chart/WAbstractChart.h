// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WABSTRACT_CHART_H_
#define CHART_WABSTRACT_CHART_H_

#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WRectF.h>

namespace Wt {

  class WAbstractItemModel;
  class WPainter;
  class WRectF;

  /*! \brief Namespace for the \ref charts
   */
  namespace Chart {

    class WAbstractChartModel;
    class WChartPalette;

/*! \defgroup charts Charts (Wt::Chart)
 *  \brief A charting library implemented using the %Wt \ref painting
 *
 * The charting library contains two main chart widget classes,
 * WCartesianChart and WPieChart, and a number of utility classes
 * for drawing simple to complex charts.
 */

/*! \class WAbstractChart Wt/Chart/WAbstractChart.h Wt/Chart/WAbstractChart.h
 *  \brief Abstract base class for MVC-based charts.
 *
 * This is an abstract class and should not be used directly.
 *
 * As an abstract base for MVC-based charts, this class manages the
 * model setModel() and provides virtual methods that listen to
 * model changes. In addition, it gives access to generic chart
 * properties such as the title setTitle() and title font
 * setTitleFont(), the chart palette setPalette(), plot area
 * padding setPlotAreaPadding(), and the background fill color
 * setBackground().
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \sa WCartesianChart, WPieChart
 *
 * \ingroup charts modelview
 */
class WT_API WAbstractChart : public WPaintedWidget
{
public:
  /*! \brief Destructor.
   */
  virtual ~WAbstractChart();

  /*! \brief Sets the model.
   *
   * The model is used by the chart to get its data.
   *
   * The default model is \c nullptr.
   *
   * This creates an internal proxy model that presents the
   * WAbstractItemModel as a WAbstractChartModel. Use
   * setModel(const std::shared_ptr<WAbstractChartModel>&)
   * directly for highest performance (avoiding the overhead
   * of any for numeric data).
   *
   * \note Setting a new model on a WCartesianChart causes the
   *       WCartesianChart::XSeriesColumn() and all series to be cleared
   *
   * \sa model()
   */
  void setModel(const std::shared_ptr<WAbstractItemModel>& model);

  /*! \brief Sets the model.
   *
   * The model is used by the chart to get its data.
   *
   * The default model is a \c nullptr.
   *
   * \note Setting a new model on a WCartesianChart causes the
   *       WCartesianChart::XSeriesColumn() and all series to be cleared
   *
   * \sa model()
   */
  void setModel(const std::shared_ptr<WAbstractChartModel>& model);

  /*! \brief Returns the model.
   *
   * \sa setModel(const std::shared_ptr<WAbstractChartModel> &)
   */
  std::shared_ptr<WAbstractChartModel> model() const { return model_; }

  /*! \brief Returns the model.
   *
   * If a model was set using setModel(const std::shared_ptr<WAbstractItemModel>&), then this
   * model will be returned by this call.
   *
   * \sa setModel(const std::shared_ptr<WAbstractChartModel> &)
   */
  std::shared_ptr<WAbstractItemModel> itemModel() const;

  /*! \brief Sets a background for the chart.
   *
   * Set the background color for the main plot area.
   *
   * The default is a completely transparent background.
   *
   * \sa background()
   */
  void setBackground(const WBrush& background);

  /*! \brief Returns the background of the chart.
   *
   * \sa setBackground(const WBrush&)
   */
  const WBrush& background() const { return background_; }

  /*! \brief Set a palette for the chart.
   *
   * A palette is used to provide the style information to render the
   * chart series.
   *
   * The default palette is dependent on the chart type.
   *
   * \sa palette()
   */
  void setPalette(const std::shared_ptr<WChartPalette>& palette);

  /*! \brief Returns the palette for the chart.
   *
   * \sa setPalette(WChartPalette *palette)
   */
  std::shared_ptr<WChartPalette> palette() const { return palette_; }

  /*! \brief Set an internal margin for the main plot area.
   *
   * This configures the area (in pixels) around the plot area that is
   * available for axes, labels, and titles.
   *
   * The default is dependent on the chart type.
   *
   * Alternatively, you can configure the chart layout to be computed
   * automatically using setAutoLayoutEnabled().
   *
   * \sa setAutoLayoutEnabled()
   */
  void setPlotAreaPadding(int padding, WFlags<Side> sides = AllSides);

  /*! \brief Returns the internal margin for the main plot area.
   *
   * This is either the paddings set through setPlotAreaPadding() or computed
   * using setAutoLayoutEnabled()
   *
   * \sa setPlotAreaPadding(int, WFlags<Side>)
   */
  int plotAreaPadding(Side side) const;

  /*! \brief Configures the chart layout to be automatic.
   *
   * This configures the plot area so that the space around it is
   * suited for the text that is rendered (axis labels and text, the
   * title, and legend).
   *
   * The default value is \c false, and the chart layout is set manually
   * using values set in setPlotAreaPadding().
   */
  void setAutoLayoutEnabled(bool enabled = true);

  /*! \brief Returns whether chart layout is computed automatically.
   *
   * \sa setAutoLayoutEnabled()
   */
  bool isAutoLayoutEnabled() const { return autoPadding_; }

  /*! \brief Set a chart title.
   *
   * The title is displayed on top of the chart, using the titleFont().
   *
   * The default title is an empty title ("").
   *
   * \sa title()
   */
  void setTitle(const WString& title);

  /*! \brief Return the chart title.
   *
   * \sa title()
   */
  const WString& title() const { return title_; }

  /*! \brief Set the font for the chart title.
   *
   * Changes the font for the chart title.
   *
   * The default title font is a 15 point Sans Serif font.
   *
   * \sa titleFont(), setTitle(const WString&)
   */
  void setTitleFont(const WFont& titleFont);

  /*! \brief Returns the font for the chart title.
   *
   * \sa setTitleFont(const WFont&)
   */
  const WFont& titleFont() const { return titleFont_; }

  void setAxisTitleFont(const WFont& titleFont);
  const WFont& axisTitleFont() const { return titleFont_; }

  /*! \brief Paint the chart in a rectangle of the given painter.
   *
   * Paints the chart inside the <i>painter</i>, in the area indicated
   * by <i>rectangle</i>.  When <i>rectangle</i> is a null rectangle,
   * the entire painter \link WPainter::window() window\endlink is
   * used.
   */
  virtual void paint(WPainter& painter, const WRectF& rectangle = WRectF())
    const = 0;

protected:
  WAbstractChart();

private:
  std::shared_ptr<WAbstractChartModel> model_;
  std::shared_ptr<WChartPalette> palette_;
  WBrush background_;
  bool autoPadding_;
  int padding_[4];
  WString title_;
  WFont titleFont_;
  WFont axisTitleFont_;

  // connections with the current model, used to disconnect from a model
  // when the model changes.
  std::vector<Wt::Signals::connection> modelConnections_;

protected:
  virtual void modelChanged();

  virtual void modelReset();

private:
  template <typename T>
  void set(T& m, const T& v);

  void modelDataChanged(int row1, int column1, int row2, int column2);
};

template <typename T>
void WAbstractChart::set(T& m, const T& v)
{
  if (m != v) {
    m = v;
    update();
  }
}


  }
}

#endif // CHART_WABSTRACT_CHART_H_
