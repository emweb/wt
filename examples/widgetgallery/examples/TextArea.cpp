#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WDateTime>

SAMPLE_BEGIN(TextArea)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WTextArea *ta = new Wt::WTextArea(container);
ta->setColumns(80);
ta->setRows(5);
ta->setText("Change this text... \n"
            "and click outside the text area to get a changed event.");

Wt::WText *out = new Wt::WText("<p></p>", container);
out->addStyleClass("help-block");

ta->changed().connect(std::bind([=] () {
    out->setText("<p>Text area changed at " +
                 Wt::WDateTime::currentDateTime().toString() + ".</p>");
}));

SAMPLE_END(return container)
