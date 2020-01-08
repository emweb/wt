/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>

#include "SimpleChatServer.h"
#include "PopupChatWidget.h"

/**
 * @addtogroup chatexample
 */
/*@{*/

/*! \brief A chat demo application.
 */
class ChatApplication : public Wt::WApplication
{
public:
  /*! \brief Create a new instance.
   */
  ChatApplication(const Wt::WEnvironment& env, SimpleChatServer& server);

private:
  SimpleChatServer&         server_;
  Wt::WText                    *javaScriptError_;
  const Wt::WEnvironment&       env_;
  std::unique_ptr<Wt::WTimer>   timer_;

  /*! \brief Add another chat client.
   */
  void addChatWidget();
  void javaScriptTest();
  void emptyFunc();
};

ChatApplication::ChatApplication(const Wt::WEnvironment& env,
				 SimpleChatServer& server)
  : WApplication(env),
    server_(server),
    env_(env)
{
  setTitle("Wt Chat");
  useStyleSheet("chatapp.css");

  messageResourceBundle().use(appRoot() + "simplechat");

  javaScriptTest();

  root()->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::WString::tr("introduction")));

  SimpleChatWidget *chatWidget =
      root()->addWidget(Wt::cpp14::make_unique<SimpleChatWidget>(server_));
  chatWidget->setStyleClass("chat");

  root()->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::WString::tr("details")));

  Wt::WPushButton *b =
      root()->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("I'm schizophrenic ..."));
  b->clicked().connect(b, &Wt::WPushButton::hide);
  b->clicked().connect(this, &ChatApplication::addChatWidget);
}

void ChatApplication::javaScriptTest()
{
  if(!env_.javaScript()){
    javaScriptError_ =
	root()->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::WString::tr("serverpushwarning")));

    // The 5 second timer is a fallback for real server push. The updated
    // server state will piggy back on the response to this timeout.
    timer_ = Wt::cpp14::make_unique<Wt::WTimer>();
    timer_->setInterval(std::chrono::milliseconds{5000});
    timer_->timeout().connect(this, &ChatApplication::emptyFunc);
    timer_->start();
  }
}

void ChatApplication::emptyFunc()
{}

void ChatApplication::addChatWidget()
{
  SimpleChatWidget *chatWidget2 =
      root()->addWidget(Wt::cpp14::make_unique<SimpleChatWidget>(server_));
  chatWidget2->setStyleClass("chat");
}

/*! \brief A chat application widget.
 */
class ChatWidget : public Wt::WApplication
{
public:
  ChatWidget(const Wt::WEnvironment& env, SimpleChatServer& server);

private:
  Wt::JSignal<Wt::WString> login_;
};

ChatWidget::ChatWidget(const Wt::WEnvironment& env, SimpleChatServer& server)
  : Wt::WApplication(env),
    login_(this, "login")
{
  setCssTheme("");
  useStyleSheet("chatwidget.css");
  useStyleSheet("chatwidget_ie6.css", "lt IE 7");

  messageResourceBundle().use(appRoot() + "simplechat");

  const std::string *div = env.getParameter("div");
  std::string defaultDiv = "div";
  if (!div)
   div = &defaultDiv;

  if (div) {
    setJavaScriptClass(*div);
    std::unique_ptr<PopupChatWidget> chatWidgetPtr =
	Wt::cpp14::make_unique<PopupChatWidget>(server, *div);
    PopupChatWidget *chatWidget = chatWidgetPtr.get();
    bindWidget(std::move(chatWidgetPtr), *div);

    login_.connect(chatWidget, &PopupChatWidget::setName);

    std::string chat = javaScriptClass();
    doJavaScript("if (window." + chat + "User) "
		 + chat + ".emit(" + chat + ", 'login', " + chat + "User);"
		 + "document.body.appendChild(" + chatWidget->jsRef() + ");");
  } else {
    std::cerr << "Missing: parameter: 'div'" << std::endl;
    quit();
  }
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env,
				SimpleChatServer& server)
{
  return Wt::cpp14::make_unique<ChatApplication>(env, server);
}

std::unique_ptr<Wt::WApplication> createWidget(const Wt::WEnvironment& env, SimpleChatServer& server)
{
  return Wt::cpp14::make_unique<ChatWidget>(env, server);
}

int main(int argc, char **argv)
{
  Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);
  SimpleChatServer chatServer(server);

  /*
   * We add two entry points: one for the full-window application,
   * and one for a widget that can be integrated in another page.
   */
  server.addEntryPoint(Wt::EntryPointType::Application,
                       std::bind(createApplication, std::placeholders::_1,
                                   std::ref(chatServer)));
  server.addEntryPoint(Wt::EntryPointType::WidgetSet,
                       std::bind(createWidget, std::placeholders::_1,
                                   std::ref(chatServer)), "/chat.js");

  if (server.start()) {
    int sig = Wt::WServer::waitForShutdown();
    std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
    server.stop();
  }
}

/*@}*/
