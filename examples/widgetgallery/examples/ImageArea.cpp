#include <Wt/WBreak.h>
#include <Wt/WCircleArea.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WPolygonArea.h>
#include <Wt/WRectArea.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>

SAMPLE_BEGIN(ImageArea)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WImage *image = container->addNew<Wt::WImage>(Wt::WLink("pics/sintel_trailer.jpg"));
image->setAlternateText("Sintel trailer");

container->addNew<Wt::WBreak>();
Wt::WText *out = container->addNew<Wt::WText>();

auto circlePtr = std::make_unique<Wt::WCircleArea>(427, 149, 58);
auto circle = circlePtr.get();
circle->setToolTip("tree");
circle->setCursor(Wt::Cursor::Cross);
image->addArea(std::move(circlePtr));

auto rectPtr = std::make_unique<Wt::WRectArea>(294, 226, 265, 41);
auto rect = rectPtr.get();
rect->setToolTip("title");
rect->setCursor(Wt::Cursor::Cross);
image->addArea(std::move(rectPtr));

auto polygonPtr = std::make_unique<Wt::WPolygonArea>();
auto polygon = polygonPtr.get();
#ifndef WT_TARGET_JAVA
std::vector<Wt::WPoint> points = { Wt::WPoint(92,330), Wt::WPoint(66,261), Wt::WPoint(122,176),
                               Wt::WPoint(143,33), Wt::WPoint(164,33), Wt::WPoint(157,88),
                               Wt::WPoint(210,90), Wt::WPoint(263,264), Wt::WPoint(228,330),
                               Wt::WPoint(92,330) };
#else // WT_TARGET_JAVA
std::vector<Wt::WPoint> points;
points.push_back(Wt::WPoint(92,330));
points.push_back(Wt::WPoint(66,261));
points.push_back(Wt::WPoint(122,176));
points.push_back(Wt::WPoint(143,33));
points.push_back(Wt::WPoint(164,33));
points.push_back(Wt::WPoint(157,88));
points.push_back(Wt::WPoint(210,90));
points.push_back(Wt::WPoint(263,264));
points.push_back(Wt::WPoint(228,330));
points.push_back(Wt::WPoint(92,330));
#endif // WT_TARGET_JAVA
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
