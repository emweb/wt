#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

namespace Wt {
  class WTemplate;
}

class WebSocketApplication : public Wt::WApplication
{
public:
  WebSocketApplication(const Wt::WEnvironment& env, const std::string& resourceURL);

private:
  void createLayout();

  const std::string& webSocketResourceURL_;

  Wt::WTemplate* templ_ = nullptr;
};
