#include <Wt/WBreak>
#include <Wt/WCircleArea>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WPolygonArea>
#include <Wt/WRectArea>
#include <Wt/WText>

SAMPLE_BEGIN(ImageArea)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WImage *image = new Wt::WImage(Wt::WLink("pics/sintel_trailer.jpg"),
				   container);
image->setAlternateText("Sintel trailer");

new Wt::WBreak(container);
Wt::WText *out = new Wt::WText(container);

Wt::WCircleArea *circle = new Wt::WCircleArea(427, 149, 58);
circle->setToolTip("tree");
circle->setCursor(Wt::CrossCursor);
image->addArea(circle);

Wt::WRectArea *rect = new Wt::WRectArea(294, 226, 265, 41);
rect->setToolTip("title");
rect->setCursor(Wt::CrossCursor);
image->addArea(rect);

Wt::WPolygonArea *polygon = new Wt::WPolygonArea();
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
polygon->setPoints(points);

// In C++11, the above block is one statement:
//polygon->setPoints({ {92,330}, {66,261}, {122,176}, {143,33}, {164,33},
//                    {157,88}, {210,90}, {263,264}, {228,330}, {92,330} });
polygon->setToolTip("person");
polygon->setCursor(Wt::CrossCursor);
image->addArea(polygon);

circle->clicked().connect(std::bind([=] () {
    out->setText("You clicked the tree.");
}));

rect->clicked().connect(std::bind([=] () {
    out->setText("You clicked the title.");
}));

polygon->clicked().connect(std::bind([=] () {
    out->setText("You clicked the person.");
}));

image->mouseMoved().connect(std::bind([=] (const Wt::WMouseEvent& e) {
    out->setText("You're pointing the background at "
		 "(" + boost::lexical_cast<std::string>(e.widget().x) +
		 "," + boost::lexical_cast<std::string>(e.widget().y) +
		 ").");
}, std::placeholders::_1));

SAMPLE_END(return container)
