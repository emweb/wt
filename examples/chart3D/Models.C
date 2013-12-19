#include "Models.h"

#include "Wt/WColor"

SombreroData::SombreroData(int nbXPts, int nbYPts,
			   double xStart, double xEnd,
			   double yStart, double yEnd,
			   WObject *parent)
  : WAbstractTableModel(parent),
    nbXPts_(nbXPts), nbYPts_(nbYPts),
    xStart_(xStart), xEnd_(xEnd), yStart_(yStart), yEnd_(yEnd)
{}

int SombreroData::rowCount(const Wt::WModelIndex& parent) const
{
  return nbXPts_+1;
}

int SombreroData::columnCount(const Wt::WModelIndex& parent) const
{
  return nbYPts_+1;
}

boost::any SombreroData::data(int row, int column, int role,
			      const WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

boost::any SombreroData::data(const Wt::WModelIndex& index,
			      int role) const
{ 
  double delta_y = (yEnd_ - yStart_)/(nbYPts_-1);
  if (index.row() == 0) { // give back y-abscis
    if (index.column() == 0)
      return 0.0;
    return yStart_ + (index.column()-1)*delta_y;
  }
  
  double delta_x = (xEnd_ - xStart_)/(nbXPts_-1);
  if (index.column() == 0) { // give back x-abscis
    if (index.row() == 0)
      return 0.0;
    return xStart_ + (index.row()-1)*delta_x;
  }

  double x, y;
  y = yStart_ + (index.column()-1)*delta_y;
  x = xStart_ + (index.row()-1)*delta_x;

  if (role == MarkerBrushColorRole) {
    return boost::any();
  } else if (role == DisplayRole) {
    return 4*sin(sqrt(pow(x,2) + pow(y,2))) / (sqrt (pow(x,2) + pow(y,2)));
  } else {
    return boost::any();
  }
}

boost::any SombreroData::headerData(int section,
				 Wt::Orientation orientation,
				 int role) const
{
  return 0.0; // unimplemented
}

PlaneData::PlaneData(int nbXPts, int nbYPts,
		     double xStart, double xDelta,
		     double yStart, double yDelta,
		     bool Yvariation,
		     double colorRoleBound, double sizeRoleBound,
		     WObject *parent)
  : WAbstractTableModel(parent),
    nbXPts_(nbXPts), nbYPts_(nbYPts),
    xStart_(xStart), xDelta_(xDelta), yStart_(yStart), yDelta_(yDelta),
    yVar_(Yvariation),
    colorRoleBound_(colorRoleBound), sizeRoleBound_(sizeRoleBound)
{}

int PlaneData::rowCount(const Wt::WModelIndex& parent) const
{
  return nbXPts_;
}

int PlaneData::columnCount(const Wt::WModelIndex& parent) const
{
  return nbYPts_;
}

boost::any PlaneData::data(int row, int column, int role,
			   const WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

boost::any PlaneData::data(const Wt::WModelIndex& index,
			   int role) const
{


  double x, y, value;
  y = yStart_ + index.column() * yDelta_;
  x = xStart_ + index.row() * xDelta_;
  if (yVar_)
    value = 0.5*y;
  else 
    value = 0.5*x;

  if (role == DisplayRole) {
    return value;
  } else if (role == MarkerBrushColorRole) {
    if (value > colorRoleBound_)
      return WColor(blue);
    else
      return boost::any();
  } else if (role == MarkerScaleFactorRole) {
    if (value > sizeRoleBound_)
      return 5;
    else
      return boost::any();
} else {
return boost::any();
}
}

boost::any PlaneData::headerData(int section,
				 Wt::Orientation orientation,
				 int role) const
{
  return 0.0; // unimplemented
}

PointsData::PointsData(int nbPts, WObject *parent)
  : nbPts_(nbPts)
{}

int PointsData::rowCount(const Wt::WModelIndex& parent) const
{
  return nbPts_;
}

int PointsData::columnCount(const Wt::WModelIndex& parent) const
{
  return 3;
}

boost::any PointsData::data(int row, int column, int role,
			    const WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

boost::any PointsData::data(const Wt::WModelIndex& index,
			    int role) const
{
  if (role != DisplayRole) {
    return boost::any();
  }


  const double pi = 3.141592;
  double XYangle = index.row() * (8*pi/nbPts_);
  if (index.column() == 0) {
    return cos(XYangle);
  }
  if (index.column() == 1) {
    return sin(XYangle);
  }
  if (index.column() == 2) {
    return -5.0 + index.row() * (10.0/nbPts_);
  }
  return boost::any();
}

boost::any PointsData::headerData(int section,
				  Wt::Orientation orientation,
				  int role) const
{
  return 0.0; // unimplemented
}


Parabola::Parabola(double xMin, double deltaX, double yMin, double deltaY,
		   double factor, double minimum, bool withColorRoles,
		   double colorRoleBoundary, WObject *parent)
  : xMin_(xMin), deltaX_(deltaX), yMin_(yMin), deltaY_(deltaY),
    factor_(factor), minimum_(minimum), colorRoles_(withColorRoles),
    colorRoleBoundary_(colorRoleBoundary)
{
}

int Parabola::rowCount(const Wt::WModelIndex& parent) const
{
  return 41;
}

int Parabola::columnCount(const Wt::WModelIndex& parent) const
{
  return 41;
}

boost::any Parabola::data(int row, int column, int role,
			  const WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

boost::any Parabola::data(const Wt::WModelIndex& index,
			  int role) const
{
  // double value = factor_ * (xMin_+index.row()*deltaX_)*(yMin_+index.column()*deltaY_);
  double value = factor_ * ( (xMin_+index.row()*deltaX_)*(xMin_+index.row()*deltaX_) + (yMin_+index.column()*deltaY_)*(yMin_+index.column()*deltaY_) ) + minimum_;

  if (role == MarkerBrushColorRole) {
    if (!colorRoles_)
      return boost::any();
    else
      return value > colorRoleBoundary_ ? boost::any() : WColor(blue);
  } else if (role != DisplayRole) {
    return boost::any();
  } else {
    return value;
  }
}

boost::any Parabola::headerData(int section,
				  Wt::Orientation orientation,
				  int role) const
{
  return 0.0; // unimplemented
}
