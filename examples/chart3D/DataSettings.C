#include "DataSettings.h"

#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WCheckBox>
#include <Wt/WSlider>
#include <Wt/WTemplate>
#include <Wt/WIntValidator>
#include <Wt/Chart/WAbstractDataSeries3D>
#include <Wt/Chart/WGridData>
#include <Wt/Chart/WScatterData>
#include <Wt/Chart/WStandardColorMap>

DataSettings::DataSettings()
  : data_(0)
{
  setName_ = new WLineEdit(this);
  pointsize_ = new WLineEdit(this);
  pointsize_->setValidator(new WIntValidator(1, 10));
  pointSprite_ = new WComboBox(this);
  pointSprite_->addItem("None");
  pointSprite_->addItem("diamond (5x5)");
  pointSprite_->addItem("cross (5x5)");
  colormap_ = new WComboBox(this);
  colormap_->addItem("None");
  colormap_->addItem("Continuous");
  colormap_->addItem("Continuous (5 bands)");
  colormap_->addItem("Continuous (10 bands)");
  showColormap_ = new WCheckBox(this);
  colormapSide_ = new WComboBox(this);
  colormapSide_->addItem("Left");
  colormapSide_->addItem("Right");

  hide_ = new WCheckBox(this);

  // hook up the UI to the dataset
  setName_->changed().connect(std::bind([&] () {
	data_->setTitle(setName_->text());
      }));
  pointsize_->changed().connect(std::bind([&] () {
	data_->setPointSize(Wt::asNumber(pointsize_->text()));
      }));
  pointSprite_->changed().connect(std::bind([&] () {
	switch (pointSprite_->currentIndex()) {
	case 0:
	  data_->setPointSprite("");
	  break;
	case 1:
	  data_->setPointSprite("diamond.png");
	  break;
	case 2:
	  data_->setPointSprite("cross.png");
	  break;
	}
      }));
  colormap_->changed().connect(std::bind([&] () {
	WStandardColorMap *colMap;
	switch (colormap_->currentIndex()) {
	case 0:
	  data_->setColorMap(0); break;
	case 1:
	    data_->setColorMap(new WStandardColorMap(data_->minimum(ZAxis_3D), data_->maximum(ZAxis_3D), true));
	    break;
	case 2:
	  colMap = new WStandardColorMap(data_->minimum(ZAxis_3D), data_->maximum(ZAxis_3D), true);
	  colMap->discretise(5);
	  data_->setColorMap(colMap);
	  break;
	case 3:
	  colMap = new WStandardColorMap(data_->minimum(ZAxis_3D), data_->maximum(ZAxis_3D), true);
	  colMap->discretise(10);
	  data_->setColorMap(colMap);
	  break;
	}
      }));
  showColormap_->changed().connect(std::bind([&] () {
	data_->setColorMapVisible(showColormap_->checkState() == Checked);
      }));
  colormapSide_->changed().connect(std::bind([&] () {
	if (colormapSide_->currentIndex() == 0)
	  data_->setColorMapSide(Left);
	else
	  data_->setColorMapSide(Right);
      }));
  hide_->changed().connect(std::bind([&] () {
	data_->setHidden(hide_->checkState() == Checked);
      }));
}

// the correct bind-points must be present in the template
void DataSettings::bindBaseToTemplate(WTemplate* configtemplate)
{
  configtemplate->bindWidget("setname", setName_);
  configtemplate->bindWidget("ptsize", pointsize_);
  configtemplate->bindWidget("ptsprite", pointSprite_);
  configtemplate->bindWidget("colormap", colormap_);
  configtemplate->bindWidget("showcolormap", showColormap_);
  configtemplate->bindWidget("colormapside", colormapSide_);
  configtemplate->bindWidget("hide", hide_);
}

