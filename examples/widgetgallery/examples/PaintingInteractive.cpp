#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WSlider.h>
#include <Wt/WSpinBox.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PaintingInteractiveWidget : public Wt::WPaintedWidget
{
public:
    PaintingInteractiveWidget()
        : WPaintedWidget(),
	  rotateSlot(1, this)
    {
	resize(300, 300);  // Provide a default size.

	transform = createJSTransform(); // Create a client side transform

	rotateSlot.setJavaScript(
	    "function(o,e,deg) {"
		"if (" + objJsRef() + ") {"
		    "var rad = deg / 180 * Math.PI;"
		    "var c = Math.cos(rad);"
		    "var s = Math.sin(rad);" +
		    // You can assign a new 6 element array to a transform,
		    // or change its individual elements.
		    transform.jsRef() + " = [c,-s,s,c,0,0];" +
		    repaintSlot().execJs() + ";"
		"}"
	    "}");
    }

    void rotate(int degrees)
    {
	double radians = degrees / 180.0 * M_PI;
	double c = std::cos(radians);
	double s = std::sin(radians);
	// Changes in value will be synced from server to client
	transform.setValue(Wt::WTransform(c, -s, s, c, 0, 0));
	update();
    }

    Wt::JSlot rotateSlot;

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);

	painter.translate(150, 150);

	// Set a 5px wide pen
	Wt::WPen pen;
	pen.setWidth(5);
	painter.setPen(pen);

	// Draw an arrow shape
	Wt::WPainterPath path;
	path.moveTo(-50,100);
	path.lineTo(50,100);
	path.lineTo(50,20);
	path.lineTo(100,20);
	path.lineTo(0,-100);
	path.lineTo(-100,20);
	path.lineTo(-50, 20);
	path.lineTo(-50,100);
	path.lineTo(50, 100);

	// Client side transforms can be set with
	// setWorldTransform, or applied to a path
	// with the map() function. In the latter case,
	// line thickness will not be affected by the
	// transform.
	Wt::WPainterPath transformedPath =
	    transform.value().map(path);
	painter.drawPath(transformedPath);
    }

private:
    Wt::WJavaScriptHandle<Wt::WTransform> transform;
};

SAMPLE_BEGIN(PaintingInteractive)
auto container = std::make_unique<Wt::WContainerWidget>();

PaintingInteractiveWidget *widget = container->addNew<PaintingInteractiveWidget>();

Wt::WSpinBox *sb = container->addNew<Wt::WSpinBox>();
sb->setWidth(300);
sb->setRange(0, 360);
sb->setValue(0);

Wt::WSlider *slider = container->addNew<Wt::WSlider>(Wt::Orientation::Horizontal);
slider->resize(300, 50);
slider->setRange(0, 360);

// This will not cause a server roundtrip
slider->sliderMoved().connect(widget->rotateSlot);

sb->valueChanged().connect([=] {
    slider->setValue(sb->value());
    widget->rotate(sb->value());
});
sb->enterPressed().connect([=] {
    slider->setValue(sb->value());
    widget->rotate(sb->value());
});

slider->valueChanged().connect([=] {
    sb->setValue(slider->value());
    widget->rotate(slider->value());
});

SAMPLE_END(return std::move(container))
