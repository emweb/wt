#include "Wt/Chart/WGridData.h"

#include "Wt/WAbstractItemModel.h"
#include "Wt/WException.h"
#include "WebUtils.h"

namespace Wt {
  namespace Chart {

WGridData::WGridData(std::shared_ptr<WAbstractItemModel> model)
  : WAbstractGridData(model),
    XAbscisColumn_(0),
    YAbscisRow_(0)
{}

WGridData::~WGridData()
{}

void WGridData::setXSeriesColumn(int modelColumn)
{
  XAbscisColumn_ = modelColumn;
  rangeCached_ = false;
}

void WGridData::setYSeriesRow(int modelRow)
{
  YAbscisRow_ = modelRow;
  rangeCached_ = false;
}

int WGridData::nbXPoints() const
{
  return model_->rowCount() - 1;
}

int WGridData::nbYPoints() const
{
  return model_->columnCount() - 1;
}

WString WGridData::axisLabel(int u, Axis axis) const
{
  if (axis == Axis::X3D) {
    if (u >= YAbscisRow_)
      u++;
    return asString(model_->data(u, XAbscisColumn_));
  } else if (axis == Axis::Y3D) {
    if (u >= XAbscisColumn_)
      u++;
    return asString(model_->data(YAbscisRow_, u));
  } else {
    throw WException("WGridData: don't know this type of axis");
  }
}

int WGridData::countSimpleData() const
{
  int result = 0;
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  for (int i=0; i<nbModelRows; i++) {
    if (i == YAbscisRow_)
      continue;
    for (int j=0; j<nbModelCols; j++) {
      if (j == XAbscisColumn_)
	continue;

      if (!cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerBrushColor))) {
	result++;
      }
    }
  }

  return result;
}

cpp17::any WGridData::data(int i, int j) const
{
  if (i >= XAbscisColumn_) {
    i++;
  }
  if (j >= YAbscisRow_) {
    j++;
  }

  return model_->data(i,j);
}

double WGridData::minimum(Axis axis) const
{
  if (axis == Axis::X3D) {
    if (YAbscisRow_ != 0) {
      return Wt::asNumber(model_->data(0, XAbscisColumn_));
    } else {
      return Wt::asNumber(model_->data(1, XAbscisColumn_));
    }
  } else if (axis == Axis::Y3D) {
    if (XAbscisColumn_ != 0) {
      return Wt::asNumber(model_->data(YAbscisRow_, 0));
    } else {
      return Wt::asNumber(model_->data(YAbscisRow_, 1));
    }
  } else if (axis == Axis::Z3D) {
    if (!rangeCached_) {
      findRange();
    }
    return zMin_;
  } else {
    throw WException("WAbstractGridData.C: unknown Axis-type");
  }
}

double WGridData::maximum(Axis axis) const
{
  if (axis == Axis::X3D) {
    if (seriesType_ == Series3DType::Bar) {
      return model_->rowCount() - 1 - 0.5;
    }
    if (YAbscisRow_ != model_->rowCount()) {
      return Wt::asNumber(model_->data(model_->rowCount()-1, XAbscisColumn_));
    } else {
      return Wt::asNumber(model_->data(model_->rowCount()-2, XAbscisColumn_));
    }
  } else if (axis == Axis::Y3D) {
    if (seriesType_ == Series3DType::Bar) {
      return model_->columnCount() - 1 - 0.5;
    }
    if (XAbscisColumn_ != model_->columnCount()) {
      return Wt::asNumber(model_->data(YAbscisRow_, model_->columnCount()-1));
    } else {
      return Wt::asNumber(model_->data(YAbscisRow_, model_->columnCount()-2));
    }
  } else if (axis == Axis::Z3D) {
    if (!rangeCached_) {
      findRange();
    }
    return zMax_;
  } else {
    throw WException("WAbstractGridData.C: unknown Axis-type");
  }
}

