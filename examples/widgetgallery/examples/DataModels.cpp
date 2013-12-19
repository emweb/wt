#include "DataModels.h"

#include "Wt/WColor"

#include <cmath>

using namespace Wt;

SombreroData::SombreroData(int nbXPts, int nbYPts,
			   double xStart, double xEnd,
			   double yStart, double yEnd,
			   WObject *parent)
  : EquidistantGrid(parent),
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
  if (role != DisplayRole) {
    return boost::any();
  }

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
  return 4*std::sin(std::sqrt(std::pow(x,2) + std::pow(y,2))) 
    / (std::sqrt (std::pow(x,2) + std::pow(y,2)));
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
		     WObject *parent)
  : EquidistantGrid(parent),
    nbXPts_(nbXPts), nbYPts_(nbYPts),
    xStart_(xStart), xDelta_(xDelta), yStart_(yStart), yDelta_(yDelta)
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
  if (role != DisplayRole) {
    return boost::any();
  }

  double x, y;
  y = yStart_ + index.column() * yDelta_;
  x = xStart_ + index.row() * xDelta_;
  return 0.5*y;
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
  if (role == MarkerBrushColorRole) {
    //return WColor(rand()%256, rand()%256, rand()%256);
    return WColor(0, 255, 0);
  } else if (role != DisplayRole) {
    return boost::any();
  }


  const double pi = 3.141592;
  double XYangle = index.row() * (8*pi/nbPts_);
  if (index.column() == 0) {
    return std::cos(XYangle);
  }
  if (index.column() == 1) {
    return std::sin(XYangle);
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
