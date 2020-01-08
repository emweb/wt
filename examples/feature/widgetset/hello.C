/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>

using namespace Wt;

class HelloApplication : public WApplication
{
public:
  HelloApplication(const WEnvironment& env, bool embedded);

private:
  WLineEdit *nameEdit_;
  WText *greeting_;

  void greet();
};

HelloApplication::HelloApplication(const WEnvironment& env, bool embedded)
  : WApplication(env)
{
  WContainerWidget *top;

  setTitle("Hello world");

  if (!embedded) {
    /*
     * In Application mode, we have the root() is a container
     * corresponding to the entire browser window
     */
    top = root();

  } else {
    /*
     * In WidgetSet mode, we create and bind containers to existing
     * divs in the web page. In this example, we create a single div
     * whose DOM id was passed as a request argument.
     */

      std::unique_ptr<WContainerWidget> topPtr
          = cpp14::make_unique<WContainerWidget>();
      top = topPtr.get();

      const std::string *div = env.getParameter("div");
      if (div) {
          setJavaScriptClass(*div);
          bindWidget(std::move(topPtr), *div);
      } else {
          std::cerr << "Missing: parameter: 'div'" << std::endl;
          return;
      }
  }


  if (!embedded)
    root()->addWidget(cpp14::make_unique<WText>(
       "<p><emph>Note: you can also run this application "
       "from within <a href=\"hello.html\">a web page</a>.</emph></p>"));

  /*
   * Everything else is business as usual.
   */

  top->addWidget(cpp14::make_unique<WText>("Your name, please ? "));
  nameEdit_ = top->addWidget(cpp14::make_unique<WLineEdit>());
  nameEdit_->setFocus();

  auto b = top->addWidget(cpp14::make_unique<WPushButton>("Greet me."));
  b->setMargin(5, Side::Left);

  top->addWidget(cpp14::make_unique<WBreak>());

  greeting_ = top->addWidget(cpp14::make_unique<WText>());

  /*
   * Connect signals with slots
   */
  b->clicked().connect(this, &HelloApplication::greet);
  nameEdit_->enterPressed().connect(this, &HelloApplication::greet);
}

void HelloApplication::greet()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  greeting_->setText("Hello there, " + nameEdit_->text());
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<HelloApplication>(env, false);
}

std::unique_ptr<WApplication> createWidgetSet(const WEnvironment& env)
{
  return cpp14::make_unique<HelloApplication>(env, true);
}

int main(int argc, char **argv)
{
  // Use default server configuration: command line arguments and the
  // wthttpd configuration file.
  WServer server(argc, argv, WTHTTP_CONFIGURATION);

  // Application entry points. Each entry point binds an URL with an
  // application (with a callback function used to bootstrap a new
  // application).

  // The following is the default entry point. It configures a
  // standalone Wt application at the deploy path configured in the
  // server configuration.
  server.addEntryPoint(EntryPointType::Application, createApplication);

  // The following adds an entry point for a widget set. A widget set
  // must be loaded as a JavaScript from an HTML page.
  server.addEntryPoint(EntryPointType::WidgetSet, createWidgetSet, "/hello.js");

  // Start the server (in the background if there is threading support)
  // and wait for a shutdown signal (e.g. Ctrl C, SIGKILL), then cleanly
  // terminate all sessions
  server.run();
}

