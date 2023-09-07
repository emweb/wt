#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>

#include "orderedit.C"

class TestApplication : public Wt::WApplication
{
public:
    TestApplication(const Wt::WEnvironment& env)
      : WApplication(env)
    {
      setTitle("TableView Drag & Drop Example");

      root()->addWidget(makeOrderEdit());
    }
};

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
    return std::make_unique<TestApplication>(env);
  });
}