void WGridData::findRange() const
{
  double minSoFar = std::numeric_limits<double>::max();
  double maxSoFar = -std::numeric_limits<double>::max();

  double zVal;
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  for (int i=0; i<nbModelRows; i++) {
    if (i == YAbscisRow_)
      continue;
    for (int j=0; j<nbModelCols; j++) {
      if (j == XAbscisColumn_)
	continue;
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

void WGridData::pointDataFromModel(FloatBuffer& simplePtsArray,
				   FloatBuffer& simplePtsSize,
				   FloatBuffer& coloredPtsArray,
				   FloatBuffer& coloredPtsSize,
				   FloatBuffer& coloredPtsColor) const {
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();

  // scale the x- and y-axis to the plotbox
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(nbModelRows-1);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(nbModelCols-1);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i<nbModelRows; i++) {
    if (i == YAbscisRow_)
      continue;
    scaledXAxis.push_back((float)((Wt::asNumber(model_->data(i,XAbscisColumn_)) 
				   - xMin)/(xMax - xMin)));
  }
  for (int j=0; j<nbModelCols; j++) {
    if (j == XAbscisColumn_)
      continue;
    scaledYAxis.push_back((float)((Wt::asNumber(model_->data(YAbscisRow_,j))
				   - yMin)/(yMax - yMin)));
  }

  int rowOffset = 0, colOffset = 0;
  for (int i=0; i<nbModelRows; i++) {
    if (i == YAbscisRow_) {
      rowOffset = 1;
      continue;
    }
    colOffset = 0;
    for (int j=0; j<nbModelCols; j++) {
      if (j == XAbscisColumn_) {
	colOffset = 1;
	continue;
      }
      if (!cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerBrushColor))) {
	simplePtsArray.push_back(scaledXAxis[i-rowOffset]);
	simplePtsArray.push_back(scaledYAxis[j-colOffset]);
	simplePtsArray.push_back((float)((Wt::asNumber(model_->data(i,j,ItemDataRole::Display))-zMin)/(zMax-zMin)));
	if (cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerScaleFactor))) {
	  simplePtsSize.push_back((float)(Wt::asNumber(model_->data(i,j,ItemDataRole::MarkerScaleFactor))));
	} else {
	  simplePtsSize.push_back((float)pointSize());
	}
      } else {
	coloredPtsArray.push_back(scaledXAxis[i-rowOffset]);
	coloredPtsArray.push_back(scaledYAxis[j-colOffset]);
	coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,j))-zMin)
					  /(zMax-zMin)));
	WColor color = cpp17::any_cast<WColor>(model_->data(i,j,ItemDataRole::MarkerBrushColor));
	coloredPtsColor.push_back((float)color.red());
	coloredPtsColor.push_back((float)color.green());
	coloredPtsColor.push_back((float)color.blue());
	coloredPtsColor.push_back((float)color.alpha());

	if (cpp17::any_has_value(model_->data(i,j,ItemDataRole::MarkerScaleFactor))) {
	  coloredPtsSize.push_back
	    ((float)(Wt::asNumber
		     (model_->data(i,j, ItemDataRole::MarkerScaleFactor))));
	} else {
	  coloredPtsSize.push_back((float)pointSize());
	}
      }
    }
  }
}

