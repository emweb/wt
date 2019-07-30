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

    auto messageBox =
#ifndef WT_TARGET_JAVA
      c->addChild(Wt::cpp14::make_unique<Wt::WMessageBox>(
#else // WT_TARGET_JAVA
            new Wt::WMessageBox(
#endif // WT_TARGET_JAVA
                  "Status",
	          "<p>Ready to launch the rocket...</p>"
	          "<p>Launch the rocket immediately?</p>",
                  Wt::Icon::Information,
#ifndef WT_TARGET_JAVA
                  Wt::StandardButton::Yes | Wt::StandardButton::No));
#else // WT_TARGET_JAVA
                  Wt::WFlags<Wt::StandardButton>(Wt::StandardButton::Yes) | Wt::StandardButton::No);
#endif // WT_TARGET_JAVA

    messageBox->setModal(false);

    messageBox->buttonClicked().connect([=] {
        if (messageBox->buttonResult() == Wt::StandardButton::Yes)
	    out->setText("The rocket is launched!");
	else
	    out->setText("The rocket is ready for launch...");

#ifndef WT_TARGET_JAVA
        c->removeChild(messageBox);
#else // WT_TARGET_JAVA
        delete messageBox;
#endif // WT_TARGET_JAVA
    });

    messageBox->show();
});

SAMPLE_END(return std::move(container))
