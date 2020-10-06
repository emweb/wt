#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WSpinBox.h>

class MyPaintedWidget : public Wt::WPaintedWidget
{
public:
    MyPaintedWidget()
        : WPaintedWidget(), end_(100)
    {
	resize(200, 60);   // Provide a default size.
    }

    void setEnd(int end) {
	end_ = end;
	update();          // Trigger a repaint.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);
        painter.setBrush(Wt::WBrush(Wt::WColor(Wt::StandardColor::Blue)));
        painter.drawRect(0, 0, end_, 50);
    }

private:
    int end_;
};

SAMPLE_BEGIN(PaintingEvent)
auto container = std::make_unique<Wt::WContainerWidget>();

MyPaintedWidget *painting = container->addNew<MyPaintedWidget>();

Wt::WSpinBox *sb = container->addNew<Wt::WSpinBox>();
sb->setRange(10,200);
sb->setValue(100);

sb->changed().connect([=] {
    painting->setEnd(sb->value());
});

SAMPLE_END(return std::move(container))
