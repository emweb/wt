/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WPushButton>
#include <Wt/WServer>
#include <Wt/WText>
#include <Wt/WTimer>

#include "SimpleChatServer.h"
#include "PopupChatWidget.h"

using namespace Wt;

/**
 * @addtogroup chatexample
 */
/*@{*/

/*! \brief A chat demo application.
 */
class ChatApplication : public WApplication
{
public:
  /*! \brief Create a new instance.
   */
  ChatApplication(const WEnvironment& env, SimpleChatServer& server);

private:
  SimpleChatServer& server_;
   Wt::WText *javaScriptError_;
   const WEnvironment& env_;
   Wt::WTimer *timer_;

  /*! \brief Add another chat client.
   */
  void addChatWidget();
  void javaScriptTest();
  void emptyFunc();
};

ChatApplication::ChatApplication(const WEnvironment& env,
				 SimpleChatServer& server)
  : WApplication(env),
    server_(server),
    env_(env)
{
  setTitle("Wt Chat");
  useStyleSheet("chatapp.css");

  messageResourceBundle().use(appRoot() + "simplechat");

  javaScriptTest();

  root()->addWidget(new WText(WString::tr("introduction")));

  SimpleChatWidget *chatWidget =
      new SimpleChatWidget(server_, root());
  chatWidget->setStyleClass("chat");

  root()->addWidget(new WText(WString::tr("details")));

  WPushButton *b = new WPushButton("I'm schizophrenic ...", root());
  b->clicked().connect(b, &WPushButton::hide);
  b->clicked().connect(this, &ChatApplication::addChatWidget);
}

void ChatApplication::javaScriptTest()
{
  if(!env_.javaScript()){
    javaScriptError_ = new WText(WString::tr("serverpushwarning"), root());

    // The 5 second timer is a fallback for real server push. The updated
    // server state will piggy back on the response to this timeout.
    timer_ = new Wt::WTimer(root());
    timer_->setInterval(5000);
    timer_->timeout().connect(this, &ChatApplication::emptyFunc);
    timer_->start();
  }
}

void ChatApplication::emptyFunc()
{}

void ChatApplication::addChatWidget()
{
  SimpleChatWidget *chatWidget2 = new SimpleChatWidget(server_, root());
  chatWidget2->setStyleClass("chat");
}

/*! \brief A chat application widget.
 */
class ChatWidget : public WApplication
{
public:
  ChatWidget(const WEnvironment& env, SimpleChatServer& server);

private:
  JSignal<WString> login_;
};

ChatWidget::ChatWidget(const WEnvironment& env, SimpleChatServer& server)
  : WApplication(env),
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
    PopupChatWidget *chatWidget = new PopupChatWidget(server, *div);
    bindWidget(chatWidget, *div);

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

WApplication *createApplication(const WEnvironment& env,
				SimpleChatServer& server)
{
  return new ChatApplication(env, server);
}

WApplication *createWidget(const WEnvironment& env, SimpleChatServer& server)
{
  return new ChatWidget(env, server);
}

int main(int argc, char **argv)
{
  Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);
  SimpleChatServer chatServer(server);

  /*
   * We add two entry points: one for the full-window application,
   * and one for a widget that can be integrated in another page.
   */
  server.addEntryPoint(Wt::Application,
		       boost::bind(createApplication, _1,
				   boost::ref(chatServer)));
  server.addEntryPoint(Wt::WidgetSet,
		       boost::bind(createWidget, _1,
				   boost::ref(chatServer)), "/chat.js");

  if (server.start()) {
    int sig = Wt::WServer::waitForShutdown();
    std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
    server.stop();
  }
}

/*@}*/
