// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LOGIN_WIDGET_H_
#define LOGIN_WIDGET_H_

#include <Wt/WCompositeWidget>

namespace Wt {
  class WLineEdit;
  class WText;
  class WComboBox;
  class WTemplate;
  class WContainerWidget;
}

class Session;

class LoginWidget : public Wt::WCompositeWidget
{
public:
  LoginWidget(Session *session, Wt::WContainerWidget *parent = 0);

  Wt::Signal<>& loggedIn() { return loggedIn_; }
  
private:
  Wt::WText              *introText_;
  Wt::WLineEdit          *userName_;
  Wt::WLineEdit          *passWord_;
  Wt::WComboBox          *language_;

  Wt::WTemplate          *impl_;

  Session                *session_;

  Wt::Signal<>            loggedIn_;

  void checkCredentials();
};

#endif //LOGIN_WIDGET_H_
