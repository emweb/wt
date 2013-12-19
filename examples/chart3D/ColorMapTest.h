// This may look like C code, but it's really -*- C++ -*-
#include <Wt/WContainerWidget>
#include <Wt/WPaintDevice>
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>
#include <Wt/WColor>
#include <Wt/WFont>
#include <Wt/WRectF>
#include <Wt/Chart/WStandardColorMap>

#include <map>

using namespace Wt;
using namespace Wt::Chart;

class ColorMapTest : public Wt::WPaintedWidget
{
public:
  ColorMapTest(Wt::WContainerWidget *parent = 0)
    : Wt::WPaintedWidget(parent)
  {
    resize(500, 300);
    std::vector<WStandardColorMap::Pair> colormap;
    colormap.push_back(WStandardColorMap::Pair(0,WColor(darkRed)));
    colormap.push_back(WStandardColorMap::Pair(1,WColor(red)));
    colormap.push_back(WStandardColorMap::Pair(2,WColor(gray)));
    colormap_ = new WStandardColorMap(0,3,colormap, false);
    colormap2_ = new WStandardColorMap(0,2,colormap, true);
  }

  ~ColorMapTest()
  {
    delete colormap_;
    delete colormap2_;
  }
  
protected:
  void paintEvent(Wt::WPaintDevice *paintDevice) {
    WPainter painter(paintDevice);

    painter.translate(50,0);
    colormap_->paintLegend(&painter);
    
    painter.translate(150,0);
    colormap2_->paintLegend(&painter);
    
    painter.translate(150,0);
    colormap2_->discretise(5);
    colormap2_->paintLegend(&painter);
  }
  
private:
  Wt::Chart::WStandardColorMap *colormap_;
  Wt::Chart::WStandardColorMap *colormap2_;
};
