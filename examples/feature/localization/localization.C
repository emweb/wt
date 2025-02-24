/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>
#include <Wt/WText.h>


#include <Wt/WLogger.h>
#include "LanguageButton.h"
#include "LocalStringExample.h"
#include "ConcatenationExamples.h"
#include "RefreshExamples.h"
#include "FormatExample.h"

class LocalizedApplication : public Wt::WApplication
{
public:
  explicit LocalizedApplication(const Wt::WEnvironment& env)
    : WApplication(env)
  {
    auto app = this;

    setTheme(std::make_shared<Wt::WBootstrap5Theme>());
    app->messageResourceBundle().use(appRoot() + "/messages");
    app->messageResourceBundle().use(appRoot() + "/templates");
    
    Wt::WTemplate *t = app->root()->addWidget(
      std::make_unique<Wt::WTemplate>(Wt::WString::tr("Main-Page")));
    
    // Binding strings and widgets to the variables in the template
    t->bindString("intro", Wt::WString::tr("intro"));
    t->bindWidget("language-btn", std::make_unique<LanguageButton>());
    
    t->addFunction("tr", &Wt::WTemplate::Functions::tr);
    t->addFunction("block", &Wt::WTemplate::Functions::block);

    // Adding the examples
    t->bindWidget("LocalStr-example", std::make_unique<LocalStringExample>());
    t->bindWidget("Concat-example", std::make_unique<ConcatenationExamples>());
    t->bindWidget("Refresh-example", std::make_unique<RefreshExamples>());
    t->bindWidget("Format-example", std::make_unique<FormatExample>());
  }
};

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<LocalizedApplication>(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server{argc, argv, WTHTTP_CONFIGURATION};

    server.addEntryPoint(Wt::EntryPointType::Application, createApplication);

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << '\n';
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << '\n';
  }
}
