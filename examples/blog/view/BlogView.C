/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PostView.h"
#include "BlogView.h"
#include "EditUsers.h"

#include "../BlogSession.h"

#include "../model/Comment.h"
#include "../model/Post.h"
#include "../model/Tag.h"
#include "../model/User.h"

#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>

#include <Wt/Dbo/backend/Sqlite3>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace Wt;
namespace dbo = Wt::Dbo;

class BlogImpl : public WContainerWidget
{
public:
  BlogImpl(const std::string& basePath, const std::string& sqliteDb,
	   const std::string& rssFeedUrl)
    : basePath_(basePath),
      rssFeedUrl_(rssFeedUrl),
      session_(sqliteDb),
      panel_(0),
      register_(0),
      profile_(0),
      authorPanel_(0),
      users_(0),
      userEditor_(0),
      mustLoginWarning_(0),
      mustBeAdministratorWarning_(0),
      invalidUser_(0)
  {
    WApplication *app = wApp;

    app->messageResourceBundle().use(WApplication::appRoot() + "blog");
    app->useStyleSheet("css/blog.css");
    app->useStyleSheet("css/asciidoc.css");
    app->internalPathChanged().connect(this, &BlogImpl::handlePathChange);
    login_ = new WTemplate(this);
    panel_ = new WStackedWidget(this);
    items_ = new WContainerWidget(this);

    init();
  }

  ~BlogImpl() {
    clear();
  }

private:
  std::string basePath_, rssFeedUrl_;
  BlogSession session_;

  WTemplate *login_;
  WStackedWidget* panel_;
  WTemplate *register_;
  WTemplate *profile_;
  WTemplate *authorPanel_;
  EditUsers *users_;
  EditUser  *userEditor_;
  WTemplate *mustLoginWarning_;
  WTemplate *mustBeAdministratorWarning_;
  WTemplate *invalidUser_;
  WContainerWidget *items_;

  void logout() {
    if (boost::starts_with(wApp->internalPath(), basePath_ + "author/"))
      wApp->setInternalPath(basePath_, true);

    init();
  }

  void init() {
    session_.setUser(dbo::ptr<User>());
    refresh();
    panel_->hide();

    login_->clear();
    login_->setTemplateText(tr("blog-login"));

    WLineEdit *name = new WLineEdit();
    WLineEdit *passwd = new WLineEdit();
    WPushButton *loginButton = new WPushButton(tr("login"));

    passwd->setEchoMode(WLineEdit::Password);
    passwd->enterPressed().connect(this, &BlogImpl::login);
    loginButton->clicked().connect(this, &BlogImpl::login);

    name->hide();
    passwd->hide();
    loginButton->hide();

    WText *loginLink = new WText(tr("login"));
    loginLink->setStyleClass("link");

    loginLink->clicked().connect(name, &WWidget::show);
    loginLink->clicked().connect(passwd, &WWidget::show);
    loginLink->clicked().connect(loginButton, &WWidget::show);
    loginLink->clicked().connect(loginLink, &WWidget::hide);
    loginLink->clicked().connect(name, &WFormWidget::setFocus);

    WText *registerLink = new WText(tr("register"));
    registerLink->setStyleClass("link");

    registerLink->clicked().connect(this, &BlogImpl::newUser);

    login_->bindWidget("name", name);
    login_->bindWidget("passwd", passwd);
    login_->bindWidget("login-button", loginButton);
    login_->bindWidget("login-link", loginLink);
    login_->bindWidget("register-link", registerLink);
    login_->bindString("feed-url", rssFeedUrl_);
  }

  void login() {
    WLineEdit *name = login_->resolve<WLineEdit *>("name");
    WLineEdit *passwd = login_->resolve<WLineEdit *>("passwd");

    dbo::Transaction t(session_);

    dbo::ptr<User> user
      = session_.find<User>("where name = ?").bind(name->text());

    if (user) {
      if (user->authenticate(passwd->text().toUTF8())) {
	loginAs(user);
      } else {
	name->setStyleClass("");
	passwd->setStyleClass("invalid");
      }
    } else
      name->setStyleClass("invalid");

    t.commit();
  }

