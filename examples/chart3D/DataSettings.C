#include "DataSettings.h"

#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WSlider.h>
#include <Wt/WTemplate.h>
#include <Wt/WIntValidator.h>
#include <Wt/Chart/WAbstractDataSeries3D.h>
#include <Wt/Chart/WGridData.h>
#include <Wt/Chart/WScatterData.h>
#include <Wt/Chart/WStandardColorMap.h>

DataSettings::DataSettings()
  : data_(0)
{
}

// the correct bind-points must be present in the template
void DataSettings::bindBaseToTemplate(Wt::WTemplate* configtemplate)
{
  auto setName = std::make_unique<Wt::WLineEdit>();
  setName_ = configtemplate->bindWidget("setname", std::move(setName));

  auto pointsize = std::make_unique<Wt::WLineEdit>();
  pointsize_ = configtemplate->bindWidget("ptsize", std::move(pointsize));
  pointsize_->setValidator(std::make_shared<Wt::WIntValidator>(1, 10));

  auto pointSprite = std::make_unique<Wt::WComboBox>();
  pointSprite_ = configtemplate->bindWidget("ptsprite", std::move(pointSprite));
  pointSprite_->addItem("None");
  pointSprite_->addItem("diamond (5x5)");
  pointSprite_->addItem("cross (5x5)");

  auto colormap = std::make_unique<Wt::WComboBox>();
  colormap_ = configtemplate->bindWidget("colormap", std::move(colormap));
  colormap_->addItem("None");
  colormap_->addItem("Continuous");
  colormap_->addItem("Continuous (5 bands)");
  colormap_->addItem("Continuous (10 bands)");

  auto showColormap = std::make_unique<Wt::WCheckBox>();
  showColormap_ = configtemplate->bindWidget("showcolormap", std::move(showColormap));

  auto colormapSide = std::make_unique<Wt::WComboBox>();
  colormapSide_ = configtemplate->bindWidget("colormapside", std::move(colormapSide));
  colormapSide_->addItem("Left");
  colormapSide_->addItem("Right");

  auto hide = std::make_unique<Wt::WCheckBox>();
  hide_ = configtemplate->bindWidget("hide", std::move(hide));

  // hook up the UI to the dataset
  setName_->changed().connect([&] () {
        data_->setTitle(setName_->text());
      });
  pointsize_->changed().connect([&] () {
        data_->setPointSize(Wt::asNumber(pointsize_->text()));
      });
  pointSprite_->changed().connect([&] () {
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
      });
  colormap_->changed().connect([&] () {
        std::shared_ptr<Wt::Chart::WStandardColorMap> colMap = nullptr;
        switch (colormap_->currentIndex()) {
        case 0:
          data_->setColorMap(nullptr); break;
        case 1:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(data_->minimum(Wt::Chart::Axis::Z3D),
                                                                  data_->maximum(Wt::Chart::Axis::Z3D), true);
          break;
        case 2:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(data_->minimum(Wt::Chart::Axis::Z3D),
                                                                  data_->maximum(Wt::Chart::Axis::Z3D), true);
          colMap->discretise(5);
          break;
        case 3:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(data_->minimum(Wt::Chart::Axis::Z3D),
                                                                  data_->maximum(Wt::Chart::Axis::Z3D), true);
          colMap->discretise(10);
          break;
        }
        data_->setColorMap(colMap);
      });
  showColormap_->changed().connect([&] () {
        data_->setColorMapVisible(showColormap_->checkState() == Wt::CheckState::Checked);
      });
  colormapSide_->changed().connect([&] () {
        if (colormapSide_->currentIndex() == 0)
          data_->setColorMapSide(Wt::Side::Left);
        else
          data_->setColorMapSide(Wt::Side::Right);
      });
  hide_->changed().connect([&] () {
        data_->setHidden(hide_->checkState() == Wt::CheckState::Checked);
      });
}

