
#include "FormWidgets.h"
#include "GraphicsWidgets.h"
#include "Layout.h"
#include "Media.h"
#include "Navigation.h"
#include "TreesTables.h"
#include "WidgetGallery.h"

#include <Wt/WApplication.h>
#include <Wt/WMenu.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

namespace {

const std::string showMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Show menu";
const std::string closeMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Close menu";

}

WidgetGallery::WidgetGallery()
  : BaseTemplate("tpl:widget-gallery"),
    openMenuButton_(nullptr),
    menuOpen_(false) // once applies when responsive UI
{
  contentsStack_ = bindNew<Wt::WStackedWidget>("contents");

  Wt::WAnimation animation(Wt::AnimationEffect::Fade,
                           Wt::TimingFunction::Linear,
                           200);
  contentsStack_->setTransitionAnimation(animation, true);

  /*
   * Setup the top-level menu
   */
  auto menu = bindNew<Wt::WMenu>("menu", contentsStack_);
  menu->addStyleClass("flex-column");
  menu->setInternalPathEnabled();
  menu->setInternalBasePath("/");

  openMenuButton_ = bindNew<Wt::WPushButton>("open-menu");
  openMenuButton_->setTextFormat(Wt::TextFormat::UnsafeXHTML);
  openMenuButton_->setText(showMenuText);

  openMenuButton_->clicked().connect(this, &WidgetGallery::toggleMenu);

  auto contentsCover = bindNew<Wt::WContainerWidget>("contents-cover");
  contentsCover->clicked().connect(this, &WidgetGallery::closeMenu);

  addToMenu(menu, "Layout", std::make_unique<Layout>());
  addToMenu(menu, "Forms", std::make_unique<FormWidgets>());
  addToMenu(menu, "Navigation", std::make_unique<Navigation>());
  addToMenu(menu, "Trees & Tables", std::make_unique<TreesTables>())
    ->setPathComponent("trees-tables");
  addToMenu(menu, "Graphics & Charts", std::make_unique<GraphicsWidgets>())
    ->setPathComponent("graphics-charts");
  addToMenu(menu, "Media", std::make_unique<Media>());

  if (menu->currentIndex() < 0) {
    menu->select(0);
    menu->itemAt(0)->menu()->select(0);
  }
}

Wt::WMenuItem *WidgetGallery::addToMenu(Wt::WMenu *menu,
                                        const Wt::WString& name,
                                        std::unique_ptr<Topic> topicPtr)
{
#ifndef WT_TARGET_JAVA
  auto topic = addChild(std::move(topicPtr));
#else // WT_TARGET_JAVA
  auto topic = topicPtr.release();
#endif // WT_TARGET_JAVA
  auto result = std::make_unique<Wt::WContainerWidget>();

  auto subMenuPtr = std::make_unique<Wt::WMenu>(contentsStack_);
  auto subMenu = subMenuPtr.get();
  auto itemPtr = std::make_unique<Wt::WMenuItem>(name);
  itemPtr->setMenu(std::move(subMenuPtr));

  auto item = menu->addItem(std::move(itemPtr));
  subMenu->addStyleClass("nav-stacked submenu");

  subMenu->itemSelected().connect(this, &WidgetGallery::closeMenu);

  subMenu->setInternalPathEnabled("/" + item->pathComponent());

  topic->populateSubMenu(subMenu);

  return item;
}

void WidgetGallery::toggleMenu()
{
  if (menuOpen_) {
    closeMenu();
  } else {
    openMenu();
  }
}

void WidgetGallery::openMenu()
{
  if (menuOpen_)
    return;

  openMenuButton_->setText(closeMenuText);
  addStyleClass("menu-open");

  menuOpen_ = true;
}

void WidgetGallery::closeMenu()
{
  if (!menuOpen_)
    return;

  openMenuButton_->setText(showMenuText);
  removeStyleClass("menu-open");

  menuOpen_ = false;
}
