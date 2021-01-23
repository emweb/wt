/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>
#include <Wt/Dbo/QueryModel.h>

#include "DboFixture.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>

#include <boost/optional.hpp>

#ifdef WT_CXX17
#if __has_include(<optional>)
#include <optional>
#endif // __has_include(<optional>)
#endif // WT_CXX17

//#define SCHEMA "test."
#define SCHEMA ""

#define DEBUG(x) x
//#define DEBUG(x)

#ifdef WT_WIN32
#define timegm _mkgmtime
#endif

namespace dbo = Wt::Dbo;

class A;
class B;
class C;
class D;
class E;
class F;

bool fractionalSeconds = true;

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

enum class Pet {
  Cat,
  Dog,
  Other
};

std::string petToString(Pet p) {
  switch (p) {
  case Pet::Cat:
    return "cat";
  case Pet::Dog:
    return "dog";
  case Pet::Other:
    return "other";
  }
  throw std::invalid_argument("Unknown pet type: " + std::to_string(static_cast<int>(p)));
}

Pet petFromString(const std::string &s) {
  if (s == "cat")
    return Pet::Cat;
  else if (s == "dog")
    return Pet::Dog;
  else if (s == "other")
    return Pet::Other;
  else
    throw std::invalid_argument("Unknown pet type: " + s);
}

std::ostream& operator<< (std::ostream& o, const Coordinate& c)
{
  return o << "(" << c.x << ", " << c.y << ")";
}

namespace Wt {
  namespace Dbo {

    template <class Action>
    void field(Action& action, Coordinate& coordinate, const std::string& name,
	       int size = -1)
    {
      field(action, coordinate.x, name + "_x", 1000);
      field(action, coordinate.y, name + "_y", 1000);
    }

    template<>
    struct sql_value_traits<Pet>
    {
      static std::string type(SqlConnection *conn, int size)
      {
        return sql_value_traits<std::string>::type(conn, size);
      }

      static void bind(Pet p, SqlStatement *statement, int column, int size)
      {
        statement->bind(column, petToString(p));
      }

      static bool read(Pet &p, SqlStatement *statement, int column, int size)
      {
        std::string s;
        bool result = statement->getResult(column, &s, size);
        if (!result)
          return false;
        p = petFromString(s);
        return true;
      }
    };
  }
}

struct DboFixture : DboFixtureBase
{
  DboFixture() :
    DboFixtureBase()
  {
    session_->mapClass<A>(SCHEMA "table_a");
    session_->mapClass<B>(SCHEMA "table_b");
    session_->mapClass<C>(SCHEMA "table_c");
    session_->mapClass<D>(SCHEMA "table_d");
    session_->mapClass<E>(SCHEMA "table_e");
    session_->mapClass<F>(SCHEMA "table_f");

    try {
      session_->dropTables();
    } catch (...) {
    }

    std::cerr << session_->tableCreationSql() << std::endl;

    //session_->dropTables();

    session_->createTables();

    Wt::registerType<Coordinate>();
  }
};

namespace Wt {
  namespace Dbo {

template<>
struct dbo_traits<D> : public dbo_default_traits
{
  typedef Coordinate IdType;
  static IdType invalidId() { return Coordinate(); }
  static const char *surrogateIdField() { return 0; }
  static const char *versionField() { return 0; }
};

template<> struct dbo_traits<const D> : dbo_traits<D> {};

template<>
struct dbo_traits<E> : public dbo_default_traits
{
  static const char *versionField() { return 0; }
};

template<> struct dbo_traits<const E> : dbo_traits<E> {};

  }
}

typedef dbo::collection<dbo::ptr<A>> As;
typedef dbo::collection<dbo::ptr<B>> Bs;
typedef dbo::collection<dbo::ptr<C>> Cs;
typedef dbo::collection<dbo::ptr<D>> Ds;

class A : public dbo::Dbo<A> {
public:
  dbo::ptr<B> b;
  dbo::ptr<D> dthing;
  dbo::ptr<A> parent;
  dbo::ptr<C> c;

  std::vector<unsigned char> binary;
  Wt::WDate date;
  Wt::WTime time;
  Wt::WDateTime datetime;
  Wt::WString wstring;
  Wt::WString wstring2;
  std::string string;
  std::string string2;
  boost::optional<std::string> string3;
#ifdef WT_CXX17
#if __has_include(<optional>)
  std::optional<std::string> string4;
#endif // __has_include(<optional>)
#endif // WT_CXX17
  std::chrono::system_clock::time_point timepoint;
  std::chrono::duration<int, std::milli> timeduration;
  bool checked;
  int i;
  Pet pet;
  ::int64_t i64;
  long long ll;
  float f;
  double d;

  A()
    : timeduration(0),
      checked(false),
      i(0),
      pet(Pet::Other),
      i64(0),
      ll(0),
      f(0),
      d(0)
  { }

  bool operator== (const A& other) const {
    if (binary.size() != other.binary.size()){
          DEBUG(std::cerr << "ERROR: binary.size = " << binary.size()
                << " | " << other.binary.size() << std::endl);
      return false;
    }

    for (unsigned j = 0; j < binary.size(); ++j)
      if (binary[j] != other.binary[j]){
        DEBUG(std::cerr << "ERROR: binary" << std::endl);
        return false;
      }

    //Debug printings:
    if (date != other.date) {
      DEBUG(std::cerr << "ERROR: date = " << date.toString() << " | "
            << other.date.toString() << std::endl);
    }
    if (time  != other.time) {
      DEBUG(std::cerr << "ERROR: time = " << time.toString() << " | "
            << other.time.toString() << std::endl);
    }
    if (datetime  != other.datetime) {
      DEBUG(std::cerr << "ERROR: datetime = " << datetime.toString() << " | "
            << other.datetime.toString() << std::endl);
    }
    if (wstring  != other.wstring) {
      DEBUG(std::cerr << "ERROR: wstring = " << wstring.toUTF8() << " | "
            << other.wstring.toUTF8() << std::endl);
    }
    if (wstring2  != other.wstring2) {
      DEBUG(std::cerr << "ERROR: wstring2 = " << wstring2.toUTF8() << " | "
            << other.wstring2.toUTF8() << std::endl);
    }
    if (string  != other.string) {
      DEBUG(std::cerr << "ERROR: string = " << string << " | " << other.string
            << std::endl);
    }
    if (string2  != other.string2) {
      DEBUG(std::cerr << "ERROR: string2 = " << string2 << " | "
            << other.string2 << std::endl);
    }
    if (string3  != other.string3) {
      DEBUG(std::cerr << "ERROR: string3 = " << (string3 ? *string3 : "<optional empty>") << " | "
        << (other.string3 ? *other.string3 : "<optional empty>") << std::endl);
    }
#ifdef WT_CXX17
#if __has_include(<optional>)
    if (string4 != other.string4) {
      DEBUG(std::cerr << "ERROR: string4 = " << (string4 ? *string4 : "<optional empty>") << " | "
        << (other.string4 ? *other.string4 : "<optional empty>") << std::endl);
    }
#endif // __has_include(<optional>)
#endif // WT_CXX17
    if (timepoint  != other.timepoint) {
      std::time_t t = std::chrono::system_clock::to_time_t(timepoint);
      std::tm *tm = std::gmtime(&t);
      char str[100];
      std::strftime(str, sizeof(str), "%Y-%b-%d %H:%M:%S", tm);
      std::time_t tother = std::chrono::system_clock::to_time_t(other.timepoint);
      std::tm *tmother = std::gmtime(&tother);
      char strother[100];
      std::strftime(strother, sizeof(strother), "%Y-%b-%d %H:%M:%S", tmother);
      DEBUG(std::cerr << "ERROR: timepoint = " <<  str << " | " << strother
            << std::endl);
    }
    if (timeduration  != other.timeduration) {
      std::chrono::system_clock::time_point tp(timeduration);
      std::time_t t = std::chrono::system_clock::to_time_t(tp);
      std::tm *tm = std::gmtime(&t);
      char str[100];
      std::strftime(str, sizeof(str), "%Y-%b-%d %H:%M:%S", tm);
      std::chrono::system_clock::time_point tpother(other.timeduration);
      std::time_t tother = std::chrono::system_clock::to_time_t(tpother);
      std::tm *tmother = std::gmtime(&tother);
      char strother[100];
      std::strftime(strother, sizeof(strother), "%Y-%b-%d %H:%M:%S", tmother);
      std::cout << "Current " << timeduration.count() << " | other " << other.timeduration.count() << std::endl;
      DEBUG(std::cerr << "ERROR: timeduration = " << str << " | "
            << strother << std::endl);
    }
    if (i != other.i) {
      DEBUG(std::cerr << "ERROR: i = " << i << " | " << other.i << std::endl);
    }
    if (pet != other.pet) {
      DEBUG(std::cerr << "ERROR: pet = " << petToString(pet) << " | " << petToString(other.pet) << std::endl);
    }
    if (i64 != other.i64) {
      DEBUG(std::cerr << "ERROR: i64 = " <<i64  << " | "
            << other.i64 << std::endl);
    }
    if (ll != other.ll) {
      DEBUG(std::cerr << "ERROR: ll = " << ll << " | "
            << other.ll << std::endl);
    }
    if (checked != other.checked) {
      DEBUG(std::cerr << "ERROR: checked = " << checked << " | "
            << other.checked << std::endl);
    }
    if (f != other.f) {
      DEBUG(std::cerr << "ERROR: f = " << std::setprecision(10) << f << " | " << other.f << std::endl);
    }
    if (d != other.d) {
      // NOTE: this may fail in dbo_test1 when run under valgrind with postgres on 64-bit Linux
      //       probably due to valgrind emulation of floating point
      DEBUG(std::cerr << "ERROR: d = " << std::setprecision(20) << d << " | " << other.d << std::endl);
    }
    if (b != other.b) {
      DEBUG(std::cerr << "ERROR: b  = " << b  << " | " << other.b << std::endl);
    }
    if (dthing != other.dthing) {
      DEBUG(std::cerr << "ERROR: dthing = " << dthing << " | " << other.dthing
            << std::endl);
    }
    if (parent != other.parent) {
      DEBUG(std::cerr << "ERROR: parent = " << parent << " | " << other.parent
            << std::endl);
    }

    std::cout << "time = " << time.toString("hh:mm:ss.zzz") << " ; " << other.time.toString("hh:mm:ss.zzz") << std::endl;
    std::cout << "string = " << string << " ; " << other.string << std::endl;
    std::cout << "wstring = " << wstring.toUTF8() << " ; " << other.wstring.toUTF8() << std::endl;
    std::cout << "date = " << date.toString() << " ; " << other.date.toString() << std::endl;
    std::cout << "datetime = " << datetime.toString() << " ; " << other.datetime.toString() << std::endl;
    std::cout << "i = " << i << " ; " << other.i << std::endl;
    std::cout << "pet = " << petToString(pet) << " ; " << petToString(other.pet) << std::endl;
    std::cout << "i64 = " << i64 << " ; " << other.i64 << std::endl;
    std::cout << "ll = " << ll << " ; " << other.ll << std::endl;
    std::cout << "f = " << f << " ; " << other.f << std::endl;
    std::cout << "d = " << d << " ; " << other.d << std::endl;
    std::cout << "b = " << b << " ; " << other.b << std::endl;
    std::cout << "dthing = " << dthing << " ; " << other.dthing << std::endl;
    std::cout << "parent = " << parent << " ; " << other.parent << std::endl;

    return date == other.date
      && (time == other.time || !fractionalSeconds)
      && datetime == other.datetime
      && wstring == other.wstring
      && wstring2 == other.wstring2
      && string == other.string
      && string2 == other.string2
      && string3 == other.string3
#ifdef WT_CXX17
#if __has_include(<optional>)
      && string4 == other.string4
#endif // __has_include(<optional>)
#endif // WT_CXX17
      && timepoint == other.timepoint
      && timeduration == other.timeduration
      && i == other.i
      && pet == other.pet
      && i64 == other.i64
      && ll == other.ll
      && checked == other.checked
      && f == other.f
      && d == other.d
      && b == other.b
      && dthing == other.dthing
      && parent == other.parent;
  }

