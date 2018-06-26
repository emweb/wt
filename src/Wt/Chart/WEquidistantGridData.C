#include "WEquidistantGridData.h"

#include "Wt/WAbstractItemModel.h"
#include "Wt/WException.h"
#include "WebUtils.h"

namespace Wt {
  namespace Chart {

WEquidistantGridData::WEquidistantGridData(std::shared_ptr<WAbstractItemModel> model,
					   double XMin,
					   double deltaX,
					   double YMin,
					   double deltaY)
  : WAbstractGridData(model),
    XMinimum_(XMin), deltaX_(deltaX),
    YMinimum_(YMin), deltaY_(deltaY)
{
}

void WEquidistantGridData::setXAbscis(double XMinimum, double deltaX)
{
  XMinimum_ = XMinimum;
  deltaX_ = deltaX;

  if (chart_) {
    chart_->updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
  }
}

void WEquidistantGridData::setYAbscis(double YMinimum, double deltaY)
{
  YMinimum_ = YMinimum;
  deltaY_ = deltaY;

  if (chart_) {
    chart_->updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
  }
}

double WEquidistantGridData::minimum(Axis axis) const
{
  if (axis == Axis::X3D) {
    return XMinimum_;
  } else if (axis == Axis::Y3D) {
    return YMinimum_;
  } else if (axis == Axis::Z3D) {
    if (!rangeCached_) {
      findRange();
    }
    return zMin_;
  } else {
    throw WException("WEquidistantGridData.C: unknown Axis-type");
  }
}

double WEquidistantGridData::maximum(Axis axis) const
{
  int Nx, Ny;
  if (axis == Axis::X3D) {
    Nx = model_->rowCount();
    return XMinimum_ + (Nx-1)*deltaX_;
  } else if (axis == Axis::Y3D) {
    Ny = model_->columnCount();
    return YMinimum_ + (Ny-1)*deltaY_;
  } else if (axis == Axis::Z3D) {
    if (!rangeCached_) {
      findRange();
    }
    return zMax_;
  } else {
    throw WException("WEquidistantGridData.C: unknown Axis-type");
  }
}

void WEquidistantGridData::findRange() const
{
  double minSoFar = std::numeric_limits<double>::max();
  double maxSoFar = std::numeric_limits<double>::min();

  double zVal;
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  for (int i=0; i<nbModelRows; i++) {
    for (int j=0; j<nbModelCols; j++) {
      zVal = Wt::asNumber(model_->data(i,j));
      if (zVal < minSoFar) {
	minSoFar = zVal;
      }
      if (zVal > maxSoFar) {
	maxSoFar = zVal;
      }
    }
  }
  
  zMin_ = minSoFar;
  zMax_ = maxSoFar;
  rangeCached_ = true;
}

int WEquidistantGridData::countSimpleData() const
{
  int result;
#ifndef WT_TARGET_JAVA
  result = 0;
#else
  result = 0;
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  for (int i=0; i<nbModelRows; i++) {
    for (int j=0; j<nbModelCols; j++) {
      if (!cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerBrushColor))) {
	result++;
      }
    }
  }
#endif

  return result;
}