void WGridData::surfaceDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const {
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();

  // scale the x- and y-axis to the plotbox
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(nbModelRows-1);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(nbModelCols-1);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i<nbModelRows; i++) {
    if (i == YAbscisRow_)
      continue;
    scaledXAxis.push_back((float)((Wt::asNumber(model_->data(i,XAbscisColumn_)) 
				   - xMin)/(xMax - xMin)));
  }
  for (int j=0; j<nbModelCols; j++) {
    if (j == XAbscisColumn_)
      continue;
    scaledYAxis.push_back((float)((Wt::asNumber(model_->data(YAbscisRow_,j))
				   - yMin)/(yMax - yMin)));
  }

  int nbXaxisBuffers = nbXPoints()/(SURFACE_SIDE_LIMIT-1);
  int nbYaxisBuffers = nbYPoints()/(SURFACE_SIDE_LIMIT-1);
  if (nbXPoints() % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbXaxisBuffers++;
  }
  if (nbYPoints() % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbYaxisBuffers++;
  }
  int SURFACE_SIDE_LIMIT = WGridData::SURFACE_SIDE_LIMIT - 1;
  int rowOffset = 0, colOffset = 0;
  int bufferIndex = 0;
  for (int k=0; k < nbXaxisBuffers-1; k++) {
    for (int l=0; l < nbYaxisBuffers-1; l++) {
      bufferIndex = k*nbYaxisBuffers + l;
      int cnt1 = 0;
      int i = k*SURFACE_SIDE_LIMIT;
      rowOffset = 0;
      for (; i < (k+1)*SURFACE_SIDE_LIMIT + 1; i++) {
	if (i >= YAbscisRow_) {
	  rowOffset = 1;
	}
	int cnt2 = 0;
	int j = l*SURFACE_SIDE_LIMIT;
	colOffset = 0;
	for (; j < (l+1)*SURFACE_SIDE_LIMIT + 1; j++) {
	  if (j >= XAbscisColumn_) {
	    colOffset = 1;
	  }
	  simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	  simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	  simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i+rowOffset,j+colOffset))-zMin)/(zMax-zMin)));
	  cnt2++;
	}
	cnt1++;
      }
    }
    bufferIndex = k*nbYaxisBuffers + nbYaxisBuffers - 1;
    int cnt1 = 0;
    int i = k*SURFACE_SIDE_LIMIT;
    rowOffset = 0;
    for (; i < (k+1)*SURFACE_SIDE_LIMIT + 1; i++) {
      if (i >= YAbscisRow_) {
	rowOffset = 1;
      }
      int j = (nbYaxisBuffers-1)*SURFACE_SIDE_LIMIT;
      colOffset = 0;
      for (; j < nbModelCols - 1; j++) {
	if (j >= XAbscisColumn_) {
	  colOffset = 1;
	}
	simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i+rowOffset,j+colOffset))-zMin)/(zMax-zMin)));
      }
      cnt1++;
    }
  }
  for (int l=0; l < nbYaxisBuffers-1; l++) {
    bufferIndex = (nbXaxisBuffers-1)*nbYaxisBuffers + l;
    int i = (nbXaxisBuffers-1)*SURFACE_SIDE_LIMIT;
    rowOffset = 0;
    for (; i < nbModelRows - 1; i++) {
      if (i >= YAbscisRow_) {
	rowOffset = 1;
      }
      int cnt2 = 0;
      int j = l*SURFACE_SIDE_LIMIT;
      colOffset = 0;
      for (; j < (l+1)*SURFACE_SIDE_LIMIT + 1; j++) {
	if (j >= XAbscisColumn_) {
	  colOffset = 1;
	}
	simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
	simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i+rowOffset,j+colOffset))-zMin)/(zMax-zMin)));
	cnt2++;
      }
    }
  }
  bufferIndex = (nbXaxisBuffers-1)*nbYaxisBuffers + (nbYaxisBuffers-1);
  int i = (nbXaxisBuffers-1)*SURFACE_SIDE_LIMIT;
  rowOffset = 0;
  for (; i < nbModelRows - 1; i++) {
    if (i >= YAbscisRow_) {
      rowOffset = 1;
    }
    int j = (nbYaxisBuffers-1)*SURFACE_SIDE_LIMIT;
    colOffset = 0;
    for (; j < nbModelCols - 1; j++) {
      if (j >= XAbscisColumn_) {
	colOffset = 1;
      }
      simplePtsArrays[bufferIndex].push_back(scaledXAxis[i]);
      simplePtsArrays[bufferIndex].push_back(scaledYAxis[j]);
      simplePtsArrays[bufferIndex].push_back((float)((Wt::asNumber(model_->data(i+rowOffset,j+colOffset))-zMin)/(zMax-zMin)));
    }
  }
}

