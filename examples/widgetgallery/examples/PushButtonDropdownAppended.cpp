#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonDropdownAppended)

auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("appendedDropdownButton-template"));

auto popup =
    Wt::cpp14::make_unique<Wt::WPopupMenu>();

// Create some menu items for the popup menu
popup->addItem("Choose a button type");
popup->addSeparator();
popup->addItem("One-time hit button")->setLink(Wt::WLink("#one-time"));
popup->addItem("Navigation button")->setLink(Wt::WLink("#navigation"));
popup->addItem("Button style")->setLink(Wt::WLink("#style"));

auto input = Wt::cpp14::make_unique<Wt::WLineEdit>();
result->bindWidget("input", std::move(input));

auto appendedDropdownButton = Wt::cpp14::make_unique<Wt::WPushButton>("Action");
auto button = result->bindWidget("appendedButton", std::move(appendedDropdownButton));
button->setMenu(std::move(popup));

SAMPLE_END(return std::move(result))

