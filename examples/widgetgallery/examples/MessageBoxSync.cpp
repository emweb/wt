#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(MessageBoxSync)

auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Start");

Wt::WText *out = container->addNew<Wt::WText>();
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
