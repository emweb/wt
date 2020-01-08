// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EMWEB_LOADING_INDICATOR_H_
#define EMWEB_LOADING_INDICATOR_H_

#include <Wt/WText.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLoadingIndicator.h>

class EmwebLoadingIndicator : public WContainerWidget,
                              public WLoadingIndicator
{
public:
  EmwebLoadingIndicator();

  virtual WWidget *widget() { return this; }
  virtual void setMessage(const WString& text);

private:
  WContainerWidget *cover_;
  WContainerWidget *center_;
  WText            *text_;
};

#endif // EMWEB_LOADING_INDICATOR_H_
