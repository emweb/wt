#include "Theme.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

#include <memory>

namespace {
  const std::string templateText = R"=(
    <div class="container mt-2 mb-2">
      <div class="row">
        <div class="col">
          <h1>Customized Bootstrap 5 theme</h1>
        </div>
      </div>
      <div class="row">
        <div class="col-auto d-flex align-items-center">
          ${button class="btn-primary"}
        </div>
        <div class="col">
          This example demonstrates how Wt's Bootstrap 5 theme can be changed. Notice how the primary
          button before this text is colored turquoise instead of the default Bootstrap blue.
        </div>
      </div>
    </div>
  )=";
}

class Application final : public Wt::WApplication {
public:
  explicit Application(const Wt::WEnvironment& env);
};

Application::Application(const Wt::WEnvironment &env)
  : Wt::WApplication(env)
{
  setTheme(std::make_shared<Theme>());

  auto tpl = root()->addNew<Wt::WTemplate>(templateText);
  tpl->bindNew<Wt::WPushButton>("button", "Primary");
}

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<Application>(env);
  });
}
