// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EMWEB_LOADING_INDICATOR_H_
#define EMWEB_LOADING_INDICATOR_H_

#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WLoadingIndicator>

class EmwebLoadingIndicator : public Wt::WContainerWidget, 
			      public Wt::WLoadingIndicator
{
public:
  EmwebLoadingIndicator();

  virtual Wt::WWidget *widget() { return this; }
  virtual void setMessage(const Wt::WString& text);

private:
  Wt::WContainerWidget *cover_;
  Wt::WContainerWidget *center_;
  Wt::WText            *text_;
};

#endif // EMWEB_LOADING_INDICATOR_H_