void DataSettings::bindBaseDataSet(Wt::Chart::WAbstractDataSeries3D *data)
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
    const Wt::Chart::WStandardColorMap *map = dynamic_cast<const Wt::Chart::WStandardColorMap*>
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
    showColormap_->setCheckState(Wt::CheckState::Checked);
  else
    showColormap_->setCheckState(Wt::CheckState::Unchecked);

  if (data->colorMapSide() == Wt::Side::Left)
    colormapSide_->setCurrentIndex(0);
  else
    colormapSide_->setCurrentIndex(1);

  if (data->isHidden())
    hide_->setCheckState(Wt::CheckState::Checked);
  else
    hide_->setCheckState(Wt::CheckState::Unchecked);
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
  Wt::WTemplate* template_ =
      this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("numgriddata-template")));
  bindBaseToTemplate(template_);

  auto typeSelection = std::make_unique<Wt::WComboBox>();
  typeSelection_ = template_->bindWidget("datatype", std::move(typeSelection));
  typeSelection_->addItem("Points");
  typeSelection_->addItem("Surface");

  auto enableMesh = std::make_unique<Wt::WCheckBox>();
  enableMesh_ = template_->bindWidget("enablemesh", std::move(enableMesh));

  auto penSize = std::make_unique<Wt::WLineEdit>();
  penSize_ = template_->bindWidget("pensize", std::move(penSize));
  penSize_->setValidator(std::make_shared<Wt::WIntValidator>(1, 10));

  auto penColor = std::make_unique<Wt::WComboBox>();
  penColor_ = template_->bindWidget("pencolor", std::move(penColor));
  penColor_->addItem("black");
  penColor_->addItem("red");
  penColor_->addItem("green");
  penColor_->addItem("blue");

  auto xClippingMin = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  xClippingMin_ = template_->bindWidget("x-clipping-min", std::move(xClippingMin));
  xClippingMin_->setMinimum(-100);
  xClippingMin_->setMaximum(100);
  xClippingMin_->setValue(-100);

  auto xClippingMax = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  xClippingMax_ = template_->bindWidget("x-clipping-max", std::move(xClippingMax));
  xClippingMax_->setMinimum(-100);
  xClippingMax_->setMaximum(100);
  xClippingMax_->setValue(100);

  auto yClippingMin = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  yClippingMin_ = template_->bindWidget("y-clipping-min", std::move(yClippingMin));
  yClippingMin_->setMinimum(-100);
  yClippingMin_->setMaximum(100);
  yClippingMin_->setValue(-100);

  auto yClippingMax = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  yClippingMax_ = template_->bindWidget("y-clipping-max", std::move(yClippingMax));
  yClippingMax_->setMinimum(-100);
  yClippingMax_->setMaximum(100);
  yClippingMax_->setValue(100);

  auto zClippingMin = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  zClippingMin_ = template_->bindWidget("z-clipping-min", std::move(zClippingMin));
  zClippingMin_->setMinimum(-100);
  zClippingMin_->setMaximum(100);
  zClippingMin_->setValue(-100);

  auto zClippingMax = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
  zClippingMax_ = template_->bindWidget("z-clipping-max", std::move(zClippingMax));
  zClippingMax_->setMinimum(-100);
  zClippingMax_->setMaximum(100);
  zClippingMax_->setValue(100);

  auto showClippingLines = std::make_unique<Wt::WCheckBox>();
  showClippingLines_ = template_->bindWidget("clippinglines", std::move(showClippingLines));

  auto clippingLinesColor = std::make_unique<Wt::WComboBox>();
  clippingLinesColor_ = template_->bindWidget("clippinglines-color", std::move(clippingLinesColor));
  clippingLinesColor_->addItem("black");
  clippingLinesColor_->addItem("red");
  clippingLinesColor_->addItem("green");
  clippingLinesColor_->addItem("blue");
  clippingLinesColor_->addItem("cyan");
  clippingLinesColor_->addItem("magenta");
  clippingLinesColor_->addItem("yellow");

  auto showIsolines = std::make_unique<Wt::WCheckBox>();
  showIsolines_ = template_->bindWidget("isolines", std::move(showIsolines));

  auto isolineColormap = std::make_unique<Wt::WComboBox>();
  isolineColormap_ = template_->bindWidget("isoline-colormap", std::move(isolineColormap));
  isolineColormap_->addItem("None (use surface's colormap)");
  isolineColormap_->addItem("Continuous");
  isolineColormap_->addItem("Continuous (5 bands)");
  isolineColormap_->addItem("Continuous (10 bands)");

  // make connections
  xClippingMin_->sliderMoved().connect(changeXClippingMin_);
  xClippingMax_->sliderMoved().connect(changeXClippingMax_);
  yClippingMin_->sliderMoved().connect(changeYClippingMin_);
  yClippingMax_->sliderMoved().connect(changeYClippingMax_);
  zClippingMin_->sliderMoved().connect(changeZClippingMin_);
  zClippingMax_->sliderMoved().connect(changeZClippingMax_);

  showClippingLines_->checked().connect([&] () {
	gridData_->setClippingLinesEnabled(true);
      });
  showClippingLines_->unChecked().connect([&] () {
	gridData_->setClippingLinesEnabled(false);
      });

  typeSelection_->changed().connect([&] () {
	switch (typeSelection_->currentIndex()) {
	case 0:
          gridData_->setType(Wt::Chart::Series3DType::Point);
	  break;
	case 1:
          gridData_->setType(Wt::Chart::Series3DType::Surface);
	  break;
	}
      });
  enableMesh_->changed().connect([&] () {
        gridData_->setSurfaceMeshEnabled(enableMesh_->checkState() == Wt::CheckState::Checked);
      });
  penSize_->changed().connect([&] () {
        Wt::WPen pen = gridData_->pen();
	pen.setWidth(asNumber(penSize_->text()));
	gridData_->setPen(pen);
      });
  penColor_->changed().connect([&] () {
        Wt::WPen pen = gridData_->pen();
	switch (penColor_->currentIndex()) {
	case 0:
          pen.setColor(Wt::WColor(Wt::StandardColor::Black)); break;
	case 1:
          pen.setColor(Wt::WColor(Wt::StandardColor::Red)); break;
	case 2:
          pen.setColor(Wt::WColor(Wt::StandardColor::Green)); break;
	case 3:
          pen.setColor(Wt::WColor(Wt::StandardColor::Blue)); break;
	}
	gridData_->setPen(pen);
      });
  clippingLinesColor_->changed().connect([&] () {
	switch (clippingLinesColor_->currentIndex()) {
	case 0:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Black)); break;
	case 1:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Red)); break;
	case 2:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Green)); break;
	case 3:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Blue)); break;
	case 4:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Cyan)); break;
	case 5:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Magenta)); break;
	case 6:
          gridData_->setClippingLinesColor(Wt::WColor(Wt::StandardColor::Yellow)); break;
	}
      });
  showIsolines_->checked().connect([&] () {
	std::vector<double> isoLevels;
	for (double z = -20.0; z <= 20.0; z += 0.5) {
	  isoLevels.push_back(z);
	}
	gridData_->setIsoLevels(isoLevels);
      });
  showIsolines_->unChecked().connect([&] () {
	gridData_->setIsoLevels(std::vector<double>());
      });
  isolineColormap_->changed().connect([&] () {
        std::shared_ptr<Wt::Chart::WStandardColorMap> colMap = nullptr;
	switch (isolineColormap_->currentIndex()) {
	case 0:
	  break;
	case 1:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(gridData_->minimum(Wt::Chart::Axis::Z3D),
                                                                  gridData_->maximum(Wt::Chart::Axis::Z3D), true);
	  break;
	case 2:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(gridData_->minimum(Wt::Chart::Axis::Z3D),
                                                                  gridData_->maximum(Wt::Chart::Axis::Z3D), true);
          colMap->discretise(5);
          break;
        case 3:
          colMap = std::make_shared<Wt::Chart::WStandardColorMap>(gridData_->minimum(Wt::Chart::Axis::Z3D),
                                                                  gridData_->maximum(Wt::Chart::Axis::Z3D), true);
	  colMap->discretise(10);
	  break;
	}
	gridData_->setIsoColorMap(colMap);
      });
}

