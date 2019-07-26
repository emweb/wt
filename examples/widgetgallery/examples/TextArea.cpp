#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WDateTime.h>

SAMPLE_BEGIN(TextArea)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WTextArea *ta =
    container->addNew<Wt::WTextArea>();
ta->setColumns(80);
ta->setRows(5);
ta->setText("Change this text... \n"
            "and click outside the text area to get a changed event.");

Wt::WText *out = container->addNew<Wt::WText>("<p></p>");
out->addStyleClass("help-block");

ta->changed().connect([=] {
    out->setText("<p>Text area changed at " +
                 Wt::WDateTime::currentDateTime().toString() + ".</p>");
});

SAMPLE_END(return std::move(container))
