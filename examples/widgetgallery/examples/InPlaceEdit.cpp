#include <Wt/WContainerWidget>
#include <Wt/WInPlaceEdit>
#include <Wt/WText>

SAMPLE_BEGIN(InPlaceEdit)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Right);

Wt::WInPlaceEdit *ipe
    = new Wt::WInPlaceEdit("This is editable text", container);

ipe->setEmptyText("You deleted the text!");
ipe->setButtonsEnabled(false);

ipe->valueChanged().connect(std::bind([=] () {
    out->setText("In-place edit is set to... ");
}));

SAMPLE_END(return container)
