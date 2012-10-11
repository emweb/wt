/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WText>

#include "TreeViewExample.h"

using namespace Wt;

class TreeViewApplication: public WApplication
{
public:
  TreeViewApplication(const WEnvironment &env):
    WApplication(env)
  {
    WStandardItemModel *model = TreeViewExample::createModel(true, this);

    root()->addWidget
      (new TreeViewExample(model, WString::tr("treeview-introduction"))); 

    /*
     * Stub for the drink info
     */
    aboutDrink_ = new WText("", root());
    
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

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new TreeViewApplication(env);
  app->setTitle("WTreeView example");
  app->messageResourceBundle().use(WApplication::appRoot() + "drinks");
  app->styleSheet().addRule("button", "margin: 2px");
  //app->useStyleSheet("treeview.css");
  
  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
