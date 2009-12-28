/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PostView.h"
#include "BlogView.h"

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
      session_(sqliteDb),
      rssFeedUrl_(rssFeedUrl)
  {
    WApplication *app = wApp;

    app->messageResourceBundle().use("blog");
    app->useStyleSheet("css/blog.css");
    app->useStyleSheet("css/asciidoc.css");
    app->internalPathChanged().connect(SLOT(this, BlogImpl::handlePathChange));

    login_ = new WTemplate(this);
    register_ = 0;
    items_ = new WContainerWidget(this);

    logout();
  }

  ~BlogImpl() {
    clear();
  }

private:
  std::string basePath_, rssFeedUrl_;
  BlogSession session_;

  WTemplate *login_;
  WTemplate *register_;
  WContainerWidget *items_;

  void logout() {
    session_.setUser(dbo::ptr<User>());
    refresh();

    login_->clear();
    login_->setTemplateText(tr("blog-login"));

    WLineEdit *name = new WLineEdit();
    WLineEdit *passwd = new WLineEdit();
    WPushButton *loginButton = new WPushButton(tr("login"));

    passwd->setEchoMode(WLineEdit::Password);
    passwd->enterPressed().connect(SLOT(this, BlogImpl::login));
    loginButton->clicked().connect(SLOT(this, BlogImpl::login));

    name->hide();
    passwd->hide();
    loginButton->hide();

    WText *loginLink = new WText(tr("login"));
    loginLink->setStyleClass("link");

    loginLink->clicked().connect(SLOT(name, WWidget::show));
    loginLink->clicked().connect(SLOT(passwd, WWidget::show));
    loginLink->clicked().connect(SLOT(loginButton, WWidget::show));
    loginLink->clicked().connect(SLOT(loginLink, WWidget::hide));
    loginLink->clicked().connect(SLOT(name, WFormWidget::setFocus));

    WText *registerLink = new WText(tr("register"));
    registerLink->setStyleClass("link");

    registerLink->clicked().connect(SLOT(this, BlogImpl::newUser));

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

    if (user->role == User::Admin) {
      wApp->setInternalPath(basePath_ + "author/" + user->name, true);
    } else
      refresh();

    login_->clear();
    login_->setTemplateText(tr("blog-logout"));

    cancelRegister();

    WText *logoutLink = new WText(tr("logout"));
    logoutLink->setStyleClass("link");
    logoutLink->clicked().connect(SLOT(this, BlogImpl::logout));

    login_->bindString("feed-url", rssFeedUrl_);
    login_->bindString("user", user->name);
    login_->bindWidget("logout-link", logoutLink);
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

      okButton->clicked().connect(SLOT(this, BlogImpl::doRegister));
      cancelButton->clicked().connect(SLOT(this, BlogImpl::cancelRegister));

      register_->bindWidget("name", name);
      register_->bindWidget("passwd", passwd);
      register_->bindWidget("passwd2", passwd);
      register_->bindWidget("ok-button", okButton);
      register_->bindWidget("cancel-button", cancelButton);
      register_->bindWidget("error", error);
    }
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
      } else {
	std::string remainder = app->internalPath().substr(basePath_.length());
	showPostsByDateTopic(remainder, items_);
      }

      t.commit();
    }
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
    if (user == session_.user()) {
      WTemplate *authorPanel = new WTemplate(tr("blog-author-panel"), items_);

      WPushButton *newPost = new WPushButton(tr("new-post"));
      newPost->clicked().connect(SLOT(this, BlogImpl::newPost));

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

    showPosts(user->latestPosts(), items_);
  }

  void newPost() {
    dbo::Transaction t(session_);

    WTemplate *panel = dynamic_cast<WTemplate *>(items_->widget(0));
    WContainerWidget *unpublishedPosts
      = panel->resolve<WContainerWidget *>("unpublished-posts");

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
