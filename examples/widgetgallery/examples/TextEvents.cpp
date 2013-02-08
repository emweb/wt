#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(TextEvents)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

// Create four text widgets.
Wt::WText *text1 =
    new Wt::WText("This text reacts to <tt>clicked()</tt>", container);
text1->setStyleClass("reactive");

Wt::WText *text2 =
    new Wt::WText("This text reacts to <tt>doubleClicked()</tt>",
                  container);
text2->setStyleClass("reactive");

Wt::WText *text3 =
    new Wt::WText("This text reacts to <tt>mouseWentOver()</tt>",
                  container);
text3->setStyleClass("reactive");

Wt::WText *text4 =
    new Wt::WText("This text reacts to <tt>mouseWentOut()</tt>",
                  container);
text4->setStyleClass("reactive");

// Create an additional text control to show status messages.
Wt::WText *out = new Wt::WText(container);

// Assign a signal/slot mechanism to the text controls.
text1->clicked().connect(std::bind([=] () {
    out->setText("<p>Text was clicked.</p>");
}));

text2->doubleClicked().connect(std::bind([=] () {
    out->setText("<p>Text was double clicked.</p>");
}));

text3->mouseWentOver().connect(std::bind([=] () {
    out->setText("<p>Mouse went over text.</p>");
}));

text4->mouseWentOut().connect(std::bind([=] () {
    out->setText("<p>Mouse went out text.</p>");
}));

SAMPLE_END(return container)
