#include <Wt/WLineEdit>
#include <Wt/WLink>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButtonDropdownAppended)

Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("appendedDropdownButton-template"));

Wt::WPopupMenu *popup = new Wt::WPopupMenu();

// Create some menu items for the popup menu
popup->addItem("Choose a button type");
popup->addSeparator();
popup->addItem("One-time hit button")->setLink(Wt::WLink("#one-time"));
popup->addItem("Navigation button")->setLink(Wt::WLink("#navigation"));
popup->addItem("Button style")->setLink(Wt::WLink("#style"));

Wt::WLineEdit *input = new Wt::WLineEdit();
result->bindWidget("input", input);

Wt::WPushButton *appendedDropdownButton = new Wt::WPushButton("Action");
result->bindWidget("appendedButton", appendedDropdownButton);
appendedDropdownButton->setMenu(popup);

SAMPLE_END(return result)

