// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_LAYOUT_IMPL_H_
#define EXT_LAYOUT_IMPL_H_

#include "Wt/Ext/LayoutItemImpl.h"

namespace Wt {

  class WLayout;
  class WLayoutItem;

  namespace Ext {

class WidgetItem;

class LayoutImpl : public LayoutItemImpl
{
public:
  LayoutImpl(WLayout *layout);
  virtual ~LayoutImpl();

  void updateAddItem(WLayoutItem *);
  void updateRemoveItem(WLayoutItem *);

  virtual Container   *container() const;
  virtual WLayoutItem *layoutItem() const;
  
protected:
  virtual void createComponent(DomElement *parentContainer);
  virtual void containerAddWidgets(Container *container);
  virtual std::string componentVar() const;
  virtual std::string componentId() const;

  virtual void createConfig(std::ostream& config);

  WLayout *layout() const { return layout_; }

private:
  WLayout                   *layout_;
  Container                 *container_;
  std::vector<WLayoutItem *> itemsAdded_;
  std::string                jsUpdates_;

  std::string               parentContainerVar() const;

  virtual void createComponents(DomElement *parentContainer);
  virtual void addLayoutConfig(LayoutItemImpl *item, std::ostream& config);
  virtual void getLayoutChanges(const std::string& parentId,
				std::vector<DomElement *>& result);

  static LayoutItemImpl *getImpl(WLayoutItem *item); 

  void addUpdateJS(const std::string& js);
  std::string containerRef() const;
  std::string elRef() const;
  std::string elVar() const;

  void setContainer(Container *container);

  friend class Container;
  friend class LayoutItemImpl;
};

  }
}

#endif // EXT_LAYOUT_H_