void DataSettings::bindBaseDataSet(WAbstractDataSeries3D *data)
{
  data_ = data;

  // Update the UI values
  setName_->setText(data->title());

  pointsize_->setText(Wt::asString(data->pointSize()));

  const std::string &sprite = data->pointSprite();
  if (sprite == "") {
    pointSprite_->setCurrentIndex(0);
  } else if (sprite == "diamond.png") {
    pointSprite_->setCurrentIndex(1);
  } else if (sprite == "cross.png") {
    pointSprite_->setCurrentIndex(2);
  }

  if (data->colorMap() == 0) {
    colormap_->setCurrentIndex(0);
  } else {
    const WStandardColorMap *map = dynamic_cast<const WStandardColorMap*>
      (data->colorMap());
    if (!map->continuous()) {
      if (map->colorValuePairs().size() == 5)
	colormap_->setCurrentIndex(2);
      else if (map->colorValuePairs().size() == 10)
	colormap_->setCurrentIndex(3);
    } else {
      colormap_->setCurrentIndex(1);
    }
  }

  if (data->colorMapVisible())
    showColormap_->setCheckState(Checked);
  else
    showColormap_->setCheckState(Unchecked);

  if (data->colorMapSide() == Left)
    colormapSide_->setCurrentIndex(0);
  else
    colormapSide_->setCurrentIndex(1);

  if (data->isHidden())
    hide_->setCheckState(Checked);
  else
    hide_->setCheckState(Unchecked);
}


/*
 * Definition of class responsible for configuring numerical grid data-sets
 */
