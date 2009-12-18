#include "WidgetGallery.h"
#include "BasicControls.h"
#include "ChartWidgets.h"
#include "DialogWidgets.h"
#include "EventsDemo.h"
#include "EventDisplayer.h"
#include "ExtWidgets.h"
#include "FormWidgets.h"
#include "GraphicsWidgets.h"
#include "MvcWidgets.h"
#include "Validators.h"
#include "StyleLayout.h"
#include "SpecialPurposeWidgets.h"

#include <Wt/WMenu>
#include <Wt/WSubMenuItem>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WStackedWidget>
#include <Wt/WText>

using namespace Wt;

WidgetGallery::WidgetGallery(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Wt widgets demo");
  setCssTheme("polished");
  // load text bundles (for the tr() function)
  messageResourceBundle().use("text");
  messageResourceBundle().use("charts");
  messageResourceBundle().use("treeview");

  contentsStack_ = new WStackedWidget();
  // Show scrollbars when needed ...
  contentsStack_->setOverflow(WContainerWidget::OverflowAuto);
  // ... and work around a bug in IE (see setOverflow() documentation)
  contentsStack_->setPositionScheme(Relative);
  contentsStack_->setStyleClass("contents");

  EventDisplayer *eventDisplayer = new EventDisplayer(0);

  /*
   * Setup the menu (and submenus)
   */
  WMenu *menu = new WMenu(contentsStack_, Vertical, 0);
  menu->setRenderAsList(true);
  menu->setStyleClass("menu");
  menu->setInternalPathEnabled();
  menu->setInternalBasePath("/");

  addToMenu(menu, "Basics", new BasicControls(eventDisplayer));
  addToMenu(menu, "Form Widgets", new FormWidgets(eventDisplayer));
  addToMenu(menu, "Form Validators", new Validators(eventDisplayer));
  addToMenu(menu, "Ext Widgets", new ExtWidgets(eventDisplayer));
  addToMenu(menu, "Vector Graphics", new GraphicsWidgets(eventDisplayer));
  addToMenu(menu, "Special Purpose", new SpecialPurposeWidgets(eventDisplayer));
  addToMenu(menu, "Dialogs", new DialogWidgets(eventDisplayer));
  addToMenu(menu, "Charts", new ChartWidgets(eventDisplayer));
  addToMenu(menu, "MVC Widgets", new MvcWidgets(eventDisplayer));
  addToMenu(menu, "Events", new EventsDemo(eventDisplayer));
  addToMenu(menu, "Style and Layout", new StyleLayout(eventDisplayer));

  /*
   * Add it all inside a layout
   */
  WHBoxLayout *horizLayout = new WHBoxLayout(root());
  WVBoxLayout *vertLayout = new WVBoxLayout;

  horizLayout->addWidget(menu, 0);
  horizLayout->addLayout(vertLayout, 1);
  vertLayout->addWidget(contentsStack_, 1);
  vertLayout->addWidget(eventDisplayer);

  /*
   * Set our style sheet last, so that it loaded after the ext stylesheets.
   */
  useStyleSheet("everywidget.css");
  useStyleSheet("dragdrop.css");
  useStyleSheet("combostyle.css");
  useStyleSheet("popupmenu.css");
}

void WidgetGallery::addToMenu(WMenu *menu, const WString& name,
			      ControlsWidget *controls)
{
  if (controls->hasSubMenu()) {
    WSubMenuItem *smi = new WSubMenuItem(name, controls);
    WMenu *subMenu = new WMenu(contentsStack_, Vertical, 0);
    subMenu->setRenderAsList(true);

    smi->setSubMenu(subMenu);
    menu->addItem(smi);

    subMenu->setInternalPathEnabled();
    subMenu->setInternalBasePath("/" + smi->pathComponent());
    subMenu->setStyleClass("menu submenu");

    controls->populateSubMenu(subMenu);
  } else
    menu->addItem(name, controls);
}
