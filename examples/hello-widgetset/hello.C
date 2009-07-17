/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WServer>
#include <Wt/WText>

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
  /*
   * By default, "dynamic script tags" are used to relay event information
   * in WidgetSet mode. This has the benefit of allowing an application to
   * be embedded from within a web page on another domain.
   *
   * You can revert to plain AJAX requests using the following call. This will
   * only work if your application is hosted on the same domain as the
   * web page in which it is embedded.
   */
  setAjaxMethod(XMLHttpRequest);

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
    top = new WContainerWidget();
    const std::string *div = env.getParameter("div");
    if (div)
      bindWidget(top, *div);
    else {
      std::cerr << "Missing: parameter: 'div'" << std::endl;
      return;
    }
  }

  if (!embedded)
    new WText("<p><emph>Note: you can also run this application "
	      "from within <a href=\"hello.html\">a web page</a>.</emph></p>",
	      root());

  /*
   * Everything else is business as usual.
   */

  top->addWidget(new WText("Your name, please ? "));
  nameEdit_ = new WLineEdit(top);
  nameEdit_->setFocus();

  WPushButton *b = new WPushButton("Greet me.", top);
  b->setMargin(5, Left);

  top->addWidget(new WBreak());

  greeting_ = new WText(top);

  /*
   * Connect signals with slots
   */
  b->clicked().connect(SLOT(this, HelloApplication::greet));
  nameEdit_->enterPressed().connect(SLOT(this, HelloApplication::greet));
}

void HelloApplication::greet()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  greeting_->setText("Hello there, " + nameEdit_->text());
}

WApplication *createApplication(const WEnvironment& env)
{
  return new HelloApplication(env, false);
}

WApplication *createWidgetSet(const WEnvironment& env)
{
  return new HelloApplication(env, true);
}

int main(int argc, char **argv)
{
  WServer server(argv[0]);

  // Use default server configuration: command line arguments and the
  // wthttpd configuration file.
  server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

  // Application entry points. Each entry point binds an URL with an
  // application (with a callback function used to bootstrap a new
  // application).

  // The following is the default entry point. It configures a
  // standalone Wt application at the deploy path configured in the
  // server configuration.
  server.addEntryPoint(Application, createApplication);

  // The following adds an entry point for a widget set. A widget set
  // must be loaded as a JavaScript from an HTML page.
  server.addEntryPoint(WidgetSet, createWidgetSet, "/hello.wtjs");

  // Start the server (in the background if there is threading support)
  // and wait for a shutdown signal (e.g. Ctrl C, SIGKILL)
  if (server.start()) {
    WServer::waitForShutdown();

    // Cleanly terminate all sessions
    server.stop();
  }
}

