/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WIN32
#define _XOPEN_SOURCE
#include <unistd.h>
#endif

#include "User.h"
#include "Comment.h"
#include "Post.h"
#include "Tag.h"

#include <Wt/Dbo/Impl>

DBO_INSTANTIATE_TEMPLATES(User);

#ifndef WIN32
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
#ifndef WIN32
  password_ = md5(password, generateSalt());
#else
  // This needs some improvement for production use
  password_ = password;
#endif
}

bool User::authenticate(const std::string& password) const
{
#ifndef WIN32
  return md5(password, password_) == password_;
#else
  return password_ == password;
#endif
}

Posts User::latestPosts(int count) const
{
  return posts.session()->find<Post>
    ("where author_id = ? and state = ? "
     "order by date desc "
     "limit ?")
    .bind(posts.arg())
    .bind(Post::Published)
    .bind(count);
}

Posts User::allPosts(Post::State state) const
{
  return posts.session()->find<Post>
    ("where author_id = ? and state = ? "
     "order by date desc")
    .bind(posts.arg())
    .bind(state);
}
