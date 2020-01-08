/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PostView.h"
#include "BlogView.h"
#include "EditUsers.h"
#include "BlogLoginWidget.h"

#include "../model/BlogSession.h"
#include "../model/Post.h"

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>

#include <Wt/Dbo/backend/Sqlite3.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace dbo = Wt::Dbo;

namespace {
  static int try_stoi(const std::string &v)
  {
    std::size_t pos;
    auto result = std::stoi(v, &pos);
    if (pos != v.length())
      throw std::invalid_argument("stoi() of " + v + " failed");
    return result;
  }
}

class BlogImpl : public Wt::WContainerWidget
{
public:
  BlogImpl(const std::string& basePath, dbo::SqlConnectionPool& connectionPool,
	   const std::string& rssFeedUrl, BlogView *blogView)
    : basePath_(basePath),
      rssFeedUrl_(rssFeedUrl),
      session_(connectionPool),
      panel_(nullptr),
      authorPanel_(nullptr),
      users_(nullptr),
      userEditor_(nullptr),
      mustLoginWarning_(nullptr),
      mustBeAdministratorWarning_(nullptr),
      invalidUser_(nullptr)
  {
    Wt::WApplication *app = Wt::WApplication::instance();

    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "blog");
    app->useStyleSheet("/css/blog.css");
    app->useStyleSheet("/css/asciidoc.css");
    app->internalPathChanged().connect(this, &BlogImpl::handlePathChange);

