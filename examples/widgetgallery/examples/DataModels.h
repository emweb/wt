#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <Wt/WStandardItemModel.h>

#ifdef WT_TARGET_JAVA
// Smuggling a using namespace into NumericalCharts3D.cpp to make operator | work.
using namespace Wt;
#endif

class SombreroData : public Wt::WStandardItemModel {
public:
  SombreroData(unsigned nbXpts, unsigned nbYpts);

  Wt::cpp17::any data(const Wt::WModelIndex& index,
                  Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  const double xStart_, xEnd_, yStart_, yEnd_;
};

class PlaneData : public Wt::WStandardItemModel {
public:
  PlaneData(unsigned nbXpts, unsigned nbYpts);

  Wt::cpp17::any data(const Wt::WModelIndex& index,
                  Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  const double xStart_, xEnd_, yStart_, yEnd_;
};

class HorizontalPlaneData : public Wt::WStandardItemModel {
public:
  HorizontalPlaneData(unsigned nbXpts, unsigned nbYpts);

  Wt::cpp17::any data(const Wt::WModelIndex& index,
                  Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  const double xStart_, xEnd_, yStart_, yEnd_;
};

class SpiralData : public Wt::WStandardItemModel {
public:
  SpiralData(unsigned nbPts);

  Wt::cpp17::any data(const Wt::WModelIndex& index,
                  Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  unsigned nbPts_;
};

#endif
