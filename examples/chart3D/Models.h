#ifndef MODELS_H
#define MODELS_H

#include <Wt/WAbstractTableModel.h>

class SombreroData : public Wt::WAbstractTableModel {
public:
  SombreroData(int nbXPts, int nbYPts,
	       double xStart, double xEnd,
	       double yStart, double yEnd);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;

  Wt::cpp17::any data(int row, int column, Wt::ItemDataRole role = Wt::ItemDataRole::Display, const Wt::WModelIndex &parent = Wt::WModelIndex()) const;

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  void update(double xStart, double xEnd, double yStart, double yEnd,
	      int nbXPts, int nbYPts) {
    nbXPts_ = nbXPts;
    nbYPts_ = nbYPts;
    xStart_ = xStart;
    xEnd_ = xEnd;
    yStart_ = yStart;
    yEnd_ = yEnd;
  }
  
private:
  int nbXPts_, nbYPts_;
  double xStart_, xEnd_, yStart_, yEnd_;
};

class PlaneData : public Wt::WAbstractTableModel {
public:
  PlaneData(int nbXPts, int nbYPts,
	    double xStart, double xDelta,
	    double yStart, double yDelta,
	    bool Yvariation,
	    double colorRoleBound, double sizeRoleBound);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;

  Wt::cpp17::any data(int row, int column, Wt::ItemDataRole role = Wt::ItemDataRole::Display,
                  const Wt::WModelIndex &parent = Wt::WModelIndex()) const;

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  void update(double xStart, double xDelta, double yStart, double yDelta,
	      int nbXPts, int nbYPts) {
    nbXPts_ = nbXPts;
    nbYPts_ = nbYPts;
    xStart_ = xStart;
    xDelta_ = xDelta;
    yStart_ = yStart;
    yDelta_ = yDelta;
  }
  
private:
  int nbXPts_, nbYPts_;
  double xStart_, xDelta_, yStart_, yDelta_;
  bool yVar_;

  double colorRoleBound_, sizeRoleBound_;
};

class PointsData : public Wt::WAbstractTableModel {
public:
  PointsData(int nbPts);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;

  Wt::cpp17::any data(int row, int column, Wt::ItemDataRole role = Wt::ItemDataRole::Display,
                  const Wt::WModelIndex &parent = Wt::WModelIndex()) const ;

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  int nbPts_;
};

class Parabola : public Wt::WAbstractTableModel {
public:
  Parabola(double xMin, double deltaX, double yMin, double deltaY,
	   double factor, double minimum,
	   bool withColorRoles, double colorRoleBoundary);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const override;

  Wt::cpp17::any data(int row, int column, Wt::ItemDataRole role = Wt::ItemDataRole::Display,
                  const Wt::WModelIndex &parent = Wt::WModelIndex()) const ;

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

private:
  double xMin_, deltaX_;
  double yMin_, deltaY_;

  double factor_;
  double minimum_;

  bool colorRoles_;
  double colorRoleBoundary_;
};

#endif
