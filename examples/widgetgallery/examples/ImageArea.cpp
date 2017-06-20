#include <Wt/WBreak.h>
#include <Wt/WCircleArea.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WPolygonArea.h>
#include <Wt/WRectArea.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>

SAMPLE_BEGIN(ImageArea)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WImage *image =
    container->addWidget(Wt::cpp14::make_unique<Wt::WImage>(Wt::WLink("pics/sintel_trailer.jpg")));
image->setAlternateText("Sintel trailer");

container->addWidget(Wt::cpp14::make_unique<Wt::WBreak>());
Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());

auto circlePtr = Wt::cpp14::make_unique<Wt::WCircleArea>(427, 149, 58);
auto circle = circlePtr.get();
circle->setToolTip("tree");
circle->setCursor(Wt::Cursor::Cross);
image->addArea(std::move(circlePtr));

auto rectPtr = Wt::cpp14::make_unique<Wt::WRectArea>(294, 226, 265, 41);
auto rect = rectPtr.get();
rect->setToolTip("title");
rect->setCursor(Wt::Cursor::Cross);
image->addArea(std::move(rectPtr));

auto polygonPtr = Wt::cpp14::make_unique<Wt::WPolygonArea>();
auto polygon = polygonPtr.get();
std::vector<Wt::WPoint> points = { Wt::WPoint(92,330), Wt::WPoint(66,261), Wt::WPoint(122,176),
                               Wt::WPoint(143,33), Wt::WPoint(164,33), Wt::WPoint(157,88),
                               Wt::WPoint(210,90), Wt::WPoint(263,264), Wt::WPoint(228,330),
                               Wt::WPoint(92,330) };
polygon->setPoints(points);
polygon->setToolTip("person");
polygon->setCursor(Wt::Cursor::Cross);
image->addArea(std::move(polygonPtr));

circle->clicked().connect([=] {
    out->setText("You clicked the tree.");
});

rect->clicked().connect([=] {
    out->setText("You clicked the title.");
});

polygon->clicked().connect([=] {
    out->setText("You clicked the person.");
});

image->mouseMoved().connect([=] (const Wt::WMouseEvent& e) {
    out->setText("You're pointing the background at "
                 "(" + std::to_string(e.widget().x) +
                 "," + std::to_string(e.widget().y) +
		 ").");
});


SAMPLE_END(return std::move(container))
