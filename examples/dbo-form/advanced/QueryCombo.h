// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QUERY_COMBO_H_
#define QUERY_COMBO_H_

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WComboBox.h>

template<class C>
class QueryCombo : public Wt::WComboBox
{
public:
  QueryCombo(std::shared_ptr<Wt::Dbo::QueryModel<C>> model)
    : model_(model)
  {
    setNoSelectionEnabled(true);

    setModel(model_);
  }

  C selectedItem()
  {
    if (currentIndex() < 0) {
      return C{};
    }

    return model_->stableResultRow(currentIndex());
  }

  void selectItem(C item)
  {
    setCurrentIndex(model_->indexOf(item));
  }

private:
  std::shared_ptr<Wt::Dbo::QueryModel<C>> model_;
};

#endif // QUERY_COMBO_H_
