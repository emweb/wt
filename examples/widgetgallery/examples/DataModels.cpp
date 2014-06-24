#include "DataModels.h"

#include "Wt/WColor"

#include <cmath>

using namespace Wt;

SombreroData::SombreroData(unsigned nbXpts, unsigned nbYpts, WObject *parent)
  : WStandardItemModel(nbXpts+1, nbYpts+1, parent),
    xStart_(-10.0), xEnd_(10.0), yStart_(-10.0), yEnd_(10.0)
{}

boost::any SombreroData::data(const Wt::WModelIndex& index, int role) const
{
  if (role != DisplayRole) {
    return WStandardItemModel::data(index, role);
  }

  double delta_y = (yEnd_ - yStart_)/(columnCount()-2);
  if (index.row() == 0) { // give back y-abscis
    if (index.column() == 0)
      return 0.0;
    return yStart_ + (index.column()-1)*delta_y;
  }
  
  double delta_x = (xEnd_ - xStart_)/(rowCount()-2);
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

PlaneData::PlaneData(unsigned nbXpts, unsigned nbYpts, WObject *parent)
  : WStandardItemModel(nbXpts+1, nbYpts+1, parent),
    xStart_(-10.0), xEnd_(10.0), yStart_(-10.0), yEnd_(10.0)
{}

boost::any PlaneData::data(const Wt::WModelIndex& index, int role) const
{
  if (role != DisplayRole) {
    return WStandardItemModel::data(index, role);
  }

  double delta_x = (xEnd_ - xStart_)/(rowCount()-2);
  double delta_y = (yEnd_ - yStart_)/(columnCount()-2);
  double x = xStart_ + index.row() * delta_x;
  double y = yStart_ + index.column() * delta_y;

  return 0.2*x - 0.2*y;
}

HorizontalPlaneData::HorizontalPlaneData(unsigned nbXpts, unsigned nbYpts, WObject *parent)
  : WStandardItemModel(nbXpts+1, nbYpts+1, parent),
    xStart_(-10.0), xEnd_(10.0), yStart_(-10.0), yEnd_(10.0)
{}

boost::any HorizontalPlaneData::data(const Wt::WModelIndex& index, int role) const
{
  if (role != DisplayRole) {
    return WStandardItemModel::data(index, role);
  }

  return 0.0;
}

SpiralData::SpiralData(unsigned nbPts, WObject *parent)
  : WStandardItemModel(nbPts, 3, parent), nbPts_(nbPts)
{}

boost::any SpiralData::data(const Wt::WModelIndex& index, int role) const
{
  if (role != DisplayRole) {
    return WStandardItemModel::data(index, role);
  }

  const double pi = 3.141592;
  double XYangle = index.row() * (8*pi/nbPts_);
  double heightRatio = (float)index.row() / rowCount();
  double radius = 1.0 + heightRatio * 5.0;
  if (index.column() == 0) {
    return radius * std::cos(XYangle);
  } else if (index.column() == 1) {
    return radius * std::sin(XYangle);
  } else if (index.column() == 2) {
    return 5.0 - index.row() * (10.0/nbPts_);
  } else {
    return boost::any();
  }
}
