/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "model/TestDboObject.h"
#include "model/TestSession.h"
#include "EnumFormDelegate.h"
#include "PtrCollectionFormDelegate.h"
#include "PtrFormDelegate.h"
#include "TextEditFormDelegate.h"
#include "TextFormDelegate.h"

#include <Wt/WServer.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

#include <Wt/Form/Dbo/FormModel.h>
#include <Wt/Form/Dbo/FormView.h>

#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WIntValidator.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>

#include <fstream>

namespace Util {
  namespace Enum {
template<>
Wt::WString enumToString<TestDboObject::Enum>(TestDboObject::Enum value)
{
  switch (value) {
    case TestDboObject::Enum::Value1:
      return "Value 1";
    case TestDboObject::Enum::Value2:
      return "Value 2";
    case TestDboObject::Enum::Value3:
      return "Value 3";
    default:
      return "";
  }
}
  }
}

class TestDboModel : public Wt::Form::Dbo::FormModel<TestDboObject>
{
public:
  TestDboModel(Wt::Dbo::Session& session, Wt::Dbo::ptr<TestDboObject> item)
    : Wt::Form::Dbo::FormModel<TestDboObject>(session, item)
  {
    addAllDboColumnsAsFields();
    initDboValues();
  }
};

class TestDboView : public Wt::Form::Dbo::FormView<TestDboObject>
{
public:
  TestDboView(const Wt::WString& text, std::shared_ptr<TestDboModel> model)
    : Wt::Form::Dbo::FormView<TestDboObject>(text)
  {
    setFormDelegate("ptr", std::make_shared<PtrFormDelegate>(model->session()));
    setFormDelegate("ptr_collection", std::make_shared<PtrCollectionFormDelegate>(model->session()));
    setFormDelegate("other_string_value", std::make_shared<TextEditFormDelegate>());

    setFormModel(model);

    std::unique_ptr<Wt::WPushButton> saveBtn = std::make_unique<Wt::WPushButton>("Save");
    saveBtn->clicked().connect(this, &TestDboView::saveView);
    bindWidget("save-btn", std::move(saveBtn));

    saved().connect(this, &TestDboView::onSaveSuccess);
    validationFailed().connect(this, &TestDboView::onValidationFailed);
  }

protected:
  void customizeValidator(Wt::WFormModel::Field field, Wt::WValidator *validator) override
  {
    if (field == std::string("int_value")) {
      Wt::WIntValidator *intValidator = dynamic_cast<Wt::WIntValidator *>(validator);
      intValidator->setRange(0, 100);
    } else {
      Wt::Form::Dbo::FormView<TestDboObject>::customizeValidator(field, validator);
    }
  }

private:
  void saveView()
  {
    save();
  }

  void onSaveSuccess()
  {
    Wt::WMessageBox *box = addChild(std::make_unique<Wt::WMessageBox>("Saved", "Item has been saved.",
                                                                      Wt::Icon::Information, Wt::StandardButton::Ok));
    box->button(Wt::StandardButton::Ok)->clicked().connect(box, &Wt::WMessageBox::accept);
    box->finished().connect(std::bind([this, box]() {
      removeChild(box);
    }));
    box->show();
  }

  void onValidationFailed()
  {
    Wt::WMessageBox *box = addChild(std::make_unique<Wt::WMessageBox>("Failed", "Item could not be saved.",
                                                                      Wt::Icon::Warning, Wt::StandardButton::Ok));
    box->button(Wt::StandardButton::Ok)->clicked().connect(box, &Wt::WMessageBox::accept);
    box->finished().connect(std::bind([this, box]() {
      removeChild(box);
    }));
    box->show();
  }
};

class TestApplication : public Wt::WApplication
{
public:
  TestApplication(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& pool)
    : Wt::WApplication(env),
      session_(pool, true)
  {
    messageResourceBundle().use(appRoot() + "templates");
    messageResourceBundle().use(appRoot() + "strings");

    std::shared_ptr<Wt::WBootstrapTheme> theme = std::make_shared<Wt::WBootstrapTheme>();
    theme->setVersion(Wt::BootstrapVersion::v3);
    setTheme(theme);

    root()->addStyleClass("container");

    Wt::Dbo::Transaction t(session_);
    Wt::Dbo::ptr<TestDboObject> item = session_.find<TestDboObject>();

    std::shared_ptr<TestDboModel> model = std::make_shared<TestDboModel>(session_, item);

    root()->addNew<TestDboView>(Wt::WString::tr("dbo-form-view"), model);
  }

private:
  TestSession session_;
};

int main(int argc, char *argv[])
{
  Wt::WServer server(argc, argv);

  std::unique_ptr<Wt::Dbo::SqlConnectionPool> testDb = TestSession::createConnectionPool(server.appRoot() + "dbo-form.db");

  server.addEntryPoint(Wt::EntryPointType::Application, [db = testDb.get()](const Wt::WEnvironment& env) {
    return std::make_unique<TestApplication>(env, *db);
  }, "/");

  server.run();
}