  As             asManyToOne;

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, date, "date");
    dbo::field(a, time, "time");
    dbo::field(a, binary, "binary");
    dbo::field(a, datetime, "datetime");
    dbo::field(a, wstring, "wstring");
    dbo::field(a, wstring2, "wstring2", 30);
    dbo::field(a, string, "string");
    dbo::field(a, string2, "string2", 50);
    dbo::field(a, string3, "string3");
#ifdef WT_CXX17
#if __has_include(<optional>)
    dbo::field(a, string4, "string4");
#endif // __has_include(<optional>)
#endif // WT_CXX17
    dbo::field(a, timepoint, "timepoint");
    dbo::field(a, timeduration, "timeduration");
    dbo::field(a, i, "i");
    dbo::field(a, pet, "pet");
    dbo::field(a, i64, "i64");
    dbo::field(a, ll, "ll");
    dbo::field(a, checked, "checked");
    dbo::field(a, f, "f");
    dbo::field(a, d, "d");

    dbo::belongsTo(a, b, "b");
    dbo::belongsTo(a, c);

    if (a.session()) {
      dbo::belongsTo(a, dthing);

      dbo::belongsTo(a, parent, a.session()->template tableName<A>()
		     + std::string("_parent"));
      dbo::hasMany(a, asManyToOne, dbo::ManyToOne,
		   a.session()->template tableName<A>()
		   + std::string("_parent"));
    }
  }
};

class B {
public:
  enum State {
    State1 = 0,
    State2 = 1
  };

  std::string name;
  State state;

  As    asManyToOne;
  Cs    csManyToMany;
  Cs    csManyToOne;

  B() : state(State1) { }

  B(const std::string& aName, State aState)
    : name(aName), state(aState)
  { }

  bool operator== (const B& other) const {
    return name == other.name
      && state == other.state;
  }

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, state, "state");
    dbo::field(a, name, "name", 1000);

    dbo::hasMany(a, asManyToOne, dbo::ManyToOne, "b");
    dbo::hasMany(a, csManyToMany, dbo::ManyToMany, SCHEMA "b_c", "the_b",
		   dbo::NotNull
		 | dbo::OnDeleteCascade
		 | dbo::OnUpdateCascade);
    dbo::hasMany(a, csManyToOne, dbo::ManyToOne, "b2");
  }
};

class C {
public:
  std::string name;
  
  dbo::weak_ptr<A> aOneToOne;
  dbo::ptr<B> b;
  dbo::weak_ptr<D> dOneToOne;

  Bs    bsManyToMany;
  Ds    dsManyToMany;

  C() { }

  C(const std::string& aName)
    : name(aName)
  { }

  bool operator== (const C& other) const {
    return name == other.name;
  }

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name", 1000);

    dbo::belongsTo(a, b, "b2");

    dbo::hasMany(a, bsManyToMany, dbo::ManyToMany, SCHEMA "b_c", "the_c",
		   dbo::NotNull
		 | dbo::OnDeleteCascade
		 | dbo::OnUpdateCascade );
    dbo::hasMany(a, dsManyToMany, dbo::ManyToMany, SCHEMA "c_d");
    dbo::hasOne(a, aOneToOne);
    dbo::hasOne(a, dOneToOne, "c_d2");
  }
};

class D {
public:
  Coordinate id;
  std::string name;

  dbo::ptr<C> c;

  As    asManyToOne;
  Cs    csManyToMany;

  D() { }
  D(const Coordinate& anId, const std::string& aName)
    : id(anId),
      name(aName)
  { }

  template <class Action>
  void persist(Action& a)
  {
    dbo::id(a, id, "id");
    dbo::field(a, name, "name", 1000);

    dbo::belongsTo(a, c, "c_d2");
    dbo::hasMany(a, asManyToOne, dbo::ManyToOne);
    dbo::hasMany(a, csManyToMany, dbo::ManyToMany, SCHEMA "c_d");
  }
};

class E {
public:
  std::string name;

  E() { }

  E(const std::string& aName)
    : name(aName)
  { }

  bool operator== (const E& other) const {
    return name == other.name;
  }

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name", 1000);
  }
};

class F {
public:
  std::string firstName;
  std::string lastName;
  std::string gender;

  F() { }

  F(const std::string& aFirstName, const std::string& aLastName, const std::string& aGender)
    : firstName(aFirstName), lastName(aLastName), gender(aGender)
  { }

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, firstName, "first_name", 64);
    dbo::field(a, lastName,  "last_name",  64);
    dbo::field(a, gender,  "gender",  64);
  }
};

BOOST_AUTO_TEST_SUITE( DBO_TEST_SUITE_NAME )

BOOST_AUTO_TEST_CASE( dbo_test1 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  A a1;
  a1.datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
  for (unsigned i = 0; i < 255; ++i)
    a1.binary.push_back(i);
  a1.date = Wt::WDate(1976, 6, 14);
  a1.time = Wt::WTime(13, 14, 15, 102);
  // There is a bug in the implementation of milliseconds in mariadb c client
#ifdef MYSQL
  a1.time = Wt::WTime(13, 14, 15);
#endif //MYSQL

  a1.wstring = "Hello";

  a1.wstring2 = Wt::WString::fromUTF8("Kitty euro\xe2\x82\xac greek \xc6\x94 \xf0\x9f\x90\xb1");
  a1.string = "There";
  a1.string2 = "Big Owl";
  std::tm timeInfo = std::tm();
  timeInfo.tm_year = 2005 - 1900;
  timeInfo.tm_mon = 0; //Jan
  timeInfo.tm_mday = 1;
  timeInfo.tm_hour = 1;
  timeInfo.tm_min = 2;
  timeInfo.tm_sec = 3;
  std::time_t t = timegm(&timeInfo);
  a1.timepoint = std::chrono::system_clock::from_time_t(t);
  a1.timeduration = std::chrono::hours(1) + std::chrono::seconds(10);
  a1.checked = true;
  a1.i = 42;
  a1.pet = Pet::Cat;
  a1.i64 = 9223372036854775805LL;
  a1.ll = 6066005651767221LL;
  a1.f = (float)42.42;
  a1.d = 42.424242;

  /* Create an A, check that it is found during the same transaction  */
  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> ptrA = session_->addNew<A>(a1);

    BOOST_REQUIRE(ptrA->session() == session_);

    BOOST_REQUIRE(ptrA->self() == ptrA);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    BOOST_REQUIRE(*a2 == a1);

    BOOST_REQUIRE(a2->self() == ptrA);
  }

  /* Check that A is found during other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();

    BOOST_REQUIRE(*a2 == a1);

    a2.modify()->parent = a2;
  }

  /* Remove the A, check it is no longer found during the same transaction */
  {
    dbo::Transaction t(*session_);

    {
      As allAs = session_->find<A>();

      BOOST_REQUIRE(allAs.size() == 1);
      dbo::ptr<A> a2 = *allAs.begin();

      BOOST_REQUIRE(a2->parent == a2);

      // NOTE: if you do not reset self-reference in parent, memory is leaked
      a2.modify()->parent.reset();

#ifdef MYSQL
      a2.flush();
#endif

      a2.remove();
    }

    {
      As allAs = session_->find<A>();
      BOOST_REQUIRE(allAs.size() == 0);
    }

    t.commit();
  }

  /* Check it is no longer found during other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 0);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test2 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  A a1;
  a1.datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
  a1.date = Wt::WDate(1980, 12, 4);
  a1.time = Wt::WTime(12, 13, 14, 123);
  // There is a bug in the implementation of milliseconds in mariadb c client
#ifdef MYSQL
  a1.time = Wt::WTime(13, 14, 15);
#endif //MYSQL
  a1.timeduration = std::chrono::duration<int, std::milli>(0);
  a1.wstring = "Hello";
  a1.string = "There";
  a1.checked = false;
  a1.i = 42;
  a1.pet = Pet::Dog;
  a1.i64 = 9223372036854775804LL;
  a1.ll = 6066005651767221LL;
  a1.f = (float)42.42;
  a1.d = 42.424242;

  B b1;
  b1.name = "b1";
  b1.state = B::State1;
  /* Create an A + B  */
  {
    dbo::Transaction t(*session_);
    a1.b = session_->addNew<B>(b1);
    dbo::ptr<A> a = session_->addNew<A>(a1);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    BOOST_REQUIRE(*a2 == a1);
  }

  /* Check that A + B are found in other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    std::cout << "a2 time " << a2->time.toString() << std::endl;
    std::cout << "a1 time " << a1.time.toString() << std::endl;
    std::cout << "a2 " << a2->timeduration.count() << std::endl;
    std::cout << "a1 " << a1.timeduration.count() << std::endl;
    BOOST_REQUIRE(*a2 == a1);
    BOOST_REQUIRE(*a2->b == b1);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test3 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  /* Create B's many-to-many C's  */
  {
    dbo::Transaction t(*session_);

    dbo::ptr<B> b1 = session_->addNew<B>("b1", B::State1);
    dbo::ptr<B> b2 = session_->addNew<B>("b2", B::State2);
    dbo::ptr<B> b3 = session_->addNew<B>("b3", B::State1);

    dbo::ptr<C> c1 = session_->addNew<C>("c1");
    dbo::ptr<C> c2 = session_->addNew<C>("c2");
    dbo::ptr<C> c3 = session_->addNew<C>("c3");

    B *m = b1.modify();
    m->csManyToMany.insert(c1);

    BOOST_REQUIRE(b1->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->bsManyToMany.size() == 1);

    BOOST_REQUIRE(c1->bsManyToMany.count(b1) == 1);
    BOOST_REQUIRE(c1->bsManyToMany.count(b2) == 0);
    BOOST_REQUIRE(c1->bsManyToMany.count(b3) == 0);

    BOOST_REQUIRE(b1->csManyToMany.count(c1) == 1);
    BOOST_REQUIRE(b1->csManyToMany.count(c2) == 0);
    BOOST_REQUIRE(b1->csManyToMany.count(c3) == 0);

    b1.modify()->csManyToMany.insert(c2);

    BOOST_REQUIRE(b1->csManyToMany.size() == 2);
    BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->bsManyToMany.size() == 1);
    BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

    Cs cs = b1->csManyToMany;
    for (Cs::const_iterator i = cs.begin(); i != cs.end(); ++i) {
      std::cerr << "C: " << (*i)->name << std::endl;
    }

    b1.modify()->csManyToMany.erase(c2);

    BOOST_REQUIRE(b1->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->bsManyToMany.size() == 0);
    BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

    b1.modify()->csManyToMany.insert(c2);
    b1.modify()->csManyToMany.erase(c2);

    BOOST_REQUIRE(b1->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->bsManyToMany.size() == 0);
    BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

    /*
     * I had to write this example to convince myself that I actually
     * implemented this so that this kind of things simply work !
     */
    dbo::collection<std::string> names
      = session_->query<std::string>("select \"name\" from " SCHEMA "\"table_b\"");

    for (dbo::collection<std::string>::const_iterator i = names.begin();
	 i != names.end(); ++i)
      std::cerr << *i << std::endl;
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<B> b1 = session_->query<dbo::ptr<B>>
      ("select distinct B from " SCHEMA "\"table_b\" B ").where("B.\"name\" = ?").bind("b1");

    std::size_t count = session_->query<dbo::ptr<B>>
      ("select distinct B from " SCHEMA "\"table_b\" B ").where("B.\"name\" = ?").bind("b1")
      .resultList().size();

    dbo::ptr<C> c1 = session_->find<C>().where("\"name\" = ?").bind("c1");

    BOOST_REQUIRE(count == 1);
    BOOST_REQUIRE(b1->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->bsManyToMany.size() == 1);

    //this test case does not work in firebird,
    //the name field is of the 'blob subtype text' datatype,
    //and according to the firebird documentation,
    //order by is not supported on columns of this datatype 
    //http://www.firebirdsql.org/refdocs/langrefupd21-blob.html
