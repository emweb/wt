#include "PtrFormDelegate.h"

#include "model/TestDboObject.h"
#include "QueryCombo.h"

#include <Wt/WLogger.h>

PtrFormDelegate::PtrFormDelegate(Wt::Dbo::Session& session)
  : session_(session)
{
}

std::unique_ptr<Wt::WWidget> PtrFormDelegate::createFormWidget()
{
  Wt::Dbo::Transaction t(session_);

  auto model = std::make_shared<Wt::Dbo::QueryModel<Wt::Dbo::ptr<TestDboPtr>>>();
  model->setQuery(session_.find<TestDboPtr>());
  model->addColumn("id", "ID");

  return std::make_unique<QueryCombo<Wt::Dbo::ptr<TestDboPtr>>>(model);
}

void PtrFormDelegate::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  auto combo = dynamic_cast<QueryCombo<Wt::Dbo::ptr<TestDboPtr>> *>(edit);
  if (combo) {
    model->setValue(field, combo->selectedItem());
  } else {
    Wt::log("error") << "PtrFormDelegate" << ": " << "Could not cast edit to QueryCombo!";
  }
}

void PtrFormDelegate::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  auto combo = dynamic_cast<QueryCombo<Wt::Dbo::ptr<TestDboPtr>> *>(edit);
  if (combo) {
    Wt::cpp17::any v = model->value(field);

    try {
      Wt::Dbo::ptr<TestDboPtr> value = Wt::cpp17::any_cast<Wt::Dbo::ptr<TestDboPtr>>(v);
      combo->selectItem(value);
    } catch (std::exception& e) {
      Wt::log("error") << "PtrFormDelegate" << ": " << "Could not convert value to TestDboPtr: " << e.what();
    }

  } else {
    Wt::log("error") << "PtrFormDelegate" << ": " << "Could not cast edit to QueryCombo!";
  }
}
