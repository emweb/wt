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

class HelloApplication : public Wt::WApplication
{
public:
  HelloApplication(const Wt::WEnvironment& env, bool embedded);

private:
  Wt::WLineEdit *nameEdit_;
  Wt::WText *greeting_;

  void greet();
};

HelloApplication::HelloApplication(const Wt::WEnvironment& env, bool embedded)
  : WApplication(env)
{
  Wt::WContainerWidget *top;

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
    top = new Wt::WContainerWidget();
    const std::string *div = env.getParameter("div");
    if (div) {
      setJavaScriptClass(*div);
      bindWidget(top, *div);
    } else {
      std::cerr << "Missing: parameter: 'div'" << std::endl;
      return;
    }
  }

  if (!embedded)
    new Wt::WText
      ("<p><emph>Note: you can also run this application "
       "from within <a href=\"hello.html\">a web page</a>.</emph></p>",
       root());

  /*
   * Everything else is business as usual.
   */

  top->addWidget(new Wt::WText("Your name, please ? "));
  nameEdit_ = new Wt::WLineEdit(top);
  nameEdit_->setFocus();

  Wt::WPushButton *b = new Wt::WPushButton("Greet me.", top);
  b->setMargin(5, Wt::Left);

  top->addWidget(new Wt::WBreak());

  greeting_ = new Wt::WText(top);

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

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new HelloApplication(env, false);
}

Wt::WApplication *createWidgetSet(const Wt::WEnvironment& env)
{
  return new HelloApplication(env, true);
}

int main(int argc, char **argv)
{
  // Use default server configuration: command line arguments and the
  // wthttpd configuration file.
  Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

  // Application entry points. Each entry point binds an URL with an
  // application (with a callback function used to bootstrap a new
  // application).

  // The following is the default entry point. It configures a
  // standalone Wt application at the deploy path configured in the
  // server configuration.
  server.addEntryPoint(Wt::Application, createApplication);

  // The following adds an entry point for a widget set. A widget set
  // must be loaded as a JavaScript from an HTML page.
  server.addEntryPoint(Wt::WidgetSet, createWidgetSet, "/hello.js");

  // Start the server (in the background if there is threading support)
  // and wait for a shutdown signal (e.g. Ctrl C, SIGKILL), then cleanly
  // terminate all sessions
  server.run();
}

