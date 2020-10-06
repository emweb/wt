/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifdef SQLITE3

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Json.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include <Wt/WGlobal.h>

namespace dbo = Wt::Dbo;

namespace JsonDboTest {

class Post;
class User;
class NestedThing;

class Post {
public:
  dbo::ptr<User> user;
  dbo::weak_ptr<NestedThing> nestedThing;
  std::string title;
  std::string body;

  template<class Action>
  void persist(Action& a)
  {
    dbo::belongsTo(a, user, "user");
    dbo::hasOne(a, nestedThing, "post");
    dbo::field(a, title, "title");
    dbo::field(a, body, "body");
  }
};

class NestedThing {
public:
  int one;
  int two;
  int three;
  dbo::ptr<Post> post;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, one, "one");
    dbo::field(a, two, "two");
    dbo::field(a, three, "three");
    dbo::belongsTo(a, post, "post");
  }
};

class Empty {
public:
  template<class Action>
  void persist(Action& a)
  {
  }
};

class Settings {
public:
  std::string theme;

  dbo::ptr<User> user;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, theme, "theme");
    dbo::belongsTo(a, user);
  }
};

class User {
public:
  enum Role {
    Visitor = 0,
    Admin = 1,
    Alien = 42
  };

  std::string name;
  std::string password;
  Role        role;
  int         karma;

  dbo::collection< dbo::ptr<Post> > posts;
  dbo::weak_ptr<Settings> settings;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name,     "name");
    dbo::field(a, password, "password");
    dbo::field(a, role,     "role");
    dbo::field(a, karma,    "karma");

    dbo::hasMany(a, posts, dbo::ManyToOne, "user");
    dbo::hasOne(a, settings);
  }
};

class HasSurrogate {
public:
  std::string foo;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, foo, "foo");
  }
};

class HasNatural {
public:
  std::string myNaturalId;
  std::string foo;

  template<class Action>
  void persist(Action& a)
  {
    dbo::id(a, myNaturalId, "natural_id", 20);
    dbo::field(a, foo, "foo");
  }
};

struct Coordinate {
  int x, y;

  Coordinate()
    : x(-1), y(-1) { }

  Coordinate(int an_x, int an_y)
    : x(an_x), y(an_y) { }

  bool operator== (const Coordinate& other) const {
    return x == other.x && y == other.y;
  }
  
  bool operator< (const Coordinate& other) const {
    if (x < other.x)
      return true;
    else if (x == other.x)
      return y < other.y;
    else
      return false;
  }
};

std::ostream& operator<< (std::ostream& o, const Coordinate& c)
{
  return o << "{" << c.x << ", " << c.y << ")";
}

class HasCoordinateId;

}

namespace Wt {
  namespace Dbo {

    template <class Action>
    void field(Action& action, JsonDboTest::Coordinate& coordinate, const std::string& name, int size = -1)
    {
      field(action, coordinate.x, name + "_x");
      field(action, coordinate.y, name + "_y");
    }

    template<>
    struct dbo_traits<JsonDboTest::HasSurrogate> : public dbo_default_traits {
      static const char *surrogateIdField() { return "alternate_id"; }
    };
    template<>
    struct dbo_traits<JsonDboTest::HasNatural> : public dbo_default_traits {
      typedef std::string IdType;

      static IdType invalidId() {
	return std::string();
      }

