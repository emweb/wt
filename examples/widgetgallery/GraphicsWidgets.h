// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef GRAPHICS_WIDGETS_H_
#define GRAPHICS_WIDGETS_H_

#include "TopicWidget.h"

class GraphicsWidgets : public TopicWidget
{
public:
  GraphicsWidgets();

  virtual void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *painting2d();
  Wt::WWidget *paintbrush();
  Wt::WWidget *categoryChart();
  Wt::WWidget *scatterPlot();
  Wt::WWidget *pieChart();
  Wt::WWidget *googleMap();
  Wt::WWidget *painting3d();
  Wt::WWidget *numCharts3d();
  Wt::WWidget *catCharts3d();

  Wt::WAbstractItemModel *readCsvFile(const std::string &fname,
                  Wt::WContainerWidget *parent);
};

#endif // GRAPHICS_WIDGETS_H_
