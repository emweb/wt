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

#include "SimpleChatServer.h"
#include "PopupChatWidget.h"

using namespace Wt;

/**
 * @addtogroup chatexample
 */
/*@{*/

/*! \brief The single chat server instance.
 */
SimpleChatServer theServer;

/*! \brief A chat demo application.
 */
class ChatApplication : public WApplication
{
public:
  /*! \brief Create a new instance.
   */
  ChatApplication(const WEnvironment& env);

private:
  /*! \brief Add another chat client.
   */
  void addChatWidget();
};

ChatApplication::ChatApplication(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Wt Chat");
  useStyleSheet("chatapp.css");
  messageResourceBundle().use(Wt::WApplication::appRoot() + "simplechat");

  root()->addWidget(new WText(WString::tr("introduction")));

  SimpleChatWidget *chatWidget = new SimpleChatWidget(theServer, root());
  chatWidget->setStyleClass("chat");

  root()->addWidget(new WText(WString::tr("details")));

  WPushButton *b = new WPushButton("I'm schizophrenic ...", root());
  b->clicked().connect(b, &WPushButton::hide);
  b->clicked().connect(this, &ChatApplication::addChatWidget);
}

void ChatApplication::addChatWidget()
{
  SimpleChatWidget *chatWidget2 = new SimpleChatWidget(theServer, root());
  chatWidget2->setStyleClass("chat");
}

/*! \brief A chat application widget.
 */
class ChatWidget : public WApplication
{
public:
  ChatWidget(const WEnvironment& env);

private:
  JSignal<WString> login_;
};

ChatWidget::ChatWidget(const WEnvironment& env)
  : WApplication(env),
    login_(this, "login")
{
  useStyleSheet("chatwidget.css");
  useStyleSheet("chatwidget_ie6.css", "lt IE 7");

  const std::string *div = env.getParameter("div");

  if (div) {
    setJavaScriptClass(*div);
    PopupChatWidget *chatWidget = new PopupChatWidget(theServer);
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

WApplication *createApplication(const WEnvironment& env)
{
  return new ChatApplication(env);
}

WApplication *createWidget(const WEnvironment& env)
{
  return new ChatWidget(env);
}

int main(int argc, char **argv)
{
  Wt::WServer server(argv[0]);

  server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
  server.addEntryPoint(Wt::Application, createApplication);
  server.addEntryPoint(Wt::WidgetSet, createWidget, "/chat.js");

  if (server.start()) {
    Wt::WServer::waitForShutdown();
    server.stop();
  }
}

/*@}*/
