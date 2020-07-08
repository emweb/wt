/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/*****
 * This file is part of the Wt::Dbo tutorial:
 * http://www.webtoolkit.eu/wt/doc/tutorial/dbo/tutorial.html
 *****/

/*****
 * Dbo tutorial section 7.6
 *  Specifying a natural primary key that is also a foreign key
 *****/

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

namespace dbo = Wt::Dbo;

class UserInfo;
class User;

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<UserInfo> : public dbo_default_traits {
      typedef ptr<User> IdType;

      static IdType invalidId() {
        return ptr<User>();
      }

      static const char *surrogateIdField() { return 0; }
    };

  }
}

class User {
public:
  std::string name;

  dbo::collection< dbo::ptr<UserInfo> > infos;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");

    // In fact, this is really constrained to hasOne() ...
    dbo::hasMany(a, infos, dbo::ManyToOne, "user");
  }
};

class UserInfo {
public:
  dbo::ptr<User> user;
  std::string info;

  template<class Action>
  void persist(Action& a)
  {
    dbo::id(a, user, "user", dbo::OnDeleteCascade);
    dbo::field(a, info, "info");
  }
};

void run()
{
  /*
   * Setup a session, would typically be done once at application startup.
   */
  std::unique_ptr<dbo::backend::Sqlite3> sqlite3(new dbo::backend::Sqlite3(":memory:"));
  sqlite3->setProperty("show-queries", "true");
  dbo::Session session;
  session.setConnection(std::move(sqlite3));

  session.mapClass<User>("user");
  session.mapClass<UserInfo>("user_info");

  /*
   * Try to create the schema (will fail if already exists).
   */
  session.createTables();

  {
    dbo::Transaction transaction(session);

    std::unique_ptr<User> user{new User()};
    user->name = "Joe";

    dbo::ptr<User> userPtr = session.add(std::move(user));

    std::unique_ptr<UserInfo> userInfo{new UserInfo()};
    userInfo->user = userPtr;
    userInfo->info = "great guy";

    session.add(std::move(userInfo));
  }

  {
    dbo::Transaction transaction(session);

    dbo::ptr<UserInfo> info = session.find<UserInfo>();
    
    std::cerr << info->user->name << " is a " << info->info << std::endl;
  }
}

int main(int argc, char **argv)
{
  run();
}
