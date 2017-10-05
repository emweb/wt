// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WPIE_CHART_H_
#define CHART_WPIE_CHART_H_

#include <Wt/Chart/WAbstractChart.h>

#include <Wt/WRectF.h>

namespace Wt {

  class WPainter;

  namespace Chart {

    class WChartPalette;

/*! \brief Enumeration that specifies options for the labels.
 *
 * \sa WPieChart::setDisplayLabels(WFlags<LabelOption>)
 *
 * \ingroup charts
 */
enum class LabelOption {
  None           = 0x00, //!< Do not display labels (default).
  Inside         = 0x01, //!< Display labels inside each segment.
  Outside        = 0x02, //!< Display labels outside each segment.
  TextLabel      = 0x10, //!< Display the label text
  TextPercentage = 0x20  //!< Display the value (as percentage)
};

W_DECLARE_OPERATORS_FOR_FLAGS(LabelOption)

/*! \class WPieChart Wt/Chart/WPieChart.h Wt/Chart/WPieChart.h
 *  \brief A pie chart.
 *
 * A pie chart renders a single data series as segments of a circle, so that
 * the area of each segment is proportional to the value in the data series.
 *
 * To use a pie chart, you need to set a model using setModel(), and use
 * setLabelsColumn() and setDataColumn() to specify the model column that
 * contains the category labels and data.
 *
 * The pie chart may be customized visually by enabling a 3D effect
 * (setPerspectiveEnabled()), or by specifying the angle of the first
 * segment. One or more segments may be exploded, which separates the
 * segment from the rest of the pie chart, using setExplode().
 *
 * The segments may be labeled in various ways using
 * setDisplayLabels().
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \image html ChartWPieChart-1.png "Example of a pie chart" 
 *
 * \sa WCartesianChart
 *
 * \ingroup charts modelview
 */
class WT_API WPieChart : public WAbstractChart
{
public:
  /*! \brief Creates a new pie chart.
   */
  WPieChart();

  /*! \brief Sets the model column that holds the labels.
   *
   * The labels are used only when setDisplayLabels() is called with
   * the \link Chart::LabelOption::TextLabel LabelOption::TextLabel\endlink option.
   *
   * The default value is -1 (not defined).
   *
   * \sa setModel(WAbstractItemModel *), setDisplayLabels(), setDataColumn(int)
   */
  void setLabelsColumn(int column);

  /*! \brief Returns the model column used for the labels.
   *
   * \sa setLabelsColumn(int)
   */
  int labelsColumn() const { return labelsColumn_; }

  /*! \brief Sets the model column that holds the data.
   *
   * \if cpp 
   * The data column should contain data that can be converted to
   * a number, but should not necessarily be of a number type, see
   * also asNumber(const boost::any&).
   * \elseif java
   * The data column should contain data that can be converted to
   * a number, but should not necessarily be of a number type, see
   * also {javadoclink StringUtils#asNumber(Object)}.
   * \endif
   *
   * The default value is -1 (not defined).
   *
   * \sa setModel(WAbstractItemModel *), setLabelsColumn(int)
   */
  void setDataColumn(int modelColumn);

  /*! \brief Returns the model column used for the data.
   *
   * \sa setDataColumn(int)
   */
  int dataColumn() const { return dataColumn_; }

  /*! \brief Customizes the brush used for a pie segment.
   *
   * By default, the brush is taken from the palette(). You can use
   * this method to override the palette's brush for a particular
   * <i>modelRow</i>.
   *
   * \sa setPalette(WChartPalette *)
   */
  void setBrush(int modelRow, const WBrush& brush);

  /*! \brief Returns the brush used for a pie segment.
   *
   * \sa setBrush(int, const WBrush&)
   */
  WBrush brush(int modelRow) const;

