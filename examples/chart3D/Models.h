#ifndef MODELS_H
#define MODELS_H

#include <Wt/WAbstractTableModel>

using namespace Wt;

class SombreroData : public WAbstractTableModel {
public:
  SombreroData(int nbXPts, int nbYPts,
	       double xStart, double xEnd,
	       double yStart, double yEnd,
	       WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=DisplayRole, const WModelIndex &parent=WModelIndex()) const ;

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const;

  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const;

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

class PlaneData : public WAbstractTableModel {
public:
  PlaneData(int nbXPts, int nbYPts,
	    double xStart, double xDelta,
	    double yStart, double yDelta,
	    bool Yvariation,
	    double colorRoleBound, double sizeRoleBound,
	    WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=DisplayRole,
		  const WModelIndex &parent=WModelIndex()) const ;

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const;

  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const;

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

class PointsData : public WAbstractTableModel {
public:
  PointsData(int nbPts, WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=DisplayRole,
		  const WModelIndex &parent=WModelIndex()) const ;

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const;

  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const;

private:
  int nbPts_;
};

class Parabola : public WAbstractTableModel {
public:
  Parabola(double xMin, double deltaX, double yMin, double deltaY,
	   double factor, double minimum,
	   bool withColorRoles, double colorRoleBoundary, 
	   WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=DisplayRole,
		  const WModelIndex &parent=WModelIndex()) const ;

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const;

  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const;

private:
  double xMin_, deltaX_;
  double yMin_, deltaY_;

  double factor_;
  double minimum_;

  bool colorRoles_;
  double colorRoleBoundary_;
};

#endif