void NumGridDataSettings::bindDataSet(Wt::Chart::WAbstractGridData *data)
{
  for (auto &conn : clippingConnections_) {
    conn.disconnect();
  }

  gridData_ = data;

  changeXClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(Wt::Chart::Axis::X3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeXClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(Wt::Chart::Axis::X3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeYClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(Wt::Chart::Axis::Y3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeYClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(Wt::Chart::Axis::Y3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeZClippingMin_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMin(Wt::Chart::Axis::Z3D).execJs("o","e","pos / 5.0") + " }", 1);
  changeZClippingMax_.setJavaScript(
      "function (o,e,pos) { " + gridData_->changeClippingMax(Wt::Chart::Axis::Z3D).execJs("o","e","pos / 5.0") + " }", 1);

  xClippingMin_->setValue(std::max(gridData_->clippingMin(Wt::Chart::Axis::X3D) * 5, -100.0f));
  xClippingMax_->setValue(std::min(gridData_->clippingMax(Wt::Chart::Axis::X3D) * 5, 100.0f));
  yClippingMin_->setValue(std::max(gridData_->clippingMin(Wt::Chart::Axis::Y3D) * 5, -100.0f));
  yClippingMax_->setValue(std::min(gridData_->clippingMax(Wt::Chart::Axis::Y3D) * 5, 100.0f));
  zClippingMin_->setValue(std::max(gridData_->clippingMin(Wt::Chart::Axis::Z3D) * 5, -100.0f));
  zClippingMax_->setValue(std::min(gridData_->clippingMax(Wt::Chart::Axis::Z3D) * 5, 100.0f));

  clippingConnections_.push_back(xClippingMin_->valueChanged().connect([&] () {
    gridData_->setClippingMin(Wt::Chart::Axis::X3D, xClippingMin_->value() / 5.0);
  }));
  clippingConnections_.push_back(xClippingMax_->valueChanged().connect([&] () {
    gridData_->setClippingMax(Wt::Chart::Axis::X3D, xClippingMax_->value() / 5.0);
  }));
  clippingConnections_.push_back(yClippingMin_->valueChanged().connect([&] () {
    gridData_->setClippingMin(Wt::Chart::Axis::Y3D, yClippingMin_->value() / 5.0);
  }));
  clippingConnections_.push_back(yClippingMax_->valueChanged().connect([&] () {
    gridData_->setClippingMax(Wt::Chart::Axis::Y3D, yClippingMax_->value() / 5.0);
  }));
  clippingConnections_.push_back(zClippingMin_->valueChanged().connect([&] () {
    gridData_->setClippingMin(Wt::Chart::Axis::Z3D, zClippingMin_->value() / 5.0);
  }));
  clippingConnections_.push_back(zClippingMax_->valueChanged().connect([&] () {
    gridData_->setClippingMax(Wt::Chart::Axis::Z3D, zClippingMax_->value() / 5.0);
  }));

  showClippingLines_->setChecked(gridData_->clippingLinesEnabled());

  // update UI fields
  DataSettings::bindBaseDataSet(data);

  if (gridData_->type() == Wt::Chart::Series3DType::Point)
    typeSelection_->setCurrentIndex(0);
  else if (gridData_->type() == Wt::Chart::Series3DType::Surface)
    typeSelection_->setCurrentIndex(1);

  enableMesh_->setCheckState(gridData_->isSurfaceMeshEnabled() ?
                             Wt::CheckState::Checked : Wt::CheckState::Unchecked);

  penSize_->setText(Wt::asString(gridData_->pen().width().value()));

  Wt::WPen pen = gridData_->pen();
  Wt::WColor penColor = pen.color();
  if (penColor == Wt::WColor(Wt::StandardColor::Black)) {
    penColor_->setCurrentIndex(0);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Red)) {
    penColor_->setCurrentIndex(1);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Green)) {
    penColor_->setCurrentIndex(2);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Blue)) {
    penColor_->setCurrentIndex(3);
  }

  Wt::WColor clippingLinesColor = gridData_->clippingLinesColor();
  if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Black)) {
    clippingLinesColor_->setCurrentIndex(0);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Red)) {
    clippingLinesColor_->setCurrentIndex(1);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Green)) {
    clippingLinesColor_->setCurrentIndex(2);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Blue)) {
    clippingLinesColor_->setCurrentIndex(3);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Cyan)) {
    clippingLinesColor_->setCurrentIndex(4);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Magenta)) {
    clippingLinesColor_->setCurrentIndex(5);
  } else if (clippingLinesColor == Wt::WColor(Wt::StandardColor::Yellow)) {
    clippingLinesColor_->setCurrentIndex(6);
  }

  showIsolines_->setChecked(gridData_->isoLevels().size() > 0);

  if (gridData_->isoColorMap() == 0) {
    isolineColormap_->setCurrentIndex(0);
  } else {
    const std::shared_ptr<Wt::Chart::WStandardColorMap> map
        = std::dynamic_pointer_cast<Wt::Chart::WStandardColorMap>(gridData_->isoColorMap());
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
  Wt::WTemplate* template_ = this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("catgriddata-template")));
  bindBaseToTemplate(template_);

  barWidthX_ = template_->bindWidget("widthx", std::make_unique<Wt::WLineEdit>());
  barWidthY_ = template_->bindWidget("widthy", std::make_unique<Wt::WLineEdit>());

  barWidthX_->changed().connect([&] () {
        gridData_->setBarWidth(Wt::asNumber(barWidthX_->text()),
                               Wt::asNumber(barWidthY_->text()));
      });
  barWidthY_->changed().connect([&] () {
        gridData_->setBarWidth(Wt::asNumber(barWidthX_->text()),
                               Wt::asNumber(barWidthY_->text()));
      });
}

