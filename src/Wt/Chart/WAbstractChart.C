/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractItemModel"
#include "Wt/WLogger"
#include "Wt/Chart/WAbstractChart"
#include "Wt/Chart/WAbstractChartModel"
#include "Wt/Chart/WChartPalette"
#include "Wt/Chart/WStandardChartProxyModel"

namespace Wt {

LOGGER("Chart.WAbstractChart");

  namespace Chart {

WAbstractChart::WAbstractChart(WContainerWidget *parent)
  : WPaintedWidget(parent),
    model_(0),
    background_(white),
    palette_(0),
    autoPadding_(false)
{
  titleFont_.setFamily(WFont::SansSerif);
  titleFont_.setSize(WFont::FixedSize, WLength(15, WLength::Point));

  setPlotAreaPadding(5, Left | Right);
  setPlotAreaPadding(5, Top | Bottom);
}

WAbstractChart::~WAbstractChart()
{
  delete palette_;
}

void WAbstractChart::setPalette(WChartPalette *palette)
{
  delete palette_;
  palette_ = palette;

  update();
}

void WAbstractChart::setPlotAreaPadding(int padding, WFlags<Side> sides)
{
  if (sides & Top)
    padding_[0] = padding;
  if (sides & Right)
    padding_[1] = padding;
  if (sides & Bottom)
    padding_[2] = padding;
  if (sides & Left)
    padding_[3] = padding;
}

int WAbstractChart::plotAreaPadding(Side side) const
{
  switch (side) {
  case Top:
    return padding_[0];
  case Right:
    return padding_[1];
  case Bottom:
    return padding_[2];
  case Left:
    return padding_[3];
  default:
    LOG_ERROR("plotAreaPadding(): improper side.");
    return 0;
  }
}

void WAbstractChart::setAutoLayoutEnabled(bool enabled)
{
  autoPadding_ = enabled;
}

void WAbstractChart::setBackground(const WBrush& background)
{
  set(background_, background);
}

void WAbstractChart::setTitle(const WString& title)
{
  set(title_, title);
}

void WAbstractChart::setTitleFont(const WFont& titleFont)
{
  set(titleFont_, titleFont);
}

void WAbstractChart::setAxisTitleFont(const WFont& titleFont)
{
  set(axisTitleFont_, titleFont);
}

void WAbstractChart::setModel(WAbstractChartModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();

    modelConnections_.clear();

#ifndef WT_TARGET_JAVA
    if (model_->parent() == this) {
      WObject::removeChild(model_);
      delete model_;
    }
#endif
  }

  model_ = model;

  modelConnections_.push_back(model_->changed().connect
		      (this, &WAbstractChart::modelReset));

  modelChanged();
}

void WAbstractChart::setModel(WAbstractItemModel *model)
{
#ifndef WT_TARGET_JAVA
  setModel(new WStandardChartProxyModel(model, this));
#else
  setModel(new WStandardChartProxyModel(model));
#endif
}

WAbstractItemModel *WAbstractChart::itemModel() const
{
  WStandardChartProxyModel *proxy
    = dynamic_cast<WStandardChartProxyModel *>(model_);
  if (proxy)
    return proxy->sourceModel();
  else
    return 0;
}

void WAbstractChart::modelChanged()
{ }

void WAbstractChart::modelReset()
{ }

  }
}