  /*! \brief Sets the explosion factor for a pie segment.
   *
   * Separates the segment corresponding to model row <i>modelRow</i>
   * from the rest of the pie. The <i>factor</i> is a positive number
   * that represents the distance from the center as a fraction of the
   * pie radius. Thus, 0 corresponds to no separation, and 0.1 to a
   * 10% separation, and 1 to a separation where the segment tip is on
   * the outer perimeter of the pie.
   *
   * The default value is 0.
   */
  void setExplode(int modelRow, double factor);

  /*! \brief Returns the explosion factor for a segment.
   *
   * \sa setExplode(int, double)
   */
  double explode(int modelRow) const;

  /*! \brief Enables a 3D perspective effect on the pie.
   *
   * A 3D perspective effect is added, which may be customized by
   * specifying the simulated <i>height</i> of the pie. The height is
   * defined as a fraction of the pie radius.
   *
   * The default value is false.
   */
  void setPerspectiveEnabled(bool enabled, double height = 1.0);

  /*! \brief Returns whether a 3D effect is enabled.
   *
   * \sa setPerspectiveEnabled(bool, double)
   */
  bool isPerspectiveEnabled() const { return height_ > 0.0; }

  /*! \brief Enables a shadow effect.
   *
   * A soft shadow effect is added.
   *
   * The default value is false.
   */
  void setShadowEnabled(bool enabled);

  /*! \brief Returns whether a shadow effect is enabled.
   *
   * \sa setShadowEnabled()
   */
  bool isShadowEnabled() const { return shadow_; }

  /*! \brief Sets the angle of the first segment.
   *
   * The default value is 45 degrees.
   */
  void setStartAngle(double degrees);

  /*! \brief Returns the angle of the first segment.
   *
   * \sa setStartAngle(double)
   */
  double startAngle() const { return startAngle_; }

  /*! \brief Sets the percentage value to avoid rendering of label texts.
   *
   * The default value is 0 percent.
   */
  void setAvoidLabelRendering(double percent);

  /*! \brief Returns the percentage to avoid label rendering.
   *
   * \sa setAvoidLabelRendering(double)
   */
  double avoidLabelRendering() const { return avoidLabelRendering_; }

  /*! \brief Configures if and how labels should be displayed
   *
   * The <i>options</i> must be the logical OR of a placement option
   * (\link Chart::LabelOption::Inside LabelOption::Inside\endlink or \link
   * Chart::LabelOption::Outside LabelOption::Outside\endlink) and \link
   * Chart::LabelOption::TextLabel LabelOption::TextLabel\endlink and/or \link
   * Chart::LabelOption::TextPercentage LabelOption::TextPercentage\endlink. If both
   * LabelOption::TextLabel and LabelOption::TextPercentage are specified, then these are
   * combined as "<label>: <percentage>".
   *
   * The default value is \link Chart::LabelOption::None LabelOption::None\endlink.
   */
  void setDisplayLabels(WFlags<LabelOption> options);

  /*! \brief Returns options set for displaying labels.
   *
   * \sa WPieChart::setDisplayLabels()
   */
  WFlags<LabelOption> displayLabels() const { return labelOptions_; }

  /*! \brief Sets the label format.
   *
   * Sets a format string which is used to format label (percentage)
   * values.
   *
   * The format string must be a format string that is accepted by
   * snprintf() and which formats one double. If the format string is
   * an empty string, then WLocale::toString() is used.
   *
   * The default value is "%.3g%%".
   *
   * \sa labelFormat()
   */
  void setLabelFormat(const WString& format);

  /*! \brief Returns the label format string.
   * 
   * \sa setLabelFormat()
   */
  WString labelFormat() const;

  /*! \brief Creates a widget which renders the a legend item.
   * 
   * Depending on the passed LabelOption flags, the legend item widget,
   * will contain a text (with or without the percentage) and/or a span with
   * the segment's color.
   */
  std::unique_ptr<WWidget> createLegendItemWidget(int index,
						  WFlags<LabelOption> options);

