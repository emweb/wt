#include <Wt/WContainerWidget>
#include <Wt/WPaintDevice>
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>
#include <Wt/WSpinBox>

class MyPaintedWidget : public Wt::WPaintedWidget
{
public:
    MyPaintedWidget(Wt::WContainerWidget *parent = 0)
	: Wt::WPaintedWidget(parent), end_(100)
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
	painter.setBrush(Wt::WBrush(Wt::WBrush(Wt::blue)));
	painter.drawRect(0, 0 ,end_, 50);
    }

private:
    int end_;
};

SAMPLE_BEGIN(PaintingEvent)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

MyPaintedWidget *painting = new MyPaintedWidget(container);

Wt::WSpinBox *sb = new Wt::WSpinBox(container);
sb->setRange(10,200);
sb->setValue(100);

sb->changed().connect(std::bind([=] () {
    painting->setEnd(sb->value());
}));

SAMPLE_END(return container)
