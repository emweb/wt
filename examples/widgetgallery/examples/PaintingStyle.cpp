#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

class StyleWidget : public Wt::WPaintedWidget
{
public:
    StyleWidget()
        : WPaintedWidget()
    {
	resize(310, 1140);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);

	// Draw a grid of rectangles; each one in a different color.
	for (int row = 0; row < 6; row++)
	    for (int col = 0; col < 6; col++) {
		// Generate a unique RGB color for each square. Only the red and
		// green values are modified; the blue channel has a fixed value.
	        Wt::WBrush brush(Wt::WColor(255 - 42*row, 255 - 42*col, 0));
		painter.fillRect(row*25, col*25, 25, 25, brush);
	    }

	painter.translate(0, 160);
	// Draw a grid of circles similar to the above example but now using the
	// strokePath method.
	Wt::WPen pen;
	pen.setWidth(3);
	for (int row = 0; row < 6; row++) {
	    for (int col = 0; col < 6; col++) {
		// Generate a unique RGB color for each circle. Only the green and
		// blue values are modified; the red channel has a fixed value.
	        Wt::WPainterPath path;
		path.addEllipse(3 + col*25, 3 + row*25, 20, 20);
		pen.setColor(Wt::WColor(0, 255 - 42*row, 255 - 42*col));
		painter.strokePath(path, pen);
	    }
	}

	painter.translate(0, 160);
	// Transparency example with rectangles
	// Create a background composed of 4 stacked rectangles.
	painter.fillRect(0, 0, 150, 37.5, Wt::WBrush(Wt::WColor(Wt::StandardColor::Yellow)));
	painter.fillRect(0, 37.5, 150, 37.5, Wt::WBrush(Wt::WColor(Wt::StandardColor::Green)));
	painter.fillRect(0, 75, 150, 37.5, Wt::WBrush(Wt::WColor(Wt::StandardColor::Blue)));
	painter.fillRect(0, 112.5, 150, 37.5, Wt::WBrush(Wt::WColor(Wt::StandardColor::Red)));
	// On top of these draw semi transparent rectangles with increasing opacity
	for (int i = 0; i < 10; i++) {
	    Wt::WBrush brush = Wt::WBrush(Wt::WColor(255, 255, 255, 255/10*i));
	    for (int j = 0; j < 4; j++) {
	        Wt::WPainterPath path;
		path.addRect(5 + i*14, 5 + j*37.5, 14, 27.5);
		painter.fillPath(path, brush);
	    }
	}

	painter.translate(0, 160);
	// Transparency example with circles
	// Create a square composed of four different colored squares.
	painter.fillRect(0, 0, 75, 75, Wt::WBrush(Wt::WColor(Wt::StandardColor::Yellow)));
	painter.fillRect(75, 0, 75, 75, Wt::WBrush(Wt::WColor(Wt::StandardColor::Green)));
	painter.fillRect(0, 75, 75, 75, Wt::WBrush(Wt::WColor(Wt::StandardColor::Blue)));
	painter.fillRect(75, 75, 75, 75,Wt::WBrush(Wt::WColor(Wt::StandardColor::Red)));

	// On top of these draw a set of semi-transparant white circles with
	// increasing diameter. The final result is a radial gradient.
	for (int i = 1; i < 8; i++) {
	    Wt::WPainterPath path;
	    path.addEllipse(75 - i*10, 75 - i*10, i*20, i*20);
	    Wt::WBrush brush = Wt::WBrush(Wt::WColor(255, 255, 255, 50));
	    painter.fillPath(path, brush);
	}
	
	painter.translate(0, 170);
	// Gradient Brush example
	// Rectangle with a linear gradient from left top to right bottom
	painter.setPen(Wt::WPen(Wt::PenStyle::None));
	Wt::WGradient linGrad;
	linGrad.setLinearGradient(0, 0, 100, 150);
	linGrad.addColorStop(0, Wt::WColor(255, 0, 0, 255));
	linGrad.addColorStop(0.5, Wt::WColor(0, 0, 255, 255));
	linGrad.addColorStop(1, Wt::WColor(0, 255, 0, 255));
	Wt::WBrush linearGradientBrush(linGrad);
	painter.setBrush(linearGradientBrush);
	painter.drawRect(0, 0, 100, 150);

	// Circle with a radial gradient
	Wt::WGradient radGrad;
	radGrad.setRadialGradient(170, 100, 50, 130, 130);
	radGrad.addColorStop(0.2, Wt::WColor(255, 0, 0, 255));
	radGrad.addColorStop(0.9, Wt::WColor(0, 0, 255, 255));
	radGrad.addColorStop(1, Wt::WColor(0, 0, 255, 0));
	Wt::WBrush radialGradientBrush(radGrad);
	painter.setBrush(radialGradientBrush);
	painter.drawEllipse(120, 50, 100, 100);

