// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ENUM_COMBO_H_
#define ENUM_COMBO_H_

#include <Wt/WComboBox.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>

template<class C>
class EnumModel : public Wt::WStandardItemModel
{
public:
  EnumModel()
  {
  }

  void addItem(C item, Wt::WString label)
  {
    std::unique_ptr<Wt::WStandardItem> modelItem = std::make_unique<Wt::WStandardItem>();
    modelItem->setData(label, Wt::ItemDataRole::Display);
    modelItem->setData(item, Wt::ItemDataRole::User);
    appendRow(std::move(modelItem));
  }

  C getItem(int row) const
  {
    auto value = data(row, 0, Wt::ItemDataRole::User);
    if (Wt::cpp17::any_has_value(value)) {
      return Wt::cpp17::any_cast<C>(value);
    } else {
      return C{};
    }
  }
};

template<class C>
class EnumCombo : public Wt::WComboBox
{
public:
  EnumCombo(const std::shared_ptr<EnumModel<C>>& model)
    : model_(model)
  {
    setModel(model);
  }

  C selectedItem()
  {
    return model_->getItem(currentIndex());
  }

  void selectItem(C item)
  {
    setCurrentIndex(indexOf(item));
  }

private:
  std::shared_ptr<EnumModel<C>> model_;

  int indexOf(C item)
  {
    for (int i = 0; i < model_->rowCount(); ++i) {
      C row = model_->getItem(i);
      if (row == item) {
        return i;
      }
    }
    return -1;
  }
};

#endif // ENUM_COMBO_H_
