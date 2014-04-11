/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PostView.h"
#include "BlogView.h"
#include "EditUsers.h"
#include "BlogLoginWidget.h"

#include "../model/BlogSession.h"
#include "../model/Comment.h"
#include "../model/Post.h"
#include "../model/Tag.h"
#include "../model/Token.h"
#include "../model/User.h"

#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>

#include <Wt/Auth/PasswordService>
#include <Wt/Auth/PasswordVerifier>

#include <Wt/Dbo/backend/Sqlite3>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace Wt;
namespace dbo = Wt::Dbo;

class BlogImpl : public WContainerWidget
{
public:
  BlogImpl(const std::string& basePath, dbo::SqlConnectionPool& connectionPool,
	   const std::string& rssFeedUrl, BlogView *blogView)
    : basePath_(basePath),
      rssFeedUrl_(rssFeedUrl),
      session_(connectionPool),
      panel_(0),
      authorPanel_(0),
      users_(0),
      userEditor_(0),
      mustLoginWarning_(0),
      mustBeAdministratorWarning_(0),
      invalidUser_(0)
  {
    WApplication *app = wApp;

    app->messageResourceBundle().use(WApplication::appRoot() + "blog");
    app->useStyleSheet("/css/blog.css");
    app->useStyleSheet("/css/asciidoc.css");
    app->internalPathChanged().connect(this, &BlogImpl::handlePathChange);

    loginStatus_ = new WTemplate(tr("blog-login-status"), this);
    panel_ = new WStackedWidget(this);
    items_ = new WContainerWidget(this);

    session_.login().changed().connect(this, &BlogImpl::onUserChanged);

    loginWidget_ = new BlogLoginWidget(session_, basePath);
    loginWidget_->hide();

    WText *loginLink = new WText(tr("login"));
    loginLink->setStyleClass("link");
    loginLink->clicked().connect(loginWidget_, &WWidget::show);
    loginLink->clicked().connect(loginLink, &WWidget::hide);

    WText *registerLink = new WText(tr("Wt.Auth.register"));
    registerLink->setStyleClass("link");
    registerLink->clicked().connect(loginWidget_,
				    &BlogLoginWidget::registerNewUser);

    WAnchor* archiveLink = new WAnchor
      (WLink(WLink::InternalPath, basePath_ + "all"), tr("archive"));

    loginStatus_->bindWidget("login", loginWidget_);
    loginStatus_->bindWidget("login-link", loginLink);
    loginStatus_->bindWidget("register-link", registerLink);
    loginStatus_->bindString("feed-url", rssFeedUrl_);
    loginStatus_->bindWidget("archive-link", archiveLink);

    onUserChanged();

    loginWidget_->processEnvironment();
  }

  void onUserChanged() {
    if (session_.login().loggedIn())
      loggedIn();
    else
      loggedOut();
  }

  BlogSession& session() { return session_; }

  void setInternalBasePath(const std::string& basePath) {
    basePath_ = basePath;
    refresh();
  }

  ~BlogImpl() {
    clear();
  }

private:
  std::string basePath_, rssFeedUrl_;
  BlogSession session_;
  BlogLoginWidget *loginWidget_;

  WStackedWidget* panel_;
  WTemplate *authorPanel_;
  EditUsers *users_;
  EditUser  *userEditor_;
  WTemplate *mustLoginWarning_;
  WTemplate *mustBeAdministratorWarning_;
  WTemplate *invalidUser_;
  WTemplate *loginStatus_;
  WContainerWidget *items_;

  void logout() {
    session_.login().logout();
  }

  void loggedOut() {
    loginStatus_->bindEmpty("profile-link");
    loginStatus_->bindEmpty("author-panel-link");
    loginStatus_->bindEmpty("userlist-link");

    loginStatus_->resolveWidget("login")->hide();
    loginStatus_->resolveWidget("login-link")->show();
    loginStatus_->resolveWidget("register-link")->show();
    
    refresh();
    panel_->hide();
  }

  void loggedIn() {
    WApplication::instance()->changeSessionId();

    refresh();

    loginStatus_->resolveWidget("login")->show();
    loginStatus_->resolveWidget("login-link")->hide();
    loginStatus_->resolveWidget("register-link")->hide();

    WText *profileLink = new WText(tr("profile"));
    profileLink->setStyleClass("link");
    profileLink->clicked().connect(this, &BlogImpl::editProfile);

    dbo::ptr<User> user = session().user();

    if (user->role == User::Admin) {
      WText *editUsersLink = new WText(tr("edit-users"));
      editUsersLink->setStyleClass("link");
      editUsersLink->clicked().connect(SLOT(this, BlogImpl::editUsers));
      loginStatus_->bindWidget("userlist-link", editUsersLink);

      WText *authorPanelLink = new WText(tr("author-post"));
      authorPanelLink->setStyleClass("link");
      authorPanelLink->clicked().connect(SLOT(this, BlogImpl::authorPanel));
      loginStatus_->bindWidget("author-panel-link", authorPanelLink);
    } else {
      loginStatus_->bindEmpty("userlist-link");
      loginStatus_->bindEmpty("author-panel-link");
    }
 
    loginStatus_->bindWidget("profile-link", profileLink);
 
    bindPanelTemplates();
  }