#ifndef FIREBIRD
    typedef dbo::collection<dbo::ptr<C>> Cs;

    Cs c2 = session_->find<C>().orderBy("\"name\" desc");
    Cs c3 = session_->find<C>().orderBy("\"name\" desc").limit(2);

    std::vector<std::string> c2_compare;
    c2_compare.push_back("c3");
    c2_compare.push_back("c2");
    c2_compare.push_back("c1");

    std::vector<std::string> c3_compare;
    c3_compare.push_back("c3");
    c3_compare.push_back("c2");

    int c = 0; 
    BOOST_REQUIRE(c2.size() == c2_compare.size());
    for (Cs::const_iterator i = c2.begin(); i != c2.end(); ++i)
      BOOST_REQUIRE((*i)->name == c2_compare[c++]);

    c = 0;
    BOOST_REQUIRE(c3.size() == c3_compare.size());
    for (Cs::const_iterator i = c3.begin(); i != c3.end(); ++i)
      BOOST_REQUIRE((*i)->name == c3_compare[c++]);
#endif
  }
}

BOOST_AUTO_TEST_CASE( dbo_test4 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();

    BOOST_REQUIRE(a1->self() == a1);

    a1.modify()->datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					  Wt::WTime(12, 11, 31));
    a1.modify()->date = Wt::WDate(1980, 12, 4);
    a1.modify()->wstring = "Hello";
    a1.modify()->string = "There";
    a1.modify()->i = 42;
    a1.modify()->pet = Pet::Cat;
    a1.modify()->i64 = 9223372036854775803LL;
    a1.modify()->ll = 6066005651767220LL;
    a1.modify()->f = (float)42.42;
    a1.modify()->d = 42.424242;

    dbo::ptr<A> a2 = dbo::make_ptr<A>(*a1);
    a2.modify()->wstring = "Oh my god";
    a2.modify()->i = 142;

    dbo::ptr<B> b = dbo::make_ptr<B>();
    b.modify()->name = "b";
    b.modify()->state = B::State1;

    a1.modify()->b = b;
    a2.modify()->b = b;

    session_->add(a1);
    session_->add(a2);
    session_->add(b);

    typedef std::tuple<dbo::ptr<B>, dbo::ptr<A>> BA;
    typedef dbo::collection<BA> BAs;

    // The query below becomes:
    //    select count(1) from ( select B."id", B."name", A."id", A."date", A."b_id" 
    //    from "table_b" B join "table_a" A on A."b_id" = B."id"); 
    // when it is used to return the size of the collection.
    // This is not valid SQL by the SQL standard definition,
    // because 2 id fields are mentioned in the select clause.
    //
    // A valid alternative would be:
    //   select count(1) from ( select B."id", B."name", A."id" as id2, A."date",
    //   A."b_id" from "table_b" B join "table_a" A on A."b_id" = B."id");
    //
    // Firebird, MySQL and SQL Server are not able to execute this query.

#if !defined(FIREBIRD) && !defined(MYSQL) && !defined(MSSQLSERVER)
    dbo::Query<BA> q = session_->query<BA>
      ("select B, A "
       "from " SCHEMA "\"table_b\" B join " SCHEMA "\"table_a\" A on A.\"b_id\" = B.\"id\"")
      .orderBy("A.\"i\"");

    std::vector<dbo::FieldInfo> fields = q.fields();
    std::vector<dbo::FieldInfo> fields2 = q.fields();

    BOOST_REQUIRE(fields.size() == fields2.size());

    BAs bas;
    bas = q.resultList();
    BOOST_REQUIRE(bas.size() == 2);

    int ii = 0;
    for (BAs::const_iterator i = bas.begin(); i != bas.end(); ++i) {
      dbo::ptr<A> a_result;
      dbo::ptr<B> b_result;
      std::tie(b_result, a_result) = *i;

      if (ii == 0) {
	BOOST_REQUIRE(a_result == a1);
	BOOST_REQUIRE(b_result == b);
      } else if (ii == 1) {
	BOOST_REQUIRE(a_result == a2);
	BOOST_REQUIRE(b_result == b);
      }

      ++ii;
    }

    BOOST_REQUIRE(ii == 2);
#endif // !defined(FIREBIRD) && !defined(MYSQL) && !defined(MSSQLSERVER)
  }
}

BOOST_AUTO_TEST_CASE( dbo_test4b )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    dbo::ptr<A> a2 = dbo::make_ptr<A>();
    dbo::ptr<A> a3 = dbo::make_ptr<A>();
    dbo::ptr<A> a4 = dbo::make_ptr<A>();
    dbo::ptr<A> a5 = dbo::make_ptr<A>();
    dbo::ptr<A> a6 = dbo::make_ptr<A>();

    dbo::ptr<B> b1 = dbo::make_ptr<B>();
    dbo::ptr<B> b2 = dbo::make_ptr<B>();
    dbo::ptr<B> b3 = dbo::make_ptr<B>();

    dbo::ptr<C> c1 = dbo::make_ptr<C>();
    dbo::ptr<C> c2 = dbo::make_ptr<C>();
    dbo::ptr<C> c3 = dbo::make_ptr<C>();

    a1.modify()->wstring = "a1";
    a2.modify()->wstring = "a2";
    a3.modify()->wstring = "a3";
    a4.modify()->wstring = "a4";
    a5.modify()->wstring = "a5";
    a6.modify()->wstring = "a6";

    b1.modify()->name = "b1";
    b2.modify()->name = "b2";
    b3.modify()->name = "b3";

    c1.modify()->name = "c1";
    c2.modify()->name = "c2";
    c3.modify()->name = "c3";

    a1.modify()->b = b1;
    a2.modify()->b = b1;
    a3.modify()->b = b2;
    a4.modify()->b = b2;
    a5.modify()->b = b3;
    a6.modify()->b = b3;

    c1.modify()->b = b1;
    c2.modify()->b = b2;
    c3.modify()->b = b3;

    session_->add(b1);
    session_->add(b2);
    session_->add(b3);
    session_->add(a1);
    session_->add(a2);
    session_->add(a3);
    session_->add(a4);
    session_->add(a5);
    session_->add(a6);
    session_->add(c1);
    session_->add(c2);
    session_->add(c3);
  }

  {
    dbo::Transaction t(*session_);

    typedef std::tuple<dbo::ptr<A>, dbo::ptr<B>, dbo::ptr<C>> ABC;
    typedef dbo::collection<ABC> C_ABCs;
    typedef std::vector<ABC> ABCs;

#if !defined(FIREBIRD) && !defined(MYSQL)
    dbo::Query<ABC> q = session_->query<ABC>
      ("select A, B, C " 
       "from " SCHEMA "\"table_a\" A join " SCHEMA "\"table_b\" B on (A.\"b_id\" = B.\"id\") join " SCHEMA "\"table_c\" C on (C.\"b2_id\" = B.\"id\")")
        .orderBy("A.\"id\"");

    C_ABCs c_abcs = q.resultList();
    ABCs abcs(c_abcs.begin(), c_abcs.end());

    BOOST_REQUIRE(abcs.size() == 6);

    int ii = 0;
    for (ABCs::const_iterator i = abcs.begin(); i != abcs.end(); ++i) {
      dbo::ptr<A> a_result;
      dbo::ptr<B> b_result;
      dbo::ptr<C> c_result;
      std::tie(a_result, b_result, c_result) = *i;

      switch (ii)
      {
        case 0:
            BOOST_REQUIRE(a_result->wstring == "a1");
            BOOST_REQUIRE(b_result->name == "b1");
            BOOST_REQUIRE(c_result->name == "c1");
            break;
        case 1:
            BOOST_REQUIRE(a_result->wstring == "a2");
            BOOST_REQUIRE(b_result->name == "b1");
            BOOST_REQUIRE(c_result->name == "c1");
            break;
        case 2:
            BOOST_REQUIRE(a_result->wstring == "a3");
            BOOST_REQUIRE(b_result->name == "b2");
            BOOST_REQUIRE(c_result->name == "c2");
            break;
        case 3:
            BOOST_REQUIRE(a_result->wstring == "a4");
            BOOST_REQUIRE(b_result->name == "b2");
            BOOST_REQUIRE(c_result->name == "c2");
            break;
        case 4:
            BOOST_REQUIRE(a_result->wstring == "a5");
            BOOST_REQUIRE(b_result->name == "b3");
            BOOST_REQUIRE(c_result->name == "c3");
            break;
        case 5:
            BOOST_REQUIRE(a_result->wstring == "a6");
            BOOST_REQUIRE(b_result->name == "b3");
            BOOST_REQUIRE(c_result->name == "c3");
            break;
      }

      ++ii;
    }

    BOOST_REQUIRE(ii == 6);
#endif //FIREBIRD && MYSQL
  }
}

BOOST_AUTO_TEST_CASE( dbo_test4c )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    dbo::ptr<A> a2 = dbo::make_ptr<A>();
    dbo::ptr<A> a3 = dbo::make_ptr<A>();
    dbo::ptr<A> a4 = dbo::make_ptr<A>();

    dbo::ptr<B> b1 = dbo::make_ptr<B>();
    dbo::ptr<B> b2 = dbo::make_ptr<B>();

    dbo::ptr<C> c1 = dbo::make_ptr<C>();
    dbo::ptr<C> c2 = dbo::make_ptr<C>();

    a1.modify()->wstring = "a1";
    a2.modify()->wstring = "a2";
    a3.modify()->wstring = "a3";
    a4.modify()->wstring = "a4";

    b1.modify()->name = "b1";
    b2.modify()->name = "b2";

    c1.modify()->name = "c1";
    c2.modify()->name = "c2";

    a1.modify()->b = b1;
    a1.modify()->c = c1;

    // a2 has no c
    a2.modify()->b = b2;

    // a3 has no b
    a3.modify()->c = c2;

    // a4 has no b or c

    session_->add(b1);
    session_->add(b2);
    session_->add(c1);
    session_->add(c2);
    session_->add(a1);
    session_->add(a2);
    session_->add(a3);
    session_->add(a4);
  }

  {
    dbo::Transaction t(*session_);

    typedef std::tuple<dbo::ptr<A>, dbo::ptr<B>, dbo::ptr<C>> ABC;
    typedef dbo::collection<ABC> C_ABCs;
    typedef std::vector<ABC> ABCs;

#if !defined(FIREBIRD) && !defined(MYSQL)
    dbo::Query<ABC> q = session_->query<ABC>
      ("select A, B, C "
       "from " SCHEMA "\"table_a\" A "
       "left join " SCHEMA "\"table_b\" B on A.\"b_id\" = B.\"id\" "
       "left join " SCHEMA "\"table_c\" C on A.\"" SCHEMA "table_c_id\" = C.\"id\"")
      .orderBy("A.\"id\"");

    C_ABCs c_abcs = q.resultList();
    ABCs abcs(c_abcs.begin(), c_abcs.end());

    BOOST_REQUIRE(abcs.size() == 4);

    int ii = 0;
    for (ABCs::const_iterator i = abcs.begin(); i != abcs.end(); ++i) {
      dbo::ptr<A> a_result;
      dbo::ptr<B> b_result;
      dbo::ptr<C> c_result;
      std::tie(a_result, b_result, c_result) = *i;

      switch (ii)
      {
        case 0:
            BOOST_REQUIRE(a_result->wstring == "a1");
            BOOST_REQUIRE(b_result->name == "b1");
            BOOST_REQUIRE(c_result->name == "c1");
            break;
        case 1:
            BOOST_REQUIRE(a_result->wstring == "a2");
            BOOST_REQUIRE(b_result->name == "b2");
            BOOST_REQUIRE(!a_result->c);
            break;
        case 2:
            BOOST_REQUIRE(a_result->wstring == "a3");
            BOOST_REQUIRE(!a_result->b);
            BOOST_REQUIRE(c_result->name == "c2");
            break;
        case 3:
            BOOST_REQUIRE(a_result->wstring == "a4");
            BOOST_REQUIRE(!a_result->b);
            BOOST_REQUIRE(!a_result->c);
            break;
      }

      ++ii;
    }

    BOOST_REQUIRE(ii == 4);
#endif //FIREBIRD && MYSQL
  }
}

