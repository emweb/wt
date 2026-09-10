// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ENUM_FORM_DELEGATE_H_
#define ENUM_FORM_DELEGATE_H_

#include <Wt/Form/WFormDelegate.h>
#include <Wt/WLogger.h>

#include "EnumCombo.h"

#include <type_traits>

namespace Util {
  namespace Enum {
template<typename Enum>
Wt::WString enumToString(Enum)
{
  throw Wt::WException("Enum to string not implemented!");
}
  }
}

namespace Wt {
  namespace Form {
/*! \brief Form delegate form enum
 *
 * This will create a specialized WComboBox to display the Enum
 * value in the View.
 */
template<typename Enum>
class WFormDelegate<Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> : public WAbstractFormDelegate
{
public:
  WFormDelegate() = default;

  std::unique_ptr<Wt::WWidget> createFormWidget() override
  {
    std::shared_ptr<EnumModel<Enum>> model = std::make_shared<EnumModel<Enum>>();
    for (std::underlying_type_t<Enum> i = 0; i < static_cast<std::underlying_type_t<Enum>>(Enum::LAST_ELEMENT); ++i) {
      Enum value = static_cast<Enum>(i);
      model->addItem(value, Util::Enum::enumToString(value));
    }
    return std::make_unique<EnumCombo<Enum>>(model);
  }

  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override
  {
    EnumCombo<Enum> *combo = dynamic_cast<EnumCombo<Enum>*>(edit);
    if (combo) {
      model->setValue(field, combo->selectedItem());
    } else {
      Wt::log("error") << "WFormDelegate" << ": " << "Could not cast edit to EnumCombo!";
    }
  }

  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override
  {
    EnumCombo<Enum> *combo = dynamic_cast<EnumCombo<Enum>*>(edit);
    if (combo) {
      Wt::cpp17::any v = model->value(field);

      try {
        Enum value = Wt::cpp17::any_cast<Enum>(v);
        combo->selectItem(value);
      } catch (std::exception& e) {
        Wt::log("error") << "WFormDelegate" << ": " << "Could not convert value to Enum: " << e.what();
      }

    } else {
      Wt::log("error") << "WFormDelegate" << ": " << "Could not cast edit to EnumCombo!";
    }
  }
};
  }
}


#endif // ENUM_FORM_DELEGATE_H_
