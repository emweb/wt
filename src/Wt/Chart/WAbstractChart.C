/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractItemModel.h"
#include "Wt/WLogger.h"
#include "Wt/Chart/WAbstractChart.h"
#include "Wt/Chart/WAbstractChartModel.h"
#include "Wt/Chart/WChartPalette.h"
#include "Wt/Chart/WStandardChartProxyModel.h"

namespace Wt {

LOGGER("Chart.WAbstractChart");

  namespace Chart {

WAbstractChart::WAbstractChart()
  : background_(StandardColor::White),
    autoPadding_(false)
{
  titleFont_.setFamily(FontFamily::SansSerif);
  titleFont_.setSize(WLength(15, LengthUnit::Point));

  setPlotAreaPadding(5, WFlags<Side>(Side::Left) | Side::Right);
  setPlotAreaPadding(5, WFlags<Side>(Side::Top) | Side::Bottom);
}

WAbstractChart::~WAbstractChart()
{ }

void WAbstractChart::setPalette(const std::shared_ptr<WChartPalette>& palette)
{
  palette_ = palette;

  update();
}

void WAbstractChart::setPlotAreaPadding(int padding, WFlags<Side> sides)
{
  if (sides.test(Side::Top))
    padding_[0] = padding;
  if (sides.test(Side::Right))
    padding_[1] = padding;
  if (sides.test(Side::Bottom))
    padding_[2] = padding;
  if (sides.test(Side::Left))
    padding_[3] = padding;
}

int WAbstractChart::plotAreaPadding(Side side) const
{
  switch (side) {
  case Side::Top:
    return padding_[0];
  case Side::Right:
    return padding_[1];
  case Side::Bottom:
    return padding_[2];
  case Side::Left:
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

void WAbstractChart::setModel(const std::shared_ptr<WAbstractChartModel>& model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  modelConnections_.push_back(model_->changed().connect
		      (this, &WAbstractChart::modelReset));

  modelChanged();
}

void WAbstractChart::setModel(const std::shared_ptr<WAbstractItemModel>& model)
{
  setModel(std::shared_ptr<WAbstractChartModel>(std::make_shared<WStandardChartProxyModel>(model)));
}

std::shared_ptr<WAbstractItemModel> WAbstractChart::itemModel() const
{
  WStandardChartProxyModel *proxy
    = dynamic_cast<WStandardChartProxyModel *>(model_.get());
  if (proxy)
    return proxy->sourceModel();
  else
    return nullptr;
}

void WAbstractChart::modelChanged()
{ }

void WAbstractChart::modelReset()
{ }

  }
}
