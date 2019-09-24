#include <Wt/WCssDecorationStyle.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEvent.h>
#include <Wt/WPainter.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WPushButton.h>
#include <Wt/WRectF.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WToolBar.h>

class PaintBrush : public Wt::WPaintedWidget
{
public:
    PaintBrush(int width, int height)
        : WPaintedWidget()
    {
	resize(width, height);

	decorationStyle().setCursor("icons/pencil.cur", Wt::Cursor::Cross);

	mouseDragged().connect(this, &PaintBrush::mouseDrag);
	mouseWentDown().connect(this, &PaintBrush::mouseDown);
	touchStarted().connect(this, &PaintBrush::touchStart);
	touchMoved().connect(this, &PaintBrush::touchMove);
	touchMoved().preventDefaultAction();
  
        color_ = Wt::WColor(Wt::StandardColor::Black);
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
        painter.setRenderHint(Wt::RenderHint::Antialiasing);
  
        Wt::WPen pen;
	pen.setWidth(3);
	pen.setColor(color_);
	pen.setCapStyle(Wt::PenCapStyle::Flat);
	pen.setJoinStyle(Wt::PenJoinStyle::Miter);
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
	update(Wt::PaintFlag::Update);
    }

    void touchStart(const Wt::WTouchEvent& e) {
        Wt::Coordinates c = e.touches()[0].widget();
        path_ = Wt::WPainterPath(Wt::WPointF(c.x, c.y));
    }

    void touchMove(const Wt::WTouchEvent& e) {
        Wt::Coordinates c = e.touches()[0].widget();
	path_.lineTo(c.x, c.y);
	update(Wt::PaintFlag::Update);
    }
};

namespace {

extern 
Wt::WPushButton *createColorToggle(const char *className, const Wt::WColor& color,
				   PaintBrush *canvas)
{
    auto button = new Wt::WPushButton();
    button->setTextFormat(Wt::TextFormat::XHTML);
    button->setText("&nbsp;");
    button->setCheckable(true);
    button->addStyleClass(className);
    button->setWidth(30);
    button->checked().connect([=] {
	canvas->setColor(color);
    });

    return button;
}

}

SAMPLE_BEGIN(Paintbrush)

/* Approximate bootstrap standard colors */
const Wt::WColor blue(0, 110, 204);                     // btn-primary
const Wt::WColor red(218, 81, 76);                      // btn-danger
const Wt::WColor green(59, 195, 95);                    // btn-success
const Wt::WColor orange(250, 168, 52);                  // btn-warning
const Wt::WColor black = Wt::WColor(Wt::StandardColor::Black);  // btn-inverse
const Wt::WColor gray(210, 210, 210);                   // (default)

auto result = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto canvas = Wt::cpp14::make_unique<PaintBrush>(710, 400);
auto canvas_ = canvas.get();
canvas->setColor(blue);
canvas->decorationStyle().setBorder
    (Wt::WBorder(Wt::BorderStyle::Solid, Wt::BorderWidth::Medium, black));

#ifndef WT_TARGET_JAVA
std::vector<Wt::WPushButton *> colorButtons {
  createColorToggle("btn-primary", blue, canvas.get()),
  createColorToggle("btn-danger", red, canvas.get()),
  createColorToggle("btn-success", green, canvas.get()),
  createColorToggle("btn-warning", orange, canvas.get()),
  createColorToggle("btn-inverse", black, canvas.get()),
  createColorToggle("" /* default */, gray, canvas.get())
};
#else // WT_TARGET_JAVA
std::vector<Wt::WPushButton *> colorButtons;
colorButtons.push_back(createColorToggle("btn-primary", blue, canvas.get()));
colorButtons.push_back(createColorToggle("btn-danger", red, canvas.get()));
colorButtons.push_back(createColorToggle("btn-success", green, canvas.get()));
colorButtons.push_back(createColorToggle("btn-warning", orange, canvas.get()));
colorButtons.push_back(createColorToggle("btn-inverse", black, canvas.get()));
colorButtons.push_back(createColorToggle("" /* default */, gray, canvas.get()));
#endif // WT_TARGET_JAVA

auto toolBar = Wt::cpp14::make_unique<Wt::WToolBar>();

for (unsigned i = 0; i < colorButtons.size(); ++i) {
    Wt::WPushButton *button = colorButtons[i];
    button->setChecked(i == 0);
    toolBar->addButton(std::unique_ptr<Wt::WPushButton>(button));

    // Implement a radio button group
        for (unsigned j = 0; j < colorButtons.size(); ++j) {
            if (i != j) {
                Wt::WPushButton * const other = colorButtons[j];
                button->checked().connect(other, &Wt::WPushButton::setUnChecked);
            }
        }
}

auto clearButton = Wt::cpp14::make_unique<Wt::WPushButton>("Clear");

clearButton->clicked().connect([=] {
    canvas_->clear();
});

toolBar->addSeparator();
toolBar->addButton(std::move(clearButton));

#ifndef WT_TARGET_JAVA
result->addWidget(std::move(toolBar));
result->addWidget(std::move(canvas));
#else // WT_TARGET_JAVA
result->addWidget(std::unique_ptr<Wt::WWidget>(toolBar));
result->addWidget(std::unique_ptr<Wt::WWidget>(canvas));
#endif // WT_TARGET_JAVA

SAMPLE_END(return std::move(result))
