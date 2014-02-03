#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WTextArea>

SAMPLE_BEGIN(Menu)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

// Create a stack where the contents will be located.
Wt::WStackedWidget *contents = new Wt::WStackedWidget();

Wt::WMenu *menu = new Wt::WMenu(contents, Wt::Vertical, container);
menu->setStyleClass("nav nav-pills nav-stacked");
menu->setWidth(150);

// Add menu items using the default lazy loading policy.
menu->addItem("Internal paths", new Wt::WTextArea("Internal paths contents"));
menu->addItem("Anchor", new Wt::WTextArea("Anchor contents"));
menu->addItem("Stacked widget", new Wt::WTextArea("Stacked widget contents"));
menu->addItem("Tab widget", new Wt::WTextArea("Tab widget contents"));
menu->addItem("Menu", new Wt::WTextArea("Menu contents"));

container->addWidget(contents);

SAMPLE_END(return container)