void WGridData::barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const
{
  // search for previously initialized barSeries data
  const std::vector<WAbstractDataSeries3D*> dataseries = chart_->dataSeries();
  std::vector<WAbstractGridData*> prevDataseries;
  bool first = true;
  int xDim = 0, yDim = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    if (dynamic_cast<WAbstractGridData*>(dataseries[i]) != nullptr) {
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
    throw WException("WGridData.C: Dimensions of multiple bar-series data do not match");
  }

  // scale the x- and y-axis to the plotbox
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(nbModelRows-1);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(nbModelCols-1);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i<nbModelRows-1; i++) {
    scaledXAxis.push_back((float)((xMin + 0.5 + i - xMin)/(xMax-xMin)));
  }
  for (int j=0; j<nbModelCols-1; j++) {
    scaledYAxis.push_back((float)((yMin + 0.5 + j - yMin)/(yMax-yMin)));
  }

  // fill the buffers
  int rowOffset = 0, colOffset = 0;
  int simpleBufferIndex = 0;
  int simpleCount = 0;
  for (int i=0; i < nbModelRows - 1; i++) {
    if (i >= YAbscisRow_) {
      rowOffset = 1;
    }
    colOffset = 0;
    for (int j=0; j < nbModelCols - 1; j++) {
      if (j >= XAbscisColumn_) {
	colOffset = 1;
      }
      float z0 = stackAllValues(prevDataseries, i,j);

      if (simpleCount == BAR_BUFFER_LIMIT) {
	simpleBufferIndex++;
	simpleCount = 0;
      }
      simplePtsArrays[simpleBufferIndex].push_back(scaledXAxis[i]);
      simplePtsArrays[simpleBufferIndex].push_back(scaledYAxis[j]);
      // first the value of all previous series stacked,then the current value
      simplePtsArrays[simpleBufferIndex].push_back(z0);
      double modelVal = Wt::asNumber(model_->data(i+rowOffset,j+colOffset));
      float delta = (modelVal <= 0) ? zeroBarCompensation : 0;
      simplePtsArrays[simpleBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin))+delta);
      simpleCount++;
    }
  }
}

void WGridData::barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays,
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
    throw WException("WGridData.C: Dimensions of multiple bar-series data do not match");
  }

  // scale the x- and y-axis to the plotbox
  int nbModelRows = model_->rowCount();
  int nbModelCols = model_->columnCount();
  FloatBuffer scaledXAxis = Utils::createFloatBuffer(nbModelRows-1);
  FloatBuffer scaledYAxis = Utils::createFloatBuffer(nbModelCols-1);

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  for (int i=0; i<nbModelRows-1; i++) {
    scaledXAxis.push_back((float)((xMin + 0.5 + i - xMin)/(xMax-xMin)));
  }
  for (int j=0; j<nbModelCols-1; j++) {
    scaledYAxis.push_back((float)((yMin + 0.5 + j - yMin)/(yMax-yMin)));
  }

  // fill the buffers
  int rowOffset = 0, colOffset = 0;
  int simpleBufferIndex = 0, coloredBufferIndex = 0;
  int simpleCount = 0, coloredCount = 0;
  for (int i=0; i < nbModelRows - 1; i++) {
    if (i >= YAbscisRow_) {
      rowOffset = 1;
    }
    colOffset = 0;
    for (int j=0; j < nbModelCols - 1; j++) {
      if (j >= XAbscisColumn_) {
	colOffset = 1;
      }
      float z0 = stackAllValues(prevDataseries, i,j);

      if (!cpp17::any_has_value(model_->data(i+rowOffset,j+colOffset,ItemDataRole::MarkerBrushColor))) {
	if (simpleCount == BAR_BUFFER_LIMIT) {
	  simpleBufferIndex++;
	  simpleCount = 0;
	}
	simplePtsArrays[simpleBufferIndex].push_back(scaledXAxis[i]);
	simplePtsArrays[simpleBufferIndex].push_back(scaledYAxis[j]);
	// first the value of all previous series stacked,then the current value
	simplePtsArrays[simpleBufferIndex].push_back(z0);
	double modelVal = Wt::asNumber(model_->data(i+rowOffset,j+colOffset));
	float delta = (modelVal <= 0) ? zeroBarCompensation : 0;
	simplePtsArrays[simpleBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin))+delta);
	simpleCount++;
      } else {
	if (coloredCount == BAR_BUFFER_LIMIT) {
	  coloredBufferIndex++;
	  coloredCount = 0;
	}
	coloredPtsArrays[coloredBufferIndex].push_back(scaledXAxis[i]);
	coloredPtsArrays[coloredBufferIndex].push_back(scaledYAxis[j]);
	coloredPtsArrays[coloredBufferIndex].push_back(z0);
	double modelVal = Wt::asNumber(model_->data(i+rowOffset,j+colOffset));
	float delta = (modelVal <= 0) ? zeroBarCompensation : 0;
	coloredPtsArrays[coloredBufferIndex].push_back((float)((modelVal-zMin)/(zMax-zMin))+delta);

	WColor color = cpp17::any_cast<WColor>(model_->data(i+rowOffset,j+colOffset,ItemDataRole::MarkerBrushColor));
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



  }
}
