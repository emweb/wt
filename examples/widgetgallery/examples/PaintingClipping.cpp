#include <Wt/WBrush.h>
#include <Wt/WCalendar.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

#include <algorithm>
#include <cstdlib>

class ClippingWidget : public Wt::WPaintedWidget
{
public:
    ClippingWidget()
        : WPaintedWidget()
    {
	resize(310, 150);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);

	for (int i = 0; i < 2; i++) {   // Create two separate drawings.
	    painter.translate(i*160, 0);
	    // Draw the background
	    painter.fillRect(0, 0, 150, 150, Wt::WBrush(Wt::WColor(Wt::StandardColor::Black)));

	    // Create a path and fill it with blue.
	    Wt::WPainterPath path;
	    path.addEllipse(15, 15, 120, 120);
	    painter.fillPath(path, Wt::WBrush(Wt::WColor(Wt::StandardColor::Blue)));

	    // Use the previously defined path also for clipping.
	    painter.setClipPath(path);
	    painter.setClipping(i != 0); // Clipping is applied from the 2nd drawing.

	    drawStars(painter);
	}
  }

private:
    void drawStar(Wt::WPainter& painter, double radius) {
	painter.save();
	Wt::WPainterPath circlePath;
	circlePath.addEllipse(0, 0, radius, radius);
	circlePath.closeSubPath();
	painter.fillPath(circlePath, Wt::WBrush(Wt::WColor(Wt::StandardColor::White)));
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

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

container->addNew<ClippingWidget>();

SAMPLE_END(return std::move(container))
