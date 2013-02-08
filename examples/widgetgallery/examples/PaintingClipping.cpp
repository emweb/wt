#include <Wt/WBrush>
#include <Wt/WCalendar>
#include <Wt/WColor>
#include <Wt/WContainerWidget>
#include <Wt/WPaintDevice>
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>

#include <cstdlib>

class ClippingWidget : public Wt::WPaintedWidget
{
public:
    ClippingWidget(Wt::WContainerWidget *parent = 0)
	: Wt::WPaintedWidget(parent)
    {
	resize(310, 150);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
	Wt::WPainter painter(paintDevice);

	for (int i = 0; i < 2; i++) {   // Create two separate drawings.
	    painter.translate(i*160, 0);
	    // Draw the background
	    painter.fillRect(0, 0, 150, 150, Wt::WBrush(Wt::WColor(Wt::black)));

	    // Create a path and fill it with blue.
	    Wt::WPainterPath path = Wt::WPainterPath();
	    path.addEllipse(15, 15, 120, 120);
	    painter.fillPath(path, Wt::WBrush(Wt::WColor(Wt::blue)));

	    // Use the previously defined path also for clipping.
	    painter.setClipPath(path);
	    painter.setClipping(i != 0); // Clipping is applied from the 2nd drawing.

	    drawStars(painter);
	}
  }

private:
    void drawStar(Wt::WPainter& painter, double radius) {
	painter.save();
	Wt::WPainterPath circlePath = Wt::WPainterPath();
	circlePath.addEllipse(0, 0, radius, radius);
	circlePath.closeSubPath();
	painter.fillPath(circlePath, Wt::WBrush(Wt::WColor(Wt::white)));
	painter.restore();
    }

    void drawStars(Wt::WPainter& painter) {
	std::srand(Wt::WDateTime::currentDateTime().toTime_t());
	painter.save();
	painter.translate(75,75);
	for (int star = 1; star < 50; star++){
	    painter.save();
	    painter.translate(75 - std::rand() % 150 + 1,
			      75 - std::rand() % 150 + 1);
	    drawStar(painter, std::max(0, std::rand() % 4) + 2);
	    painter.restore();
	}
	painter.restore();
    }
};

SAMPLE_BEGIN(PaintingClipping)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new ClippingWidget(container);

SAMPLE_END(return container)
