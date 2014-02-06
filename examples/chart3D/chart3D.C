#include "NumericalExample.h"
#include "CategoryExample.h"
#include "ColorMapTest.h"

#include <Wt/WApplication>
#include <Wt/WBootstrapTheme>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WStackedWidget>

using namespace Wt;

class TestApp : public WApplication
{
public:
  TestApp(const WEnvironment& env);

private:
  WStackedWidget *stack_;
  WComboBox *chartExPicker_;

  NumericalExample *numEx_;
  CategoryExample *categoryEx_;
  ColorMapTest *colormap_;

  void switchExamples();
};

TestApp::TestApp(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("3D Charts Demo");

  setTheme(new WBootstrapTheme(this));
  //require("WebGL-Inspector/core/embed.js");

  messageResourceBundle().use(appRoot() + "configTemplates");

  WContainerWidget *wrapper = new WContainerWidget(root());
  wrapper->setContentAlignment(AlignCenter);
  wrapper->addWidget(new WText("<h1>3D Charts Demo</h1>", wrapper));
  chartExPicker_ = new WComboBox(wrapper);
  chartExPicker_->addItem("Numerical Grid-Based Data");
  chartExPicker_->addItem("Categorical Data");
  chartExPicker_->addItem("Colormap Example");
  chartExPicker_->changed().connect(this, &TestApp::switchExamples);
  
  stack_ = new WStackedWidget(wrapper);
  numEx_ = new NumericalExample(stack_);
  categoryEx_ = new CategoryExample(stack_);
  colormap_ = new ColorMapTest(stack_);
  
  chartExPicker_->setCurrentIndex(0);
  stack_->setCurrentWidget(numEx_);
}

void TestApp::switchExamples()
{
  switch (chartExPicker_->currentIndex()) {
  case 0:
    stack_->setCurrentWidget(numEx_);
    break;
  case 1:
    stack_->setCurrentWidget(categoryEx_);
    break;
  case 2:
    stack_->setCurrentWidget(colormap_);
    break;
  }
}

WApplication *createApplication(const WEnvironment& env)
{
  return new TestApp(env);
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
