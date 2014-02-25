#include "EditUsers.h"

#include <Wt/Dbo/Dbo>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WSignalMapper>
#include <Wt/WTemplate>
#include <Wt/WText>

using namespace Wt;
namespace dbo = Wt::Dbo;

EditUsers::EditUsers(dbo::Session& aSession, const std::string& basePath)
: session_(aSession), basePath_(basePath)
{
  setStyleClass("user-editor");
  setTemplateText(tr("edit-users-list"));
  limitEdit_  = new WLineEdit;
  WPushButton* goLimit = new WPushButton(tr("go-limit"));
  goLimit->clicked().connect(SLOT(this,EditUsers::limitList));
  bindWidget("limit-edit",limitEdit_);
  bindWidget("limit-button",goLimit);
  limitList();
}

void EditUsers::limitList()
{
  WContainerWidget* list = new WContainerWidget;
  bindWidget("user-list",list);

  typedef dbo::collection<dbo::ptr<User> > UserList;
  dbo::Transaction t(session_);
  UserList users = session_.find<User>().where("name like ?").bind("%"+limitEdit_->text()+"%").orderBy("name");

  for (UserList::const_iterator i = users.begin(); i != users.end(); ++i) {
    WText* t = new WText((*i)->name, list);
    t->setStyleClass("link");
    new WBreak(list);
    t->clicked().connect(boost::bind(&EditUsers::onUserClicked, this, (*i).id()));
  }
  if (!users.size())
    new WText(tr("no-users-found"),list);
}

void EditUsers::onUserClicked(Wt::Dbo::dbo_traits<User>::IdType id)
{
  wApp->setInternalPath(basePath_+"edituser/"+boost::lexical_cast<std::string>(id), true);
}


EditUser::EditUser(Wt::Dbo::Session& aSession)
: WTemplate(tr("edit-user")),
  session_(aSession),
  roleButton_(new WPushButton)
{
  bindWidget("role-button",roleButton_);
  roleButton_->clicked().connect(SLOT(this, EditUser::switchRole));
}

void EditUser::switchUser(Wt::Dbo::ptr<User> target)
{
  target_ = target;
  bindTemplate();
}

void EditUser::bindTemplate()
{
  bindString("username", target_->name);
  if (target_->role == User::Admin)
    roleButton_->setText(tr("demote-admin"));
  else
    roleButton_->setText(tr("promote-user"));
}

void EditUser::switchRole()
{
  dbo::Transaction t(session_);
  target_.reread();
  if (target_->role == User::Admin)
    target_.modify()->role = User::Visitor;
  else
    target_.modify()->role = User::Admin;
  t.commit();
  bindTemplate();
}
