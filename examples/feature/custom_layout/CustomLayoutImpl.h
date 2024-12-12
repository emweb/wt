/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CUSTOM_LAYOUT_IMPL_H_
#define CUSTOM_LAYOUT_IMPL_H_

#include <Wt/StdLayoutImpl.h>

using namespace Wt;

class CustomLayoutImpl : public StdLayoutImpl
{
public:
  CustomLayoutImpl(WLayout *layout);

  ~CustomLayoutImpl();

  int minimumWidth() const override;
  int minimumHeight() const override;
  int maximumWidth() const override;
  int maximumHeight() const override;

  void itemAdded(WLayoutItem *item) override;
  void itemRemoved(WLayoutItem *item) override;

  void updateDom(DomElement& parent) override;

  DomElement *createDomElement(DomElement *parent, bool fitWidth,
                               bool fitHeight, WApplication *app) override;

  bool itemResized(WLayoutItem *item) override;
  bool parentResized() override;

private:
  std::vector<WLayoutItem *> items_;
  std::vector<WLayoutItem *> addedItems_;
  std::vector<std::string> removedItems_;
  std::string elId_;

  DomElement *createElement(WLayoutItem *item, WApplication *app);
  
};

#endif // CUSTOM_LAYOUT_IMPL_H_