  void loginAs(dbo::ptr<User> user) {
    session_.setUser(user);

    if (user->role == User::Admin)
      wApp->setInternalPath(basePath_ + "author/" + user->name, true);

    refresh();

    login_->clear();
    login_->setTemplateText(tr("blog-logout"));

    cancelRegister();

    WText *profileLink = new WText(tr("profile"));
    profileLink->setStyleClass("link");
    profileLink->clicked().connect(this, &BlogImpl::editProfile);

    if (user->role == User::Admin) {
      WText *editUsersLink = new WText(tr("edit-users"));
      editUsersLink->setStyleClass("link");
      editUsersLink->clicked().connect(SLOT(this, BlogImpl::editUsers));
      login_->bindWidget("userlist-link", editUsersLink);
      WText *authorPanelLink = new WText(tr("author-post"));
      authorPanelLink->setStyleClass("link");
      authorPanelLink->clicked().connect(SLOT(this, BlogImpl::authorPanel));
      login_->bindWidget("author-panel-link", authorPanelLink);
    }
    else
    {
      login_->bindWidget("userlist-link", new WText(""));
      login_->bindWidget("author-panel-link", new WText(""));
    }
 
    WText *logoutLink = new WText(tr("logout"));
    logoutLink->setStyleClass("link");
    logoutLink->clicked().connect(this, &BlogImpl::logout);

    login_->bindString("feed-url", rssFeedUrl_);
    login_->bindString("user", user->name);
    login_->bindWidget("profile-link", profileLink);
    login_->bindWidget("logout-link", logoutLink);

    bindPanelTemplates();
  }

  void newUser() {
    if (!register_) {
      register_ = new WTemplate();
      insertWidget(1, register_);
      register_->setTemplateText(tr("blog-register"));

      WLineEdit *name = new WLineEdit();
      WLineEdit *passwd = new WLineEdit();
      WLineEdit *passwd2 = new WLineEdit();
      WPushButton *okButton = new WPushButton(tr("register"));
      WPushButton *cancelButton = new WPushButton(tr("cancel"));
      WText *error = new WText();

      passwd->setEchoMode(WLineEdit::Password);
      passwd2->setEchoMode(WLineEdit::Password);

      okButton->clicked().connect(this, &BlogImpl::doRegister);
      cancelButton->clicked().connect(this, &BlogImpl::cancelRegister);

      register_->bindWidget("name", name);
      register_->bindWidget("passwd", passwd);
      register_->bindWidget("passwd2", passwd2);
      register_->bindWidget("ok-button", okButton);
      register_->bindWidget("cancel-button", cancelButton);
      register_->bindWidget("error", error);
    }
  }

  void bindPanelTemplates() {
    if (!session_.user()) return;
    Wt::Dbo::Transaction t(session_);
    if (authorPanel_) {
      WPushButton *newPost = new WPushButton(tr("new-post"));
      newPost->clicked().connect(SLOT(this, BlogImpl::newPost));
      WContainerWidget *unpublishedPosts = new WContainerWidget();
      showPosts(session_.user()->allPosts(Post::Unpublished), unpublishedPosts);

      authorPanel_->bindString("user", session_.user()->name);
      authorPanel_->bindInt("unpublished-count",
			    (int)session_.user()->allPosts(Post::Unpublished).size());
      authorPanel_->bindInt("published-count",
			    (int)session_.user()->allPosts(Post::Published).size());
      authorPanel_->bindWidget("new-post", newPost);
      authorPanel_->bindWidget("unpublished-posts", unpublishedPosts);
    }
    if (profile_)
      profile_->bindString("user",session_.user()->name);
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
    panel_->show();
    if (!profile_) {
      profile_ = new WTemplate(tr("blog-profile"));
      panel_->addWidget(profile_);
      bindPanelTemplates();

      WLineEdit *passwd = new WLineEdit();
      WLineEdit *passwd2 = new WLineEdit();
      WPushButton *okButton = new WPushButton(tr("save"));
      WPushButton *cancelButton = new WPushButton(tr("cancel"));
      WText *error = new WText();

      passwd->setEchoMode(WLineEdit::Password);
      passwd2->setEchoMode(WLineEdit::Password);
      okButton->clicked().connect(this, &BlogImpl::saveProfile);
      cancelButton->clicked().connect(this, &BlogImpl::cancelProfile);

      profile_->bindWidget("passwd", passwd);
      profile_->bindWidget("passwd2", passwd2);
      profile_->bindWidget("ok-button", okButton);
      profile_->bindWidget("cancel-button", cancelButton);
      profile_->bindWidget("error", error);
    }
    panel_->setCurrentWidget(profile_);
  }

