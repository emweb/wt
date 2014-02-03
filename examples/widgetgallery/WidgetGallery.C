
#include "EventsDemo.h"
#include "FormWidgets.h"
#include "GraphicsWidgets.h"
#include "Layout.h"
#include "Media.h"
#include "Navigation.h"
//#include "StyleLayout.h"
#include "TreesTables.h"
#include "WidgetGallery.h"

#include <Wt/WHBoxLayout>
#include <Wt/WMenu>
#include <Wt/WNavigationBar>
#include <Wt/WStackedWidget>
#include <Wt/WText>
#include <Wt/WVBoxLayout>

WidgetGallery::WidgetGallery()
  : WContainerWidget()
{
  setOverflow(OverflowHidden);

  navigation_ = new Wt::WNavigationBar();
  navigation_->addStyleClass("main-nav");
  navigation_->setTitle("Wt Widget Gallery",
			"http://www.webtoolkit.eu/widgets");
  navigation_->setResponsive(true);

  contentsStack_ = new Wt::WStackedWidget();

  Wt::WAnimation animation(Wt::WAnimation::Fade,
			   Wt::WAnimation::Linear,
			   200);
  contentsStack_->setTransitionAnimation(animation, true);

  /*
   * Setup the top-level menu
   */
  Wt::WMenu *menu = new Wt::WMenu(contentsStack_, 0);
  menu->setInternalPathEnabled();
  menu->setInternalBasePath("/");

  addToMenu(menu, "Layout", new Layout());
  addToMenu(menu, "Forms", new FormWidgets());
  addToMenu(menu, "Navigation", new Navigation());
  addToMenu(menu, "Trees & Tables", new TreesTables())
    ->setPathComponent("trees-tables");
  addToMenu(menu, "Graphics & Charts", new GraphicsWidgets())
    ->setPathComponent("graphics-charts");
  // addToMenu(menu, "Events", new EventsDemo());
  addToMenu(menu, "Media", new Media());

  navigation_->addMenu(menu);

  /*
   * Add it all inside a layout
   */
  Wt::WVBoxLayout *layout = new Wt::WVBoxLayout(this);
  layout->addWidget(navigation_);
  layout->addWidget(contentsStack_, 1);
  layout->setContentsMargins(0, 0, 0, 0);
}

Wt::WMenuItem *WidgetGallery::addToMenu(Wt::WMenu *menu,
					const Wt::WString& name,
					TopicWidget *topic)
{
  Wt::WContainerWidget *result = new Wt::WContainerWidget();

  Wt::WContainerWidget *pane = new Wt::WContainerWidget();

  Wt::WVBoxLayout *vLayout = new Wt::WVBoxLayout(result);
  vLayout->setContentsMargins(0, 0, 0, 0);
  vLayout->addWidget(topic);
  vLayout->addWidget(pane, 1);

  Wt::WHBoxLayout *hLayout = new Wt::WHBoxLayout(pane);

  Wt::WMenuItem *item = new Wt::WMenuItem(name, result);
  menu->addItem(item);

  Wt::WStackedWidget *subStack = new Wt::WStackedWidget();
  subStack->addStyleClass("contents");
  subStack->setOverflow(WContainerWidget::OverflowAuto);

  /*
  Wt::WAnimation animation(Wt::WAnimation::Fade,
			   Wt::WAnimation::Linear,
			   100);
  subStack->setTransitionAnimation(animation, true);
  */

  Wt::WMenu *subMenu = new Wt::WMenu(subStack);
  subMenu->addStyleClass("nav-pills nav-stacked submenu");
  subMenu->setWidth(200);

  hLayout->addWidget(subMenu);
  hLayout->addWidget(subStack,1);

  subMenu->setInternalPathEnabled();
  subMenu->setInternalBasePath("/" + item->pathComponent());

  topic->populateSubMenu(subMenu);

  return item;
}
