/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WText.h>

#include "TreeViewExample.h"

using namespace Wt;

class TreeViewApplication: public WApplication
{
public:
  TreeViewApplication(const WEnvironment &env):
    WApplication(env)
  {
    std::shared_ptr<WStandardItemModel>
        model(TreeViewExample::createModel(true));

    root()->addWidget
      (std::make_unique<TreeViewExample>(
         model, WString::tr("treeview-introduction")));

    /*
     * Stub for the drink info
     */
    aboutDrink_ = root()->addWidget(std::make_unique<WText>(""));
    
    internalPathChanged().connect(this, &TreeViewApplication::handlePathChange);

    this->handlePathChange();
  }
private:
  WText *aboutDrink_;

  void handlePathChange() {
    if (internalPathMatches("/drinks/")) {
      std::string drink = internalPathNextPart("/drinks/");
      aboutDrink_->setText(WString::tr("drink-" + drink));
    }
  }

};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<TreeViewApplication>(env);
  app->setTitle("WTreeView example");
  app->messageResourceBundle().use(WApplication::appRoot() + "drinks");
  app->styleSheet().addRule("button", "margin: 2px");
  app->useStyleSheet("treeview.css");
  
  return std::move(app);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
