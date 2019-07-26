#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WSpinBox.h>

class ShapesWidget : public Wt::WPaintedWidget
{
public:
    ShapesWidget()
        : WPaintedWidget()
    {
	resize(310, 400);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);
        painter.setPen(Wt::WColor(Wt::StandardColor::Red));

	// DRAWING PRIMITIVE SHAPES
	// Draw a line from (0, 0) to (200, 0) and then 30 px downwards.
	painter.drawLine(0, 0, 200, 0);
	painter.drawLine(200, 0, 200, 30);

	// Draw and fill a rectangle from (0, 25) with width 80 and height 25
	// using the current pen (red) and brush (default: white).
	painter.drawRect(0, 25, 80, 50);
	// Set the current brush with the global color name 'green'.
	painter.setBrush(Wt::WBrush(Wt::WColor(Wt::StandardColor::Green)));
	// Draw the same rectangle from (100, 25) using the current pen and brush.
	painter.drawRect(100, 25, 80, 50);
	// Fill a rectangle from (100, 25) with width 80 and height 25.
	// Choose a color with red=0, green=255, blue=0, alpha=64.
	painter.fillRect(220, 25, 80, 50, Wt::WBrush(Wt::WColor(0, 255, 0, 64)));

	// Draw the outline of an ellipse.
	painter.drawEllipse(0, 100, 80, 50);
	// Draw the upper segment of the ellipse (angle = 180 x 1/16th of a degree)
	painter.drawChord(100, 100, 80, 50, 0, 180*16);

	// Draw an open arc and a closed arc.
	painter.drawArc(220, 100, 50, 50, 90*16, 90*16);
	painter.drawArc(240, 100, 50, 50, 0, 90*16);
	painter.drawLine(265, 100, 265, 125);
	painter.drawLine(265, 125, 290, 125);

	// Draw a 6-point polygon and fill it with the current brush.
	const Wt::WPointF points[]
	    = { Wt::WPointF(120, 170),   Wt::WPointF(160, 170),
	        Wt::WPointF(180, 204.6), Wt::WPointF(160, 239.2),
	        Wt::WPointF(120, 239.2), Wt::WPointF(100, 204.6) };
	painter.drawPolygon(points, 6);
	// DRAWING SHAPES USING A PATH
	// Create an ellipse path and fill it.
	Wt::WPainterPath filledEllipsePath = Wt::WPainterPath();
	filledEllipsePath.addEllipse(0, 180, 80, 50);
	filledEllipsePath.closeSubPath();
	painter.drawPath(filledEllipsePath);

	// Create a new path for a triangle.
	Wt::WPainterPath filledTrianglePath = Wt::WPainterPath();
	filledTrianglePath.moveTo(0, 270);
	filledTrianglePath.lineTo(80,270);
	filledTrianglePath.lineTo(0, 350);
	filledTrianglePath.closeSubPath();
	// Draw the path and fill it.
	painter.drawPath(filledTrianglePath);

	// strokePath draws a path but doesn't fill it.
	Wt::WPainterPath strokedTrianglePath = Wt::WPainterPath();
	strokedTrianglePath.moveTo(100,270);
	strokedTrianglePath.lineTo(100,350);
	strokedTrianglePath.lineTo(20, 350);
	strokedTrianglePath.closeSubPath();
	Wt::WPen pen = Wt::WPen();
	pen.setWidth(3);
	painter.strokePath(strokedTrianglePath, pen);

	// Draw a balloon with quadratic bezier curves.
	Wt::WPainterPath quadraticCurvePath = Wt::WPainterPath();
	quadraticCurvePath.moveTo(250,150);
	quadraticCurvePath.quadTo(200,150, 200,187.5);
	quadraticCurvePath.quadTo(200,225, 225,225);
	quadraticCurvePath.quadTo(225,245, 205,250);
	quadraticCurvePath.quadTo(235,245, 240,225);
	quadraticCurvePath.quadTo(300,225, 300,187.5);
	quadraticCurvePath.quadTo(300,150, 250,150);
	painter.strokePath(quadraticCurvePath, pen);

	// Draw a heart with cubic bezier curves.
	Wt::WPainterPath bezierCurvePath = Wt::WPainterPath();
	bezierCurvePath.moveTo( 255,285);
	bezierCurvePath.cubicTo(255,282,  250,270,  230,270);
	bezierCurvePath.cubicTo(200,270,  200,307.5,200,307.5);
	bezierCurvePath.cubicTo(200,325,  220,357,  255,365);
	bezierCurvePath.cubicTo(290,347,  310,325,  310,307.5);
	bezierCurvePath.cubicTo(310,307.5,310,270,  290,270);
	bezierCurvePath.cubicTo(265,270,  255,282,  255,285);
	painter.setBrush(Wt::WBrush(Wt::WColor(Wt::StandardColor::Red)));
	painter.drawPath(bezierCurvePath);
    }
};

SAMPLE_BEGIN(PaintingShapes)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

container->addNew<ShapesWidget>();

SAMPLE_END(return std::move(container))

