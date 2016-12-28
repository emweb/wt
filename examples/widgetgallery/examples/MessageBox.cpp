#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(MessageBox)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Status"));

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());
out->setMargin(10, Wt::Side::Left);

auto c = container.get();
button->clicked().connect(std::bind([=] () {
    out->setText("The status button is clicked.");

    auto messageBoxPtr = Wt::cpp14::make_unique<Wt::WMessageBox>("Status",
	 "<p>Ready to launch the rocket...</p>"
	 "<p>Launch the rocket immediately?</p>",
         Wt::Icon::Information, Wt::StandardButton::Yes | Wt::StandardButton::No);
    auto messageBox = messageBoxPtr.get();

    messageBox->setModal(false);

    messageBox->buttonClicked().connect(std::bind([=] () {
        if (messageBox->buttonResult() == Wt::StandardButton::Yes)
	    out->setText("The rocket is launched!");
	else
	    out->setText("The rocket is ready for launch...");

        c->removeChild(messageBox);
    }));

    messageBox->show();
    c->addChild(std::move(messageBoxPtr));
}));

SAMPLE_END(return std::move(container))