void WEquidistantGridData::pointDataFromModel(FloatBuffer& simplePtsArray,
					      FloatBuffer& simplePtsSize,
					      FloatBuffer& coloredPtsArray,
					      FloatBuffer& coloredPtsSize,
					      FloatBuffer& coloredPtsColor) const {
  int Nx = model_->rowCount();
  int Ny = model_->columnCount();

  // scale the x- and y-axis to the plotbox
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(Nx);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(Ny);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();
  
  for (int i=0; i<Nx; i++) {
    scaledXAxis.push_back((float)(((XMinimum_ + i*deltaX_) - xMin)
				  /(xMax - xMin)));
  }
  for (int j=0; j<Ny; j++) {
    scaledYAxis.push_back((float)(((YMinimum_ + j*deltaY_) - yMin)
				  /(yMax - yMin)));
  }

  for (int i=0; i < Nx; i++) {
    for (int j=0; j < Ny; j++) {
      if (!cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerBrushColor))) {
	simplePtsArray.push_back(scaledXAxis[i]);
	simplePtsArray.push_back(scaledYAxis[j]);
	simplePtsArray.push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)/(zMax-zMin)));
	if (cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerScaleFactor))) {
	  simplePtsSize.push_back((float)(Wt::asNumber(model_->data(i,j,ItemDataRole::MarkerScaleFactor))));
	} else {
	  simplePtsSize.push_back((float)pointSize());
	}
      } else {
	coloredPtsArray.push_back(scaledXAxis[i]);
	coloredPtsArray.push_back(scaledYAxis[j]);
	coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)
					  /(zMax-zMin)));
	WColor color = cpp17::any_cast<WColor>
	  (model_->data(i,j, ItemDataRole::MarkerBrushColor));
	coloredPtsColor.push_back((float)color.red());
	coloredPtsColor.push_back((float)color.green());
	coloredPtsColor.push_back((float)color.blue());
	coloredPtsColor.push_back((float)color.alpha());

	if (cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerScaleFactor))) {
	  coloredPtsSize.push_back((float)(Wt::asNumber(model_->data(i,j,ItemDataRole::MarkerScaleFactor))));
	} else {
	  coloredPtsSize.push_back((float)pointSize());
	}
      }
    }
  }
}

void WEquidistantGridData::surfaceDataFromModel(std::vector<FloatBuffer>& 
						simplePtsArrays) const {
  int Nx = model_->rowCount();
  int Ny = model_->columnCount();

  // scale the x- and y-axis to the plotbox
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(Nx);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(Ny);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i<Nx; i++) {
    scaledXAxis.push_back((float)(((XMinimum_ + i*deltaX_) - xMin)
				  /(xMax - xMin)));
  }
  for (int j=0; j<Ny; j++) {
    scaledYAxis.push_back((float)(((YMinimum_ + j*deltaY_) - yMin)
				  /(yMax - yMin)));
  }

  int nbXaxisBuffers = nbXPoints()/(SURFACE_SIDE_LIMIT-1);
  int nbYaxisBuffers = nbYPoints()/(SURFACE_SIDE_LIMIT-1);
  if (nbXPoints() % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbXaxisBuffers++;
  }
  if (nbYPoints() % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbYaxisBuffers++;
  }
  int SURFACE_SIDE_LIMIT = WAbstractGridData::SURFACE_SIDE_LIMIT - 1;
  int bufferIndex = 0;
  for (int k=0; k < nbXaxisBuffers-1; k++) {
    for (int l=0; l < nbYaxisBuffers-1; l++) {
      bufferIndex = k*nbYaxisBuffers + l;
      int cnt1 = 0;
      int i = k*SURFACE_SIDE_LIMIT;
      for (; i < (k+1)*SURFACE_SIDE_LIMIT + 1; i++) {
	int cnt2 = 0;
	int j = l*SURFACE_SIDE_LIMIT;
	for (; j < (l+1)*SURFACE_SIDE_LIMIT + 1; j++) {
	  simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	  simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	  simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)/(zMax-zMin)));
	  cnt2++;
	}
	cnt1++;
      }
    }
    bufferIndex = k*nbYaxisBuffers + nbYaxisBuffers - 1;
    int cnt1 = 0;
    int i = k*SURFACE_SIDE_LIMIT;
    for (; i < (k+1)*SURFACE_SIDE_LIMIT + 1; i++) {
      int j = (nbYaxisBuffers-1)*SURFACE_SIDE_LIMIT;
      for (; j < Ny; j++) {
	simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)/(zMax-zMin)));
      }
      cnt1++;
    }
  }
  for (int l=0; l < nbYaxisBuffers-1; l++) {
    bufferIndex = (nbXaxisBuffers-1)*nbYaxisBuffers + l;
    int i = (nbXaxisBuffers-1)*SURFACE_SIDE_LIMIT;
    for (; i < Nx; i++) {
      int cnt2 = 0;
      int j = l*SURFACE_SIDE_LIMIT;
      for (; j < (l+1)*SURFACE_SIDE_LIMIT + 1; j++) {
	simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)/(zMax-zMin)));
	cnt2++;
      }
    }
  }
  bufferIndex = (nbXaxisBuffers-1)*nbYaxisBuffers + (nbYaxisBuffers-1);
  int i = (nbXaxisBuffers-1)*SURFACE_SIDE_LIMIT;
  for (; i < Nx; i++) {
    int j = (nbYaxisBuffers-1)*SURFACE_SIDE_LIMIT;
    for (; j < Ny; j++) {
      simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
      simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
      simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)/(zMax-zMin)));
    }
  }
}

