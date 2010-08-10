/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "SimpleChatServer.h"
#include "SimpleChatWidget.h"

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
  useStyleSheet("simplechat.css");
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

WApplication *createApplication(const WEnvironment& env)
{
  return new ChatApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

/*@}*/
