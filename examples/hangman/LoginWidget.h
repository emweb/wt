/* this is a -*-C++-*- file
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LOGINWIDGET_H_
#define LOGINWIDGET_H_

#include <Wt/WContainerWidget>

#include "Dictionary.h"

namespace Wt {
  class WLineEdit;
  class WText;
  class WComboBox;
}

using namespace Wt;

class LoginWidget : public WContainerWidget
{
public:
  LoginWidget(WContainerWidget *parent = 0);
  
  Wt::Signal<Dictionary> startPlaying;
  
private:
  WText     *IntroText;
  WLineEdit *Username;
  WLineEdit *Password;
  WComboBox *Language;
  
  Dictionary   Dict;
  
  void confirmLogin(const std::string text);
  
  void checkCredentials();
  
  void onStartClicked();
};

#endif
