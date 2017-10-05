#include <Wt/WContainerWidget.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SelectionBoxSimple)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WSelectionBox *sb1 =
    container->addWidget(Wt::cpp14::make_unique<Wt::WSelectionBox>());
sb1->addItem("Heavy");
sb1->addItem("Medium");
sb1->addItem("Light");
sb1->setCurrentIndex(1); // Select 'medium' by default.
sb1->setMargin(10, Wt::Side::Right);

Wt::WText *out =
    container->addWidget(Wt::cpp14::make_unique<Wt::WText>(""));

sb1->activated().connect([=] {
    out->setText(Wt::WString("You selected {1}.")
		 .arg(sb1->currentText()));
});

SAMPLE_END(return std::move(container))
