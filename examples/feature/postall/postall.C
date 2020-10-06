#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

#include <cassert>

class PostAllExample : public Wt::WApplication {
public:
  PostAllExample(const Wt::WEnvironment &env)
    : WApplication{env}
  {
    // Enable server push
    enableUpdates(true);

    auto msgEdit = root()->addWidget(std::make_unique<Wt::WLineEdit>());
    auto sendBtn = root()->addWidget(
          std::make_unique<Wt::WPushButton>("Send message"));
    root()->addWidget(std::make_unique<Wt::WBreak>());
    lastMsg_ = root()->addWidget(
          std::make_unique<Wt::WText>("No messages received yet."));

    sendBtn->clicked().connect([msgEdit]{
      auto server = Wt::WServer::instance();
      Wt::WString msg = msgEdit->text();

      // Send msg to all active sessions with WServer::postAll()
      server->postAll([msg]{
        auto app = dynamic_cast<PostAllExample*>(Wt::WApplication::instance());
        assert(app != nullptr);
        app->updateMsg(msg);

        // Push the changes to the client
        app->triggerUpdate();
      });
    });
  }

  void updateMsg(Wt::WString msg)
  {
    lastMsg_->setText(Wt::WString{"Last received message: {1}"}.arg(msg));
  }

private:
  Wt::WText *lastMsg_;
};

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env){
    return std::make_unique<PostAllExample>(env);
  });
}
