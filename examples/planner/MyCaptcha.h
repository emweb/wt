// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MY_CAPTCHA_H_
#define MY_CAPTCHA_H_

#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include <Wt/WText>

#include "ShapesWidget.h"

class MyCaptcha : public Wt::WContainerWidget
{
public:
  MyCaptcha(Wt::WContainerWidget* parent, const int width, const int height);

  Wt::Signal<void>& completed() { return completed_; }
  
private:
  Wt::Signal<void> completed_;
  ShapesWidget* shapesWidget_;
  Wt::WText* captchaMessage_;

  void regenerate();
  void handleClick(const Wt::WMouseEvent& me);  
};

#endif //MY_CAPTCHA_H_
