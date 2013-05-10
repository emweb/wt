#include <Wt/WLineEdit>
#include <Wt/WLink>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButtonDropdownPrepended)

Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("prependedDropdownButton-template"));

Wt::WPopupMenu *popup = new Wt::WPopupMenu();
popup->addItem("Choose a button type");
popup->addSeparator();
popup->addSeparator();
popup->addItem("One-time hit button")->setLink(Wt::WLink("#one-time"));
popup->addItem("Navigation button")->setLink(Wt::WLink("#navigation"));
popup->addItem("Button style")->setLink(Wt::WLink("#style"));

Wt::WLineEdit *input  = new Wt::WLineEdit();
result->bindWidget("input", input);
input->setStyleClass("span2");

Wt::WPushButton *prependedDropdownButton = new Wt::WPushButton("Action");
result->bindWidget("prependedButton", prependedDropdownButton);
prependedDropdownButton->setMenu(popup);

SAMPLE_END(return result)
