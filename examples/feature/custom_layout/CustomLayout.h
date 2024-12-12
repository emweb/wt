/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CUSTOM_LAYOUT_H
#define CUSTOM_LAYOUT_H

#include <Wt/WLayout.h>

#include <vector>

using namespace Wt;

class CustomLayout : public WLayout
{
public:
  CustomLayout();

  void addItem(std::unique_ptr<WLayoutItem> item) override;
  std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) override;
  WLayoutItem *itemAt(int index) const override;
  int indexOf(WLayoutItem *item) const override;
  int count() const override;

  void iterateWidgets(const HandleWidgetMethod& method) const override;
  bool implementationIsFlexLayout() const override;

private:
  struct Item {
    std::unique_ptr<WLayoutItem> item_;
    Item(std::unique_ptr<WLayoutItem> item = nullptr);
    Item(Item&& other) = default;
    Item& operator=(Item&& other) = default;
  };
  std::vector<Item> items_;
};

#endif // CUSTOM_LAYOUT_H
