// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LOGINWIDGET_H_
#define LOGINWIDGET_H_

#include <Wt/WCompositeWidget>

namespace Wt {
  class WLineEdit;
  class WText;
  class WComboBox;
  class WTemplate;
  class WContainerWidget;
}

class LoginWidget : public Wt::WCompositeWidget
{
public:
  LoginWidget(Wt::WContainerWidget *parent = 0);
  
private:
  Wt::WText              *introText_;
  Wt::WLineEdit          *userName_;
  Wt::WLineEdit          *passWord_;
  Wt::WComboBox          *language_;

  Wt::WTemplate          *impl_;

  void checkCredentials();
};

#endif
