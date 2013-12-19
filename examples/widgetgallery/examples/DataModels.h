#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <Wt/WModelIndex>
#include <Wt/WAbstractTableModel>

class EquidistantGrid : public Wt::WAbstractTableModel {
public:
  EquidistantGrid(Wt::WObject *parent = 0)
    : Wt::WAbstractTableModel(parent)
    {}

  virtual double xMin() const = 0;
  virtual double xMax() const = 0;
  virtual double yMin() const = 0;
  virtual double yMax() const = 0;
  virtual int nbXPts() const = 0;
  virtual int nbYPts() const = 0;

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const = 0;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const = 0;
  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const = 0;
  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const = 0;
};

class SombreroData : public EquidistantGrid {
public:
  SombreroData(int nbXPts, int nbYPts,
	      double xStart, double xEnd,
	      double yStart, double yEnd,
	      Wt::WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=Wt::DisplayRole, const Wt::WModelIndex &parent=Wt::WModelIndex()) const ;

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

    reset();
  }

  double xMin() const { return xStart_; }
  double xMax() const { return xEnd_; }
  double yMin() const { return yStart_; }
  double yMax() const { return yEnd_; }
  int nbXPts() const { return nbXPts_; }
  int nbYPts() const { return nbYPts_; }
  
private:
  int nbXPts_, nbYPts_;
  double xStart_, xEnd_, yStart_, yEnd_;
};

class PlaneData : public EquidistantGrid {
public:
  PlaneData(int nbXPts, int nbYPts,
	    double xStart, double xDelta,
	    double yStart, double yDelta,
	    Wt::WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=Wt::DisplayRole,
		  const Wt::WModelIndex &parent=Wt::WModelIndex()) const ;

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

    reset();
  }

  double xMin() const { return xStart_; }
  double xMax() const { return xStart_ + nbXPts_*xDelta_; }
  double yMin() const { return yStart_; }
  double yMax() const { return yStart_ + nbYPts_*yDelta_; }
  int nbXPts() const { return nbXPts_; }
  int nbYPts() const { return nbYPts_; }
  
private:
  int nbXPts_, nbYPts_;
  double xStart_, xDelta_, yStart_, yDelta_;
};

class PointsData : public Wt::WAbstractTableModel {
public:
  PointsData(int nbPts, Wt::WObject *parent = 0);

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;

  boost::any data(int row, int column, int role=Wt::DisplayRole,
		  const Wt::WModelIndex &parent=Wt::WModelIndex()) const ;

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const;

  virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const;

private:
  int nbPts_;
};


#endif
