#include "WLegend3D.h"

#include "Wt/Chart/WAbstractColorMap.h"
#include "Wt/WPainter.h"
#include "Wt/WPen.h"
#include "Wt/WRectF.h"

namespace {
static int boxPadding = 5;
}

namespace Wt {
  namespace Chart {

void WLegend3D
::renderLegend(WPainter* painter,
	       const std::vector<std::unique_ptr<WAbstractDataSeries3D> >
	       & dataseries)
{
  if (!legendEnabled_)
    return;

  painter->save();

  int nbItems = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    WAbstractDataSeries3D *series = dataseries[i].get();
    if (series->isLegendEnabled() && !series->isHidden())
      nbItems++;
  }

  double textHeight = legendFont_.sizeLength().toPixels();
  double labelWidth = textHeight * 0.618;
  double lineHeight = textHeight * 1.5;
  double offset = (lineHeight-textHeight)/2;

  painter->setPen(legendBorder_);
  painter->setFont(legendFont_);
  painter->setBrush(legendBackground_);
  int nbRows = nbItems/legendColumns_;
  if (nbItems % legendColumns_)
    nbRows++;
  painter->drawRect(0, 0,
		    legendColumns_*legendColumnWidth_.value() + 2*boxPadding,
		    nbRows*lineHeight + 2*boxPadding);
  painter->translate(boxPadding, boxPadding);

  int count = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    WAbstractDataSeries3D *series = dataseries[i].get();
    if (!series->isLegendEnabled() ||
	series->isHidden())
      continue;

    count++;
    // paint icon
    if (series->colorMap() == 0) { // if chartpalette
      WColor seriesColor = series->chartpaletteColor();
      painter->fillRect(0, offset, labelWidth, textHeight,
			WBrush(seriesColor));
    } else { // else colormap
      series->colorMap()->createStrip(painter, WRectF(0, offset, labelWidth, textHeight));
    }

    // draw label
    painter->drawText( labelWidth + 10, 0, 100, lineHeight,
		       WFlags<AlignmentFlag>(AlignmentFlag::Left) | AlignmentFlag::Middle,
		       series->title() );

    // offset painter
    if (count == legendColumns_) {
      painter->translate(-(legendColumns_-1)*legendColumnWidth_.value(),
			 lineHeight);
      count = 0;
    } else {
      painter->translate(legendColumnWidth_.value(), 0);
    }
  }

  painter->restore();
}

int WLegend3D::width()
{
  return (int)(legendColumns_*legendColumnWidth_.value() + 2*boxPadding);
}

int WLegend3D::height(const std::vector<std::unique_ptr<WAbstractDataSeries3D> >
		      &dataseries)
{
  int nbItems = 0;
  for (unsigned i = 0; i < dataseries.size(); i++) {
    WAbstractDataSeries3D *series = dataseries[i].get();
    if (series->isLegendEnabled())
      nbItems++;
  }

  double lineHeight = legendFont_.sizeLength().value() * 1.5;

  int nbRows = nbItems/legendColumns_;
  if (nbItems % legendColumns_)
    nbRows++;

  return (int)(nbRows*lineHeight + 2*boxPadding);
}

  }
}