	painter.translate(0, 170);
	// LineWidth example

	// You can use WPainter::drawLine() or WPainter::strokePath() to draw a
	// line. Using strokePath() you can draw thicker lines.

	// The line is centered on the path. In other words, the area that's drawn
	// extends to half the line width on either side of the path. Because
	// canvas coordinates do not directly reference pixels, you have to take
	// special care to obtain crisp horizontal and vertical lines.

	// All lines with an odd integer width thickness in the example below do
	// not appear crisp, because of the path's positioning.
	for (int i = 0; i < 11; i++) {
	    Wt::WPainterPath path;
	    path.moveTo(i*14, 0);
	    path.lineTo(i*14, 150);
	    pen = Wt::WPen();
	    pen.setWidth(i+1);
	    painter.strokePath(path, pen);
	}

	painter.translate(160, 0);

	// LineWidth example with crisp lines

	// To obtain a crisp line for an odd integer width thickness line you have
	// to be very precise in your path creation, e.g. a 1.0 width line will
	// extend half a unit to either side of the path.
	for (int i = 0; i < 11; i++) {
	    Wt::WPainterPath path;
	    if (i % 2 == 0) {
		path.moveTo(i*14-0.5, 0);
		path.lineTo(i*14-0.5, 150);
	    } else {
		path.moveTo(i*14, 0);
		path.lineTo(i*14, 150);
	    }

	    pen = Wt::WPen();
	    pen.setCapStyle(Wt::PenCapStyle::Flat);   // Now, all lines will have equal length.
	    pen.setWidth(i+1);
	    painter.strokePath(path, pen);
	}

	painter.translate(-160, 170);

	// PenCapStyle example
	// The PenCapStyle can be FlatCap, SquareCap or RoundCap.
	// Start with drawing guides:
	Wt::WPainterPath guidePath;
	guidePath.moveTo(0, 10);
	guidePath.lineTo(150,10);
	guidePath.moveTo(0,140);
	guidePath.lineTo(150,140);
	pen = Wt::WPen(Wt::WColor(Wt::StandardColor::Blue));
	painter.strokePath(guidePath, pen);

	// Draw lines with different cap styles
	// Create three parallel paths:
	std::vector<Wt::WPainterPath> paths;
	for (int i = 0; i < 3; i++) {
	    Wt::WPainterPath path;
	    path.moveTo(25+i*50, 10);
	    path.lineTo(25+i*50, 140);
	    paths.push_back(path);
	}

	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setCapStyle(Wt::PenCapStyle::Flat);
	painter.strokePath(paths[0], pen);

	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setCapStyle(Wt::PenCapStyle::Square);
	painter.strokePath(paths[1], pen);

	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setCapStyle(Wt::PenCapStyle::Round);
	painter.strokePath(paths[2], pen);

	painter.translate(0, 170);

	// PenJoinStyle example
	// The PenJoinStyle can be MiterJoin, BevelJoin or RoundJoin.
	// Create three parallel paths:

	paths.clear();
	for (int i = 0; i < 3; i++) {
	    Wt::WPainterPath path;
	    path.moveTo(15,  5+i*40);
	    path.lineTo(45, 45+i*40);
	    path.lineTo(75,  5+i*40);
	    path.lineTo(105,45+i*40);
	    path.lineTo(135, 5+i*40);
	    paths.push_back(path);
	}

	// Draw the first path with miter joins.
	// The connected segments are joined by extending their outside edges to
	// connect at a single point.
	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setJoinStyle(Wt::PenJoinStyle::Miter);
	painter.strokePath(paths[0], pen);

	// Draw the second path with bevel joins.
	// An additional triangular area is filled between the common endpoint of
	// connected segments and the separate outside rectangular corners of each
	// segment.
	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setJoinStyle(Wt::PenJoinStyle::Bevel);
	painter.strokePath(paths[1], pen);

	// Draw the third path with round joins.
	// The corners of the shape are rounded off by filling an aditonal sector
	// of disc centered at the common endpoint of connected segments. The
	// radius of the rounded corners is equal to the line width.
	pen = Wt::WPen();
	pen.setWidth(20);
	pen.setJoinStyle(Wt::PenJoinStyle::Round);
	painter.strokePath(paths[2], pen);
    }
};

SAMPLE_BEGIN(PaintingStyle)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<StyleWidget>();

SAMPLE_END(return std::move(container))
