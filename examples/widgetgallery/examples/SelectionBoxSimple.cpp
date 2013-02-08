#include <Wt/WContainerWidget>
#include <Wt/WSelectionBox>
#include <Wt/WText>

SAMPLE_BEGIN(SelectionBoxSimple)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WSelectionBox *sb1 = new Wt::WSelectionBox(container);
sb1->addItem("Heavy");
sb1->addItem("Medium");
sb1->addItem("Light");
sb1->setCurrentIndex(1); // Select 'medium' by default.
sb1->setMargin(10, Wt::Right);

Wt::WText *out = new Wt::WText("", container);

sb1->activated().connect(std::bind([=] () {
    out->setText(Wt::WString::fromUTF8("You selected {1}.")
		 .arg(sb1->currentText()));
}));

SAMPLE_END(return container)