  void cancelProfile() {
    WLineEdit *passwd = profile_->resolve<WLineEdit *>("passwd");
    WLineEdit *passwd2 = profile_->resolve<WLineEdit *>("passwd2");
    WText *error = profile_->resolve<WText *>("error");
    passwd->setText(WString());
    passwd2->setText(WString());
    error->setText(WString());
    panel_->hide();
  }

  void saveProfile() {
    WLineEdit *passwd = profile_->resolve<WLineEdit *>("passwd");
    WLineEdit *passwd2 = profile_->resolve<WLineEdit *>("passwd2");
    WText *error = profile_->resolve<WText *>("error");
    if (passwd->text().empty()) {
      cancelProfile();
      return;
    }
    if (passwd->text() != passwd2->text()) {
      error->setText(tr("passwd-mismatch"));
      return;
    }
    dbo::Transaction t(session_);
    session_.user().modify()->setPassword(passwd->text().toUTF8());
    t.commit();
    cancelProfile();
  }

  void doRegister() {
    WLineEdit *name = register_->resolve<WLineEdit *>("name");
    WLineEdit *passwd = register_->resolve<WLineEdit *>("passwd");
    WLineEdit *passwd2 = register_->resolve<WLineEdit *>("passwd2");
    WText *error = register_->resolve<WText *>("error");

    if (passwd->text() != passwd2->text()) {
      error->setText(tr("passwd-mismatch"));
      return;
    }

    dbo::Transaction t(session_);

    dbo::ptr<User> user
      = session_.find<User>("where name = ?").bind(name->text());

    if (user) {
      error->setText(tr("user-exists").arg(name->text()));
      t.commit();
      return;
    } else {
      std::string n = name->text().toUTF8();
      boost::trim(n);
      if (n.length() < 4) {
	error->setText(tr("login-tooshort").arg(4));
	t.commit();
	return;
      }

      user = session_.add(new User());

      user.modify()->name = n;
      user.modify()->role = User::Visitor;
      user.modify()->setPassword(passwd->text().toUTF8());

      loginAs(user);
    }

    t.commit();
  }

  void cancelRegister() {
    delete register_;
    register_ = 0;
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
      profile_ = 0;
      users_ = 0;

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

  void showPostsByDateTopic(const std::string& path,
			    WContainerWidget *parent) {
    std::vector<std::string> parts;
    boost::split(parts, path, boost::is_any_of("/"));

    WDate lower, upper;
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
    /*
<<<<<<< HEAD:examples/blog/view/BlogView.C
    if (user == session_.user() && user->role == User::Admin) {
      WTemplate *authorPanel = new WTemplate(tr("blog-author-panel"), items_);

      WPushButton *newPost = new WPushButton(tr("new-post"));
      newPost->clicked().connect(this, &BlogImpl::newPost);

      WContainerWidget *unpublishedPosts = new WContainerWidget();
      showPosts(user->allPosts(Post::Unpublished), unpublishedPosts);

      authorPanel->bindString("user", user->name);
      authorPanel->bindInt("unpublished-count",
			   user->allPosts(Post::Unpublished).size());
      authorPanel->bindInt("published-count",
			   user->allPosts(Post::Published).size());
      authorPanel->bindWidget("new-post", newPost);
      authorPanel->bindWidget("unpublished-posts", unpublishedPosts);
    }

=======
>>>>>>> bvh_blog:examples/blog/view/BlogView.C
    */
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

BlogView::BlogView(const std::string& basePath, const std::string& sqliteDb,
		   const std::string& rssFeedUrl, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  impl_ = new BlogImpl(basePath, sqliteDb, rssFeedUrl);
  setImplementation(impl_);
}