NumGridDataSettings::NumGridDataSettings()
  : changeXClippingMin_(1, this),
    changeXClippingMax_(1, this),
    changeYClippingMin_(1, this),
    changeYClippingMax_(1, this),
    changeZClippingMin_(1, this),
    changeZClippingMax_(1, this),
    gridData_(0)
{
  WTemplate* template_ = new WTemplate(Wt::WString::tr("numgriddata-template"), this);
  bindBaseToTemplate(template_);

  typeSelection_ = new WComboBox(this);
  typeSelection_->addItem("Points");
  typeSelection_->addItem("Surface");
  template_->bindWidget("datatype", typeSelection_);
  enableMesh_ = new WCheckBox(this);
  template_->bindWidget("enablemesh", enableMesh_);
  penSize_ = new WLineEdit(this);
  penSize_->setValidator(new WIntValidator(1, 10));
  template_->bindWidget("pensize", penSize_);
  penColor_ = new WComboBox(this);
  penColor_->addItem("black");
  penColor_->addItem("red");
  penColor_->addItem("green");
  penColor_->addItem("blue");
  template_->bindWidget("pencolor", penColor_);

  xClippingMin_ = new WSlider(Wt::Horizontal, this);
  xClippingMin_->setMinimum(-100);
  xClippingMin_->setMaximum(100);
  xClippingMin_->setValue(-100);
  template_->bindWidget("x-clipping-min", xClippingMin_);
  xClippingMax_ = new WSlider(Wt::Horizontal, this);
  xClippingMax_->setMinimum(-100);
  xClippingMax_->setMaximum(100);
  xClippingMax_->setValue(100);
  template_->bindWidget("x-clipping-max", xClippingMax_);
  yClippingMin_ = new WSlider(Wt::Horizontal, this);
  yClippingMin_->setMinimum(-100);
  yClippingMin_->setMaximum(100);
  yClippingMin_->setValue(-100);
  template_->bindWidget("y-clipping-min", yClippingMin_);
  yClippingMax_ = new WSlider(Wt::Horizontal, this);
  yClippingMax_->setMinimum(-100);
  yClippingMax_->setMaximum(100);
  yClippingMax_->setValue(100);
  template_->bindWidget("y-clipping-max", yClippingMax_);
  zClippingMin_ = new WSlider(Wt::Horizontal, this);
  zClippingMin_->setMinimum(-100);
  zClippingMin_->setMaximum(100);
  zClippingMin_->setValue(-100);
  template_->bindWidget("z-clipping-min", zClippingMin_);
  zClippingMax_ = new WSlider(Wt::Horizontal, this);
  zClippingMax_->setMinimum(-100);
  zClippingMax_->setMaximum(100);
  zClippingMax_->setValue(100);
  template_->bindWidget("z-clipping-max", zClippingMax_);
  showClippingLines_ = new WCheckBox(this);
  template_->bindWidget("clippinglines", showClippingLines_);
  clippingLinesColor_ = new WComboBox(this);
  clippingLinesColor_->addItem("black");
  clippingLinesColor_->addItem("red");
  clippingLinesColor_->addItem("green");
  clippingLinesColor_->addItem("blue");
  clippingLinesColor_->addItem("cyan");
  clippingLinesColor_->addItem("magenta");
  clippingLinesColor_->addItem("yellow");
  template_->bindWidget("clippinglines-color", clippingLinesColor_);
  showIsolines_ = new WCheckBox(this);
  template_->bindWidget("isolines", showIsolines_);
  isolineColormap_ = new WComboBox(this);
  isolineColormap_->addItem("None (use surface's colormap)");
  isolineColormap_->addItem("Continuous");
  isolineColormap_->addItem("Continuous (5 bands)");
  isolineColormap_->addItem("Continuous (10 bands)");
  template_->bindWidget("isoline-colormap", isolineColormap_);

  // make connections
  xClippingMin_->sliderMoved().connect(changeXClippingMin_);
  xClippingMax_->sliderMoved().connect(changeXClippingMax_);
  yClippingMin_->sliderMoved().connect(changeYClippingMin_);
  yClippingMax_->sliderMoved().connect(changeYClippingMax_);
  zClippingMin_->sliderMoved().connect(changeZClippingMin_);
  zClippingMax_->sliderMoved().connect(changeZClippingMax_);

  showClippingLines_->checked().connect(std::bind([&] () {
	gridData_->setClippingLinesEnabled(true);
      }));
  showClippingLines_->unChecked().connect(std::bind([&] () {
	gridData_->setClippingLinesEnabled(false);
      }));

  typeSelection_->changed().connect(std::bind([&] () {
	switch (typeSelection_->currentIndex()) {
	case 0:
	  gridData_->setType(PointSeries3D);
	  break;
	case 1:
	  gridData_->setType(SurfaceSeries3D);
	  break;
	}
      }));
  enableMesh_->changed().connect(std::bind([&] () {
	gridData_->setSurfaceMeshEnabled(enableMesh_->checkState() == Checked);
      }));
  penSize_->changed().connect(std::bind([&] () {
	WPen pen = gridData_->pen();
	pen.setWidth(Wt::asNumber(penSize_->text()));
	gridData_->setPen(pen);
      }));
  penColor_->changed().connect(std::bind([&] () {
	WPen pen = gridData_->pen();
	switch (penColor_->currentIndex()) {
	case 0:
	  pen.setColor(black); break;
	case 1:
	  pen.setColor(red); break;
	case 2:
	  pen.setColor(green); break;
	case 3:
	  pen.setColor(blue); break;
	}
	gridData_->setPen(pen);
      }));
  clippingLinesColor_->changed().connect(std::bind([&] () {
	switch (clippingLinesColor_->currentIndex()) {
	case 0:
	  gridData_->setClippingLinesColor(black); break;
	case 1:
	  gridData_->setClippingLinesColor(red); break;
	case 2:
	  gridData_->setClippingLinesColor(green); break;
	case 3:
	  gridData_->setClippingLinesColor(blue); break;
	case 4:
	  gridData_->setClippingLinesColor(cyan); break;
	case 5:
	  gridData_->setClippingLinesColor(magenta); break;
	case 6:
	  gridData_->setClippingLinesColor(yellow); break;
	}
      }));
  showIsolines_->checked().connect(std::bind([&] () {
	std::vector<double> isoLevels;
	for (double z = -20.0; z <= 20.0; z += 0.5) {
	  isoLevels.push_back(z);
	}
	gridData_->setIsoLevels(isoLevels);
      }));
  showIsolines_->unChecked().connect(std::bind([&] () {
	gridData_->setIsoLevels(std::vector<double>());
      }));
  isolineColormap_->changed().connect(std::bind([&] () {
	WStandardColorMap *colMap;
	switch (isolineColormap_->currentIndex()) {
	case 0:
	  gridData_->setIsoColorMap(0); break;
	case 1:
	    gridData_->setIsoColorMap(new WStandardColorMap(gridData_->minimum(ZAxis_3D), gridData_->maximum(ZAxis_3D), true));
	    break;
	case 2:
	  colMap = new WStandardColorMap(gridData_->minimum(ZAxis_3D), gridData_->maximum(ZAxis_3D), true);
	  colMap->discretise(5);
	  gridData_->setIsoColorMap(colMap);
	  break;
	case 3:
	  colMap = new WStandardColorMap(gridData_->minimum(ZAxis_3D), gridData_->maximum(ZAxis_3D), true);
	  colMap->discretise(10);
	  gridData_->setIsoColorMap(colMap);
	  break;
	}
      }));
}

