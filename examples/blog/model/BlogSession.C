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

#include "Wt/Auth/BaseAuth"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordAuth"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleAuth"

#ifndef WIN32
#include <unistd.h>
#endif

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
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
				const std::string& salt)
      const
    {
      std::string md5Salt = "$1$" + salt;
      return crypt(msg.c_str(), md5Salt.c_str());
    }

    virtual std::string name () const {
      return "crypt";
    }
  };
#endif // HAVE_CRYPT

  class BlogBaseAuth : public Auth::BaseAuth
  {
  public:
    BlogBaseAuth()
    {
      setAuthTokensEnabled(true, "bloglogin");

#ifdef WT_WITH_SSL
      setTokenHashFunction(new Auth::SHA1HashFunction());
#else
# ifdef HAVE_CRYPT
      setTokenHashFunction(new UnixCryptHashFunction());
# endif
#endif
    }
  };

  class BlogPasswords : public Auth::PasswordAuth
  {
  public:
    BlogPasswords(const Auth::BaseAuth& baseAuth)
      : Auth::PasswordAuth(baseAuth)
    {
      Auth::PasswordVerifier *verifier = new Auth::PasswordVerifier();

#ifdef WT_WITH_SSL
      verifier->addHashFunction(new Auth::SHA1HashFunction());
#endif

#ifdef HAVE_CRYPT
      verifier->addHashFunction(new UnixCryptHashFunction());
#endif

      setVerifier(verifier);
      setAttemptThrottlingEnabled(true);
    }
  };

  class BlogOAuth : public std::vector<const Auth::OAuth *>
  {
  public:
    ~BlogOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
	delete (*this)[i];
    }
  };

  BlogBaseAuth blogBaseAuth;
  BlogPasswords blogPasswords(blogBaseAuth);
  BlogOAuth blogOAuth;
}

namespace dbo = Wt::Dbo;

void BlogSession::initAuth()
{
  if (Auth::GoogleAuth::configured())
    blogOAuth.push_back(new Auth::GoogleAuth(blogBaseAuth));
}

BlogSession::BlogSession(const std::string& sqliteDb)
  : connection_(sqliteDb),
    users_(*this)
{
  connection_.setProperty("show-queries", "true");

  setConnection(connection_);

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

    Auth::User authAdmin = users_.findIdentity(a->name);
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

Auth::PasswordAuth *BlogSession::passwordAuth() const
{
  return &blogPasswords;
}

const std::vector<const Auth::OAuth *>& BlogSession::oAuth() const
{
  return blogOAuth;
}
