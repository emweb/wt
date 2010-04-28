// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_LAYOUT_ITEM_IMPL_H_
#define EXT_LAYOUT_ITEM_IMPL_H_

#include "Wt/WLayoutItemImpl"
#include "Wt/WObject"

namespace Wt {
  class WWidget;
  class DomElement;

  namespace Ext {
    class Container;
    class LayoutImpl;

class LayoutItemImpl : public WLayoutItemImpl, public WObject
{
public:
  LayoutItemImpl();
  virtual ~LayoutItemImpl();

  virtual void update(WLayoutItem *);

  virtual Container   *container() const;
  virtual WLayoutItem *layoutItem() const = 0;
  virtual WWidget     *parentWidget() const;

  virtual void setHint(const std::string& name, const std::string& value);

protected:
  virtual void createComponent(DomElement *parentContainer) = 0;
  virtual std::string componentVar() const = 0;
  virtual std::string componentId() const = 0;
  virtual void containerAddWidgets(Container *container) = 0;
  void addConfig(std::ostream& config);

  LayoutImpl *parentLayoutImpl() const;

  virtual void getLayoutChanges(const std::string& parentId,
				std::vector<DomElement *>& result);

private:
  friend class LayoutImpl;
  friend class Container;
};

  }
}

#endif // EXT_LAYOUT_ITEM_IMPL_H_
