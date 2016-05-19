/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <cstdio>

#include "Wt/Chart/WAbstractChartModel"
#include "Wt/Chart/WPieChart"
#include "Wt/Chart/WStandardPalette"

#include "Wt/WContainerWidget"
#include "Wt/WCssDecorationStyle"
#include "Wt/WText"
#include "Wt/WPainter"
#include "Wt/WPolygonArea"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WModelIndex"

#include "WebUtils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Wt {
  namespace Chart {

WPieChart::PieData::PieData()
  : customBrush(false),
    explode(0)
{ }

WPieChart::WPieChart(WContainerWidget *parent)
  : WAbstractChart(parent),
    labelsColumn_(-1),
    dataColumn_(-1),
    height_(0.0),
    startAngle_(45),
    avoidLabelRendering_(0.0),
    labelOptions_(0),
    shadow_(false),
    labelFormat_(WString::fromUTF8("%.3g%%"))
{
  setPalette(new WStandardPalette(WStandardPalette::Neutral));
  setPlotAreaPadding(5);
}

void WPieChart::setLabelsColumn(int modelColumn)
{
  if (labelsColumn_ != modelColumn) {
    labelsColumn_ = modelColumn;
    update();
  }
}

void WPieChart::setLabelFormat(const WString& format)
{
  labelFormat_ = format;
  update();
}

WString WPieChart::labelFormat() const
{
  return labelFormat_;
}

void WPieChart::setDataColumn(int modelColumn)
{
  if (dataColumn_ != modelColumn) {
    dataColumn_ = modelColumn;
    update();
  }
}

void WPieChart::setBrush(int modelRow, const WBrush& brush)
{
  pie_[modelRow].customBrush = true;
  pie_[modelRow].brush = brush;
  update();
}

WBrush WPieChart::brush(int modelRow) const
{
  if (pie_[modelRow].customBrush)
    return pie_[modelRow].brush;
  else
    return palette()->brush(modelRow);
}

void WPieChart::setExplode(int modelRow, double factor)
{
  pie_[modelRow].explode = factor;
  update();
}

double WPieChart::explode(int modelRow) const
{
  return pie_[modelRow].explode;
}

void WPieChart::setPerspectiveEnabled(bool enabled, double height)
{
  if ((!enabled && height_ != 0.0) || height_ != height) {
    height_ = enabled ? height : 0.0;
    update();
  }
}

void WPieChart::setShadowEnabled(bool enabled)
{
  if (shadow_ != enabled) {
    shadow_ = enabled;
    update();
  }
}

void WPieChart::setStartAngle(double startAngle)
{
  if (startAngle_ != startAngle) {
    startAngle_ = startAngle;
    update();
  }
}

void WPieChart::setAvoidLabelRendering(double avoidLabelRendering)
{
  if (avoidLabelRendering_ != avoidLabelRendering) {
    avoidLabelRendering_ = avoidLabelRendering;
    update();
  }
}

void WPieChart::setDisplayLabels(WFlags<LabelOption> options)
{
  labelOptions_ = options;

  update();
}

WWidget* WPieChart::createLegendItemWidget(int index, 
					   WFlags<LabelOption> options)
{
  WContainerWidget* legendItem = new WContainerWidget();
  legendItem->setPadding(4);
  
  WText* colorText = new WText();
  legendItem->addWidget(colorText);
  colorText->setPadding(10, Left | Right);
  colorText->decorationStyle().setBackgroundColor(brush(index).color());

  if (WApplication::instance()->environment().agentIsIE())
    colorText->setAttributeValue("style", "zoom: 1;");

  double total = 0;

  if (dataColumn_ != -1)
    for (int i = 0; i < model()->rowCount(); ++i) {
      double v = model()->data(i, dataColumn_);
      if (!Utils::isNaN(v))
	total += v;
    }

  double value = model()->data(index, dataColumn_);
  if (!Utils::isNaN(value)) {
    WString label = labelText(index, value, total, options);
    if (!label.empty()) {
      WText* l = new WText(label);
      l->setPadding(5, Left);
      l->setToolTip(model()->toolTip(index, dataColumn_));
      legendItem->addWidget(l);
    }
  }

  return legendItem;
}

void WPieChart::paint(WPainter& painter, const WRectF& rectangle) const
{
  double total = 0;

  if (dataColumn_ != -1)
    for (int i = 0; i < model()->rowCount(); ++i) {
      double v = model()->data(i, dataColumn_);
      if (!Utils::isNaN(v))
	total += v;
    }

  if (!painter.isActive())
    throw WException("WPieChart::paint(): painter is not active.");

  WRectF rect = rectangle;

  if (rect.isNull() || rect.isEmpty())
    rect = painter.window();

  rect.setX(rect.x() + plotAreaPadding(Left));
  rect.setY(rect.y() + plotAreaPadding(Top));
  rect.setWidth(rect.width() - plotAreaPadding(Left) - plotAreaPadding(Right));
  rect.setHeight(rect.height() - plotAreaPadding(Top)
		 - plotAreaPadding(Bottom));

  double side = std::min(rect.width(), rect.height());

  painter.save();
  painter.translate(rect.left() + (rect.width() - side)/2,
		    rect.top() + (rect.height() - side)/2);

  if (!title().empty())
    painter.translate(0, 15);

  double cx = std::floor(side/2) + 0.5;
  double cy = cx;
  double r = (int)(side/2 + 0.5);
  double h = height_ * r;

  painter.save();
  if (h > 0.0) {
    painter.translate(0, r/2 - h/4);
    painter.scale(1, 0.5);
  }

  drawPie(painter, cx, cy, r, h, total);
  painter.restore();

  painter.translate(0, -h/4);
  if (labelOptions_) {
    if (total != 0) {
      double currentAngle = startAngle_;

      for (int i = 0; i < model()->rowCount(); ++i) {
	double v = model()->data(i, dataColumn_);
	if (Utils::isNaN(v))
	  continue;

	double spanAngle = -v / total * 360;
	double midAngle = currentAngle + spanAngle / 2.0;
	double endAngle = currentAngle + spanAngle;
	if (endAngle < 0)
	  endAngle += 360;
	if (midAngle < 0)
	  midAngle += 360;

	double width = 200;
	double height = 30;
	double left;
	double top;

	double f;
	if (labelOptions_ & Outside)
	  f = pie_[i].explode + 1.1;
	else
	  f = pie_[i].explode + 0.7;

	double px = cx + f * r * std::cos(-midAngle / 180.0 * M_PI);
	double py = cy + f * r * std::sin(-midAngle / 180.0 * M_PI)
	  * (h > 0 ? 0.5 : 1);

	WFlags<AlignmentFlag> alignment;

	WColor c = painter.pen().color();
	if (labelOptions_ & Outside) {
	  if (midAngle < 90) {
	    left = px;
	    top = py - height;
	    alignment = AlignLeft | AlignBottom;
	  } else if (midAngle < 180) {
	    left = px - width;
	    top = py - height;
	    alignment = AlignRight | AlignBottom;
	  } else if (midAngle < 270) {
	    left = px - width;
	    top = py + h/2;
	    alignment = AlignRight | AlignTop;
	  } else {
	    left = px;
	    top = py + h/2;
	    alignment = AlignLeft | AlignTop;
	  }
	} else {
	  left = px - width/2;
	  top = py - height/2;
	  alignment = AlignCenter | AlignMiddle;
	  c = palette()->fontColor(i);
	}

  if ((v / total * 100) >= avoidLabelRendering_) {
    painter.setPen(WPen(c));
    drawLabel(&painter, WRectF(left, top, width, height),
             alignment, labelText(i, v, total, labelOptions_), i);
	}

	currentAngle = endAngle;
      }
    }
  }

  if (!title().empty()) {
    WFont oldFont = painter.font();
    painter.setFont(titleFont());
    painter.drawText(cx - 50, cy - r, 100, 50,
		     AlignCenter | AlignTop, title());
    painter.setFont(oldFont);
  }

  painter.restore();
}

WContainerWidget *WPieChart::createLabelWidget(WWidget *textWidget,
    WPainter* painter, const WRectF& rect,
    Wt::WFlags<Wt::AlignmentFlag> alignmentFlags) const
{
  AlignmentFlag verticalAlign = alignmentFlags & AlignVerticalMask;
  AlignmentFlag horizontalAlign = alignmentFlags & AlignHorizontalMask;

  // style parent container
  WContainerWidget *c = new WContainerWidget();
  c->addWidget(textWidget);
  c->setPositionScheme(Wt::Absolute);
  c->setAttributeValue("style", "display: flex;");

  const WRectF& normRect = rect.normalized();
  Wt::WTransform t = painter->worldTransform();
  Wt::WPointF p = t.map(Wt::WPointF(normRect.left(), normRect.top()));

  c->setWidth(WLength(normRect.width() * t.m11()));
  c->setHeight(WLength(normRect.height() * t.m22()));
  c->decorationStyle().setFont(painter->font());

  std::stringstream containerStyle;
  containerStyle << "top:" << p.y() << "px; left:" << p.x() << "px; "
    << "display: flex;";

  switch (horizontalAlign) {
  case AlignLeft: containerStyle << " justify-content: flex-start;"; break;
  case AlignRight: containerStyle << " justify-content: flex-end;"; break;
  case AlignCenter: containerStyle << " justify-content: center;"; break;
  default: break;
  }

  c->setAttributeValue("style", containerStyle.str());

  // style inner text.

  std::stringstream innerStyle;

  switch (verticalAlign) {
  case AlignTop: innerStyle << "align-self: flex-start;"; break;
  case AlignBottom: innerStyle << "align-self: flex-end;"; break;
  case AlignMiddle: innerStyle << " align-self: center;"; break;
  default: break;
  }

  textWidget->setAttributeValue("style", innerStyle.str());

  return c;
}

void WPieChart::drawLabel(WPainter* painter, const WRectF& rect,
                         WFlags<AlignmentFlag> alignmentFlags,
                         const WString& text, int row) const
{
  painter->drawText(rect, alignmentFlags, text);
}

WString WPieChart::labelText(int index, double v, double total, 
			     WFlags<LabelOption> options) const
{
  WString text;

  if (options & TextLabel)
    if (labelsColumn_ != -1)
      text += model()->displayData(index, labelsColumn_);

  if (options & TextPercentage) {
    std::string label;
    double u = v / total * 100;

    std::string format = labelFormat().toUTF8();
    if (format.empty())
      label = WLocale::currentLocale().toString(u).toUTF8() + "%";
    else {
#ifndef WT_TARGET_JAVA
      char buf[30];
#else
      char *buf = 0;
#endif

#ifdef WT_TARGET_JAVA
      buf =
#endif // WT_TARGET_JAVA
	std::sprintf(buf, format.c_str(), u);
      label = buf;
    }

    if (!text.empty())
      text += ": ";
    text += WString::fromUTF8(label);
  }

  return text;
}

void WPieChart::setShadow(WPainter& painter) const
{
  painter.setShadow(WShadow(5, 15, WColor(0, 0, 0, 20), 40));
}

void WPieChart::drawPie(WPainter& painter, double cx, double cy,
			double r, double h, double total) const
{
  /*
   * Draw sides where applicable
   */
  if (h > 0) {
    if (total == 0) {
      if (shadow_)
	setShadow(painter);
      drawOuter(painter, cx, cy, r, 0, -180, h);
      if (shadow_)
	painter.setShadow(WShadow());
    } else {
      if (shadow_) {
	setShadow(painter);
	painter.setBrush(WBrush(black));
	drawSlices(painter, cx, cy + h, r, total, true);
	painter.setShadow(WShadow());
      }

      /*
       * Pre-processing: determine start and mid angles of each pie,
       * and get the index of the one that contains 90 degrees (which is
       * the one at the back
       */
#ifndef WT_TARGET_JAVA
      std::vector<double> startAngles(model()->rowCount());
      std::vector<double> midAngles(model()->rowCount());
#else
      std::vector<double> startAngles, midAngles;
      startAngles.insert(startAngles.end(), model()->rowCount(), 0.0);
      midAngles.insert(midAngles.end(), model()->rowCount(), 0.0);
#endif // WT_TARGET_JAVA

      int index90 = 0;

      double currentAngle = startAngle_;
      for (int i = 0; i < model()->rowCount(); ++i) {
	startAngles[i] = currentAngle;

	double v = model()->data(i, dataColumn_);
	if (Utils::isNaN(v))
	  continue;

	double spanAngle = -v / total * 360;
	midAngles[i] = currentAngle + spanAngle / 2.0;

	double endAngle = currentAngle + spanAngle;

	double to90 = currentAngle - 90;
	if (to90 < 0)
	  to90 += 360;

	if (spanAngle <= -to90)
	  index90 = i;

	if (endAngle < 0)
	  endAngle += 360;	

	currentAngle = endAngle;
      }

      /*
       * Draw clock wise side
       */
      for (int j = 0; j < model()->rowCount(); ++j) {
	int i = (index90 + j) % model()->rowCount();

	double v = model()->data(i, dataColumn_);
	if (Utils::isNaN(v))
	  continue;

	double midAngle = midAngles[i];
	double endAngle = startAngles[(i + 1) % model()->rowCount()];

	int n = nextIndex(i);

	bool visible = (endAngle <= 90) || (endAngle >= 270);

	bool drawS2 = visible
	  && ((pie_[i].explode > 0.0) || (pie_[n].explode > 0.0));

	if (drawS2) {
	  double pcx = cx + r * pie_[i].explode * std::cos(-midAngle / 180.0 * M_PI);
	  double pcy = cy + r * pie_[i].explode * std::sin(-midAngle / 180.0 * M_PI);

	  painter.setBrush(darken(brush(i)));

	  drawSide(painter, pcx, pcy, r, endAngle, h);
	}

	if (!visible)
	  break;
      }

      /*
       * Draw counter-clock wise side
       */
      for (int j = model()->rowCount(); j > 0; --j) {
	int i = (index90 + j) % model()->rowCount();

	double v = model()->data(i, dataColumn_);
	if (Utils::isNaN(v))
	  continue;

	double startAngle = startAngles[i];
	double midAngle = midAngles[i];

	int p = prevIndex(i);

	bool visible = (startAngle >= 90) && (startAngle <= 270);

	bool drawS1 = visible
	  && ((pie_[i].explode > 0.0) || (pie_[p].explode > 0.0));

	if (drawS1) {
	  double pcx = cx + r * pie_[i].explode * std::cos(-midAngle / 180.0 * M_PI);
	  double pcy = cy + r * pie_[i].explode * std::sin(-midAngle / 180.0 * M_PI);

	  painter.setBrush(darken(brush(i)));
	  drawSide(painter, pcx, pcy, r, startAngle, h);
	}

	if (!visible)
	  break;
      }

      /*
       * Outside
       */
      for (int j = 0; j < model()->rowCount(); ++j) {
	int i = (index90 + j) % model()->rowCount();

	double v = model()->data(i, dataColumn_);
	if (Utils::isNaN(v))
	  continue;

	double startAngle = startAngles[i];
	double midAngle = midAngles[i];
	double endAngle = startAngles[(i + 1) % model()->rowCount()];

	double spanAngle = endAngle - startAngle;

	if (spanAngle > 0)
	  spanAngle -= 360;

	bool drawBorder = startAngle > 180 || endAngle > 180
	  || spanAngle < -180 || model()->rowCount() == 1;

	if (drawBorder) {
	  painter.setBrush(darken(brush(i)));

	  double pcx = cx + r * pie_[i].explode * std::cos(-midAngle / 180.0 * M_PI);
	  double pcy = cy + r * pie_[i].explode * std::sin(-midAngle / 180.0 * M_PI);

	  double a1 = (startAngle < 180 ? 360 : startAngle);
	  double a2 = (endAngle < 180 ? 180 : endAngle);

	  drawOuter(painter, pcx, pcy, r, a1, a2, h);
	}
      }
    }
  }

  /*
   * Draw top
   */
  if (total == 0)
    painter.drawArc(cx - r, cy - r, r*2, r*2, 0, 16*360);
  else
    drawSlices(painter, cx, cy, r, total, false);
}

void WPieChart::drawSlices(WPainter& painter,
			   double cx, double cy, double r, double total,
			   bool shadow) const
{
  double currentAngle = startAngle_;

  for (int i = 0; i < model()->rowCount(); ++i) {
    double v = model()->data(i, dataColumn_);
    if (Utils::isNaN(v))
      continue;

    double spanAngle = -v / total * 360;
    double midAngle = currentAngle + spanAngle / 2.0;

    double pcx = cx + r * pie_[i].explode * std::cos(-midAngle / 180.0 * M_PI);
    double pcy = cy + r * pie_[i].explode * std::sin(-midAngle / 180.0 * M_PI);

    if (!shadow)
      painter.setBrush(brush(i));

    if (v/total != 1.0)
      painter.drawPie(pcx - r, pcy - r, r*2, r*2,
		      static_cast<int>(currentAngle * 16),
		      static_cast<int>(spanAngle * 16));
    else
      painter.drawEllipse(pcx - r, pcy - r, r*2, r*2);

    /*
     * See if we need to add an interactive area
     */
    if (!shadow) {
      WString toolTip = model()->toolTip(i, dataColumn_);
      WLink *link = model()->link(i, dataColumn_);
      if (!toolTip.empty() || link) {
        const int SEGMENT_ANGLE = 20;

	WPolygonArea *area = new WPolygonArea();
	WTransform t = painter.worldTransform();

	area->addPoint(t.map(WPointF(pcx, pcy)));

	double sa = std::fabs(spanAngle);

	for (double d = 0; d < sa; d += SEGMENT_ANGLE) {
	  double a;
	  if (spanAngle < 0)
	    a = currentAngle - d;
	  else
	    a = currentAngle + d;
	  area->addPoint(t.map(WPointF(pcx + r * std::cos(-a / 180.0 * M_PI),
				       pcy + r * std::sin(-a / 180.0 * M_PI))));
	}

	double a = currentAngle + spanAngle;
	area->addPoint(t.map(WPointF(pcx + r * std::cos(-a / 180.0 * M_PI),
				     pcy + r * std::sin(-a / 180.0 * M_PI))));

        area->setToolTip(toolTip);
        if (link)
          area->setLink(*link);

	addDataPointArea(i, dataColumn_, area);
      }
    }

    double endAngle = currentAngle + spanAngle;
    if (endAngle < 0)
      endAngle += 360;

    currentAngle = endAngle;
  }
}

void WPieChart::addDataPointArea(int row, int column, WAbstractArea *area) const
{
  (const_cast<WPieChart *>(this))->addArea(area);
}

WBrush WPieChart::darken(const WBrush& brush)
{
  WBrush result = brush;
  WColor c = result.color();

  c.setRgb(c.red() * 3/4,
	   c.green() * 3/4,
	   c.blue() * 3/4,
	   c.alpha());

  result.setColor(c);

  return result;
}

void WPieChart::drawSide(WPainter& painter, double pcx, double pcy, double r,
			 double angle, double h) const
{
  WPainterPath path;
  path.arcMoveTo(pcx - r, pcy - r, 2 * r, 2 * r, angle);
  path.lineTo(path.currentPosition().x(), path.currentPosition().y() + h);
  path.lineTo(pcx, pcy + h);
  path.lineTo(pcx, pcy);
  path.closeSubPath();

  painter.drawPath(path);
}

void WPieChart::drawOuter(WPainter& painter, double pcx, double pcy, double r,
			  double a1, double a2, double h) const
{
  WPainterPath path;
  path.arcMoveTo(pcx - r, pcy - r, 2 * r, 2 * r, a1);
  path.lineTo(path.currentPosition().x(), path.currentPosition().y() + h);
  path.arcTo(pcx, pcy + h, r, a1, a2 - a1);
  path.arcTo(pcx, pcy, r, a2, a1 - a2);
  path.closeSubPath();

  painter.drawPath(path);
}

void WPieChart::paintEvent(WPaintDevice *paintDevice)
{
  while (!areas().empty())
    delete areas().front();

  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing, true);
  paint(painter);
}

int WPieChart::nextIndex(int i) const
{
  int r = model()->rowCount();
  for (int n = (i + 1) % r; n != i; n = (n + 1) % r) {
    double v = model()->data(n, dataColumn_);
    if (!Utils::isNaN(v))
      return n;
  }

  return i;
}

int WPieChart::prevIndex(int i) const
{
  int r = model()->rowCount();
  for (int p = i - 1; p != i; --p) {
    if (p < 0)
      p += r;
    double v = model()->data(p, dataColumn_);
    if (!Utils::isNaN(v))
      return p;
  }

  return i;
}

void WPieChart::modelReset()
{
  if (model()->rowCount() != (int)pie_.size())
    modelChanged();
  else
    update();
}

void WPieChart::modelChanged()
{
  pie_.clear();
  pie_.insert(pie_.begin(), model()->rowCount(), PieData());

  update();
}

  }
}