void WEquidistantGridData::barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const
{
  // search for previously initialized barSeries data
  const std::vector<WAbstractDataSeries3D*> dataseries = chart_->dataSeries();
  std::vector<WAbstractGridData*> prevDataseries;
  bool first = true;
  int xDim = 0, yDim = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    if ( dynamic_cast<WAbstractGridData*>(dataseries[i]) != 0) {
      WAbstractGridData* griddata = dynamic_cast<WAbstractGridData*>(dataseries[i]);
      if (griddata == this ||
	  griddata->type() != Series3DType::Bar) {
	break;
      }
      if (first) {
	xDim = griddata->nbXPoints();
	yDim = griddata->nbYPoints();
	first = false;
      }
      if ( griddata->nbXPoints() != xDim || griddata->nbYPoints() != yDim 
	   || griddata->isHidden()) {
	continue;
      }
      prevDataseries.push_back( griddata );
    }
  }
  if ( !prevDataseries.empty() &&
       (xDim != nbXPoints() || yDim != nbYPoints()) ) {
    throw WException("WEquidistantGridData.C: Dimensions of multiple bar-series data do not match");
  }


  // scale the x- and y-axis to the plotbox
  int Nx = model_->rowCount();
  int Ny = model_->columnCount();
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(Nx);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(Ny);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i < Nx; i++) {
    scaledXAxis.push_back((float)((xMin + 0.5 + i - xMin)/(xMax-xMin)));
  }
  for (int j=0; j < Ny; j++) {
    scaledYAxis.push_back((float)((yMin + 0.5 + j - yMin)/(yMax-yMin)));
  }
  
  int simpleBufferIndex = 0;
  int simpleCount = 0;
  for (int i=0; i < Nx; i++) {
    for (int j=0; j < Ny; j++) {
      float z0 = stackAllValues(prevDataseries, i,j);
    
      if (simpleCount == BAR_BUFFER_LIMIT) {
	simpleBufferIndex++;
	simpleCount = 0;
      }
      simplePtsArrays[simpleBufferIndex].push_back(scaledXAxis[i]);
      simplePtsArrays[simpleBufferIndex].push_back(scaledYAxis[j]);
      // first the value of all previous series stacked,then the current value
      simplePtsArrays[simpleBufferIndex].push_back(z0);
      double modelVal = Wt::asNumber(model_->data(i,j));
      if (modelVal <= 0)
	modelVal = 0.00001;
      simplePtsArrays[simpleBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin)));
      simpleCount++;
    }
  }
}