    loginStatus_ = this->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("blog-login-status")));
    panel_ = this->addWidget(Wt::cpp14::make_unique<Wt::WStackedWidget>());
    items_ = this->addWidget(Wt::cpp14::make_unique<Wt::WContainerWidget>());

    session_.login().changed().connect(this, &BlogImpl::onUserChanged);

    auto loginWidget
        = Wt::cpp14::make_unique<BlogLoginWidget>(session_, basePath);
    loginWidget_ = loginWidget.get();
    loginWidget_->hide();

    auto loginLink = Wt::cpp14::make_unique<Wt::WText>(tr("login"));
    auto lPtr = loginLink.get();
    loginLink->setStyleClass("link");
    loginLink->clicked().connect(loginWidget_, &WWidget::show);
    loginLink->clicked().connect(lPtr, &WWidget::hide);

    auto registerLink = Wt::cpp14::make_unique<Wt::WText>(tr("Wt.Auth.register"));
    registerLink->setStyleClass("link");
    registerLink->clicked().connect(loginWidget_,
				    &BlogLoginWidget::registerNewUser);

    auto archiveLink
        = Wt::cpp14::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, basePath_ + "all"),
                                      tr("archive"));

    loginStatus_->bindWidget("login", std::move(loginWidget));
    loginStatus_->bindWidget("login-link", std::move(loginLink));
    loginStatus_->bindWidget("register-link", std::move(registerLink));
    loginStatus_->bindString("feed-url", rssFeedUrl_);
    loginStatus_->bindWidget("archive-link", std::move(archiveLink));

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

  Wt::WStackedWidget* panel_;
  Wt::WTemplate *authorPanel_;
  EditUsers *users_;
  EditUser  *userEditor_;
  Wt::WTemplate *mustLoginWarning_;
  Wt::WTemplate *mustBeAdministratorWarning_;
  Wt::WTemplate *invalidUser_;
  Wt::WTemplate *loginStatus_;
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
    Wt::WApplication::instance()->changeSessionId();

    refresh();

    loginStatus_->resolveWidget("login")->show();
    loginStatus_->resolveWidget("login-link")->hide();
    loginStatus_->resolveWidget("register-link")->hide();

    auto profileLink = Wt::cpp14::make_unique<Wt::WText>(tr("profile"));
    profileLink->setStyleClass("link");
    profileLink->clicked().connect(this, &BlogImpl::editProfile);

    dbo::ptr<User> user = session().user();

    if (user->role == User::Admin) {
      auto editUsersLink = Wt::cpp14::make_unique<Wt::WText>(tr("edit-users"));
      editUsersLink->setStyleClass("link");
      editUsersLink->clicked().connect(this, &BlogImpl::editUsers);
      loginStatus_->bindWidget("userlist-link", std::move(editUsersLink));

      auto authorPanelLink = Wt::cpp14::make_unique<Wt::WText>(tr("author-post"));
      authorPanelLink->setStyleClass("link");
      authorPanelLink->clicked().connect(this, &BlogImpl::authorPanel);
      loginStatus_->bindWidget("author-panel-link", std::move(authorPanelLink));
    } else {
      loginStatus_->bindEmpty("userlist-link");
      loginStatus_->bindEmpty("author-panel-link");
    }
 
    loginStatus_->bindWidget("profile-link", std::move(profileLink));
 
    bindPanelTemplates();
  }

  void bindPanelTemplates() {
    if (!session_.user())
      return;

    dbo::Transaction t(session_);

    if (authorPanel_) {
      auto newPost = Wt::cpp14::make_unique<Wt::WPushButton>(tr("new-post"));
      newPost->clicked().connect(this, &BlogImpl::newPost);
      auto unpublishedPosts = Wt::cpp14::make_unique<Wt::WContainerWidget>();
      showPosts(session_.user()->allPosts(Post::Unpublished), unpublishedPosts.get());

      authorPanel_->bindString("user", session_.user()->name);
      authorPanel_->bindInt("unpublished-count",
			    (int)session_.user()->allPosts(Post::Unpublished)
			    .size());
      authorPanel_->bindInt("published-count",
			    (int)session_.user()->allPosts(Post::Published)
			    .size());
      authorPanel_->bindWidget("new-post", std::move(newPost));
      authorPanel_->bindWidget("unpublished-posts", std::move(unpublishedPosts));
    }

    t.commit();
  }
 
  void editUsers() {
    panel_->show();

    if (!users_)
    {
      users_ =
          panel_->addWidget(Wt::cpp14::make_unique<EditUsers>(session_, basePath_));
      bindPanelTemplates();
    }

    panel_->setCurrentWidget(users_);
  }

 void authorPanel() {
    panel_->show();
    if (!authorPanel_)
    {
      authorPanel_ =
          panel_->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("blog-author-panel")));
      bindPanelTemplates();
    }
    panel_->setCurrentWidget(authorPanel_);
  }

  void editProfile() {
    loginWidget_->letUpdatePassword(session_.login().user(), true);
  }

  void refresh() {
    handlePathChange(Wt::WApplication::instance()->internalPath());
  }

  void handlePathChange(const std::string&) {
    Wt::WApplication *app = Wt::WApplication::instance();

    if (app->internalPathMatches(basePath_)) {
      dbo::Transaction t(session_);

      std::string path = app->internalPathNextPart(basePath_);

      items_->clear();

      if (users_) {
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
    dbo::dbo_traits<User>::IdType id = User::stringToId(ids);

    panel_->show();
    try {
      dbo::Transaction t(session_);
      dbo::ptr<User> target(session_.load<User>(id));
      if (!userEditor_){
        userEditor_ = panel_->addWidget(Wt::cpp14::make_unique<EditUser>(session_));
      }
      userEditor_->switchUser(target);
      panel_->setCurrentWidget(userEditor_);
    }
    catch (Wt::Dbo::ObjectNotFoundException) {
      if (!invalidUser_){
        invalidUser_ =
            panel_->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("blog-invaliduser")));
      }
      panel_->setCurrentWidget(invalidUser_);
    }
  }

  bool checkLoggedIn()
  {
    if (session_.user()) return true;
    panel_->show();
    if (!mustLoginWarning_){
      mustLoginWarning_ =
        panel_->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("blog-mustlogin")));
    }
    panel_->setCurrentWidget(mustLoginWarning_);
    return false;
  }

  bool checkAdministrator()
  {
    if (session_.user() && (session_.user()->role == User::Admin)) return true;
    panel_->show();
    if (!mustBeAdministratorWarning_){
      mustBeAdministratorWarning_ =
          panel_->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("blog-mustbeadministrator")));
    }
    panel_->setCurrentWidget(mustBeAdministratorWarning_);
    return false;
  }

  dbo::ptr<User> findUser(const std::string& name) {
    return session_.find<User>("where name = ?").bind(name);
  }
  
  bool yearMonthDiffer(const Wt::WDateTime& dt1, const Wt::WDateTime& dt2) {
    return dt1.date().year() != dt2.date().year()
      || dt1.date().month() != dt2.date().month();
  }

  void showArchive(WContainerWidget *parent) {
    static const char* dateFormat = "MMMM yyyy";
    
    parent->addWidget(Wt::cpp14::make_unique<Wt::WText>(tr("archive-title")));

    Posts posts = session_.find<Post>("order by date desc");

    Wt::WDateTime formerDate;
    for (auto post : posts) {
      if (post->state != Post::Published)
	continue;

      if (formerDate.isNull() 
          || yearMonthDiffer(formerDate, post->date)) {
        Wt::WText *title
          = parent->addWidget(Wt::cpp14::make_unique<Wt::WText>(post->date.date().toString(dateFormat)));
	title->setStyleClass("archive-month-title");
      }
      
      Wt::WAnchor *a = parent->addWidget(Wt::cpp14::make_unique<Wt::WAnchor>(
                                       Wt::WLink(Wt::LinkType::InternalPath,
                                       basePath_ + post->permaLink()),
                                       post->title));
      a->setInline(false);
      
      formerDate = post->date;
    }
  }

  void showPostsByDateTopic(const std::string& path,
			    WContainerWidget *parent) {
    std::vector<std::string> parts;
    boost::split(parts, path, boost::is_any_of("/"));

    Wt::WDate lower, upper;
    try {
      int year = try_stoi(parts[0]);

      if (parts.size() > 1) {
        int month = try_stoi(parts[1]);

        if (parts.size() > 2) {
          int day = try_stoi(parts[2]);

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
    } catch (std::invalid_argument &) {
      showError(tr("blog-no-post"));
      return;
    }

    Posts posts = session_.find<Post>
      ("where date >= ? "
       "and date < ? "
       "and (state = ? or author_id = ?)")
      .bind(Wt::WDateTime(lower))
      .bind(Wt::WDateTime(upper))
      .bind(Post::Published)
      .bind(session_.user().id());

    if (parts.size() > 3) {
      std::string title = parts[3];

      for (auto post : posts)
        if (post->titleToUrl() == title) {
          showPost(post, PostView::Detail, parent);
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

    dbo::ptr<Post> post(Wt::cpp14::make_unique<Post>());

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
    for (auto post : posts)
      showPost(post, PostView::Brief, parent);
  }

  void showPost(const dbo::ptr<Post> post, PostView::RenderType type,
		WContainerWidget *parent) {
    parent->addWidget(Wt::cpp14::make_unique<PostView>(session_, basePath_, post, type));
  }

  void showError(const Wt::WString& msg) {
    items_->addWidget(Wt::cpp14::make_unique<Wt::WText>(msg));
  }
};

BlogView::BlogView(const std::string& basePath, dbo::SqlConnectionPool& db,
                   const std::string& rssFeedUrl)
  : WCompositeWidget(),
    userChanged_()
{
  impl_ = setImplementation(Wt::cpp14::make_unique<BlogImpl>(basePath, db, rssFeedUrl, this));
}

void BlogView::setInternalBasePath(const std::string& basePath)
{
  impl_->setInternalBasePath(basePath);
}

Wt::WString BlogView::user()
{
  if (impl_->session().user())
    return impl_->session().user()->name;
  else
    return Wt::WString::Empty;
}
