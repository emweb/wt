// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLINKED_CSS_STYLE_SHEET_H_
#define WLINKED_CSS_STYLE_SHEET_H_

#include <vector>
#include <set>
#include <string>

#include <Wt/WGlobal.h>
#include <Wt/WLink.h>

namespace Wt {

class WStringStream;

/*! \class WLinkedCssStyleSheet Wt/WLinkedCssStyleSheet.h Wt/WLinkedCssStyleSheet.h
 *  \brief An external CSS style sheet.
 *
 * \sa WApplication::useStyleSheet()
 *
 * \ingroup style
 */
class WT_API WLinkedCssStyleSheet
{
public:
  /*! \brief Creates a new (external) style sheet reference.
   */
  WLinkedCssStyleSheet(const WLink& link, const std::string& media = "all");

  const WLink& link() const { return link_; }
  const std::string& media() const { return media_; }

  void cssText(WStringStream& out) const;

private:
  WLink link_;
  std::string media_;
};

}

#endif // WCSS_STYLE_SHEET_H_
