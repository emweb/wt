// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBADGE_H_
#define WBADGE_H_

#include "Wt/WText.h"

namespace Wt {

class WT_API WBadge: public WText
{
public:
  WBadge();
  WBadge(const WString& text);
  WBadge(const WString& text, TextFormat textFormat);

  bool isInline() const override { return true; }

protected:
  void updateDom(DomElement& element, bool all) override;
};

}

#endif //WBADGE_H_