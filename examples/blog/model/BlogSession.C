/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "BlogSession.h"
#include "Comment.h"
#include "Post.h"
#include "Tag.h"
#include "Token.h"
#include "User.h"
#include "../asciidoc/asciidoc.h"

#include <Wt/Auth/AuthService>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/GoogleService>

#include <Wt/Dbo/FixedSqlConnectionPool>

#ifndef WT_WIN32
#include <unistd.h>
#endif

#if !defined(WT_WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#endif

using namespace Wt;

namespace {
  const std::string ADMIN_USERNAME = "admin";
  const std::string ADMIN_PASSWORD = "admin";

#ifdef HAVE_CRYPT
  class UnixCryptHashFunction : public Auth::HashFunction
  {
  public:
    virtual std::string compute(const std::string& msg, 
				const std::string& salt) const
    {
      std::string md5Salt = "$1$" + salt;
      return crypt(msg.c_str(), md5Salt.c_str());
    }

    virtual bool verify(const std::string& msg,
			const std::string& salt,
			const std::string& hash) const
    {
      return crypt(msg.c_str(), hash.c_str()) == hash;
    }

    virtual std::string name () const {
      return "crypt";
    }
  };
#endif // HAVE_CRYPT

  class BlogOAuth : public std::vector<const Auth::OAuthService *>
  {
  public:
    ~BlogOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
	delete (*this)[i];
    }
  };

  Auth::AuthService blogAuth;
  Auth::PasswordService blogPasswords(blogAuth);
  BlogOAuth blogOAuth;
}

namespace dbo = Wt::Dbo;

void BlogSession::configureAuth()
{
  blogAuth.setAuthTokensEnabled(true, "bloglogin");

  Auth::PasswordVerifier *verifier = new Auth::PasswordVerifier();
  verifier->addHashFunction(new Auth::BCryptHashFunction(7));
#ifdef WT_WITH_SSL
  verifier->addHashFunction(new Auth::SHA1HashFunction());
#endif
#ifdef HAVE_CRYPT
  verifier->addHashFunction(new UnixCryptHashFunction());
#endif
  blogPasswords.setVerifier(verifier);
  blogPasswords.setAttemptThrottlingEnabled(true);
  blogPasswords.setStrengthValidator
    (new Auth::PasswordStrengthValidator());

  if (Auth::GoogleService::configured())
    blogOAuth.push_back(new Auth::GoogleService(blogAuth));
}

dbo::SqlConnectionPool *BlogSession::createConnectionPool(const std::string& sqliteDb)
{
  dbo::backend::Sqlite3 *connection = new dbo::backend::Sqlite3(sqliteDb);

  connection->setProperty("show-queries", "true");
  connection->setDateTimeStorage(Wt::Dbo::SqlDateTime,
				 Wt::Dbo::backend::Sqlite3::PseudoISO8601AsText);

  return new dbo::FixedSqlConnectionPool(connection, 10);
}

BlogSession::BlogSession(dbo::SqlConnectionPool& connectionPool)
  : connectionPool_(connectionPool),
    users_(*this)
{
  setConnectionPool(connectionPool_);

  mapClass<Comment>("comment");
  mapClass<Post>("post");
  mapClass<Tag>("tag");
  mapClass<Token>("token");
  mapClass<User>("user");

  try {
    dbo::Transaction t(*this);
    createTables();

    dbo::ptr<User> admin = add(new User());
    User *a = admin.modify();
    a->name = ADMIN_USERNAME;
    a->role = User::Admin;

    Auth::User authAdmin
      = users_.findWithIdentity(Auth::Identity::LoginName, a->name);
    blogPasswords.updatePassword(authAdmin, ADMIN_PASSWORD);

    dbo::ptr<Post> post = add(new Post());
    Post *p = post.modify();

    p->state = Post::Published;
    p->author = admin;
    p->title = "Welcome!";
    p->briefSrc = "Welcome to your own blog.";
    p->bodySrc = "We have created for you an " + ADMIN_USERNAME +
      " user with password " + ADMIN_PASSWORD;
    p->briefHtml = asciidoc(p->briefSrc);
    p->bodyHtml = asciidoc(p->bodySrc);
    p->date = WDateTime::currentDateTime();

    dbo::ptr<Comment> rootComment = add(new Comment());
    rootComment.modify()->post = post;

    t.commit();

    std::cerr << "Created database, and user " << ADMIN_USERNAME
	      << " / " << ADMIN_PASSWORD << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }
}

dbo::ptr<User> BlogSession::user() const
{
  if (login_.loggedIn())
    return users_.find(login_.user());
  else
    return dbo::ptr<User>();
}

Auth::PasswordService *BlogSession::passwordAuth() const
{
  return &blogPasswords;
}

const std::vector<const Auth::OAuthService *>& BlogSession::oAuth() const
{
  return blogOAuth;
}