BOOST_AUTO_TEST_CASE( dbo_test5 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					  Wt::WTime(12, 11, 31));
    a1.modify()->date = Wt::WDate(1976, 11, 1);
    a1.modify()->wstring = "Hello";
    a1.modify()->string = "There";
    a1.modify()->i = 42;
    a1.modify()->i64 = 9223372036854775802LL;
    a1.modify()->ll = 6066005651767219LL;
    a1.modify()->f = (float)42.42;
    a1.modify()->d = 42.424242;

    dbo::ptr<A> a2 = dbo::make_ptr<A>(*a1);
    a2.modify()->wstring = "Oh my god";
    a2.modify()->i = 142;

    dbo::ptr<B> b = dbo::make_ptr<B>();
    b.modify()->name = "b";
    b.modify()->state = B::State1;

    a1.modify()->b = b;
    a2.modify()->b = b;

    session_->add(a1);
    session_->add(a2);
    session_->add(b);

    session_->flush();

    t.rollback();

    a1.remove();
    a2.remove();
    b.remove();
  }

  {
    dbo::Transaction t(*session_);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test6 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					  Wt::WTime(12, 11, 31));
    a1.modify()->date = Wt::WDate(1980, 1, 1);
    a1.modify()->wstring = "Hello";
    a1.modify()->string = "There";
    a1.modify()->i = 42;
    a1.modify()->i64 = 9223372036854775802LL;
    a1.modify()->ll = 6066005651767219LL;
    a1.modify()->f = (float)42.42;
    a1.modify()->d = 42.424242;

    session_->add(a1);
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = session_->find<A>();
    a1.modify()->i = 41;
    a1.flush();
    t.rollback();

    session_->rereadAll();

    dbo::Transaction t2(*session_);

    dbo::ptr<A> a2 = session_->find<A>();

    BOOST_REQUIRE(a1 == a2);
    BOOST_REQUIRE(a1->i == 42);

    t2.commit();
  }
}

BOOST_AUTO_TEST_CASE( dbo_test7 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

#ifndef FIREBIRD
#ifndef ORACLE //todo: ask koen http://stackoverflow.com/questions/1881853/oracle-select-without-from
  {
    dbo::Transaction t(*session_);

    std::string result = session_->query<std::string > ("select 'dima '' ? '");
    BOOST_REQUIRE(result == "dima ' ? ");
  }
#endif // ORACLE
#endif //FIREBIRD 

  int aId = -1;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					  Wt::WTime(12, 11, 31));
    a1.modify()->date = Wt::WDate(1980, 1, 1);
    a1.modify()->wstring = "Hello";
    a1.modify()->string = "There";
    a1.modify()->i = 42;
    a1.modify()->i64 = 9223372036854775801LL;
    a1.modify()->ll = 6066005651767219LL;
    a1.modify()->f = (float)42.42;
    a1.modify()->d = 42.424242;

    session_->add(a1);
    a1.flush();

    aId = (int)a1.id();
  }

  {
    dbo::Transaction t(*session_);
    int id1, id2;

    std::tie(id1, id2) = session_->query<std::tuple<int, int>>
      ("select \"id\", \"id\" from " SCHEMA "\"table_a\"").resultValue();

    BOOST_REQUIRE(id1 == aId);
    BOOST_REQUIRE(id2 == aId);

#ifdef POSTGRES
    dbo::ptr<A> a;
    int id;
    std::tie(a, id) = session_->query<std::tuple<dbo::ptr<A>, int>>
      ("select (a), a.\"id\" from " SCHEMA "\"table_a\" a").resultValue();

    BOOST_REQUIRE(id == aId);
    BOOST_REQUIRE(a.id() == aId);
#endif
  }
}

BOOST_AUTO_TEST_CASE( dbo_test8 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    session_->execute("delete from " SCHEMA "\"table_a\"");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test9 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a = dbo::make_ptr<A>();

    typedef dbo::query_result_traits<dbo::ptr<A>> A_traits;

    std::vector<Wt::cpp17::any> values;
    A_traits::getValues(a, values);

    std::cerr << values.size() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( dbo_test10 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<D> d = dbo::make_ptr<D>();

    BOOST_REQUIRE(Wt::asString(Wt::cpp17::any(Coordinate(10, 4)))
		  == "(10, 4)");

    d.modify()->id = Coordinate(42, 43);
    d.modify()->name = "Object @ (42, 43)";

    session_->add(d);

    t.commit();

    // No transaction, but should just fetch it from the session.
    // This checks that saving the dbo sets the id properly
    BOOST_REQUIRE(session_->load<D>(Coordinate(42, 43)) == d);
  }

  {
    dbo::Transaction t2(*session_);

    try {
      session_->load<D>(Coordinate(10, 11));
      BOOST_REQUIRE(false); // Expected an exception
    } catch (const dbo::ObjectNotFoundException&) {
    }

    dbo::ptr<D> d2 = session_->load<D>(Coordinate(42, 43));
 
    BOOST_REQUIRE(d2 && d2.id() == Coordinate(42, 43));
     
    dbo::ptr<C> c1 = session_->addNew<C>("c1");
    dbo::ptr<C> c2 = session_->addNew<C>("c2");
    dbo::ptr<C> c3 = session_->addNew<C>("c3");

    d2.modify()->csManyToMany.insert(c1);

    BOOST_REQUIRE(d2->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->dsManyToMany.size() == 1);

    BOOST_REQUIRE(d2->csManyToMany.count(c1) == 1);
    std::cerr << "Failing now" << std::endl;

    BOOST_REQUIRE(c1->dsManyToMany.count(d2) == 1);

    d2.modify()->csManyToMany.insert(c2);

    BOOST_REQUIRE(d2->csManyToMany.size() == 2);
    BOOST_REQUIRE(c1->dsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->dsManyToMany.size() == 1);
    BOOST_REQUIRE(c3->dsManyToMany.size() == 0);

    d2.modify()->csManyToMany.erase(c2);

    BOOST_REQUIRE(d2->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->dsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->dsManyToMany.size() == 0);
    BOOST_REQUIRE(c3->dsManyToMany.size() == 0);

    d2.modify()->csManyToMany.insert(c2);
    d2.modify()->csManyToMany.erase(c2);

    BOOST_REQUIRE(d2->csManyToMany.size() == 1);
    BOOST_REQUIRE(c1->dsManyToMany.size() == 1);
    BOOST_REQUIRE(c2->dsManyToMany.size() == 0);
    BOOST_REQUIRE(c3->dsManyToMany.size() == 0);
  }

  {
    /*
     * Check that we fail gracefully when inserting an object with a
     * duplicate ID
     */
    dbo::Transaction t(*session_);

    dbo::ptr<D> d = dbo::make_ptr<D>();

    d.modify()->id = Coordinate(42, 43);
    d.modify()->name = "Object2 @ (42, 43)";

    session_->add(d);

    bool caught = false;
    try {
      std::cerr << "The test was - check that we fail gracefully when inserting "
	"an object with a duplicate ID (search try{)."<< std::endl;
      t.commit();
    } catch (std::exception& e) {
      std::cerr << "Catching exception: " << e.what() << std::endl;
      caught = true;
    }

    BOOST_REQUIRE(caught);

    t.rollback();

    {
      dbo::Transaction t2(*session_);

      dbo::ptr<D> d3 = session_->find<D>();
      BOOST_REQUIRE(d3.id() == Coordinate(42, 43));
    }
  }

  bool caught = false;

  try {
    dbo::Transaction outer(*session_);

    {
      /*
       * Check that we fail gracefully when inserting an object with a
       * duplicate ID
       */
      dbo::Transaction t(*session_);

      dbo::ptr<D> d = dbo::make_ptr<D>();

      d.modify()->id = Coordinate(42, 43);
      d.modify()->name = "Object2 @ (42, 43)";

      session_->add(d);

      BOOST_REQUIRE(t.commit() == false); // Doesn't actaully commit
    }
  } catch (std::exception& e) {
    std::cerr << "Catching exception: " << e.what() << std::endl;
    caught = true;
  }

  BOOST_REQUIRE(caught);
}

BOOST_AUTO_TEST_CASE( dbo_test11 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    session_->addNew<C>("c1");

    dbo::Query<dbo::ptr<C>> query = session_->find<C>();
    dbo::QueryModel<dbo::ptr<C>> *model = new dbo::QueryModel<dbo::ptr<C>>();

    model->setQuery(query);

    t.commit();

    Wt::cpp17::any d;

    model->addAllFieldsAsColumns();

    BOOST_REQUIRE(model->columnCount() == 4);
    BOOST_REQUIRE(model->rowCount() == 1);

    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "name");
    BOOST_REQUIRE(Wt::asString(model->headerData(3)) == "b2_id");

    d = model->data(0, 2);
    BOOST_REQUIRE(Wt::asString(d) == "c1");

    model->setData(0, 2, std::string("changed"));

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "changed");

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "changed");

    {
      dbo::Transaction t2(*session_);
      dbo::ptr<C> c = session_->find<C>();
      BOOST_REQUIRE(c->name == "changed");
    }

    model->insertRow(1);
    model->setData(1, 2, std::string("new C"));

    {
      dbo::Transaction t2(*session_);
      BOOST_REQUIRE(session_->find<C>().resultList().size() == 2);
    }

    model->removeRows(0, 2);

    {
      dbo::Transaction t2(*session_);
      BOOST_REQUIRE(session_->find<C>().resultList().size() == 0);
    }

    delete model;
  }
}