void CatGridDataSettings::bindDataSet(Wt::Chart::WAbstractGridData *data)
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
  Wt::WTemplate* template_ =
      this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("scatterdata-template")));
  bindBaseToTemplate(template_);

  auto enableDropLines = std::make_unique<Wt::WCheckBox>();
  enableDroplines_ = template_->bindWidget("enabledroplines", std::move(enableDropLines));

  auto penSize = std::make_unique<Wt::WLineEdit>();
  penSize_ = template_->bindWidget("pensize", std::move(penSize));

  auto penColor = std::make_unique<Wt::WComboBox>();
  penColor_ = template_->bindWidget("pencolor", std::move(penColor));
  penColor_->addItem("black");
  penColor_->addItem("red");
  penColor_->addItem("green");
  penColor_->addItem("blue");

  enableDroplines_->changed().connect([&] () {
        scatterData_->setDroplinesEnabled(enableDroplines_->checkState() == Wt::CheckState::Checked);
      });
  penSize_->changed().connect([&] () {
        Wt::WPen pen = scatterData_->droplinesPen();
	pen.setWidth(Wt::asNumber(penSize_->text()));
	scatterData_->setDroplinesPen(pen);
      });
  penColor_->changed().connect([&] () {
        Wt::WPen pen = scatterData_->droplinesPen();
	switch (penColor_->currentIndex()) {
	case 0:
          pen.setColor(Wt::WColor(Wt::StandardColor::Black)); break;
	case 1:
          pen.setColor(Wt::WColor(Wt::StandardColor::Red)); break;
	case 2:
          pen.setColor(Wt::WColor(Wt::StandardColor::Green)); break;
	case 3:
          pen.setColor(Wt::WColor(Wt::StandardColor::Blue)); break;
	}
	scatterData_->setDroplinesPen(pen);
      });
}

void ScatterDataSettings::bindDataSet(Wt::Chart::WScatterData *data)
{
  scatterData_ = data;

  // update the UI
  DataSettings::bindBaseDataSet(data);

  enableDroplines_->setCheckState(scatterData_->droplinesEnabled() ?
                                  Wt::CheckState::Checked : Wt::CheckState::Unchecked);

  penSize_->setText(Wt::asString(scatterData_->droplinesPen().width().value()));

  Wt::WPen pen = scatterData_->droplinesPen();
  Wt::WColor penColor = pen.color();
  if (penColor == Wt::WColor(Wt::StandardColor::Black)) {
    penColor_->setCurrentIndex(0);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Red)) {
    penColor_->setCurrentIndex(1);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Green)) {
    penColor_->setCurrentIndex(2);
  } else if (penColor == Wt::WColor(Wt::StandardColor::Blue)) {
    penColor_->setCurrentIndex(3);
  }
}
