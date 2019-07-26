#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(MessageBox)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Status");

Wt::WText *out = container->addNew<Wt::WText>();
out->setMargin(10, Wt::Side::Left);

auto c = container.get();
button->clicked().connect([=] {
    out->setText("The status button is clicked.");

    auto messageBox = c->addChild(
	    Wt::cpp14::make_unique<Wt::WMessageBox>("Status",
	          "<p>Ready to launch the rocket...</p>"
	          "<p>Launch the rocket immediately?</p>",
                  Wt::Icon::Information, Wt::StandardButton::Yes | Wt::StandardButton::No));

    messageBox->setModal(false);

    messageBox->buttonClicked().connect([=] {
        if (messageBox->buttonResult() == Wt::StandardButton::Yes)
	    out->setText("The rocket is launched!");
	else
	    out->setText("The rocket is ready for launch...");

        c->removeChild(messageBox);
    });

    messageBox->show();
});

SAMPLE_END(return std::move(container))
