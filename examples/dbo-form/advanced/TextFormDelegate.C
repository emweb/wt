#include "TextFormDelegate.h"

#include <Wt/WTextArea.h>

namespace Wt {
  namespace Form {

WFormDelegate<Text, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Text, void>::createFormWidget()
{
  return std::make_unique<Wt::WTextArea>();
}

void WFormDelegate<Text, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Text text;
  text.content = edit->valueText();
  model->setValue(field, text);
}

void WFormDelegate<Text, void>::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Text text = Wt::cpp17::any_cast<Text>(model->value(field));
  edit->setValueText(text.content);
}
  }
}