void NumGridDataSettings::bindDataSet(WAbstractGridData *data)
{
  for (auto &conn : clippingConnections_) {
    conn.disconnect();
  }

  gridData_ = data;

  changeXClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(XAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeXClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(XAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeYClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(YAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeYClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(YAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeZClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(ZAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeZClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(ZAxis_3D).execJs("o","e","pos / 5.0") + " }", 1);

  xClippingMin_->setValue(std::max(gridData_->clippingMin(XAxis_3D) * 5, -100.0f));
  xClippingMax_->setValue(std::min(gridData_->clippingMax(XAxis_3D) * 5, 100.0f));
  yClippingMin_->setValue(std::max(gridData_->clippingMin(YAxis_3D) * 5, -100.0f));
  yClippingMax_->setValue(std::min(gridData_->clippingMax(YAxis_3D) * 5, 100.0f));
  zClippingMin_->setValue(std::max(gridData_->clippingMin(ZAxis_3D) * 5, -100.0f));
  zClippingMax_->setValue(std::min(gridData_->clippingMax(ZAxis_3D) * 5, 100.0f));

  clippingConnections_.push_back(xClippingMin_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMin(XAxis_3D, xClippingMin_->value() / 5.0);
  })));
  clippingConnections_.push_back(xClippingMax_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMax(XAxis_3D, xClippingMax_->value() / 5.0);
  })));
  clippingConnections_.push_back(yClippingMin_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMin(YAxis_3D, yClippingMin_->value() / 5.0);
  })));
  clippingConnections_.push_back(yClippingMax_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMax(YAxis_3D, yClippingMax_->value() / 5.0);
  })));
  clippingConnections_.push_back(zClippingMin_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMin(ZAxis_3D, zClippingMin_->value() / 5.0);
  })));
  clippingConnections_.push_back(zClippingMax_->valueChanged().connect(std::bind([&] () {
    gridData_->setClippingMax(ZAxis_3D, zClippingMax_->value() / 5.0);
  })));

  showClippingLines_->setChecked(gridData_->clippingLinesEnabled());

  // update UI fields
  DataSettings::bindBaseDataSet(data);

  if (gridData_->type() == PointSeries3D)
    typeSelection_->setCurrentIndex(0);
  else if (gridData_->type() == SurfaceSeries3D)
    typeSelection_->setCurrentIndex(1);

  enableMesh_->setCheckState(gridData_->isSurfaceMeshEnabled() ?
			     Checked : Unchecked);

  penSize_->setText(Wt::asString(gridData_->pen().width().value()));

  WPen pen = gridData_->pen();
  WColor penColor = pen.color();
  if (penColor == WColor(black)) {
    penColor_->setCurrentIndex(0);
  } else if (penColor == WColor(red)) {
    penColor_->setCurrentIndex(1);
  } else if (penColor == WColor(green)) {
    penColor_->setCurrentIndex(2);
  } else if (penColor == WColor(blue)) {
    penColor_->setCurrentIndex(3);
  }

  WColor clippingLinesColor = gridData_->clippingLinesColor();
  if (clippingLinesColor == black) {
    clippingLinesColor_->setCurrentIndex(0);
  } else if (clippingLinesColor == red) {
    clippingLinesColor_->setCurrentIndex(1);
  } else if (clippingLinesColor == green) {
    clippingLinesColor_->setCurrentIndex(2);
  } else if (clippingLinesColor == blue) {
    clippingLinesColor_->setCurrentIndex(3);
  } else if (clippingLinesColor == cyan) {
    clippingLinesColor_->setCurrentIndex(4);
  } else if (clippingLinesColor == magenta) {
    clippingLinesColor_->setCurrentIndex(5);
  } else if (clippingLinesColor == yellow) {
    clippingLinesColor_->setCurrentIndex(6);
  }

  showIsolines_->setChecked(gridData_->isoLevels().size() > 0);

  if (gridData_->isoColorMap() == 0) {
    isolineColormap_->setCurrentIndex(0);
  } else {
    const WStandardColorMap *map = dynamic_cast<const WStandardColorMap*>
      (gridData_->isoColorMap());
    if (!map->continuous()) {
      if (map->colorValuePairs().size() == 5)
	isolineColormap_->setCurrentIndex(2);
      else if (map->colorValuePairs().size() == 10)
	isolineColormap_->setCurrentIndex(3);
    } else {
      isolineColormap_->setCurrentIndex(1);
    }
  }
}


