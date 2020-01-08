/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "User.h"
#include "Comment.h"
#include "Post.h"
#include "Tag.h"
#include "Token.h"

#include <Wt/Dbo/Impl.h>

#include <string>

DBO_INSTANTIATE_TEMPLATES(User)

User::User()
  : role(Visitor),
    failedLoginAttempts(0)
{ }

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

dbo::dbo_traits<User>::IdType User::stringToId(const std::string &s)
{
  std::size_t pos = std::string::npos;
  auto result = std::stoll(s, &pos);
  if (pos != s.size())
    return dbo::dbo_traits<User>::invalidId();
  else
    return result;
}
