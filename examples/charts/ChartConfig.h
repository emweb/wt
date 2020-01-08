// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_CONFIG_H_
#define CHART_CONFIG_H_

#include <Wt/WContainerWidget.h>
#include <Wt/Chart/WDataSeries.h>

namespace Wt {
  class WCheckBox;
  class WComboBox;
  class WFormWidget;
  class WLineEdit;
  class WStandardItemModel;
  class WTable;
  class WValidator;

  namespace Chart {
    class WAxis;
    class WCartesianChart;
  }
}

/**
 * @addtogroup chartsexample
 */
/*@{*/

/*! \brief A class that allows configuration of a cartesian chart.
 *
 * This widget provides forms for configuring chart, series, and axis properties
 * and manipulates the chart according to user settings.
 *
 * This widget is part of the %Wt charts example.
 */
class ChartConfig : public Wt::WContainerWidget
{
public:
  /*! \brief Constructor.
   */  
  ChartConfig(Wt::Chart::WCartesianChart *chart);

  void setValueFill(Wt::Chart::FillRangeType fill);

private:
  Wt::Chart::WCartesianChart  *chart_;
  Wt::Chart::FillRangeType     fill_;

  //! Struct that holds the controls for one series
  struct SeriesControl {
    Wt::WCheckBox *enabledEdit;
    Wt::WComboBox *typeEdit;
    Wt::WComboBox *markerEdit;
    Wt::WComboBox *axisEdit;
    Wt::WCheckBox *legendEdit;
    Wt::WCheckBox *shadowEdit;
    Wt::WComboBox *labelsEdit;
  };

  //! Controls for series
  std::vector<SeriesControl> seriesControls_;

  //! Struct that holds the controls for one axis
  struct AxisControl {
    Wt::WCheckBox *visibleEdit;
    Wt::WComboBox *scaleEdit;
    Wt::WCheckBox *autoEdit;
    Wt::WLineEdit *minimumEdit;
    Wt::WLineEdit *maximumEdit;
    Wt::WCheckBox *gridLinesEdit;
    Wt::WLineEdit *labelAngleEdit;
    Wt::WLineEdit *titleEdit;
    Wt::WComboBox *titleOrientationEdit;
    Wt::WComboBox *tickDirectionEdit;
    Wt::WComboBox *locationEdit;
  };

  //! Controls for axes
  std::vector<AxisControl> axisControls_;

  Wt::WLineEdit *titleEdit_;
  Wt::WLineEdit *chartWidthEdit_;
  Wt::WLineEdit *chartHeightEdit_;
  Wt::WComboBox *chartOrientationEdit_;
  Wt::WComboBox *legendLocationEdit_;
  Wt::WComboBox *legendSideEdit_;
  Wt::WComboBox *legendAlignmentEdit_;
  Wt::WCheckBox *borderEdit_;

  std::shared_ptr<Wt::WStandardItemModel> yAxesModel_, xScales_, yScales_;
  Wt::WTable *axisConfig_;
  std::shared_ptr<Wt::WValidator> anyNumberValidator_, angleValidator_;

  void connectSignals(Wt::WFormWidget *w);
  void update();
  void addYAxis();
  void addAxis(Wt::Chart::Axis axis, int yAxis);
  void removeYAxis(const Wt::Chart::WAxis *axis);
  void clearYAxes();

  static bool validate(Wt::WFormWidget *w);
};

/*@}*/

#endif // CHARTS_EXAMPLE_H_
