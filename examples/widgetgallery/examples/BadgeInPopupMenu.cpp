#include <Wt/WContainerWidget.h>
#include <Wt/WBadge.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>

SAMPLE_BEGIN(BadgeInPopupMenu)
Wt::WCssDecorationStyle redBadgeStyle;
redBadgeStyle.setBackgroundColor(Wt::WColor(255, 0, 0));

Wt::WCssDecorationStyle yellowBadgeStyle;
yellowBadgeStyle.setBackgroundColor(Wt::WColor(255, 200, 0));
yellowBadgeStyle.setForegroundColor(Wt::WColor(0, 0, 0));

auto container = std::make_unique<Wt::WContainerWidget>();
auto button = container->addWidget(std::make_unique<Wt::WPushButton>("Features"));

auto popupMenu = std::make_unique<Wt::WPopupMenu>();
popupMenu->addItem("Old boring feature");

auto newBadge = std::make_unique<Wt::WBadge>("New");
newBadge->setDecorationStyle(redBadgeStyle);
// Ensure Bootstrap 5 theme does not override our custom style
newBadge->setUseDefaultStyle(false);
popupMenu->addItem("New exciting feature")->setBadge(std::move(newBadge));

auto premiumBadge = std::make_unique<Wt::WBadge>("Premium");
premiumBadge->setDecorationStyle(yellowBadgeStyle);
// Ensure Bootstrap 5 theme does not override our custom style
premiumBadge->setUseDefaultStyle(false);
popupMenu->addItem("Premium feature")->setBadge(std::move(premiumBadge));

button->setMenu(std::move(popupMenu));

auto buttonBadge = std::make_unique<Wt::WBadge>("1");
buttonBadge->setDecorationStyle(redBadgeStyle);
// Ensure Bootstrap 5 theme does not override our custom style
buttonBadge->setUseDefaultStyle(false);
button->setBadge(std::move(buttonBadge));

SAMPLE_END(return std::move(container))
