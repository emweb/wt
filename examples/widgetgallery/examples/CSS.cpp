#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>

SAMPLE_BEGIN(CSS)
// Add an external style sheet to the application.
Wt::WApplication::instance()->useStyleSheet("style/CSSexample.css");

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
// The style sheet should be applied to this container only.
// The class .CSS-example is used as selector.
container->setStyleClass("CSS-example");

Wt::WPushButton *allB = container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Set all classes"));

Wt::WPushButton *removeB = container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Remove info class"));
removeB->setMargin(10, Wt::Side::Left | Wt::Side::Right);
removeB->disable();

Wt::WPushButton *toggleB = container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Toggle condensed"));
toggleB->disable();

Wt::WText *text = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());
text->setText("<p>These are the most import API classes and methods for"
              " working with CSS:</p>");

Wt::WTable *table = container->addWidget(Wt::cpp14::make_unique<Wt::WTable>());
table->setHeaderCount(1);
table->elementAt(0, 0)->addWidget(Wt::cpp14::make_unique<Wt::WText>("Method"));
table->elementAt(0, 1)->addWidget(Wt::cpp14::make_unique<Wt::WText>("Description"));
table->elementAt(1, 0)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("WApplication::useStyleSheet()"));
table->elementAt(1, 1)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("Adds an external style sheet"));
table->elementAt(2, 0)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("WWidget::setStyleClass()"));
table->elementAt(2, 1)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("Sets (one or more) CSS style classes"));
table->elementAt(3, 0)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("WWidget::removeStyleClass()"));
table->elementAt(3, 1)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("Removes a CSS style class"));
table->elementAt(4, 0)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("WWidget::toggleStyleClass()"));
table->elementAt(4, 1)->addWidget(
                        Wt::cpp14::make_unique<Wt::WText>("Toggles a CSS style class"));

allB->clicked().connect([=] {
    // Set style classes for the complete table.
    table->setStyleClass("table table-bordered");
    // Set the info style class for the first row after the header.
    table->rowAt(1)->setStyleClass("info");
    // Set a style class for the methods (first column, the header excluded).
    for (int i=1; i<table->rowCount(); i++)
        table->elementAt(i,0)->setStyleClass("code");
    removeB->enable();
    toggleB->enable();
});

removeB->clicked().connect([=] {
    table->rowAt(1)->removeStyleClass("info");
    removeB->disable();
});

toggleB->clicked().connect([=] {
    if (toggleB->text() == "Toggle condensed") {
        table->toggleStyleClass("table-condensed", true);
        toggleB->setText("Toggle expanded");
    } else {
        table->toggleStyleClass("table-condensed", false);
        toggleB->setText("Toggle condensed");
    }
});

SAMPLE_END(return std::move(container))