  /*! \brief Adds a data point area (used for displaying e.g. tooltips).
   *
   * You may want to specialize this is if you wish to modify (or delete)
   * the area.
   *
   * \note Currently, an area is only created if the ItemDataRole::ToolTip data at the
   *       data point is not empty.
   */
  virtual void addDataPointArea(int row, int column,
                                std::unique_ptr<WAbstractArea> area) const;

  /**
   * @brief createLabelWidget possition textWidget where the text would be
   * rendered.
   * Assuming that textWidget is added to a container with same dimensions as
   * the WPieChart.
   * This should be used in combinaltion with drawLabel().
   *
   * @return The new WContainerWidget that contains textWidget and can be placed
   * on an other layer that has the same dimensions as the WPieChart.
   * \sa drawLabel()
   *
   * Usage example, PieChart with label links.
   * \code
   * class PChart : public Wt::Chart::WPieChart {
   * private:
   *   Wt::WContainerWidget *widgetsLayer_;
   * public:
   *   PChart(Wt::WContainerWidget *widgetsLayer) :
   *     widgetsLayer_(widgetsLayer)
   *   {
   *     widgetsLayer_->resize(800, 300);
   *     resize(800, 300);
   *     widgetsLayer_->setPositionScheme(Wt::PositionScheme::Relative);
   *   }
   *   virtual void drawLabel(Wt::WPainter* painter, const Wt::WRectF& rect,
   *                            Wt::WFlags<Wt::AlignmentFlag> alignmentFlags,
   *                            const Wt::WString& text, int row) const
   *   {
   *     if (model()->link(row, dataColumn()) == 0) {
   *       WPieChart::drawLabel(painter, rect, alignmentFlags, text, row);
   *     } else {
   *       auto a = std::make_unique<Wt::WAnchor>(
   *                  *model()->link(row, dataColumn()), text);
   *       widgetsLayer_->addWidget(createLabelWidget(std::move(a), painter, rect, alignmentFlags));
   *     }
   *   }
   * };
   * \endcode
   */
  virtual std::unique_ptr<WContainerWidget> createLabelWidget(std::unique_ptr<WWidget> textWidget,
      WPainter* painter, const WRectF& rect,
      Wt::WFlags<AlignmentFlag> alignmentFlags) const;

  virtual void paint(WPainter& painter, const WRectF& rectangle = WRectF())
    const override;

protected:
  void paintEvent(Wt::WPaintDevice *paintDevice) override;

  /**
   * @brief drawLabel draw a label on the chart. Will be called by paint.
   *
   * You may want to specialize this if you wish to replace the label by
   * a widget.
   * \sa createLabelWidget() if you wish to replace the label by a Widget.
   *
   */
  virtual void drawLabel(WPainter* painter, const WRectF& rect,
                           WFlags<AlignmentFlag> alignmentFlags,
                           const WString& text, int row) const;

private:
  int                  labelsColumn_;
  int                  dataColumn_;
  double               height_;
  double               startAngle_;
  double               avoidLabelRendering_;
  WFlags<LabelOption>  labelOptions_;
  bool                 shadow_;
  WString              labelFormat_;

  struct PieData {
    bool customBrush;
    WBrush brush;
    double explode;

    PieData();
  };

  std::vector<PieData> pie_;

protected:
  virtual void modelChanged() override;
  virtual void modelReset() override;

private:
  void drawPie(WPainter& painter, double cx, double cy, double r, double h,
	       double total) const;
  void drawSlices(WPainter& painter, double cx, double cy, double r,
		  double total, bool ignoreBrush) const;
  void drawSide(WPainter& painter, double pcx, double pcy, double r,
		double angle, double h) const;
  void drawOuter(WPainter& painter, double pcx, double pcy, double r,
		 double a1, double a2, double h) const;

  void setShadow(WPainter& painter) const;

  int prevIndex(int i) const;
  int nextIndex(int i) const;

  static WBrush darken(const WBrush& brush);

  WString labelText(int index, double v, double total, 
		    WFlags<LabelOption> options) const;
};

  }
}

#endif // CHART_WPIE_CHART_H_