  void bindPanelTemplates() {
    if (!session_.user())
      return;

    dbo::Transaction t(session_);

    if (authorPanel_) {
      WPushButton *newPost = new WPushButton(tr("new-post"));
      newPost->clicked().connect(SLOT(this, BlogImpl::newPost));
      WContainerWidget *unpublishedPosts = new WContainerWidget();
      showPosts(session_.user()->allPosts(Post::Unpublished), unpublishedPosts);

      authorPanel_->bindString("user", session_.user()->name);
      authorPanel_->bindInt("unpublished-count",
			    (int)session_.user()->allPosts(Post::Unpublished)
			    .size());
      authorPanel_->bindInt("published-count",
			    (int)session_.user()->allPosts(Post::Published)
			    .size());
      authorPanel_->bindWidget("new-post", newPost);
      authorPanel_->bindWidget("unpublished-posts", unpublishedPosts);
    }

    t.commit();
  }
 
  void editUsers() {
    panel_->show();

    if (!users_) {
      users_ = new EditUsers(session_, basePath_);
      panel_->addWidget(users_);
      bindPanelTemplates();
    }

    panel_->setCurrentWidget(users_);
  }

 void authorPanel() {
    panel_->show();
    if (!authorPanel_)
    {
      authorPanel_ = new WTemplate(tr("blog-author-panel"));
      panel_->addWidget(authorPanel_);
      bindPanelTemplates();
    }
    panel_->setCurrentWidget(authorPanel_);
  }

  void editProfile() {
    loginWidget_->letUpdatePassword(session_.login().user(), true);
  }

  void refresh() {
    handlePathChange(wApp->internalPath());
  }

  void handlePathChange(const std::string& path) {
    WApplication *app = wApp;

    if (app->internalPathMatches(basePath_)) {
      dbo::Transaction t(session_);

      std::string path = app->internalPathNextPart(basePath_);

      items_->clear();

      if (users_) {
	delete users_;
	users_ = 0;
      }

      if (path.empty())
	showPosts(session_.find<Post>
		  ("where state = ? "
		   "order by date desc "
		   "limit 10").bind(Post::Published), items_);

      else if (path == "author") {
	std::string author = app->internalPathNextPart(basePath_ + path + '/');
	dbo::ptr<User> user = findUser(author);

	if (user)
	  showPosts(user);
	else
	  showError(tr("blog-no-author").arg(author));
      } else if (path == "edituser") {
	editUser(app->internalPathNextPart(basePath_ + path + '/'));
      } else if (path == "all") {
	showArchive(items_);
      } else {
	std::string remainder = app->internalPath().substr(basePath_.length());
	showPostsByDateTopic(remainder, items_);
      }

      t.commit();
    }
  }

  void editUser(const std::string& ids) {
    if (!checkLoggedIn()) return;
    if (!checkAdministrator()) return;
    dbo::dbo_traits<User>::IdType id;
    try {
      id = boost::lexical_cast<dbo::dbo_traits<User>::IdType>(ids);
    } catch (boost::bad_lexical_cast&) {
      id = dbo::dbo_traits<User>::invalidId();
    }
    panel_->show();
    try {
      dbo::Transaction t(session_);
      dbo::ptr<User> target(session_.load<User>(id));
      if (!userEditor_)
	panel_->addWidget(userEditor_ = new EditUser(session_));
      userEditor_->switchUser(target);
      panel_->setCurrentWidget(userEditor_);
    }
    catch (Dbo::ObjectNotFoundException) {
      if (!invalidUser_)
	panel_->addWidget(invalidUser_ = new WTemplate(tr("blog-invaliduser")));
      panel_->setCurrentWidget(invalidUser_);
    }
  }

  bool checkLoggedIn()
  {
    if (session_.user()) return true;
    panel_->show();
    if (!mustLoginWarning_)
      panel_->addWidget(mustLoginWarning_ = new WTemplate(tr("blog-mustlogin")));
    panel_->setCurrentWidget(mustLoginWarning_);
    return false;
  }

