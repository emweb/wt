/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FormExample.h"
#include "Form.h"

#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WStringUtil>

FormExample::FormExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  WContainerWidget *langLayout = new WContainerWidget(this);
  langLayout->setContentAlignment(AlignRight);
  new WText(tr("language"), langLayout);

  const char *lang[] = { "en", "nl" };

  for (int i = 0; i < 2; ++i) {
    WText *t = new WText(widen(lang[i]), langLayout);
    t->setMargin(5);
    t->clicked().connect(this, &FormExample::changeLanguage);

    languageSelects_.push_back(t);
  }

  /*
   * Start with the reported locale, if available
   */
  setLanguage(wApp->locale().name());

  Form *form = new Form(this);
  form->setMargin(20);
}

void FormExample::setLanguage(const std::string lang)
{
  bool haveLang = false;

  for (unsigned i = 0; i < languageSelects_.size(); ++i) {
    WText *t = languageSelects_[i];

    // prefix match, e.g. en matches en-us.
    bool isLang = lang.find(narrow(t->text().value())) == 0;
    t->setStyleClass(isLang ? L"langcurrent" : L"lang");

    haveLang = haveLang || isLang;
  }

  if (!haveLang) {
    languageSelects_[0]->setStyleClass(L"langcurrent");
    WApplication::instance()
      ->setLocale(narrow(languageSelects_[0]->text().value()));
  } else
    WApplication::instance()->setLocale(lang);
}

void FormExample::changeLanguage()
{
  WText *t = (WText *)sender();
  setLanguage(narrow(t->text().value()));
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->messageResourceBundle().use(WApplication::appRoot() + "form-example");
  app->setTitle("Form example");

  app->root()->addWidget(new FormExample());

  WCssDecorationStyle langStyle;
  langStyle.font().setSize(WFont::Smaller);
  langStyle.setCursor(PointingHandCursor);
  langStyle.setForegroundColor(blue);
  langStyle.setTextDecoration(WCssDecorationStyle::Underline);
  app->styleSheet().addRule(".lang", langStyle);

  langStyle.setCursor(ArrowCursor);
  langStyle.font().setWeight(WFont::Bold);
  app->styleSheet().addRule(".langcurrent", langStyle);

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

