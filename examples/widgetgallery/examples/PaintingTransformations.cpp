#include <Wt/WBrush>
#include <Wt/WColor>
#include <Wt/WContainerWidget>
#include <Wt/WPaintDevice>
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>

class TransformationsWidget : public Wt::WPaintedWidget
{
public:
    TransformationsWidget(Wt::WContainerWidget *parent = 0)
	: Wt::WPaintedWidget(parent)
    {
	resize(300, 500);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
	Wt::WPainter painter(paintDevice);
	painter.setPen(Wt::red);
	painter.setBrush(Wt::black);

	// SAVE AND RESTORE CANVAS STATE EXAMPLE
	painter.save();
	painter.setPen(Wt::white);
	// Draw and fill a rectangle with the current brush.
	painter.drawRect(0,0,100,100);
	painter.save();                 // Save the canvas state on a stack.
	painter.setBrush(Wt::yellow);   // Change the fill style.
	// Draw a rectangle with the current settings.
	painter.drawRect(10,10,80,80);
	painter.save();                 // Save the current canvas state.
	painter.setBrush(Wt::red);      // Change the fill style.
	painter.drawRect(20,20,60,60);
	painter.restore();              // Restore the previous canvas state.
	painter.drawRect(30,30,40,40);
	painter.restore();              // Restore the original canvas state.
	painter.drawRect(40,40,20,20);
	painter.restore();              // Restore the original pen.

	// TRANSLATING
	for (int i = 0; i < 2; i++) {
	    painter.save();
	    painter.translate(i*100, 130);
	    drawFilledPolygon(painter, Wt::WColor(0,255,0, 255 - i*200));
	    painter.restore();
	}

	// ROTATING
	painter.translate(0, 300);  // Translate the origin for a Y-offset of rings
	painter.save();
	painter.translate(90, 0);   // Translate the origin for a X-offset of rings
	for (int ring = 1; ring < 6; ring++) {  // Loop through rings
	    painter.save();
	    painter.setBrush(Wt::WBrush(Wt::WColor(51*ring, (255-51*ring), 255)));

	    for (int j = 0; j < ring * 6; j++) {           // Draw individual dots:
		painter.rotate(360 / (ring*6));            // 1. Rotate full circle in ring*6 steps
		painter.drawEllipse(0, ring*12.5, 10, 10); // 2. Draw a dot.
	    }
	    painter.restore();
	}
	painter.restore();          // Reset the X-offset.

	// SCALING
	painter.save();
	painter.translate(0,100);
	drawFilledPolygon(painter, Wt::WColor(0,255,0, 255));   // no scaling.
	painter.translate(100,0);
	painter.scale(1.2, 1);                                  // with X scaling
	drawFilledPolygon(painter, Wt::WColor(0,255,0, 55));
	painter.restore();
  }

private:
    void drawFilledPolygon(Wt::WPainter &painter, const Wt::WColor& color) {
	painter.setBrush(color);
	const Wt::WPointF points[] 
	    = { Wt::WPointF(20, 0), Wt::WPointF(60, 0),
		Wt::WPointF(80, 34.6), Wt::WPointF(60, 69.2),
		Wt::WPointF(20, 69.2), Wt::WPointF(0, 34.6),
		Wt::WPointF(20, 0) };
	painter.drawPolygon(points, 6);
    }
};

SAMPLE_BEGIN(PaintingTransformations)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new TransformationsWidget(container);

SAMPLE_END(return container)
