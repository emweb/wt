// This may look like C code, but it's really -*- C++ -*-
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WColor.h>
#include <Wt/WFont.h>
#include <Wt/WRectF.h>
#include <Wt/Chart/WStandardColorMap.h>

#include <map>

class ColorMapTest : public Wt::WPaintedWidget
{
public:
  ColorMapTest()
    : WPaintedWidget()
  {
    resize(500, 300);
    std::vector<Wt::Chart::WStandardColorMap::Pair> colormap;
    colormap.push_back(Wt::Chart::WStandardColorMap::Pair(0, Wt::WColor(Wt::StandardColor::DarkRed)));
    colormap.push_back(Wt::Chart::WStandardColorMap::Pair(1, Wt::WColor(Wt::StandardColor::Red)));
    colormap.push_back(Wt::Chart::WStandardColorMap::Pair(2, Wt::WColor(Wt::StandardColor::Gray)));
    colormap_ = std::make_unique<Wt::Chart::WStandardColorMap>(0,3,colormap, false);
    colormap2_ = std::make_unique<Wt::Chart::WStandardColorMap>(0,2,colormap, true);
  }

  ~ColorMapTest()
  {
  }
  
protected:
  void paintEvent(Wt::WPaintDevice *paintDevice) {
    Wt::WPainter painter(paintDevice);

    painter.translate(50,0);
    colormap_->paintLegend(&painter);
    
    painter.translate(150,0);
    colormap2_->paintLegend(&painter);
    
    painter.translate(150,0);
    colormap2_->discretise(5);
    colormap2_->paintLegend(&painter);
  }
  
private:
  std::unique_ptr<Wt::Chart::WStandardColorMap> colormap_;
  std::unique_ptr<Wt::Chart::WStandardColorMap> colormap2_;
};
