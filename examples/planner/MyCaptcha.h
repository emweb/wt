// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MY_CAPTCHA_H_
#define MY_CAPTCHA_H_

#include <Wt/WSignal.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

#include "ShapesWidget.h"

using namespace Wt;

class MyCaptcha : public WContainerWidget
{
public:
  MyCaptcha(const int width, const int height);

  Signal<>& completed() { return completed_; }
  
private:
  Signal<>        completed_;
  ShapesWidget   *shapesWidget_;
  WText          *captchaMessage_;

  void regenerate();
  void handleClick(const WMouseEvent& me);
};

#endif //MY_CAPTCHA_H_
