#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMenuItem.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Popup)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto popupPtr = Wt::cpp14::make_unique<Wt::WPopupMenu>();
auto popup = popupPtr.get();

auto statusPtr = Wt::cpp14::make_unique<Wt::WText>();
auto status = statusPtr.get();
status->setMargin(10, Wt::Side::Left | Wt::Side::Right);

auto outPtr = Wt::cpp14::make_unique<Wt::WText>();
auto out = outPtr.get();

// Create some menu items for the popup menu
popup->addItem("Connect")->triggered().connect([=] {
    out->setText("<p>Connecting...</p>");
});

popup->addItem("Disconnect")->triggered().connect([=] {
    out->setText("<p>You are disconnected now.</p>");
});

popup->addSeparator();

popup->addItem("icons/house.png", "I'm home")->triggered().connect([=] {
    out->setText("");
});

Wt::WMenuItem *item = popup->addItem("Don't disturb");
item->setCheckable(true);

item->triggered().connect([=] {
    out->setText(Wt::WString("<p>{1} item is {2}.</p>")
		 .arg(item->text())
		 .arg(item->isChecked() ? "checked" : "unchecked"));
});

popup->addSeparator();

// Create a submenu for the popup menu.
auto subMenuPtr = Wt::cpp14::make_unique<Wt::WPopupMenu>();
auto subMenu = subMenuPtr.get();

subMenu->addItem("Contents")->triggered().connect([=] {
    out->setText("<p>This could be a link to /contents.html.</p>");
});

subMenu->addItem("Index")->triggered().connect([=] {
    out->setText("<p>This could be a link to /index.html.</p>");
});

subMenu->addSeparator();
subMenu->addItem("About")->triggered().connect([=] {
    auto messageBox = subMenu->addChild(
	    Wt::cpp14::make_unique<Wt::WMessageBox>
	    ("About", "<p>This is a program to make connections.</p>",
	     Wt::Icon::Information, Wt::StandardButton::Ok));
    messageBox->show();
    messageBox->buttonClicked().connect([=] {
      subMenu->removeChild(messageBox);
    });
});

// Assign the submenu to the parent popup menu.
popup->addMenu("Help", std::move(subMenuPtr));

Wt::WPushButton *button = container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>());
button->setMenu(std::move(popupPtr));

// React to an item selection
popup->itemSelected().connect([=] (Wt::WMenuItem *item) {
    status->setText
        (Wt::WString("Selected menu item: {1}.")
	 .arg(item->text()));
});

container->addWidget(std::move(statusPtr));
container->addWidget(std::move(outPtr));

SAMPLE_END(return std::move(container))