      static const char *surrogateIdField() { return 0; }
    };
    template<>
    struct dbo_traits<JsonDboTest::HasCoordinateId> : public dbo_default_traits
    {
      typedef JsonDboTest::Coordinate IdType;
      static IdType invalidId() { return JsonDboTest::Coordinate(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

namespace JsonDboTest {

class HasCoordinateId {
public:
  Coordinate position;
  std::string name;

  template <class Action>
  void persist(Action& a)
  {
    dbo::id(a, position, "position");
    dbo::field(a, name, "name");
  }
};

struct JsonDboFixture
{
  JsonDboFixture()
  {
    static bool logged = false;

    if (!logged) {
      std::cerr << "JsonTest.C created a Sqlite3 connector" << std::endl;
      logged = true;
    }

    std::unique_ptr<dbo::SqlConnection> sqlite3(new dbo::backend::Sqlite3(":memory:"));

    session_ = std::unique_ptr<dbo::Session>(new dbo::Session());
    session_->setConnection(std::move(sqlite3));

    session_->mapClass<User>("user");
    session_->mapClass<Post>("post");
    session_->mapClass<NestedThing>("nestedThing");
    session_->mapClass<Settings>("settings");
    session_->mapClass<Empty>("empty");
    session_->mapClass<HasSurrogate>("hasSurrogate");
    session_->mapClass<HasNatural>("hasNatural");
    session_->mapClass<HasCoordinateId>("hasCoordinateId");

    session_->createTables();
  }

  ~JsonDboFixture()
  {
    session_->dropTables();
  }

  std::unique_ptr<dbo::Session> session_;
};

BOOST_AUTO_TEST_CASE( dbo_json_empty_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  {
    dbo::Transaction transaction(session);

    std::unique_ptr<Empty> empty{new Empty()};
    
    std::stringstream ss;
    dbo::jsonSerialize(*empty, ss);

    BOOST_REQUIRE_EQUAL(ss.str(), "{}");

    dbo::ptr<Empty> emptyPtr = session.add(std::move(empty));
  }

  dbo::Transaction transaction(session);

  dbo::collection<dbo::ptr<Empty> > empties = session.find<Empty>();

  std::stringstream ss;
  dbo::jsonSerialize(empties.front(), ss);

  BOOST_REQUIRE_EQUAL(ss.str(), "{\"id\":1}");
}

BOOST_AUTO_TEST_CASE( dbo_json_empty_and_null_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  {
    dbo::Transaction transaction(session);

    std::unique_ptr<User> user{new User()};
    user->name = "John";
    user->password = "Something";
    user->role = User::Alien;
    user->karma = 13;

    session.add(std::move(user));
  }

  dbo::Transaction transaction(session);

  dbo::ptr<User> user = session.find<User>().where("name = ?").bind("John");

  std::string expected = "{\"id\":1,\"name\":\"John\",\"password\":\"Something\",\"role\":42,"
    "\"karma\":13,\"posts_user\":[],\"settings_\":null}";
  
  std::stringstream ss;
  jsonSerialize(user, ss);

  BOOST_REQUIRE_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE( dbo_json_surrogate_id_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  dbo::ptr<HasSurrogate> hasSurrogate;
  {
    dbo::Transaction transaction(session);

    hasSurrogate = session.add(std::make_unique<HasSurrogate>());
    hasSurrogate.modify()->foo = "bar";
  }

  dbo::Transaction transaction(session);

  std::string expected = "{\"alternate_id\":1,\"foo\":\"bar\"}";

  std::stringstream ss;
  jsonSerialize(hasSurrogate, ss);

  BOOST_REQUIRE_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE( dbo_json_natural_id_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  dbo::ptr<HasNatural> hasNatural;
  {
    dbo::Transaction transaction(session);

    hasNatural = session.add(std::make_unique<HasNatural>());
    hasNatural.modify()->myNaturalId = "Nature!";
    hasNatural.modify()->foo = "bar";
  }

  dbo::Transaction transaction(session);

  std::string expected = "{\"natural_id\":\"Nature!\",\"foo\":\"bar\"}";

  std::stringstream ss;
  jsonSerialize(hasNatural, ss);

  BOOST_REQUIRE_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE( dbo_json_composite_key_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  dbo::ptr<HasCoordinateId> hasCoordinateId;
  {
    dbo::Transaction transaction(session);

    hasCoordinateId = session.add(std::make_unique<HasCoordinateId>());
    hasCoordinateId.modify()->position = Coordinate(3, 4);
    hasCoordinateId.modify()->name = "foo";
  }
  
  dbo::Transaction transaction(session);

  std::string expected = "{\"position_x\":3,\"position_y\":4,\"name\":\"foo\"}";

  std::stringstream ss;
  jsonSerialize(hasCoordinateId, ss);

  BOOST_REQUIRE_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE( dbo_json_complex_test )
{
  JsonDboFixture f;

  dbo::Session &session = *f.session_;

  {
    dbo::Transaction transaction(session);

    auto user = std::make_unique<User>();
    user->name = "Joe";
    user->password = "Secret";
    user->role = User::Visitor;
    user->karma = 13;

    dbo::ptr<User> userPtr = session.add(std::move(user));
  }

  dbo::ptr<Post> post;
  dbo::ptr<NestedThing> nestedThing;
  {
    dbo::Transaction transaction(session);

    dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

    post = session.add(std::make_unique<Post>());
    post.modify()->user = joe;
    post.modify()->body = "Lorem ipsum dolor sit amet \"escape me\" something something";
    post.modify()->title = "Hello";

    nestedThing = session.add(std::make_unique<NestedThing>());
    nestedThing.modify()->one = 1;
    nestedThing.modify()->two = 2;
    nestedThing.modify()->three = 3;
    nestedThing.modify()->post = post;
  }

  {
    dbo::Transaction transaction(session);

    dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

    dbo::ptr<Settings> settings = session.add(std::make_unique<Settings>());
    settings.modify()->theme = "fancy-pink";
    joe.modify()->settings = settings;
  }

  dbo::Transaction transaction(session);

  dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

  std::stringstream ss;
  dbo::jsonSerialize(joe, ss);

  std::string joeString = "{\"id\":1,\"name\":\"Joe\",\"password\":\"Secret\",\"role\":0,\"karma\":13,"
    "\"posts_user\":[{\"id\":1,\"user\":1,\"nestedThing_post\":{\"id\":1,\"one\":1,\"two\":2,\"three\":3,\"post\":1},"
    "\"title\":\"Hello\",\"body\":\"Lorem ipsum dolor sit amet \\\"escape me\\\" something something\"}],"
    "\"settings_\":{\"id\":1,\"theme\":\"fancy-pink\",\"user\":1}}";

  BOOST_REQUIRE_EQUAL(ss.str(), joeString);
}

}

#endif
