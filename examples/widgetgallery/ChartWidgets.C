/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ChartWidgets.h"
#include "ChartsExample.h"
#include "DeferredWidget.h"

#include <Wt/WText>
#include <Wt/WBreak>

using namespace Wt;

ChartWidgets::ChartWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("charts-intro"), this);
  new WText(tr("charts-introduction"), this);
}

void ChartWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Category Charts",
		deferCreate(boost::bind(&ChartWidgets::category, this)));
  menu->addItem("Scatter Plots",
		deferCreate(boost::bind(&ChartWidgets::scatterplot, this)));
  menu->addItem("Pie Charts",
		deferCreate(boost::bind(&ChartWidgets::pie, this)));
}

WWidget *ChartWidgets::category()
{
  WContainerWidget *retval = new WContainerWidget(0);
  topic("Chart::WCartesianChart", retval);
  new CategoryExample(retval);
  return retval;
}

WWidget *ChartWidgets::scatterplot()
{
  WContainerWidget *retval = new WContainerWidget(0);
  topic("Chart::WCartesianChart", retval);
  new TimeSeriesExample(retval);
  new ScatterPlotExample(retval);
  return retval;
}

WWidget *ChartWidgets::pie()
{
  WContainerWidget *retval = new WContainerWidget(0);
  topic("Chart::WPieChart", retval);
  new PieExample(retval);
  return retval;
}
