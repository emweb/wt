/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/Transaction.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include <Wt/WFlags.h>

#include <Wt/Form/Dbo/FormModel.h>
#include <Wt/Form/Dbo/FormView.h>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDate.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

#include <memory>
#include <string>

class Contact
{
public:
  // Map each data member to a database column. FormModel below also
  // uses the column names as field names.
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name_, "name");
    Wt::Dbo::field(a, email_, "email");
    Wt::Dbo::field(a, age_, "age");
    Wt::Dbo::field(a, dateOfBirth_, "date_of_birth");
    Wt::Dbo::field(a, balance_, "balance");
    Wt::Dbo::field(a, receivesNewsletter_, "receives_newsletter");
  }

private:
  std::string name_;
  std::string email_;
  int age_ = 0;
  // WDate selects a WDateEdit and WDateValidator automatically.
  // A default-constructed date leaves the editor empty.
  Wt::WDate dateOfBirth_;
  double balance_ = 0.0;
  bool receivesNewsletter_ = false;
};

class ContactSession : public Wt::Dbo::Session
{
public:
  // Every application has its own session, while all sessions share the
  // server-owned connection pool.
  explicit ContactSession(Wt::Dbo::SqlConnectionPool& pool)
  {
    setConnectionPool(pool);
    mapClass<Contact>("contact");
  }

  void initializeDatabase()
  {
    // Create the schema once, before the server starts accepting
    // requests.
    Wt::Dbo::Transaction transaction(*this);

    const int tables = query<int>(
      "select count(*) from sqlite_master "
      "where type = 'table' and name = 'contact'");
    if (tables == 0) {
      createTables();
    }

    transaction.commit();
  }
};

class ContactFormView : public Wt::Form::Dbo::FormView<Contact>
{
public:
  explicit ContactFormView(std::shared_ptr<Wt::Form::Dbo::FormModel<Contact> > model)
    : Wt::Form::Dbo::FormView<Contact>(Wt::WString::tr("contact-form"))
  {
    // contact-form is a localized WTemplate resource. Its placeholders
    // have the same names as the database columns mapped by
    // Contact::persist(). setFormModel() creates and binds the default
    // editor delegates for the fields added to the model.
    setFormModel(model);
    bindEmpty("status");

    auto saveButton = std::make_unique<Wt::WPushButton>("Save");
    // FormView::save() first validates the fields and then stores the
    // model.
    saveButton->clicked().connect(this, &ContactFormView::save);
    bindWidget("save-button", std::move(saveButton));

    saved().connect([this] {
      bindString("status", "Saved.");
    });
    validationFailed().connect([this] {
      bindString("status", "Please correct the highlighted fields.");
    });
  }
};

class DboFormSimpleApplication : public Wt::WApplication
{
public:
  DboFormSimpleApplication(const Wt::WEnvironment& env,
                           Wt::Dbo::SqlConnectionPool& pool)
    : Wt::WApplication(env),
      session_(pool)
  {
    setTitle("Simple Dbo Form");
    messageResourceBundle().use(appRoot() + "templates");

    // Without a persistent object, FormModel starts with Contact's
    // default values and inserts a new row when FormView saves it.
    auto model = std::make_shared<Wt::Form::Dbo::FormModel<Contact> >(session_);
    model->addAllDboColumnsAsFields();
    model->initDboValues();
    root()->addNew<ContactFormView>(model);
  }

  ~DboFormSimpleApplication() override
  {
    // The view owns the shared form model. Destroy both before the
    // application destroys its Dbo session.
    root()->clear();
  }

private:
  ContactSession session_;
};

int main(int argc, char *argv[])
{
  // The pool must outlive the server and its application sessions.
  std::unique_ptr<Wt::Dbo::SqlConnectionPool> pool;
  Wt::WServer server(argc, argv);

  auto connection = std::make_unique<Wt::Dbo::backend::Sqlite3>(server.appRoot() + "dbo-form-simple.db");
  // FixedSqlConnectionPool gives concurrent application sessions their
  // own database connection.
  pool = std::make_unique<Wt::Dbo::FixedSqlConnectionPool>(std::move(connection), 10);
  {
    // Initialize the schema with a temporary session before serving
    // requests.
    ContactSession session(*pool);
    session.initializeDatabase();
  }

  server.addEntryPoint(Wt::EntryPointType::Application,
    [db = pool.get()](const Wt::WEnvironment& env) {
      return std::make_unique<DboFormSimpleApplication>(env, *db);
    }, "/");

  server.run();
}
