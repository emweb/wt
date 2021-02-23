// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QUERY_SELECTION_BOX_H_
#define QUERY_SELECTION_BOX_H_

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WSelectionBox.h>

template<class C>
class QuerySelectionBox : public Wt::WSelectionBox
{
public:
  QuerySelectionBox(std::shared_ptr<Wt::Dbo::QueryModel<C>> model)
    : model_(model)
  {
    setSelectionMode(Wt::SelectionMode::Extended);

    setModel(model_);
  }

  std::vector<C> selectedItems()
  {
    std::vector<C> results;
    for (int selectedIndex : selectedIndexes()) {
      results.push_back(model_->stableResultRow(selectedIndex));
    }
    return results;
  }

  void selectItems(std::vector<C> items)
  {
    std::set<int> selection;
    for (const C& item : items) {
      selection.insert(model_->indexOf(item));
    }
    setSelectedIndexes(selection);
  }

private:
  std::shared_ptr<Wt::Dbo::QueryModel<C>> model_;
};

#endif // QUERY_SELECTION_BOX_H_
