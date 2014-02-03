#include <Wt/WContainerWidget>
#include <Wt/WInPlaceEdit>
#include <Wt/WText>

SAMPLE_BEGIN(InPlaceEdit)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WInPlaceEdit *ipe = new Wt::WInPlaceEdit("This is editable text", container);
ipe->setPlaceholderText("Enter something");
ipe->setButtonsEnabled(false);

SAMPLE_END(return container)
