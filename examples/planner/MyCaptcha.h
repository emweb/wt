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

class MyCaptcha : public Wt::WContainerWidget {
 public:
  MyCaptcha(Wt::WContainerWidget* parent, const int width, const int height);
  
 private:
  void regenerate();
  void handleClick(const Wt::WMouseEvent& me);

  
 public:
  Wt::Signal<void> completed;

 private:
  ShapesWidget* shapesWidget_;
  Wt::WText* captchaMessage_;
};

#endif //MY_CAPTCHA_H_