void WEquidistantGridData::barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays,
					    std::vector<FloatBuffer>& coloredPtsArrays,
					    std::vector<FloatBuffer>& coloredPtsColors) const
{
  // search for previously initialized barSeries data
  const std::vector<WAbstractDataSeries3D*> dataseries = chart_->dataSeries();
  std::vector<WAbstractGridData*> prevDataseries;
  bool first = true;
  int xDim = 0, yDim = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    if ( dynamic_cast<WAbstractGridData*>(dataseries[i]) != 0) {
      WAbstractGridData* griddata = dynamic_cast<WAbstractGridData*>(dataseries[i]);
      if (griddata == this ||
	  griddata->type() != Series3DType::Bar) {
	break;
      }
      if (first) {
	xDim = griddata->nbXPoints();
	yDim = griddata->nbYPoints();
	first = false;
      }
      if ( griddata->nbXPoints() != xDim || griddata->nbYPoints() != yDim 
	   || griddata->isHidden()) {
	continue;
      }
      prevDataseries.push_back( griddata );
    }
  }
  if ( !prevDataseries.empty() &&
       (xDim != nbXPoints() || yDim != nbYPoints()) ) {
    throw WException("WEquidistantGridData.C: Dimensions of multiple bar-series data do not match");
  }


  // scale the x- and y-axis to the plotbox
  int Nx = model_->rowCount();
  int Ny = model_->columnCount();
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(Nx);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(Ny);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i < Nx; i++) {
    scaledXAxis.push_back((float)((xMin + 0.5 + i - xMin)/(xMax-xMin)));
  }
  for (int j=0; j < Ny; j++) {
    scaledYAxis.push_back((float)((yMin + 0.5 + j - yMin)/(yMax-yMin)));
  }

  
  int simpleBufferIndex = 0, coloredBufferIndex = 0;
  int simpleCount = 0, coloredCount = 0;
  for (int i=0; i < Nx; i++) {
    for (int j=0; j < Ny; j++) {
      float z0 = stackAllValues(prevDataseries, i,j);
      
      if (!cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerBrushColor))) {
	if (simpleCount == BAR_BUFFER_LIMIT) {
	  simpleBufferIndex++;
	  simpleCount = 0;
	}
	simplePtsArrays[simpleBufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[simpleBufferIndex].push_back(scaledYAxis[j]);
	// first the value of all previous series stacked,then the current value
	simplePtsArrays[simpleBufferIndex].push_back(z0);
	double modelVal = Wt::asNumber(model_->data(i,j));
	if (modelVal <= 0)
	  modelVal = 0.00001;
	simplePtsArrays[simpleBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin)));
	simpleCount++;
      } else {
	if (coloredCount == BAR_BUFFER_LIMIT) {
	  coloredBufferIndex++;
	  coloredCount = 0;
	}
	coloredPtsArrays[coloredBufferIndex].push_back(scaledXAxis[i]);
	coloredPtsArrays[coloredBufferIndex].push_back(scaledYAxis[j]);
	coloredPtsArrays[coloredBufferIndex].push_back(z0);
	double modelVal = Wt::asNumber(model_->data(i,j));
	if (modelVal <= 0)
	  modelVal = 0.00001;
	coloredPtsArrays[coloredBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin)));

	WColor color = cpp17::any_cast<WColor>(model_->data(i,j,ItemDataRole::MarkerBrushColor));
	for (int k=0; k < 8; k++) {
	  coloredPtsColors[coloredBufferIndex].push_back((float)color.red());
	  coloredPtsColors[coloredBufferIndex].push_back((float)color.green());
	  coloredPtsColors[coloredBufferIndex].push_back((float)color.blue());
	  coloredPtsColors[coloredBufferIndex].push_back((float)color.alpha());
	}
	coloredCount++;
      }

    }
  }
}

int WEquidistantGridData::nbXPoints() const
{
  return model_->rowCount();
}

int WEquidistantGridData::nbYPoints() const
{
  return model_->columnCount();
}

WString WEquidistantGridData::axisLabel(int u, Axis axis) const
{
  if (axis == Axis::X3D) {
    return WString("{1}").arg(XMinimum_ + u*deltaX_);
  } else if (axis == Axis::Y3D) {
    return WString("{1}").arg(YMinimum_ + u*deltaY_);
  } else {
    throw WException("WEquidistantGridData: don't know this type of axis");
  }
}

cpp17::any WEquidistantGridData::data(int i, int j) const
{
  return model_->data(i,j);
}

  }
}
