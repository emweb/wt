/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "CustomLayout.h"

using namespace Wt;

class CustomLayoutApp : public WApplication
{
public:
  CustomLayoutApp(const WEnvironment& env)
    : WApplication(env)
  {
    auto layout = root()->setLayout(std::make_unique<CustomLayout>());
    auto text = std::make_unique<WText>("Item 1");
    text->decorationStyle().setBackgroundColor(Wt::WColor("green"));
    layout->addWidget(std::move(text));
    text = std::make_unique<WText>("Item 2");
    text->decorationStyle().setBackgroundColor(Wt::WColor("red"));
    layout->addWidget(std::move(text));
    text = std::make_unique<WText>("Item 3");
    text->decorationStyle().setBackgroundColor(Wt::WColor("yellow"));
    layout->addWidget(std::move(text));
    text = std::make_unique<WText>("Item 4");
    text->decorationStyle().setBackgroundColor(Wt::WColor("cyan"));
    layout->addWidget(std::move(text));
    auto addButton = std::make_unique<WPushButton>("Add a button");
    addButton->clicked().connect([=](){
      auto button = std::make_unique<WPushButton>("Click to delete");
      auto buttonPtr = button.get();
      button->clicked().connect([=](){
        layout->removeWidget(buttonPtr);
      });
      layout->addWidget(std::move(button));
    });
    layout->addWidget(std::move(addButton));
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<WApplication>(env);
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<CustomLayoutApp>(env);
  });
}
