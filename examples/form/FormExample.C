/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FormExample.h"
#include "Form.h"

#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WStringUtil.h>

FormExample::FormExample()
    : WContainerWidget()
{
  WContainerWidget *langLayout = this->addWidget(std::make_unique<WContainerWidget>());
  langLayout->setContentAlignment(AlignmentFlag::Right);
  langLayout->addWidget(std::make_unique<WText>(tr("language")));

  const char *lang[] = { "en", "nl" };

  for (int i = 0; i < 2; ++i) {
    WText *t = langLayout->addWidget(std::make_unique<WText>(lang[i]));
    t->setMargin(5);
    t->clicked().connect(std::bind(&FormExample::changeLanguage, this, t));

    languageSelects_.push_back(t);
  }

  /*
   * Start with the reported locale, if available
   */
  setLanguage(wApp->locale().name());

  Form *form = this->addWidget(std::make_unique<Form>());
  form->setMargin(20);
}

void FormExample::setLanguage(const std::string lang)
{
  bool haveLang = false;

  for (auto i : languageSelects_) {
    WText *t = i;

    // prefix match, e.g. en matches en-us.
    bool isLang = lang.find(t->text().toUTF8()) == 0;
    t->setStyleClass(isLang ? "langcurrent" : "lang");

    haveLang = haveLang || isLang;
  }

  if (!haveLang) {
    languageSelects_[0]->setStyleClass("langcurrent");
    WApplication::instance()
      ->setLocale(languageSelects_[0]->text().toUTF8());
  } else
    WApplication::instance()->setLocale(lang);
}

void FormExample::changeLanguage(WText *t)
{
  setLanguage(t->text().toUTF8());
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = std::make_unique<WApplication>(env);
  app->messageResourceBundle().use(WApplication::appRoot() + "form-example");
  app->setTitle("Form example");

  app->root()->addWidget(std::make_unique<FormExample>());

  WCssDecorationStyle langStyle;
  langStyle.font().setSize(FontSize::Smaller);
  langStyle.setCursor(Cursor::PointingHand);
  langStyle.setForegroundColor(WColor("blue"));
  langStyle.setTextDecoration(TextDecoration::Underline);
  app->styleSheet().addRule(".lang", langStyle);

  langStyle.setCursor(Cursor::Arrow);
  langStyle.font().setWeight(FontWeight::Bold);
  app->styleSheet().addRule(".langcurrent", langStyle);

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

