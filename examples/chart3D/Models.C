#include "Models.h"

#include "Wt/WColor.h"
#include <cmath>

SombreroData::SombreroData(int nbXPts, int nbYPts,
			   double xStart, double xEnd,
			   double yStart, double yEnd)
  : WAbstractTableModel(),
    nbXPts_(nbXPts), nbYPts_(nbYPts),
    xStart_(xStart), xEnd_(xEnd), yStart_(yStart), yEnd_(yEnd)
{}

int SombreroData::rowCount(const Wt::WModelIndex&) const
{
  return nbXPts_+1;
}

int SombreroData::columnCount(const Wt::WModelIndex&) const
{
  return nbYPts_+1;
}

Wt::cpp17::any SombreroData::data(int row, int column, Wt::ItemDataRole role,
                              const Wt::WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

Wt::cpp17::any SombreroData::data(const Wt::WModelIndex& index,
                              Wt::ItemDataRole role) const
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

  if (role == Wt::ItemDataRole::MarkerBrushColor) {
    return Wt::cpp17::any();
  } else if (role == Wt::ItemDataRole::Display) {
    return 4*sin(sqrt(pow(x,2) + pow(y,2))) / (sqrt (pow(x,2) + pow(y,2)));
  } else {
    return Wt::cpp17::any();
  }
}

Wt::cpp17::any SombreroData::headerData(int section,
                                 Wt::Orientation orientation,
                                 Wt::ItemDataRole role) const
{
  return 0.0; // unimplemented
}

PlaneData::PlaneData(int nbXPts, int nbYPts,
		     double xStart, double xDelta,
		     double yStart, double yDelta,
		     bool Yvariation,
		     double colorRoleBound, double sizeRoleBound)
  : WAbstractTableModel(),
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

Wt::cpp17::any PlaneData::data(int row, int column, Wt::ItemDataRole role,
                           const Wt::WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

Wt::cpp17::any PlaneData::data(const Wt::WModelIndex& index,
                           Wt::ItemDataRole role) const
{


  double x, y, value;
  y = yStart_ + index.column() * yDelta_;
  x = xStart_ + index.row() * xDelta_;
  if (yVar_)
    value = 0.5*y;
  else 
    value = 0.5*x;

  if (role == Wt::ItemDataRole::Display) {
    return value;
  } else if (role == Wt::ItemDataRole::MarkerBrushColor) {
    if (value > colorRoleBound_)
      return Wt::WColor(Wt::StandardColor::Blue);
    else
      return Wt::cpp17::any();
  } else if (role == Wt::ItemDataRole::MarkerScaleFactor) {
    if (value > sizeRoleBound_)
      return 5;
    else
      return Wt::cpp17::any();
  } else {
    return Wt::cpp17::any();
}
}

Wt::cpp17::any PlaneData::headerData(int section,
                                 Wt::Orientation orientation,
                                 Wt::ItemDataRole role) const
{
  return 0.0; // unimplemented
}

PointsData::PointsData(int nbPts)
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

Wt::cpp17::any PointsData::data(int row, int column, Wt::ItemDataRole role,
                            const Wt::WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

Wt::cpp17::any PointsData::data(const Wt::WModelIndex& index,
                            Wt::ItemDataRole role) const
{
  if (role != Wt::ItemDataRole::Display) {
    return Wt::cpp17::any();
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
  return Wt::cpp17::any();
}

Wt::cpp17::any PointsData::headerData(int section,
                                  Wt::Orientation orientation,
                                  Wt::ItemDataRole role) const
{
  return 0.0; // unimplemented
}


Parabola::Parabola(double xMin, double deltaX, double yMin, double deltaY,
		   double factor, double minimum, bool withColorRoles,
		   double colorRoleBoundary)
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

Wt::cpp17::any Parabola::data(int row, int column, Wt::ItemDataRole role,
                          const Wt::WModelIndex &parent) const
{
  return data(createIndex(row, column, (void*)0), role);
}

Wt::cpp17::any Parabola::data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role) const
{
  // double value = factor_ * (xMin_+index.row()*deltaX_)*(yMin_+index.column()*deltaY_);
  double value = factor_ * ( (xMin_+index.row()*deltaX_)*(xMin_+index.row()*deltaX_) + (yMin_+index.column()*deltaY_)*(yMin_+index.column()*deltaY_) ) + minimum_;

  if (role == Wt::ItemDataRole::MarkerBrushColor) {
    if (!colorRoles_)
      return Wt::cpp17::any();
    else
      return value > colorRoleBoundary_ ? Wt::cpp17::any() : Wt::WColor(Wt::StandardColor::Blue);
  } else if (role != Wt::ItemDataRole::Display) {
    return Wt::cpp17::any();
  } else {
    return value;
  }
}

Wt::cpp17::any Parabola::headerData(int section,
                                  Wt::Orientation orientation,
                                  Wt::ItemDataRole role) const
{
  return 0.0; // unimplemented
}
