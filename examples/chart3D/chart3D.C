#include "NumericalExample.h"
#include "CategoryExample.h"
#include "ColorMapTest.h"

#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WStackedWidget.h>

class TestApp : public Wt::WApplication
{
public:
  TestApp(const Wt::WEnvironment& env);

private:
  Wt::WStackedWidget    *stack_;
  Wt::WComboBox         *chartExPicker_;

  NumericalExample  *numEx_;
  CategoryExample   *categoryEx_;
  ColorMapTest      *colormap_;

  void switchExamples();
};

TestApp::TestApp(const Wt::WEnvironment& env)
  : WApplication(env)
{
  setTitle("3D Charts Demo");

  setTheme(std::make_shared<Wt::WBootstrapTheme>());
  //require("WebGL-Inspector/core/embed.js");

  messageResourceBundle().use(appRoot() + "configTemplates");

  Wt::WContainerWidget *wrapper = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
  wrapper->setContentAlignment(Wt::AlignmentFlag::Center);
  wrapper->addWidget(std::make_unique<Wt::WText>("<h1>3D Charts Demo</h1>"));
  chartExPicker_ = wrapper->addWidget(std::make_unique<Wt::WComboBox>());
  chartExPicker_->addItem("Numerical Grid-Based Data");
  chartExPicker_->addItem("Categorical Data");
  chartExPicker_->addItem("Colormap Example");
  chartExPicker_->changed().connect(this, &TestApp::switchExamples);
  
  stack_ = wrapper->addWidget(std::make_unique<Wt::WStackedWidget>());
  numEx_ = stack_->addWidget(std::make_unique<NumericalExample>());
  categoryEx_ = stack_->addWidget(std::make_unique<CategoryExample>());
  colormap_ = stack_->addWidget(std::make_unique<ColorMapTest>());
  
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

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<TestApp>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
