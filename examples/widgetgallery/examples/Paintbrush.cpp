#include <Wt/WCssDecorationStyle>
#include <Wt/WColor>
#include <Wt/WContainerWidget>
#include <Wt/WEvent>
#include <Wt/WPainter>
#include <Wt/WPaintedWidget>
#include <Wt/WPainterPath>
#include <Wt/WPen>
#include <Wt/WPointF>
#include <Wt/WPushButton>
#include <Wt/WRectF>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WToolBar>

class PaintBrush : public Wt::WPaintedWidget
{
public:
    PaintBrush(int width, int height, Wt::WContainerWidget *parent = 0)
	: Wt::WPaintedWidget(parent)
    {
	resize(width, height);

	decorationStyle().setCursor("icons/pencil.cur", Wt::CrossCursor);

	mouseDragged().connect(this, &PaintBrush::mouseDrag);
	mouseWentDown().connect(this, &PaintBrush::mouseDown);
	touchStarted().connect(this, &PaintBrush::touchStart);
	touchMoved().connect(this, &PaintBrush::touchMove);
	touchMoved().preventDefaultAction();
  
	color_ = Wt::black;
    }

    void clear() {
	update();
    }

    void setColor(const Wt::WColor& c) {
	color_ = c;
    }

protected:
    virtual void paintEvent(Wt::WPaintDevice *paintDevice) {
	Wt::WPainter painter(paintDevice);
	painter.setRenderHint(Wt::WPainter::Antialiasing);
  
	Wt::WPen pen;
	pen.setWidth(3);
	pen.setColor(color_);
	pen.setCapStyle(Wt::FlatCap);
	pen.setJoinStyle(Wt::MiterJoin);
	painter.setPen(pen);
	painter.drawPath(path_);

	path_ = Wt::WPainterPath(path_.currentPosition());
    }

private:
    Wt::WPainterPath path_;
    Wt::WColor color_;

    void mouseDown(const Wt::WMouseEvent& e) {
	Wt::Coordinates c = e.widget();
	path_ = Wt::WPainterPath(Wt::WPointF(c.x, c.y));
    }

    void mouseDrag(const Wt::WMouseEvent& e) {
	Wt::Coordinates c = e.widget();
	path_.lineTo(c.x, c.y);
	update(Wt::PaintUpdate);
    }

    void touchStart(const Wt::WTouchEvent& e) {
	Wt::Coordinates c = e.touches()[0].widget();
	path_ = Wt::WPainterPath(Wt::WPointF(c.x, c.y));
    }

    void touchMove(const Wt::WTouchEvent& e) {
	Wt::Coordinates c = e.touches()[0].widget();
	path_.lineTo(c.x, c.y);
	update(Wt::PaintUpdate);
    }
};

namespace {

extern 
Wt::WPushButton *createColorToggle(const char *className, const Wt::WColor& color,
				   PaintBrush *canvas)
{
    Wt::WPushButton *button = new Wt::WPushButton();
    button->setTextFormat(Wt::XHTMLText);
    button->setText("&nbsp;");
    button->setCheckable(true);
    button->addStyleClass(className);
    button->setWidth(30);
    button->checked().connect(std::bind([=] () {
	canvas->setColor(color);
    }));

    return button;
}

}

SAMPLE_BEGIN(Paintbrush)

/* Approximate bootstrap standard colors */
const Wt::WColor blue(0, 110, 204);    // btn-primary
const Wt::WColor red(218, 81, 76);     // btn-danger
const Wt::WColor green(59, 195, 95);   // btn-success
const Wt::WColor orange(250, 168, 52); // btn-warning
const Wt::WColor black = Wt::black;    // btn-inverse
const Wt::WColor gray(210, 210, 210);  // (default)

Wt::WContainerWidget* result = new Wt::WContainerWidget();

PaintBrush *canvas = new PaintBrush(710, 400);
canvas->setColor(blue);
canvas->decorationStyle().setBorder
    (Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium, Wt::black));

std::vector<Wt::WPushButton *> colorButtons;
colorButtons.push_back(createColorToggle("btn-primary", blue, canvas));
colorButtons.push_back(createColorToggle("btn-danger", red, canvas));
colorButtons.push_back(createColorToggle("btn-success", green, canvas));
colorButtons.push_back(createColorToggle("btn-warning", orange, canvas));
colorButtons.push_back(createColorToggle("btn-inverse", black, canvas));
colorButtons.push_back(createColorToggle("" /* default */, gray, canvas));

Wt::WToolBar *toolBar = new Wt::WToolBar();

for (unsigned i = 0; i < colorButtons.size(); ++i) {
    Wt::WPushButton *button = colorButtons[i];
    button->setChecked(i == 0);
    toolBar->addButton(button);

    // Implement a radio button group
    for (unsigned j = 0; j < colorButtons.size(); ++j) {
	if (i != j) {
	    Wt::WPushButton * const other = colorButtons[j];
	    button->checked().connect(other, &Wt::WPushButton::setUnChecked);
	}
    }
}

Wt::WPushButton *clearButton = new Wt::WPushButton("Clear");

clearButton->clicked().connect(std::bind([=] () {
    canvas->clear();
}));

toolBar->addSeparator();
toolBar->addButton(clearButton);

result->addWidget(toolBar);
result->addWidget(canvas);

SAMPLE_END(return result)
