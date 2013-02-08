#include <Wt/WContainerWidget>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WText>

SAMPLE_BEGIN(MessageBoxSync)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WPushButton *button = new Wt::WPushButton("Start", container);

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

button->clicked().connect(std::bind([=] () {
    Wt::StandardButton answer 
      = Wt::WMessageBox::show("Launch phase",
			      "<p>Launch the rocket?</p>",
			      Wt::Ok | Wt::Cancel);
    if (answer == Wt::Ok)
        out->setText("The rocket is launched!");
    else
        out->setText("Waiting on your decision...");
}));

SAMPLE_END(return container)
