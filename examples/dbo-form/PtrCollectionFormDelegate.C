#include "PtrCollectionFormDelegate.h"

#include "model/TestDboObject.h"
#include "QuerySelectionBox.h"

#include <Wt/WLogger.h>

PtrCollectionFormDelegate::PtrCollectionFormDelegate(Wt::Dbo::Session& session)
  : session_(session)
{
}

std::unique_ptr<Wt::WWidget> PtrCollectionFormDelegate::createFormWidget()
{
  Wt::Dbo::Transaction t(session_);

  auto model = std::make_shared<Wt::Dbo::QueryModel<Wt::Dbo::ptr<TestDboPtr>>>();
  model->setQuery(session_.find<TestDboPtr>());
  model->addColumn("id", "ID");

  return std::make_unique<QuerySelectionBox<Wt::Dbo::ptr<TestDboPtr>>>(model);
}

void PtrCollectionFormDelegate::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  auto box = dynamic_cast<QuerySelectionBox<Wt::Dbo::ptr<TestDboPtr>> *>(edit);
  if (box) {
    model->setValue(field, box->selectedItems());
  } else {
    Wt::log("error") << "PtrCollectionFormDelegate" << ": " << "Could not cast edit to QuerySelectionBox!";
  }
}

void PtrCollectionFormDelegate::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  auto box = dynamic_cast<QuerySelectionBox<Wt::Dbo::ptr<TestDboPtr>> *>(edit);
  if (box) {
    Wt::cpp17::any v = model->value(field);

    try {
      std::vector<Wt::Dbo::ptr<TestDboPtr>> values = Wt::cpp17::any_cast<std::vector<Wt::Dbo::ptr<TestDboPtr>>>(v);
      box->selectItems(values);
    } catch (std::exception& e) {
      Wt::log("error") << "PtrCollectionFormDelegate" << ": " << "Could not convert value to TestDboPtr vector: " << e.what();
    }

  } else {
    Wt::log("error") << "PtrCollectionFormDelegate" << ": " << "Could not cast edit to QuerySelectionBox!";
  }
}