/*
 * Definition of class responsible for configuring categorical grid data-sets
 */
CatGridDataSettings::CatGridDataSettings()
  : gridData_(0)
{
  WTemplate* template_ = new WTemplate(Wt::WString::tr("catgriddata-template"), this);
  bindBaseToTemplate(template_);

  barWidthX_ = new WLineEdit(this);
  template_->bindWidget("widthx", barWidthX_);
  barWidthY_ = new WLineEdit(this);
  template_->bindWidget("widthy", barWidthY_);

  barWidthX_->changed().connect(std::bind([&] () {
	gridData_->setBarWidth(Wt::asNumber(barWidthX_->text()),
			       Wt::asNumber(barWidthY_->text()));
      }));
  barWidthY_->changed().connect(std::bind([&] () {
	gridData_->setBarWidth(Wt::asNumber(barWidthX_->text()),
			       Wt::asNumber(barWidthY_->text()));
      }));
}

void CatGridDataSettings::bindDataSet(WAbstractGridData *data)
{
  gridData_ = data;

  // update UI
  DataSettings::bindBaseDataSet(data);

  barWidthX_->setText(Wt::asString(gridData_->barWidthX()));
  barWidthY_->setText(Wt::asString(gridData_->barWidthY()));
}


/*
 * Definition of class responsible for configuring scatterdata-sets (pt-per-pt)
 */
ScatterDataSettings::ScatterDataSettings()
  : scatterData_(0)
{
  WTemplate* template_ = new WTemplate(Wt::WString::tr("scatterdata-template"), this);
  bindBaseToTemplate(template_);

  enableDroplines_ = new WCheckBox(this);
  template_->bindWidget("enabledroplines", enableDroplines_);
  penSize_ = new WLineEdit(this);
  template_->bindWidget("pensize", penSize_);
  penColor_ = new WComboBox(this);
  penColor_->addItem("black");
  penColor_->addItem("red");
  penColor_->addItem("green");
  penColor_->addItem("blue");
  template_->bindWidget("pencolor", penColor_);

  enableDroplines_->changed().connect(std::bind([&] () {
	scatterData_->setDroplinesEnabled(enableDroplines_->checkState() == Checked);
      }));
  penSize_->changed().connect(std::bind([&] () {
	WPen pen = scatterData_->droplinesPen();
	pen.setWidth(Wt::asNumber(penSize_->text()));
	scatterData_->setDroplinesPen(pen);
      }));
  penColor_->changed().connect(std::bind([&] () {
	WPen pen = scatterData_->droplinesPen();
	switch (penColor_->currentIndex()) {
	case 0:
	  pen.setColor(black); break;
	case 1:
	  pen.setColor(red); break;
	case 2:
	  pen.setColor(green); break;
	case 3:
	  pen.setColor(blue); break;
	}
	scatterData_->setDroplinesPen(pen);
      }));
}

void ScatterDataSettings::bindDataSet(WScatterData *data)
{
  scatterData_ = data;

  // update the UI
  DataSettings::bindBaseDataSet(data);

  enableDroplines_->setCheckState(scatterData_->droplinesEnabled() ?
				  Checked : Unchecked);

  penSize_->setText(Wt::asString(scatterData_->droplinesPen().width().value()));

  WPen pen = scatterData_->droplinesPen();
  WColor penColor = pen.color();
  if (penColor == WColor(black)) {
    penColor_->setCurrentIndex(0);
  } else if (penColor == WColor(red)) {
    penColor_->setCurrentIndex(1);
  } else if (penColor == WColor(green)) {
    penColor_->setCurrentIndex(2);
  } else if (penColor == WColor(blue)) {
    penColor_->setCurrentIndex(3);
  }
}
