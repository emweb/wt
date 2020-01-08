// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WICON_H_
#define WICON_H_

#include <Wt/WInteractWidget.h>

namespace Wt {

class WT_API WIcon : public WInteractWidget
{
public:
  WIcon();
  WIcon(const std::string& name);

  void setName(const std::string& name);
  std::string name() const { return name_; }

  void setSize(double factor);
  double size() const;

  static void loadIconFont();

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;

private:
  std::string name_;
  bool iconChanged_;
};

}

#endif // WICON_H_