BOOST_AUTO_TEST_CASE( dbo_test12 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    session_->addNew<D>(Coordinate(5, 6), "yes");
    dbo::ptr<D> d1 = session_->find<D>();

    BOOST_REQUIRE(d1->name == "yes");
    BOOST_REQUIRE(d1->id == Coordinate(5, 6));

    session_->addNew<C>("c1");

    dbo::Query<dbo::ptr<C>> query = session_->find<C>();
    dbo::QueryModel<dbo::ptr<C>> *model = new dbo::QueryModel<dbo::ptr<C>>();

    model->setQuery(query);

    Wt::cpp17::any d;

    model->addAllFieldsAsColumns();

    BOOST_REQUIRE(model->columnCount() == 4);
    BOOST_REQUIRE(model->rowCount() == 1);

    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "name");
    BOOST_REQUIRE(Wt::asString(model->headerData(3)) == "b2_id");

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "c1");

    model->setData(0, 2, std::string("changed"));

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "changed");

    BOOST_REQUIRE(model->resultRow(0) == session_->find<C>());
    BOOST_REQUIRE(model->stableResultRow(0) == session_->find<C>());

    {
      dbo::ptr<C> c = session_->find<C>();
      BOOST_REQUIRE(c->name == "changed");
    }

    model->insertRow(1);
    model->setData(1, 2, std::string("new C"));

    BOOST_REQUIRE(session_->find<C>().resultList().size() == 2);

    model->removeRow(0);

    BOOST_REQUIRE(model->rowCount() == 1);

    model->setData(0, 2, std::string("changed again"));

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "changed again");

    BOOST_REQUIRE(session_->find<C>().resultList().size() == 1);

    {
      dbo::ptr<C> c = session_->find<C>();
      BOOST_REQUIRE(c->name == "changed again");
    }

    t.commit();

    delete model;
  }
}

BOOST_AUTO_TEST_CASE( dbo_test13 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<B> b1 = session_->addNew<B>("b1", B::State1);
    dbo::ptr<B> b2 = session_->addNew<B>("b2", B::State2);
    dbo::ptr<B> b3 = session_->addNew<B>("b3", B::State1);


    {
      dbo::collection<dbo::ptr<B>> c;
      c = session_->query<dbo::ptr<B>>
	("select B from " SCHEMA "\"table_b\" B ")
	.where("B.\"state\" = ?").orderBy("B.\"name\"")
	.limit(1).bind(0);

      BOOST_REQUIRE(c.size() == 1);
    }

    dbo::ptr<B> d = session_->query<dbo::ptr<B>>
      ("select B from " SCHEMA "\"table_b\" B ")
      .where("B.\"state\" = ?").orderBy("B.\"name\"")
      .limit(1).bind(0);


    BOOST_REQUIRE(d == b1 || d == b3);

  }
}

