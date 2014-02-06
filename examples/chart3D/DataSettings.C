#include "DataSettings.h"

#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WCheckBox>
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
  : gridData_(0)
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
  penColor_->addItem("something");
  template_->bindWidget("pencolor", penColor_);

  // make connections
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
}

void NumGridDataSettings::bindDataSet(WAbstractGridData *data)
{
  gridData_ = data;

  // update UI fields
  DataSettings::bindBaseDataSet(data);
  
  if (gridData_->type() == PointSeries3D)
    typeSelection_->setCurrentIndex(0);
  else if (gridData_->type() == SurfaceSeries3D)
    typeSelection_->setCurrentIndex(1);

  enableMesh_->setCheckState(gridData_->isSurfaceMeshEnabled() ? 
			     Checked : Unchecked);
  
  penSize_->setText(Wt::asString(gridData_->pen().width().value()));

  penColor_->setCurrentIndex(4);
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
  penColor_->addItem("something");
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

  penColor_->setCurrentIndex(4);
}
