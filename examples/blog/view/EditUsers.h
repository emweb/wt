#ifndef EDIT_USERS_H_
#define EDIT_USERS_H_

#include "../model/User.h"
#include "../model/Tag.h"
#include "../model/Token.h"
#include "../model/Comment.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTemplate.h>

namespace Wt {
  class WLineEdit;
  class WPushButton;
  namespace Dbo {
    class Session;
  }
}

class EditUsers : public Wt::WTemplate
{
public:
  EditUsers(Wt::Dbo::Session& aSesssion, const std::string& basePath);
private:
  void onUserClicked(Wt::Dbo::dbo_traits<User>::IdType id);
  void limitList();

  Wt::Dbo::Session  &session_;
  std::string    basePath_;
  Wt::WLineEdit     *limitEdit_;
};

class EditUser : public Wt::WTemplate
{
public:
  EditUser(Wt::Dbo::Session& aSesssion);
  void switchUser(Wt::Dbo::ptr<User> target);
private:
  void bindTemplate();
  void switchRole();

  Wt::Dbo::Session   &session_;
  Wt::Dbo::ptr<User>  target_;
  Wt::WPushButton    *roleButton_;
};

#endif
