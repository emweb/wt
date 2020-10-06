#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMenuItem.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(Popup)

auto container = std::make_unique<Wt::WContainerWidget>();

auto popupPtr = std::make_unique<Wt::WPopupMenu>();
auto popup = popupPtr.get();

#ifndef WT_TARGET_JAVA
auto statusPtr = std::make_unique<Wt::WText>();
auto status = statusPtr.get();
#else // WT_TARGET_JAVA
auto status = new Wt::WText();
#endif // WT_TARGET_JAVA
status->setMargin(10, Wt::Side::Left | Wt::Side::Right);

#ifndef WT_TARGET_JAVA
auto outPtr = std::make_unique<Wt::WText>();
auto out = outPtr.get();
#else // WT_TARGET_JAVA
auto out = new Wt::WText();
#endif // WT_TARGET_JAVA

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
auto subMenuPtr = std::make_unique<Wt::WPopupMenu>();
auto subMenu = subMenuPtr.get();

subMenu->addItem("Contents")->triggered().connect([=] {
    out->setText("<p>This could be a link to /contents.html.</p>");
});

subMenu->addItem("Index")->triggered().connect([=] {
    out->setText("<p>This could be a link to /index.html.</p>");
});

subMenu->addSeparator();
subMenu->addItem("About")->triggered().connect([=] {
#ifndef WT_TARGET_JAVA
    auto messageBox = subMenu->addChild(
	    std::make_unique<Wt::WMessageBox>
#else // WT_TARGET_JAVA
    auto messageBox = new Wt::WMessageBox
#endif // WT_TARGET_JAVA
	    ("About", "<p>This is a program to make connections.</p>",
	     Wt::Icon::Information,
#ifndef WT_TARGET_JAVA
             Wt::StandardButton::Ok));
#else // WT_TARGET_JAVA
             Wt::WFlags<Wt::StandardButton>(Wt::StandardButton::Ok));
#endif // WT_TARGET_JAVA
    messageBox->show();
    messageBox->buttonClicked().connect([=] {
#ifndef WT_TARGET_JAVA
      subMenu->removeChild(messageBox);
#else // WT_TARGET_JAVA
      delete messageBox;
#endif // WT_TARGET_JAVA
    });
});

// Assign the submenu to the parent popup menu.
popup->addMenu("Help", std::move(subMenuPtr));

Wt::WPushButton *button = container->addNew<Wt::WPushButton>();
button->setMenu(std::move(popupPtr));

// React to an item selection
popup->itemSelected().connect([=] (Wt::WMenuItem *selectedItem) {
    status->setText
        (Wt::WString("Selected menu item: {1}.")
	 .arg(selectedItem->text()));
});

#ifndef WT_TARGET_JAVA
container->addWidget(std::move(statusPtr));
container->addWidget(std::move(outPtr));
#else // WT_TARGET_JAVA
container->addWidget(std::unique_ptr<Wt::WWidget>(status));
container->addWidget(std::unique_ptr<Wt::WWidget>(out));
#endif // WT_TARGET_JAVA

SAMPLE_END(return std::move(container))
