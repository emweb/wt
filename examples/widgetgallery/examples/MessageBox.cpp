#include <Wt/WContainerWidget>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WText>

SAMPLE_BEGIN(MessageBox)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WPushButton *button = new Wt::WPushButton("Status", container);

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

button->clicked().connect(std::bind([=] () {
    out->setText("The status button is clicked.");

    Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Status",
	 "<p>Ready to launch the rocket...</p>"
	 "<p>Launch the rocket immediately?</p>",
	 Wt::Information, Wt::Yes | Wt::No);

    messageBox->setModal(false);

    messageBox->buttonClicked().connect(std::bind([=] () {
	if (messageBox->buttonResult() == Wt::Yes)
	    out->setText("The rocket is launched!");
	else
	    out->setText("The rocket is ready for launch...");

	delete messageBox;
    }));

    messageBox->show();
}));

SAMPLE_END(return container)
