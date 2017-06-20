#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(MessageBoxSync)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Start"));

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());
out->setMargin(10, Wt::Side::Left);

button->clicked().connect([=] {
    Wt::StandardButton answer
      = Wt::WMessageBox::show("Launch phase",
                          "<p>Launch the rocket?</p>",
                          Wt::StandardButton::Ok | Wt::StandardButton::Cancel);
    if (answer == Wt::StandardButton::Ok){
        out->setText("The rocket is launched!");
    } else {
        out->setText("Waiting on your decision...");
    }
});

SAMPLE_END(return std::move(container))
