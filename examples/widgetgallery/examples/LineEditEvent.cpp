#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WText>

SAMPLE_BEGIN(LineEditEvent)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WLineEdit *edit = new Wt::WLineEdit(container);
edit->setPlaceholderText("Edit me");

Wt::WText *out = new Wt::WText("", container);
out->addStyleClass("help-block");

edit->keyPressed().connect(std::bind([=] (const Wt::WKeyEvent& e) {
    out->setText("You pressed the '" + e.text() + "' key.");
}, std::placeholders::_1));

SAMPLE_END(return container)
