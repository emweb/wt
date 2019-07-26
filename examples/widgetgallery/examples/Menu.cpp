#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMenu.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(Menu)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

// Create a stack where the contents will be located.
auto contents = Wt::cpp14::make_unique<Wt::WStackedWidget>();

Wt::WMenu *menu = container->addNew<Wt::WMenu>(contents.get());
menu->setStyleClass("nav nav-pills nav-stacked");
menu->setWidth(150);

// Add menu items using the default lazy loading policy.
menu->addItem("Internal paths", Wt::cpp14::make_unique<Wt::WTextArea>("Internal paths contents"));
menu->addItem("Anchor", Wt::cpp14::make_unique<Wt::WTextArea>("Anchor contents"));
menu->addItem("Stacked widget", Wt::cpp14::make_unique<Wt::WTextArea>("Stacked widget contents"));
menu->addItem("Tab widget", Wt::cpp14::make_unique<Wt::WTextArea>("Tab widget contents"));
menu->addItem("Menu", Wt::cpp14::make_unique<Wt::WTextArea>("Menu contents"));

container->addWidget(std::move(contents));

SAMPLE_END(return std::move(container))
