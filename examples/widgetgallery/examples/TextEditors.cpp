#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextEdit.h>

SAMPLE_BEGIN(TextEditors)
auto container = cpp14::make_unique<WContainerWidget>();

WLineEdit *le =
    container->addWidget(cpp14::make_unique<WLineEdit>());
le->setEmptyText("Edit me");

WLineEdit *out =
    container->addWidget(cpp14::make_unique<WLineEdit>());
out->setReadOnly(true);

le->keyWentUp().connect([=] {
    out->setText("Line edit: key up event");
});

le->enterPressed().connect([=] {
    out->setText("Line edit: enter pressed event");
});

SAMPLE_END(return container)
