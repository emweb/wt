// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHARTS_EXAMPLE_H_
#define CHARTS_EXAMPLE_H_

#include <Wt/WContainerWidget>

#include <iostream>

namespace Wt {
  class WAbstractItemModel;

  namespace Ext {
    class TableView;
  }
}

/**
 * \defgroup chartsexample Charts example
 */
/*@{*/

/*! \brief A widget that demonstrates a times series chart
 */
class TimeSeriesExample: public Wt::WContainerWidget
{
public:
  /*! \brief Creates the time series scatter plot example
   */
  TimeSeriesExample(Wt::WContainerWidget *parent);
};

/*! \brief A Widget that demonstrates a category chart
 */
class CategoryExample: public Wt::WContainerWidget
{
public:
  /*! \brief Creates the category chart example
   */
  CategoryExample(Wt::WContainerWidget *parent);
};

/*! \brief A Widget that demonstrates a scatter plot
 */
class ScatterPlotExample: public Wt::WContainerWidget
{
public:
  /*! \brief Creates the scatter plot example
   */
  ScatterPlotExample(Wt::WContainerWidget *parent);
};

/*! \brief A Widget that demonstrates a Pie chart
 */
class PieExample: public Wt::WContainerWidget
{
public:
  /*! \brief Creates the pie chart example
   */
  PieExample(Wt::WContainerWidget *parent);
};

/*! \brief A widget that demonstrates various aspects of the charting lib.
 */
class ChartsExample : public Wt::WContainerWidget
{
public:
  /*! \brief Constructor.
   */
  ChartsExample(Wt::WContainerWidget *root);
};

/*@}*/

#endif // CHARTS_EXAMPLE_H_