BOOST_AUTO_TEST_CASE( dbo_test14 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  A a1;
  a1.datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
  a1.date = Wt::WDate(1980, 12, 4);
  a1.time = Wt::WTime(12, 13, 14, 123);
  a1.timeduration = std::chrono::duration<int, std::milli>(0);
  a1.wstring = "Hello";
  a1.string = "There";
  a1.checked = false;
  a1.i = 42;
  a1.pet = Pet::Other;
  a1.i64 = 9223372036854775804LL;
  a1.ll = 6066005651767221LL;
  a1.f = (float)42.42;
  a1.d = 42.424242;

  B b1;
  b1.name = "b1";
  b1.state = B::State1;

  /* Create an A + B  */
  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->addNew<A>(a1);
    dbo::ptr<B> b = session_->addNew<B>(b1);

    BOOST_REQUIRE(!a->b);

    b.modify()->asManyToOne.insert(a);

    BOOST_REQUIRE(b->asManyToOne.count(dbo::ptr<A>()) == 0);

    BOOST_REQUIRE(a->b == b);
    BOOST_REQUIRE(b->asManyToOne.count(a) == 1);

    b.modify()->asManyToOne.erase(a);

    BOOST_REQUIRE(!a->b);

    b.modify()->asManyToOne.insert(a);
  }

  /* Check that A + B are found in other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    BOOST_REQUIRE(*a2->b == b1);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test15 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a = session_->addNew<A>();
    dbo::ptr<B> b = session_->addNew<B>("b", B::State1);
    a.modify()->b = b;

    {
      dbo::collection<dbo::ptr<A>> c = session_->query<dbo::ptr<A>>
	("select A from " SCHEMA "\"table_a\" A ")
	.where("\"b_id\" = ?").bind(b);

      BOOST_REQUIRE(c.size() == 1);
    }
  }
}

BOOST_AUTO_TEST_CASE( dbo_test16 )
{
  DboFixture f;

  dbo::Session *session = f.session_;

  {
    dbo::Transaction t(*session);

    A a1;
    a1.date = Wt::WDate(1976, 6, 14);
    a1.time = Wt::WTime(13, 14, 15, 102);

    for (unsigned i = 0; i < 255; ++i)
      a1.binary.push_back(i);
    a1.datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
    a1.wstring = "Hello";
    a1.string = "There";
    std::tm timeInfo = std::tm();
    timeInfo.tm_year = 2005 - 1900;
    timeInfo.tm_mon = 0; //Jan
    timeInfo.tm_mday = 1;
    timeInfo.tm_hour = 1;
    timeInfo.tm_min = 2;
    timeInfo.tm_sec = 3;
    std::time_t timet = timegm(&timeInfo);
    a1.timepoint = std::chrono::system_clock::from_time_t(timet);
    a1.timeduration = std::chrono::hours(1) + std::chrono::seconds(10);
    a1.i = 42;
    a1.i64 = 9223372036854775805LL;
    a1.ll = 6066005651767221LL;
    a1.checked = true;
    a1.f = (float)42.42;
    a1.d = 42.424242;

    dbo::ptr<A> a = session->addNew<A>();

    t.commit();

    {
      dbo::Transaction t(*session);

      dbo::Query<dbo::ptr<A>> query = session->find<A>();
      dbo::QueryModel<dbo::ptr<A>> *model = new dbo::QueryModel<dbo::ptr<A>>();
      model->setQuery(query);
      model->addColumn ("date");
      model->addColumn ("time");
      model->addColumn ("binary");
      model->addColumn ("datetime");
      model->addColumn ("wstring");
      model->addColumn ("string");
      model->addColumn ("timepoint");
      model->addColumn ("timeduration");
      model->addColumn ("i");
      model->addColumn ("i64");
      model->addColumn ("ll");
      model->addColumn ("checked");
      model->addColumn ("f");
      model->addColumn ("d");
      
      Wt::WDate date(1982, 12, 2);
      Wt::WTime time(14, 15, 16, 0);

      std::vector<unsigned char> bin;
      for (unsigned i = 0; i < 255; ++i)
	bin.push_back(255 - i);
      Wt::WString ws("Hey");
      std::string s("Test");
      std::tm timeInfo = std::tm();
      timeInfo.tm_year = 2010 - 1900;
      timeInfo.tm_mon = 8; //Sep
      timeInfo.tm_mday = 9;
      timeInfo.tm_hour = 3;
      timeInfo.tm_min = 2;
      timeInfo.tm_sec = 1;
      std::time_t timet = timegm(&timeInfo);
      std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timet);
      std::chrono::duration<int, std::milli> duration = std::chrono::hours(1) + std::chrono::seconds(10);
      int i = 50;
      ::int64_t i64 = 8223372036854775805LL;;
      long long ll = 7066005651767221LL;
      float f = (float)53.53;
      double d = 53.5353;
      bool checked = false;
      
      model->setData(0, 0, Wt::cpp17::any(date));
      model->setData(0, 1, Wt::cpp17::any(time));
      model->setData(0, 2, Wt::cpp17::any(bin));
      model->setData(0, 3, Wt::cpp17::any(Wt::WDateTime(date, time)));
      model->setData(0, 4, Wt::cpp17::any(ws));
      model->setData(0, 5, Wt::cpp17::any(s));
      model->setData(0, 6, Wt::cpp17::any(tp));
      model->setData(0, 7, Wt::cpp17::any(duration));
      model->setData(0, 8, Wt::cpp17::any(i));
      model->setData(0, 9, Wt::cpp17::any(i64));
      model->setData(0, 10, Wt::cpp17::any(ll));
      model->setData(0, 11, Wt::cpp17::any(checked));
      model->setData(0, 12, Wt::cpp17::any(f));
      model->setData(0, 13, Wt::cpp17::any(d));

      //TODO, also set data using strings to test string to any value conversion

      dbo::ptr<A> aa = session->find<A>().resultValue();
      BOOST_REQUIRE(aa->date == date);
      BOOST_REQUIRE(aa->time == time);
      BOOST_REQUIRE(aa->binary == bin);
      BOOST_REQUIRE(aa->datetime == Wt::WDateTime(date, time));
      BOOST_REQUIRE(aa->wstring == ws);
      BOOST_REQUIRE(aa->string == s);
      BOOST_REQUIRE(aa->timepoint == tp);
      BOOST_REQUIRE(aa->timeduration == duration);
      BOOST_REQUIRE(aa->i == i);
      BOOST_REQUIRE(aa->i64 == i64);
      BOOST_REQUIRE(aa->ll == ll);
      BOOST_REQUIRE(aa->checked == checked);
      BOOST_REQUIRE(aa->f == f);
      BOOST_REQUIRE(aa->d == d);

      delete model;
    }
  }
}

BOOST_AUTO_TEST_CASE( dbo_test17 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a = session_->addNew<A>();
    dbo::ptr<B> b = session_->addNew<B>("b", B::State1);
    a.modify()->b = b;
  }

#if !defined(FIREBIRD) && !defined(MYSQL)
  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();

    dbo::ptr<B> b = a->b;
    a.modify()->b.reset(); // 1
    a.remove(); // 3 (a belongs to b)
    b.remove(); // 2
  }
#endif //!FIREBIRD && !MYSQL
}

BOOST_AUTO_TEST_CASE( dbo_test18 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a = session_->addNew<A>();
    dbo::ptr<C> c = session_->addNew<C>();

    BOOST_REQUIRE(!c->aOneToOne);

    a.modify()->c = c;

    BOOST_REQUIRE(c->aOneToOne == a);

    a.modify()->c.reset();

    BOOST_REQUIRE(!c->aOneToOne);

    c.modify()->aOneToOne = a;

    BOOST_REQUIRE(a->c == c);

    BOOST_REQUIRE(c->aOneToOne->c == c);

    dbo::ptr<A> a2 = c->aOneToOne;
  }
}

BOOST_AUTO_TEST_CASE( dbo_test19 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a = session_->addNew<A>();
    dbo::ptr<B> b = session_->addNew<B>("b", B::State1);
    a.modify()->b = b;
  }

  try {
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();

    a.remove(); // 3

    throw std::runtime_error("Auch");
  } catch (std::exception& ) {
    session_->rereadAll();
    dbo::Transaction t(*session_);
    t.commit();
  }
}

BOOST_AUTO_TEST_CASE( dbo_test20 )
{
  DboFixture f;

  // Oracle does not support: " select 1 " (must have from)
#if !defined(FIREBIRD) && !defined(ORACLE)
  {
    dbo::Session *session_ = f.session_;

    dbo::Transaction t(*session_);

    dbo::QueryModel<std::string> *model = new dbo::QueryModel<std::string>();
    model->setQuery(session_->query<std::string>("select 'dima '' ?'"));
    model->addAllFieldsAsColumns();

    std::cerr << model->columnCount() << std::endl
              << model->rowCount() << std::endl;
    std::cerr << Wt::asString(model->data(0, 0)) << std::endl;

    delete model;
  }
#endif //FIREBIRD
}

BOOST_AUTO_TEST_CASE( dbo_test21 )
{
  DboFixture f;

  {
    dbo::Session *session_ = f.session_;

    dbo::Transaction t(*session_);

    dbo::QueryModel<std::string> *model = new dbo::QueryModel<std::string>();
    model->setQuery(session_->query<std::string>
		    ("SELECT cast(round(number, 2) as text) AS column_number "
		     "FROM table"));

    model->addColumn("column_number", "label");

    delete model;
  }
}

BOOST_AUTO_TEST_CASE( dbo_test22a )
{
#ifdef POSTGRES
  DboFixture f;
  dbo::Session *session_ = f.session_;
  Wt::WDateTime datetime1 = Wt::WDateTime(Wt::WDate(2009, 10, 1),
                                          Wt::WTime(12, 11, 31));

  {
    dbo::Transaction t(*session_);
    session_->execute("SET TIME ZONE \"America/New_York\"");

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = datetime1;

    session_->add(a1);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a2 = session_->find<A>();

    BOOST_REQUIRE(a2->datetime == datetime1);
  }
#endif //POSTGRES
}

BOOST_AUTO_TEST_CASE( dbo_test22b )
{
#ifdef POSTGRES
  DboFixture f;
  dbo::Session *session_ = f.session_;
  Wt::WDateTime datetime1 = Wt::WDateTime(Wt::WDate(2009, 10, 1),
                                          Wt::WTime(12, 11, 31));

  {
    dbo::Transaction t(*session_);
    session_->execute("SET TIME ZONE \"Europe/Brussels\"");

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = datetime1;

    session_->add(a1);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a2 = session_->find<A>();

    BOOST_REQUIRE(a2->datetime == datetime1);
  }
#endif //POSTGRES
}

BOOST_AUTO_TEST_CASE( dbo_test22c )
{
#ifdef POSTGRES
  DboFixture f;
  dbo::Session *session_ = f.session_;
  Wt::WDateTime datetime1 = Wt::WDateTime(Wt::WDate(2009, 10, 1),
                                          Wt::WTime(12, 11, 31));

  {
    dbo::Transaction t(*session_);
    session_->execute("ALTER TABLE " SCHEMA "table_a ALTER COLUMN datetime "
        "TYPE TIMESTAMP WITH TIME ZONE" );
    session_->execute("SET TIME ZONE \"America/New_York\"");

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = datetime1;

    session_->add(a1);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a2 = session_->find<A>();

    BOOST_REQUIRE(a2->datetime == datetime1);
  }
#endif //POSTGRES
}

BOOST_AUTO_TEST_CASE( dbo_test22d )
{
#ifdef POSTGRES
  DboFixture f;
  dbo::Session *session_ = f.session_;
  Wt::WDateTime datetime1 = Wt::WDateTime(Wt::WDate(2009, 10, 1),
                                          Wt::WTime(12, 11, 31));

  {
    dbo::Transaction t(*session_);
    session_->execute("ALTER TABLE " SCHEMA "table_a ALTER COLUMN datetime "
        "TYPE TIMESTAMP WITH TIME ZONE" );
    session_->execute("SET TIME ZONE \"Europe/Brussels\"");

    dbo::ptr<A> a1 = dbo::make_ptr<A>();
    a1.modify()->datetime = datetime1;

    session_->add(a1);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a2 = session_->find<A>();

    BOOST_REQUIRE(a2->datetime == datetime1);
  }
#endif //POSTGRES
}

// dbo_test23x tests are dbo::ptr<const C> tests
// the main test is to make sure they compile
BOOST_AUTO_TEST_CASE( dbo_test23a )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    auto a1 = std::make_unique<A>();
    a1->ll = 123456L;
    dbo::ptr<const A> aPtr(std::move(a1));

    session_->add(aPtr);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<const A> a1 = session_->find<const A>();
    dbo::ptr<A> a2 = session_->find<A>();
    dbo::collection<dbo::ptr<const A>> as1 = session_->find<const A>();
    dbo::ptr<const A> a3 = as1.front();

    BOOST_REQUIRE(a1 == a2);
    BOOST_REQUIRE(a2 == a3);
    BOOST_REQUIRE(a3 == a2);
    BOOST_REQUIRE(a2 == a1);
    BOOST_REQUIRE(a1 == a3);
    BOOST_REQUIRE(a3 == a1);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test23b )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    auto a1 = std::make_unique<A>();
    a1->ll = 123456L;
    dbo::ptr<A> aPtr1(std::move(a1));

    auto c1 = std::make_unique<C>();
    c1->name = "Jos";
    dbo::ptr<C> cPtr1(std::move(c1));

    aPtr1.modify()->c = cPtr1;

    session_->add(aPtr1);
    session_->add(cPtr1);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<const C> c1 = session_->find<const C>();

    dbo::weak_ptr<A> a1 = c1->aOneToOne;
    dbo::weak_ptr<const A> a2 = a1;
    dbo::ptr<A> a3 = a1;
    dbo::ptr<const A> a4 = a1;

    BOOST_REQUIRE(a1 == a2);
    BOOST_REQUIRE(a2 == a1);
    BOOST_REQUIRE(a2 == a3);
    BOOST_REQUIRE(a3 == a2);
    BOOST_REQUIRE(a3 == a4);
    BOOST_REQUIRE(a4 == a3);
    BOOST_REQUIRE(a1 == a4);
    BOOST_REQUIRE(a4 == a1);
    BOOST_REQUIRE(a1 == a3);
    BOOST_REQUIRE(a3 == a1);
    BOOST_REQUIRE(a2 == a4);
    BOOST_REQUIRE(a4 == a2);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test23c )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    auto d = std::make_unique<D>();
    d->id = Coordinate(2, 4);

    dbo::ptr<D> dPtr = session_->add(std::move(d));
    dbo::ptr<C> cPtr = session_->addNew<C>();

    dPtr.modify()->c = cPtr;

    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<const D> d = session_->find<const D>();

    BOOST_REQUIRE(d.id() == Coordinate(2, 4));

    dbo::ptr<const C> c = session_->find<const C>();

    dbo::weak_ptr<const D> d2 = c->dOneToOne;

    BOOST_REQUIRE(c->dOneToOne.id() == Coordinate(2, 4));
  }
}

BOOST_AUTO_TEST_CASE( dbo_test24a )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<E> e1 = dbo::make_ptr<E>("e1");
    dbo::ptr<C> c1 = dbo::make_ptr<C>("c1");
    session_->add(e1);
    session_->add(c1);

    typedef std::tuple<dbo::ptr<E>, dbo::ptr<C>> EC;
    EC ec = session_->query< EC >
      ("select E, C from  " SCHEMA "\"table_e\" E, " SCHEMA "\"table_c\" C");

    BOOST_REQUIRE(std::get<0>(ec)->name == "e1");
    BOOST_REQUIRE(std::get<1>(ec)->name == "c1");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test24b )
{
#ifndef SQLITE3
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<E> e1 = dbo::make_ptr<E>("e1");
    session_->add(e1);

    typedef std::tuple<dbo::ptr<E>, dbo::ptr<E>> EE;
    EE ee = session_->query<EE>
      ("select \"E1\", \"E2\" from " SCHEMA "\"table_e\" \"E1\" "
       "right join " SCHEMA "\"table_e\" \"E2\" on \"E1\".\"id\" != \"E2\".\"id\"");

    BOOST_REQUIRE(std::get<1>(ee)->name == "e1");
  }
#endif // SQLITE3
}

BOOST_AUTO_TEST_CASE( dbo_test24c )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<D> d1 = dbo::make_ptr<D>(Coordinate(42, 43), "d1");
    dbo::ptr<C> c1 = dbo::make_ptr<C>("c1");
    session_->add(d1);
    session_->add(c1);

    typedef std::tuple<dbo::ptr<D>, dbo::ptr<C>> DC;
    DC dc = session_->query< DC >
      ("select D, C from " SCHEMA "\"table_d\" D, " SCHEMA "\"table_c\" C");

    BOOST_REQUIRE(std::get<0>(dc)->name == "d1");
    BOOST_REQUIRE(std::get<1>(dc)->name == "c1");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test24d )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<E> e1 = dbo::make_ptr<E>("e1");
    dbo::ptr<C> c1 = dbo::make_ptr<C>("c1");
    session_->add(e1);
    session_->add(c1);

    typedef std::tuple<dbo::ptr<E>, dbo::ptr<C>> EC;
    dbo::collection<EC> ecs = session_->query< EC >
      ("select E, C from  " SCHEMA "\"table_e\" E, " SCHEMA "\"table_c\" C");

    // Calling size() forces the count query to be executed, tests
    // whether adding aliases works properly, because some systems, like
    // MySQL and SQL Server disallow queries like
    // select count(1) from (select e."id", ..., c."id", ... from ...) dbocount
    // because that would cause there to be two dbocount."id"s
    BOOST_REQUIRE(ecs.size() == 1);

    EC ec = *ecs.begin();

    BOOST_REQUIRE(std::get<0>(ec)->name == "e1");
    BOOST_REQUIRE(std::get<1>(ec)->name == "c1");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test25 )
{
#ifndef FIREBIRD // Cannot order by on blobs in Firebird...
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<F> e1 = session_->addNew<F>("Alice",  "Kramden", "Female");
    dbo::ptr<F> e2 = session_->addNew<F>("Ralph",  "Kramden", "Male");
    dbo::ptr<F> e3 = session_->addNew<F>("Trixie", "Norton", "Female");
    dbo::ptr<F> e4 = session_->addNew<F>("Ed",     "Norton", "Male");

    t.commit();
  }

  {
    dbo::Query<dbo::ptr<F>> query = session_->find<F>().orderBy("\"id\"");
    dbo::QueryModel<dbo::ptr<F>> *model = new dbo::QueryModel<dbo::ptr<F>>();

    model->setQuery(query);
    model->addAllFieldsAsColumns();

    BOOST_REQUIRE(model->columnCount() == 5);
    BOOST_REQUIRE(model->rowCount() == 4);
    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "first_name");
    BOOST_REQUIRE(Wt::asString(model->headerData(3)) == "last_name");
    BOOST_REQUIRE(Wt::asString(model->headerData(4)) == "gender");

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Trixie");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Ed");

    // sort on first name
    model->sort(2, Wt::SortOrder::Ascending);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    delete model;
  }

  {
    class custom_sort_model : public dbo::QueryModel<dbo::ptr<F>> {
    public:
      custom_sort_model() : dbo::QueryModel<dbo::ptr<F>>() { }

      virtual std::string createOrderBy(int column, Wt::SortOrder order) override
      {
        std::string dir 
	  = (order == Wt::SortOrder::Ascending ? "asc" : "desc");
        return "\"" + fieldInfo(column).name() + "\" " + dir +
          ((column != 3) ? ", \"last_name\"" : "") +
          ((column != 2) ? ", \"first_name\"" : "");
      }
    };

    dbo::Query<dbo::ptr<F>> query = session_->find<F>().orderBy("\"id\"");

    custom_sort_model *model = new custom_sort_model();

    model->setQuery(query);
    model->addAllFieldsAsColumns();

    BOOST_REQUIRE(model->columnCount() == 5);
    BOOST_REQUIRE(model->rowCount() == 4);
    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "first_name");
    BOOST_REQUIRE(Wt::asString(model->headerData(3)) == "last_name");
    BOOST_REQUIRE(Wt::asString(model->headerData(4)) == "gender");

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Trixie");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Ed");

    // sort on last name ascending, then first name ascending
    model->sort(3, Wt::SortOrder::Ascending);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    // sort on last name descending, then first name ascending
    model->sort(3, Wt::SortOrder::Descending);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Trixie");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Ralph");

    // sort on first name, then last name ascending
    model->sort(2, Wt::SortOrder::Ascending);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    // sort on gender, then last name, then first name
    model->sort(4, Wt::SortOrder::Ascending);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Trixie");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Ed");

    delete model;
  }
#endif // FIREBIRD
}

namespace {

struct CheckExpected : Wt::WObject {
  DboFixture& f_;
  dbo::Session *session2_;

  CheckExpected(DboFixture &f) : f_(f) {
    session2_ = new dbo::Session();
    session2_->setConnectionPool(*f_.connectionPool_);
    session2_->mapClass<F>(SCHEMA "table_f");
  }

  virtual ~CheckExpected() {
    delete session2_;
  }

  bool operator() (std::string &expected) {
    {
      dbo::Transaction t2(*session2_);
      dbo::ptr<F> c = session2_->find<F>();
      if (c->firstName != expected)
	BOOST_ERROR(std::string("CheckExpected: firstName != expected, firstName: '") +
		    c->firstName + "', expected: '" + expected + "'");
      else
	BOOST_TEST_MESSAGE(std::string("CheckExpected OK: firstName: '") +
			   c->firstName + "', expected: '" + expected + "'");
    }
    return true;
  }
};

}

BOOST_AUTO_TEST_CASE( dbo_test26 )
{
#ifndef SQLITE3  // sqlite3 ":memory:" does not share database between sessions
  DboFixture f;

  dbo::Session *session_ = f.session_;

  CheckExpected checkExpected(f);

  {
    dbo::Transaction t(*session_);

    session_->addNew<F>("Alice",  "Kramden", "Female");

    dbo::Query<dbo::ptr<F>> query = session_->find<F>();
    dbo::QueryModel<dbo::ptr<F>> *model = new dbo::QueryModel<dbo::ptr<F>>();

    model->setQuery(query);
    model->addAllFieldsAsColumns();

    BOOST_REQUIRE(model->columnCount() == 5);
    BOOST_REQUIRE(model->rowCount() == 1);
    t.commit();

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");

    std::string ExpectedFirstName = "Alice";
    BOOST_REQUIRE(checkExpected(ExpectedFirstName));

    /*
     * Set-up a handler to verify that updates are visible
     * in a second session when model->dataChanged() is emitted
     */
    model->dataChanged().connect
      ([&checkExpected, &ExpectedFirstName] () {checkExpected(ExpectedFirstName); });
    
    /*
     * The setItemData() convenience method commits
     * a transaction prior to emitting dataChanged()
     */
    ExpectedFirstName = "AliceTwo";
    Wt::WAbstractItemModel::DataMap map;
    map[Wt::ItemDataRole::Edit] = ExpectedFirstName;
    model->setItemData(model->index(0, 2), map);  // checkExpected() will be called

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == ExpectedFirstName);

    /*
     * The setData() should be equivalent to above setItemData() and
     * commit a transaction prior to emitting dataChanged()
     */
    ExpectedFirstName = "AliceThree";
    model->setData(0, 2, ExpectedFirstName);  // checkExpected() will be called

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == ExpectedFirstName);

    delete model;
  }
