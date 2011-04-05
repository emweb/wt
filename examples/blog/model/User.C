/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#if !defined(WIN32) && !defined(__QNXNTO__)
#define _XOPEN_SOURCE
#include <unistd.h>
#endif

#include "User.h"
#include "Comment.h"
#include "Post.h"
#include "Tag.h"

#include <Wt/WRandom>
#include <Wt/Dbo/Impl>

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#endif

DBO_INSTANTIATE_TEMPLATES(User);

#ifdef HAVE_CRYPT
namespace {
  std::string generateSalt() {
    /* Salt generation from glibc manual */
    unsigned long seed[2];
    char salt[] = "$1$........";
    const char *const seedchars =
      "./0123456789ABCDEFGHIJKLMNOPQRST"
      "UVWXYZabcdefghijklmnopqrstuvwxyz";
     
    /* Generate a (not very) random seed. */
    seed[0] = time(NULL);
    seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);
     
    /* Turn it into printable characters from `seedchars'. */
    for (int i = 0; i < 8; i++)
      salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];

    return salt;
  }

  std::string md5(const std::string& s, const std::string& salt) {
    return crypt(s.c_str(), salt.c_str());
  }
}
#endif

void User::setPassword(const std::string& password)
{
#ifdef HAVE_CRYPT
  password_ = md5(password, generateSalt());
#else
  // This needs some improvement for production use
  password_ = password;
#endif
}

bool User::authenticate(const std::string& password) const
{
#ifdef HAVE_CRYPT
  return md5(password, password_) == password_;
#else
  return password_ == password;
#endif
}

std::string User::generateToken()
{
  token_ = Wt::WRandom::generateId(16);

  return token_;
}

Posts User::latestPosts(int count) const
{
  return posts.find().where("state = ?").bind(Post::Published)
    .orderBy("date desc")
    .limit(count);
}

Posts User::allPosts(Post::State state) const
{
  return posts.find().where("state = ?").bind(state)
    .orderBy("date desc");
}
