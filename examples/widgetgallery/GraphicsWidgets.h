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
  std::unique_ptr<WWidget> painting2d();
  std::unique_ptr<WWidget> paintbrush();
  std::unique_ptr<WWidget> categoryChart();
  std::unique_ptr<WWidget> scatterPlot();
  std::unique_ptr<WWidget> axisSliderWidget();
  std::unique_ptr<WWidget> pieChart();
  std::unique_ptr<WWidget> googleMap();
  std::unique_ptr<WWidget> painting3d();
  std::unique_ptr<WWidget> numCharts3d();
  std::unique_ptr<WWidget> catCharts3d();

  Wt::WAbstractItemModel *readCsvFile(const std::string &fname,
		  WContainerWidget *parent);
};

#endif // GRAPHICS_WIDGETS_H_