#endif // SQLITE3
}

BOOST_AUTO_TEST_CASE( dbo_test27 )
{
#ifdef MYSQL
  /* Allegedly fails, see #4734 */
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<B> b = dbo::make_ptr<B>();
    b.modify()->name = "b";
    session_->add(b);
  }

  {
    dbo::Transaction t(*session_);

    /* The count fails ! */

    std::string auth = session_->query<std::string>
      ("select sha1(?) from " SCHEMA "\"table_b\" where \"name\"=?")
      .bind("shhhh").bind("b");
  }
#endif
}

BOOST_AUTO_TEST_CASE( dbo_test28 )
{
#ifdef POSTGRES
  DboFixture f;

  dbo::Session *session_ = f.session_;

  try {
    dbo::Transaction t(*session_);
    dbo::ptr<B> b = dbo::make_ptr<B>();
    b.modify()->name = "b";
    session_->add(b);
    session_->flush();
    session_->execute("select pg_sleep(10)");
  } catch (...) {

  }

  {
    dbo::Transaction t(*session_);
    dbo::ptr<B> b = dbo::make_ptr<B>();
    b.modify()->name = "b";
    session_->add(b);
    session_->flush();
  }

#endif
}

BOOST_AUTO_TEST_CASE( dbo_test29 )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  dbo::Transaction t(*session_);

  dbo::ptr<A> a1 = session_->addNew<A>();
  a1.modify()->string2 = "B";
  a1.modify()->i = 1;
  dbo::ptr<A> a2 = session_->addNew<A>();
  a2.modify()->string2 = "A";
  a2.modify()->i = 2;
  dbo::ptr<A> a3 = session_->addNew<A>();
  a3.modify()->string2 = "B";
  a3.modify()->i = 4;
  dbo::ptr<A> a4 = session_->addNew<A>();
  a4.modify()->string2 = "A";
  a4.modify()->i = 8;

  // Should not throw
  // Test case for PostgreSQL and SQL Server:
  //  - PostgreSQL needs a subquery for the "select count(1)"
  //  - SQL Server needs an offset when using "order by" in a subquery
  dbo::collection<dbo::ptr<A> > as1 = session_->find<A>().orderBy("\"string2\"");

  BOOST_REQUIRE(as1.size() == 4);
  dbo::collection<dbo::ptr<A> >::iterator it1 = as1.begin();
  BOOST_REQUIRE((*it1)->string2 == "A");
  ++it1;
  BOOST_REQUIRE((*it1)->string2 == "A");
  ++it1;
  BOOST_REQUIRE((*it1)->string2 == "B");
  ++it1;
  BOOST_REQUIRE((*it1)->string2 == "B");

  dbo::collection<std::string> as2 =
    session_->query<std::string>("SELECT \"string2\" FROM \"table_a\"").orderBy("\"string2\"").groupBy("\"string2\"");

  BOOST_REQUIRE(as2.size() == 2);
  dbo::collection<std::string>::iterator it2 = as2.begin();
  BOOST_REQUIRE(*it2 == "A");
  ++it2;
  BOOST_REQUIRE(*it2 == "B");
  ++it2;

  dbo::collection<int> i_total = session_->query<int>("select SUM(\"i\") as \"my_sum\" from \"table_a\"");

  BOOST_REQUIRE(i_total.size() == 1);
  BOOST_REQUIRE(*i_total.begin() == 15);

  dbo::collection<int> is = session_->query<int>("select SUM(\"i\") as \"my_sum\" from \"table_a\"").orderBy("\"string2\"").groupBy("\"string2\"");

  BOOST_REQUIRE(is.size() == 2);
  dbo::collection<int>::iterator it3 = is.begin();
  BOOST_REQUIRE(*it3 == 10);
  ++it3;
  BOOST_REQUIRE(*it3 == 5);
}

BOOST_AUTO_TEST_CASE( dbo_test30 )
{
  // Up until Wt 3.3.8, calling .size() on the result of a count(*) query
  // returned the wrong result. 
  DboFixture f;

  dbo::Session *session_ = f.session_;

  dbo::Transaction t(*session_);
  dbo::ptr<A> a1 = session_->addNew<A>();
  a1.modify()->string = "B";
  a1.modify()->i = 1;
  dbo::ptr<A> a2 = session_->addNew<A>();
  a2.modify()->string = "A";
  a2.modify()->i = 2;

  dbo::collection<int> counts = session_->query<int>("select count(*) from \"table_a\"");
  BOOST_REQUIRE(counts.size() == 1);
  // counts.size() returned 2 in Wt <= 3.3.8, because instead of executing the query
  // select count(1) from (select count(*) from "table_a")
  // the query
  // select count(1) from "table_a"
  // was executed instead
  
  int count = *counts.begin();
  BOOST_REQUIRE(count == 2);
}

// The Firebird backend uses a time type limited to time of day, so negative
// times and times longer than a day are not supported.
#ifndef FIREBIRD
BOOST_AUTO_TEST_CASE( dbo_test31 )
{
  // Test long and negative durations
  std::chrono::milliseconds longDuration =
    std::chrono::hours(42) +
    std::chrono::minutes(10) +
    std::chrono::seconds(11) +
    std::chrono::milliseconds(123);
  auto negDuration = -longDuration;
  // Test whether Postgres properly serializes/deserializes
  // time when the last digit of milliseconds is 0
  std::chrono::milliseconds anotherDuration =
    std::chrono::hours(42) +
    std::chrono::minutes(10) +
    std::chrono::seconds(11) +
    std::chrono::milliseconds(120);
  // Test whether Postgres properly serializes/deserializes
  // time when the first digit of milliseconds is 0
  std::chrono::milliseconds anotherDuration2 =
    std::chrono::hours(42) +
    std::chrono::minutes(10) +
    std::chrono::seconds(11) +
    std::chrono::milliseconds(12);

  DboFixture f;

  dbo::Session *session_ = f.session_;
  {
    // Store
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->addNew<A>();
    a.modify()->timeduration = longDuration;
  }
  {
    // Retrieve
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->timeduration == longDuration);
    a.remove();
  }
  {
    // Store
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->addNew<A>();
    a.modify()->timeduration = negDuration;
  }
  {
    // Retrieve
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->timeduration == negDuration);
    a.remove();
  }
  {
    // Store
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->addNew<A>();
    a.modify()->timeduration = anotherDuration;
  }
  {
    // Retrieve
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->timeduration == anotherDuration);
    a.remove();
  }
  {
    // Store
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->addNew<A>();
    a.modify()->timeduration = anotherDuration2;
  }
  {
    // Retrieve
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->timeduration == anotherDuration2);
  }
}
#endif // !FIREBIRD

BOOST_AUTO_TEST_CASE( dbo_test32 )
{
  struct OnlyMovable {
    OnlyMovable() { }
    ~OnlyMovable() { }
    OnlyMovable(const OnlyMovable&) = delete;
    OnlyMovable& operator=(const OnlyMovable&) = delete;
    OnlyMovable(OnlyMovable&&) = default;
    OnlyMovable& operator=(OnlyMovable&&) noexcept = default;

    dbo::ptr<A> p;
    dbo::collection<dbo::ptr<A>> c;
    dbo::weak_ptr<A> wp;
  };

  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);
    session_->addNew<A>();
  }

  dbo::Transaction t(*session_);
  dbo::ptr<A> myA = session_->find<A>();
  dbo::collection<dbo::ptr<A>> aColl = session_->find<A>();

  OnlyMovable om1;
  om1.p = myA;
  om1.c = aColl;

  OnlyMovable om2{std::move(om1)};

  BOOST_REQUIRE(om1.p.get() == nullptr);
  BOOST_REQUIRE(om1.c.empty());

  BOOST_REQUIRE(om2.p == myA);
  BOOST_REQUIRE(om2.c.front() == myA);
}

BOOST_AUTO_TEST_CASE( dbo_test33 )
{
  std::vector<int> sizes;
  sizes.push_back(127);
  sizes.push_back(128);
  sizes.push_back(129);
  sizes.push_back(255);
  sizes.push_back(256);
  sizes.push_back(257);
  sizes.push_back(1023);
  sizes.push_back(1024);
  sizes.push_back(1025);
  sizes.push_back(65535);
  // MySQL "text" type only supports up to 65535 bytes
  // Firebird also throws an exception when it's over 65535 bytes
  // FIXME: can this be fixed in the Firebird backend?
#if !defined(MYSQL) && !defined(FIREBIRD)
  sizes.push_back(1024 * 1024 - 1);
  sizes.push_back(1024 * 1024);
  sizes.push_back(1024 * 1024 + 1);
#endif // !defined(MYSQL) && !defined(FIREBIRD)

  std::vector<int>::const_iterator end = sizes.end();
  for (std::vector<int>::const_iterator it = sizes.begin();
       it != end; ++it) {
    const int size = *it;

    DboFixture f;

    dbo::Session *session_ = f.session_;

    std::string longStr;
    {
      std::stringstream ss;
      for (int i = 0; i < size; ++i) {
        ss << static_cast<char>('0' + (i % 8));
      }
      longStr = ss.str();
    }
    std::vector<unsigned char> longBinary;
    {
      for (int i = 0; i < size; ++i) {
        longBinary.push_back(static_cast<unsigned char>(i % 256));
      }
    }

    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a = session_->addNew<A>();

      a.modify()->string = longStr;
      a.modify()->binary = longBinary;
    }

    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a = session_->find<A>();

      BOOST_REQUIRE(a->string == longStr);
      BOOST_REQUIRE(a->binary == longBinary);
    }
  }
}

