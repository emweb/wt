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

      Wt::WMessageBox *mb = new Wt::WMessageBox
	  ("Status",
	   "<p>Ready to launch the rocket...</p>"
	   "<p>Launch the rocket immediately?</p>",
	   Wt::Information, Wt::Yes | Wt::No);

      mb->setModal(false);

      mb->buttonClicked().connect(std::bind([=] () {
	    if (mb->buttonResult() == Wt::Yes)
	        out->setText("The rocket is launched!");
	    else
	        out->setText("The rocket is ready for launch...");

	    delete mb;
      }));

      mb->show();
}));

SAMPLE_END(return container)
