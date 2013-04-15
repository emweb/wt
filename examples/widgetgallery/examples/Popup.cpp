#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WMessageBox>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WText>

SAMPLE_BEGIN(Popup)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WPopupMenu *popup = new Wt::WPopupMenu();

Wt::WText *status = new Wt::WText();
status->setMargin(10, Wt::Left | Wt::Right);

Wt::WText *out = new Wt::WText();

// Create some menu items for the popup menu
popup->addItem("Connect")->triggered().connect(std::bind([=] () {
    out->setText("<p>Connecting...</p>");
}));

popup->addItem("Disconnect")->triggered().connect(std::bind([=] () {
    out->setText("<p>You are disconnected now.</p>");
}));

popup->addSeparator();

popup->addItem("icons/house.png", "I'm home")->triggered().connect(std::bind([=] () {
    out->setText("");
}));

Wt::WMenuItem *item = popup->addItem("Don't disturb");
item->setCheckable(true);

item->triggered().connect(std::bind([=] () {
    out->setText(Wt::WString::fromUTF8("<p>{1} item is {2}.</p>")
		 .arg(item->text())
		 .arg(item->isChecked() ? "checked" : "unchecked"));
}));

popup->addSeparator();

// Create a submenu for the popup menu.
Wt::WPopupMenu *subMenu = new Wt::WPopupMenu();

subMenu->addItem("Contents")->triggered().connect(std::bind([=] () {
    out->setText("<p>This could be a link to /contents.html.</p>");
}));

subMenu->addItem("Index")->triggered().connect(std::bind([=] () {
    out->setText("<p>This could be a link to /index.html.</p>");
}));

subMenu->addSeparator();
subMenu->addItem("About")->triggered().connect(std::bind([=] () {
    Wt::WMessageBox *messageBox = new Wt::WMessageBox
        ("About", "<p>This is a program to make connections.</p>",
	 Wt::Information, Wt::Ok);
    messageBox->show();
    messageBox->buttonClicked().connect(std::bind([=] () {
	delete messageBox;
    }));
}));

// Assign the submenu to the parent popup menu.
popup->addMenu("Help", subMenu);

Wt::WPushButton *button = new Wt::WPushButton(container);
button->setMenu(popup);

// React to an item selection
popup->itemSelected().connect(std::bind([=] (Wt::WMenuItem *item) {
    status->setText
        (Wt::WString::fromUTF8("Selected menu item: {1}.")
	 .arg(item->text()));
}, std::placeholders::_1));

container->addWidget(status);
container->addWidget(out);

SAMPLE_END(return container)