BOOST_AUTO_TEST_CASE ( dbo_test34 )
{
#if defined(POSTGRES) || defined(SQLITE3)
  const std::vector<double> ds =
    { 0.123456789123456, std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN() };
#elif defined(MYSQL) || defined(MSSQLSERVER) || defined(FIREBIRD)
  const std::vector<double> ds = { 0.12345678923456 };
#endif

  for (const double d : ds) {
    DboFixture f;
    dbo::Session *session_ = f.session_;

    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a = session_->addNew<A>();
      a.modify()->d = d;
    }

    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a = session_->find<A>();

      BOOST_REQUIRE((std::isnan(d) && std::isnan(a->d)) || a->d == d);
    }
  }
}

BOOST_AUTO_TEST_CASE( dbo_test35 )
{
  // Test reentrant statement use
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);
    for (int i = 0; i < 4; ++i)
      session_->addNew<A>();
    t.commit();
  }

  {
    dbo::Transaction t(*session_);
    As allAs1 = session_->find<A>();
    for (auto it1 = allAs1.begin(); it1 != allAs1.end(); ++it1) {
      As allAs2 = session_->find<A>();
      for (auto it2 = allAs2.begin(); it2 != allAs2.end(); ++it2) {
        BOOST_REQUIRE(true);
      }
    }
  }
}

namespace {
  void recursiveQuery(dbo::Session &session, int i = 0) {
    As allAs = session.find<A>();
    for (auto it = allAs.begin(); it != allAs.end(); ++it) {
      if (i < 20)
        recursiveQuery(session, i + 1);
      else
        BOOST_REQUIRE(true);
    }
  }
}

BOOST_AUTO_TEST_CASE( dbo_test36 )
{
  // Test reentrant statement use
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);
    session_->addNew<A>();
    t.commit();
  }

  {
    dbo::Transaction t(*session_);
    recursiveQuery(*session_);
  }
}

// Test Boost optional
BOOST_AUTO_TEST_CASE( dbo_test37 )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);
    auto a = session_->addNew<A>();
    a.modify()->string3 = "String 3";
    t.commit();
  }

  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->string3 == std::string("String 3"));
#ifdef WT_CXX17
#if __has_include(<optional>)
    BOOST_REQUIRE(a->string4 == std::nullopt);
#endif // __has_include(<optional>)
#endif // WT_CXX17
  }
}

// Test std::optional
BOOST_AUTO_TEST_CASE( dbo_test38 )
{
#ifdef WT_CXX17
#if __has_include(<optional>)
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);
    auto a = session_->addNew<A>();
    a.modify()->string4 = "String 4";
    t.commit();
  }

  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> a = session_->find<A>();
    BOOST_REQUIRE(a->string3 == boost::optional<std::string>());
    BOOST_REQUIRE(a->string4 == std::string("String 4"));
  }
#endif // __has_include(<optional>)
#endif // WT_CXX17
}

namespace {
  void setupTest39(dbo::Session &session) {
    dbo::Transaction t(session);

    Wt::Dbo::ptr<A> a1 = session.addNew<A>();
    a1.modify()->i = 3;

    Wt::Dbo::ptr<A> a2 = session.addNew<A>();
    a2.modify()->i = 4;

    Wt::Dbo::ptr<A> a3 = session.addNew<A>();
    a3.modify()->i = 5;

    t.commit();
  }
}

BOOST_AUTO_TEST_CASE( dbo_test39a )
{
  // Test UNION
  DboFixture f;
  dbo::Session *session_ = f.session_;

  setupTest39(*session_);

  {
    dbo::Transaction t(*session_);

    As as = session_->query<Wt::Dbo::ptr<A> >("select a from \"table_a\" a where a.\"i\" = 3 union select a from \"table_a\" a where a.\"i\" = 5");

    BOOST_REQUIRE(as.size() == 2);

    As::iterator it = as.begin();
    BOOST_REQUIRE((*it)->i == 3);
    ++it;
    BOOST_REQUIRE((*it)->i == 5);
    ++it;
    BOOST_REQUIRE(it == as.end());
  }
}

BOOST_AUTO_TEST_CASE( dbo_test39b )
{
#if !defined(MYSQL) && !defined(FIREBIRD) // MySQL and Firebird don't do INTERSECT
  // Test INTERSECT
  DboFixture f;
  dbo::Session *session_ = f.session_;

  setupTest39(*session_);

  {
    dbo::Transaction t(*session_);

    As as = session_->query<Wt::Dbo::ptr<A> >("select a from \"table_a\" a where a.\"i\" > 3 intersect select a from \"table_a\" a where a.\"i\" < 5");

    BOOST_REQUIRE(as.size() == 1);

    As::iterator it = as.begin();
    BOOST_REQUIRE((*it)->i == 4);
    ++it;
    BOOST_REQUIRE(it == as.end());
  }
#endif
}

BOOST_AUTO_TEST_CASE( dbo_test39c )
{
#if !defined(MYSQL) && !defined(FIREBIRD) // MySQL and Firebird don't do EXCEPT
  // Test EXCEPT
  DboFixture f;
  dbo::Session *session_ = f.session_;

  setupTest39(*session_);

  {
    dbo::Transaction t(*session_);

    As as = session_->query<Wt::Dbo::ptr<A> >("select a from \"table_a\" a where a.\"i\" >= 3 except select a from \"table_a\" a where a.\"i\" = 4");

    BOOST_REQUIRE(as.size() == 2);

    std::vector<int> results;
    for (As::iterator it = as.begin();
         it != as.end(); ++it) {
      results.push_back((*it)->i);
    }

    std::sort(results.begin(), results.end());

    BOOST_REQUIRE(results[0] == 3);
    BOOST_REQUIRE(results[1] == 5);
  }
#endif
}

BOOST_AUTO_TEST_CASE( dbo_test40 )
{
  // Test join function
  DboFixture f;
  dbo::Session &session = *f.session_;

  {
    dbo::Transaction t(session);

    dbo::ptr<A> a = session.addNew<A>();
    a.modify()->i = 5;
    dbo::ptr<B> b = session.addNew<B>();
    b.modify()->name = "Test";
    dbo::ptr<B> b2 = session.addNew<B>();
    b2.modify()->name = "Test2";

    a.modify()->b = b;
  }

  {
    dbo::Transaction t(session);

    dbo::collection<dbo::ptr<A> > results = session.query<dbo::ptr<A> >("select a from \"table_a\" a")
                                                                     .join("\"table_b\" b on a.\"b_id\" = b.\"id\"")
                                                                     .where("b.\"name\" = ?")
                                                                     .bind("Test");

    BOOST_REQUIRE(results.size() == 1);

    dbo::ptr<A> first = *results.begin();

    BOOST_REQUIRE(first);

    BOOST_REQUIRE(first->i == 5);
  }

  {
    // This was a bug: AbstractQuery's copy ctor would not copy over the join_ field,
    // causing an error

    dbo::Transaction t(session);
    dbo::Query<dbo::ptr<A> > q = session.query<dbo::ptr<A> >("select a from \"table_a\" a")
                                                            .join("\"table_b\" b on a.\"b_id\" = b.\"id\"");

    q.where("b.\"name\" = ?").bind("Test");

    dbo::collection<dbo::ptr<A> > results = q.resultList();

    BOOST_REQUIRE(results.size() == 1);

    dbo::ptr<A> first = *results.begin();

    BOOST_REQUIRE(first);

    BOOST_REQUIRE(first->i == 5);
  }
}

BOOST_AUTO_TEST_CASE( dbo_test41 )
{
  // Test orWhere function
  DboFixture f;
  dbo::Session &session = *f.session_;

  {
    dbo::Transaction t(session);

    dbo::ptr<B> b = session.addNew<B>();
    b.modify()->name = "Test";
    dbo::ptr<B> b2 = session.addNew<B>();
    b2.modify()->name = "Test2";
    dbo::ptr<B> b3 = session.addNew<B>();
    b3.modify()->name = "Test3";
    dbo::ptr<B> b4 = session.addNew<B>();
    b4.modify()->name = "Test4";
  }

  {
    dbo::Transaction t(session);
    dbo::collection<dbo::ptr<B>> results = session.query<dbo::ptr<B>>("select b from \"table_b\" b")
                                                                     .where("b.\"name\" = ?").bind("Test")
                                                                     .orWhere("b.\"name\" = ?").bind("Test2");

    BOOST_REQUIRE(results.size() == 2);

    std::vector<std::string> names;
    for (dbo::ptr<B> b : results)
      names.push_back(b->name);

    BOOST_REQUIRE(names[0] == "Test" || names[0] == "Test2");
    BOOST_REQUIRE(names[1] == "Test" || names[1] == "Test2");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test42 )
{
  // Test distinct on
#ifdef POSTGRES
  DboFixture f;
  dbo::Session &session = *f.session_;

  {
    dbo::Transaction t(session);

    dbo::ptr<A> a = session.addNew<A>();
    a.modify()->i = 1;
    a.modify()->ll = 1L;
    dbo::ptr<A> a2 = session.addNew<A>();
    a2.modify()->i = 1;
    a2.modify()->ll = 2L;
    dbo::ptr<A> a3 = session.addNew<A>();
    a3.modify()->i = 2;
    a3.modify()->ll = 3L;
    dbo::ptr<A> a4 = session.addNew<A>();
    a4.modify()->i = 2;
    a4.modify()->ll = 4L;
  }

  {
    dbo::Transaction t(session);
    dbo::collection<dbo::ptr<A>> results = session.query<dbo::ptr<A>>("select distinct on (a.\"i\") a "
                                                                      "from \"table_a\" a order by a.\"i\" asc");

    BOOST_REQUIRE(results.size() == 2);

    std::vector<dbo::ptr<A>> as(results.begin(), results.end());

    BOOST_REQUIRE(as[0]->i == 1);
    BOOST_REQUIRE(as[0]->ll == 1L || as[0]->ll == 2L);
    BOOST_REQUIRE(as[1]->i == 2);
    BOOST_REQUIRE(as[1]->ll == 3L || as[1]->ll == 4L);
  }
#endif // POSTGRES
}

BOOST_AUTO_TEST_CASE( dbo_test43 )
{
  // Test for collection iterator's operator== for pull request #177
  // This will crash without that patch
  DboFixture f;
  dbo::Session &session = *f.session_;

  {
    dbo::Transaction t(session);
    dbo::ptr<B> b1 = session.addNew<B>();
    b1.modify()->name = "Test1";
    dbo::ptr<B> b2 = session.addNew<B>();
    b2.modify()->name = "Test2";
  }

  {
    dbo::Transaction t(session);

    dbo::collection<dbo::ptr<B>> collection = session.find<B>().resultList();

    auto begin = collection.begin();
    auto end = collection.end();

    BOOST_REQUIRE(begin == begin);
    BOOST_REQUIRE(!(begin != begin));
    BOOST_REQUIRE(end == end);
    BOOST_REQUIRE(!(end != end));
    BOOST_REQUIRE(!(begin == end));
    BOOST_REQUIRE(begin != end);
    BOOST_REQUIRE(!(end == begin));
    BOOST_REQUIRE(end != begin);
  }
}

BOOST_AUTO_TEST_SUITE_END()