  bool checkAdministrator()
  {
    if (session_.user() && (session_.user()->role == User::Admin)) return true;
    panel_->show();
    if (!mustBeAdministratorWarning_)
      panel_->addWidget(mustBeAdministratorWarning_ = new WTemplate(tr("blog-mustbeadministrator")));
    panel_->setCurrentWidget(mustBeAdministratorWarning_);
    return false;
  }

  dbo::ptr<User> findUser(const std::string& name) {
    return session_.find<User>("where name = ?").bind(name);
  }
  
  bool yearMonthDiffer(const WDateTime& dt1, const WDateTime& dt2) {
    return dt1.date().year() != dt2.date().year()
      || dt1.date().month() != dt2.date().month();
  }

  void showArchive(WContainerWidget *parent) {
    static const char* dateFormat = "MMMM yyyy";
    
    new WText(tr("archive-title"), parent);

    Posts posts = session_.find<Post>("order by date desc");

    WDateTime formerDate;
    for (Posts::const_iterator i = posts.begin(); i != posts.end(); ++i) {
      if ((*i)->state != Post::Published)
	continue;

      if (formerDate.isNull() 
	  || yearMonthDiffer(formerDate, (*i)->date)) {
	WText *title
	  = new WText((*i)->date.date().toString(dateFormat), parent);
	title->setStyleClass("archive-month-title");
      }
      
      WAnchor *a = new WAnchor(WLink(WLink::InternalPath,
				     basePath_ + (*i)->permaLink()),
			       (*i)->title, parent);
      a->setInline(false);
      
      formerDate = (*i)->date;
    }
  }

  void showPostsByDateTopic(const std::string& path,
			    WContainerWidget *parent) {
    std::vector<std::string> parts;
    boost::split(parts, path, boost::is_any_of("/"));

    WDate lower, upper;
    try {
      int year = boost::lexical_cast<int>(parts[0]);

      if (parts.size() > 1) {
	int month = boost::lexical_cast<int>(parts[1]);

	if (parts.size() > 2) {
	  int day = boost::lexical_cast<int>(parts[2]);

	  lower.setDate(year, month, day);
	  upper = lower.addDays(1);
	} else {
	  lower.setDate(year, month, 1);
	  upper = lower.addMonths(1);
	}
      } else {
	lower.setDate(year, 1, 1);
	upper = lower.addYears(1);
      }
    } catch (boost::bad_lexical_cast& e) {
      showError(tr("blog-no-post"));
      return;
    }

    Posts posts = session_.find<Post>
      ("where date >= ? "
       "and date < ? "
       "and (state = ? or author_id = ?)")
      .bind(WDateTime(lower))
      .bind(WDateTime(upper))
      .bind(Post::Published)
      .bind(session_.user().id());

    if (parts.size() > 3) {
      std::string title = parts[3];

      for (Posts::const_iterator i = posts.begin(); i != posts.end(); ++i)
	if ((*i)->titleToUrl() == title) {
	  showPost(*i, PostView::Detail, parent);
	  return;
	}

      showError(tr("blog-no-post"));
    } else {
      showPosts(posts, parent);
    }
  }

  void showPosts(dbo::ptr<User> user) {
    showPosts(user->latestPosts(), items_);
  }

  void newPost() {
    dbo::Transaction t(session_);

	authorPanel();
	WContainerWidget *unpublishedPosts
      = authorPanel_->resolve<WContainerWidget *>("unpublished-posts");

    dbo::ptr<Post> post(new Post);

    Post *p = post.modify();
    p->state = Post::Unpublished;
    p->author = session_.user();
    p->title = "Title";
    p->briefSrc = "Brief ...";
    p->bodySrc = "Body ...";

    showPost(post, PostView::Edit, unpublishedPosts);

    t.commit();
  }

  void showPosts(const Posts& posts, WContainerWidget *parent) {
    for (Posts::const_iterator i = posts.begin(); i != posts.end(); ++i)
      showPost(*i, PostView::Brief, parent);
  }

  void showPost(const dbo::ptr<Post> post, PostView::RenderType type,
		WContainerWidget *parent) {
    parent->addWidget(new PostView(session_, basePath_, post, type));
  }

  void showError(const WString& msg) {
    items_->addWidget(new WText(msg));
  }
};

BlogView::BlogView(const std::string& basePath, dbo::SqlConnectionPool& db,
		   const std::string& rssFeedUrl, WContainerWidget *parent)
  : WCompositeWidget(parent),
    userChanged_(this)
{
  impl_ = new BlogImpl(basePath, db, rssFeedUrl, this);
  setImplementation(impl_);
}

void BlogView::setInternalBasePath(const std::string& basePath)
{
  impl_->setInternalBasePath(basePath);
}

WString BlogView::user()
{
  if (impl_->session().user())
    return impl_->session().user()->name;
  else
    return WString::Empty;
}
