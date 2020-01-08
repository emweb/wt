// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBREAK_H_
#define WBREAK_H_

#include <Wt/WWebWidget.h>

namespace Wt {

/*! \class WBreak Wt/WBreak.h Wt/WBreak.h
 *  \brief A widget that provides a line break between inline widgets.
 *
 * This is an \link WWidget::setInline(bool) inline \endlink widget
 * that provides a line break inbetween its sibling widgets (such as WText).
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;br /&gt;</tt> tag and
 * does not provide styling. Styling through CSS is not applicable.
 */
class WT_API WBreak : public WWebWidget
{
public:
  /*! \brief Construct a line break.
   */
  WBreak();

protected:
  virtual DomElementType domElementType() const override;
};

}

#endif // WBREAK_H_
