#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(TextEvents)
auto container = std::make_unique<Wt::WContainerWidget>();

// Create four text widgets.
Wt::WText *text1 =
    container->addNew<Wt::WText>("This text reacts to <tt>clicked()</tt>");
text1->setStyleClass("reactive");

Wt::WText *text2 =
    container->addNew<Wt::WText>("This text reacts to <tt>doubleClicked()</tt>");
text2->setStyleClass("reactive");

Wt::WText *text3 =
    container->addNew<Wt::WText>("This text reacts to <tt>mouseWentOver()</tt>");
text3->setStyleClass("reactive");

Wt::WText *text4 =
    container->addNew<Wt::WText>("This text reacts to <tt>mouseWentOut()</tt>");
text4->setStyleClass("reactive");

// Create an additional text control to show status messages.
Wt::WText *out = container->addNew<Wt::WText>();

// Assign a signal/slot mechanism to the text controls.
text1->clicked().connect([=] {
    out->setText("<p>Text was clicked.</p>");
});

text2->doubleClicked().connect([=] {
    out->setText("<p>Text was double clicked.</p>");
});

text3->mouseWentOver().connect([=] {
    out->setText("<p>Mouse went over text.</p>");
});

text4->mouseWentOut().connect([=] {
    out->setText("<p>Mouse went out text.</p>");
});

SAMPLE_END(return std::move(container))
