#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WTextEdit>

SAMPLE_BEGIN(TextEditors)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WLineEdit *le = new Wt::WLineEdit(container);
le->setEmptyText("Edit me");

Wt::WLineEdit *out = new Wt::WLineEdit(container);
out->setReadOnly(true);

le->keyWentUp().connect(std::bind([=] () {
    out->setText("Line edit: key up event");
}));

le->enterPressed().connect(std::bind([=] () {
    out->setText("Line edit: enter pressed event");
}));

SAMPLE_END(return container)
