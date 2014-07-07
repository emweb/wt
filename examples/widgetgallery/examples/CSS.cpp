#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTable>

SAMPLE_BEGIN(CSS)
// Add an external style sheet to the application.
Wt::WApplication::instance()->useStyleSheet("style/CSSexample.css");

Wt::WContainerWidget *container = new Wt::WContainerWidget();
// The style sheet should be applied to this container only.
// The class .CSS-example is used as selector.
container->setStyleClass("CSS-example");

Wt::WPushButton *allB = new Wt::WPushButton("Set all classes", container);

Wt::WPushButton *removeB = new Wt::WPushButton("Remove info class", container);
removeB->setMargin(10, Wt::Left | Wt::Right);
removeB->disable();

Wt::WPushButton *toggleB = new Wt::WPushButton("Toggle condensed", container);
toggleB->disable();

Wt::WText *text = new Wt::WText(container);
text->setText("<p>These are the most import API classes and methods for"
              " working with CSS:</p>");

Wt::WTable *table = new Wt::WTable(container);
table->setHeaderCount(1);
table->elementAt(0, 0)->addWidget(new Wt::WText("Method"));
table->elementAt(0, 1)->addWidget(new Wt::WText("Description"));
table->elementAt(1, 0)->addWidget(
                        new Wt::WText("WApplication::useStyleSheet()"));
table->elementAt(1, 1)->addWidget(
                        new Wt::WText("Adds an external style sheet"));
table->elementAt(2, 0)->addWidget(
                        new Wt::WText("WWidget::setStyleClass()"));
table->elementAt(2, 1)->addWidget(
                        new Wt::WText("Sets (one or more) CSS style classes"));
table->elementAt(3, 0)->addWidget(
                        new Wt::WText("WWidget::removeStyleClass()"));
table->elementAt(3, 1)->addWidget(
                        new Wt::WText("Removes a CSS style class"));
table->elementAt(4, 0)->addWidget(
                        new Wt::WText("WWidget::toggleStyleClass()"));
table->elementAt(4, 1)->addWidget(
                        new Wt::WText("Toggles a CSS style class"));

allB->clicked().connect(std::bind([=] () {
    // Set style classes for the complete table.
    table->setStyleClass("table table-bordered");
    // Set the info style class for the first row after the header.
    table->rowAt(1)->setStyleClass("info");
    // Set a style class for the methods (first column, the header excluded).
    for (int i=1; i<table->rowCount(); i++)
        table->elementAt(i,0)->setStyleClass("code");
    removeB->enable();
    toggleB->enable();
}));

removeB->clicked().connect(std::bind([=] () {
    table->rowAt(1)->removeStyleClass("info");
    removeB->disable();
}));

toggleB->clicked().connect(std::bind([=] () {
    if (toggleB->text() == "Toggle condensed") {
        table->toggleStyleClass("table-condensed", true);
        toggleB->setText("Toggle expanded");
    } else {
        table->toggleStyleClass("table-condensed", false);
        toggleB->setText("Toggle condensed");
    }
}));

SAMPLE_END(return container)
