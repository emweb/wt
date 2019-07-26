#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(LineEditEvent)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WLineEdit *edit = container->addNew<Wt::WLineEdit>();
edit->setPlaceholderText("Edit me");

Wt::WText *out = container->addNew<Wt::WText>("");
out->addStyleClass("help-block");

edit->keyPressed().connect([=] (const Wt::WKeyEvent& e) {
    out->setText("You pressed the '" + e.text() + "' key.");
});

SAMPLE_END(return std::move(container))
