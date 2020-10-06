#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(PushButtonDropdownPrepended)

auto result =
    std::make_unique<WTemplate>(WString::tr("prependedDropdownButton-template"));

auto popup =
    std::make_unique<WPopupMenu>();
popup->addItem("Choose a button type");
popup->addSeparator();
popup->addSeparator();
popup->addItem("One-time hit button")->setLink(WLink("#one-time"));
popup->addItem("Navigation button")->setLink(WLink("#navigation"));
popup->addItem("Button style")->setLink(WLink("#style"));

auto input  = std::make_unique<WLineEdit>();
auto input_ = result->bindWidget("input", std::move(input));
input_->setStyleClass("span2");

auto prependedDropdownButton = std::make_unique<WPushButton>("Action");
auto button_ = result->bindWidget("prependedButton", std::move(prependedDropdownButton));
button_->setMenu(std::move(popup));

SAMPLE_END(return result)
