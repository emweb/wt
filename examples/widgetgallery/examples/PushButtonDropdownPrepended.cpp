#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonDropdownPrepended)

auto result =
    cpp14::make_unique<WTemplate>(WString::tr("prependedDropdownButton-template"));

auto popup =
    cpp14::make_unique<WPopupMenu>();
popup->addItem("Choose a button type");
popup->addSeparator();
popup->addSeparator();
popup->addItem("One-time hit button")->setLink(WLink("#one-time"));
popup->addItem("Navigation button")->setLink(WLink("#navigation"));
popup->addItem("Button style")->setLink(WLink("#style"));

auto input  = cpp14::make_unique<WLineEdit>();
auto input_ = result->bindWidget("input", std::move(input));
input_->setStyleClass("span2");

auto prependedDropdownButton = cpp14::make_unique<WPushButton>("Action");
auto button_ = result->bindWidget("prependedButton", std::move(prependedDropdownButton));
button_->setMenu(std::move(popup));

SAMPLE_END(return result)
