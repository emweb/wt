/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>
#include <Wt/Dbo/QueryModel>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "DboFixture.h"

//#define SCHEMA "test."
#define SCHEMA ""

#define DEBUG(x) x
//#define DEBUG(x)

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

typedef dbo::collection<dbo::ptr<A> > As;
typedef dbo::collection<dbo::ptr<B> > Bs;
typedef dbo::collection<dbo::ptr<C> > Cs;
typedef dbo::collection<dbo::ptr<D> > Ds;

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
  boost::posix_time::ptime ptime;
  boost::posix_time::time_duration pduration;
  bool checked;
  int i;
  ::int64_t i64;
  long long ll;
  float f;
  double d;

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
      DEBUG(std::cerr << "ERROR: wstring = " << wstring << " | "
            << other.wstring << std::endl);
    }
    if (wstring2  != other.wstring2) {
      DEBUG(std::cerr << "ERROR: wstring2 = " << wstring2 << " | "
            << other.wstring2 << std::endl);
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
    if (ptime  != other.ptime) {
      DEBUG(std::cerr << "ERROR: ptime = " <<  ptime<< " | " << other.ptime
            << std::endl);
    }
    if (pduration  != other.pduration) {
      DEBUG(std::cerr << "ERROR: pduration = " << pduration << " | "
            << other.pduration << std::endl);
    }
    if (i != other.i) {
      DEBUG(std::cerr << "ERROR: i = " << i << " | " << other.i << std::endl);
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
      && ptime == other.ptime
      && pduration == pduration
      && i == other.i
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
    dbo::field(a, ptime, "ptime");
    dbo::field(a, pduration, "pduration");
    dbo::field(a, i, "i");
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

  a1.wstring2 = Wt::WString::fromUTF8("Kitty euro\xe2\x82\xac greek \xc6\x94");
  a1.string = "There";
  a1.string2 = "Big Owl";
  a1.ptime = boost::posix_time::ptime
    (boost::gregorian::date(2005,boost::gregorian::Jan,1),
     boost::posix_time::time_duration(1,2,3));
  a1.pduration = boost::posix_time::hours(1) + 
    boost::posix_time::seconds(10);
  a1.checked = true;
  a1.i = 42;
  a1.i64 = 9223372036854775805LL;
  a1.ll = 6066005651767221LL;
  a1.f = (float)42.42;
  a1.d = 42.424242;

  /* Create an A, check that it is found during the same transaction  */
  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> ptrA = session_->add(new A(a1));

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
  a1.wstring = "Hello";
  a1.string = "There";
  a1.checked = false;
  a1.i = 42;
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
    a1.b = session_->add(new B(b1));
    dbo::ptr<A> a = session_->add(new A(a1));

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

    dbo::ptr<B> b1 = session_->add(new B("b1", B::State1));
    dbo::ptr<B> b2 = session_->add(new B("b2", B::State2));
    dbo::ptr<B> b3 = session_->add(new B("b3", B::State1));

    dbo::ptr<C> c1 = session_->add(new C("c1"));
    dbo::ptr<C> c2 = session_->add(new C("c2"));
    dbo::ptr<C> c3 = session_->add(new C("c3"));

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

    dbo::ptr<B> b1 = session_->query< dbo::ptr<B> >
      ("select distinct B from " SCHEMA "\"table_b\" B ").where("B.\"name\" = ?").bind("b1");

    std::size_t count = session_->query< dbo::ptr<B> >
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
    typedef dbo::collection<dbo::ptr<C> > Cs;

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

    dbo::ptr<A> a1(new A());

    BOOST_REQUIRE(a1->self() == a1);

    a1.modify()->datetime = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					  Wt::WTime(12, 11, 31));
    a1.modify()->date = Wt::WDate(1980, 12, 4);
    a1.modify()->wstring = "Hello";
    a1.modify()->string = "There";
    a1.modify()->i = 42;
    a1.modify()->i64 = 9223372036854775803LL;
    a1.modify()->ll = 6066005651767220LL;
    a1.modify()->f = (float)42.42;
    a1.modify()->d = 42.424242;

    dbo::ptr<A> a2(new A(*a1));
    a2.modify()->wstring = "Oh my god";
    a2.modify()->i = 142;

    dbo::ptr<B> b(new B());
    b.modify()->name = "b";
    b.modify()->state = B::State1;

    a1.modify()->b = b;
    a2.modify()->b = b;

    session_->add(a1);
    session_->add(a2);
    session_->add(b);

    typedef dbo::ptr_tuple<B, A>::type BA;
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
    // Firebird & mysql are not able to execute this query.

#if !defined(FIREBIRD) && !defined(MYSQL)
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
      boost::tie(b_result, a_result) = *i;

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
#endif //FIREBIRD && MYSQL
  }
}

BOOST_AUTO_TEST_CASE( dbo_test4b )
{
  DboFixture f;

  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<A> a1(new A());
    dbo::ptr<A> a2(new A());
    dbo::ptr<A> a3(new A());
    dbo::ptr<A> a4(new A());
    dbo::ptr<A> a5(new A());
    dbo::ptr<A> a6(new A());

    dbo::ptr<B> b1(new B());
    dbo::ptr<B> b2(new B());
    dbo::ptr<B> b3(new B());

    dbo::ptr<C> c1(new C());
    dbo::ptr<C> c2(new C());
    dbo::ptr<C> c3(new C());

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

    typedef dbo::ptr_tuple<A, B, C>::type ABC;
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
      boost::tie(a_result, b_result, c_result) = *i;

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

    dbo::ptr<A> a1(new A());
    dbo::ptr<A> a2(new A());
    dbo::ptr<A> a3(new A());
    dbo::ptr<A> a4(new A());

    dbo::ptr<B> b1(new B());
    dbo::ptr<B> b2(new B());

    dbo::ptr<C> c1(new C());
    dbo::ptr<C> c2(new C());

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

    typedef dbo::ptr_tuple<A, B, C>::type ABC;
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
      boost::tie(a_result, b_result, c_result) = *i;

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

    dbo::ptr<A> a1(new A());
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

    dbo::ptr<A> a2(new A(*a1));
    a2.modify()->wstring = "Oh my god";
    a2.modify()->i = 142;

    dbo::ptr<B> b(new B());
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

    dbo::ptr<A> a1(new A());
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

    dbo::ptr<A> a1(new A());
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

    boost::tie(id1, id2) = session_->query<boost::tuple<int, int> >
      ("select \"id\", \"id\" from " SCHEMA "\"table_a\"").resultValue();

    BOOST_REQUIRE(id1 == aId);
    BOOST_REQUIRE(id2 == aId);

#ifdef POSTGRES
    dbo::ptr<A> a;
    int id;
    boost::tie(a, id) = session_->query<boost::tuple<dbo::ptr<A>, int> >
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

    dbo::ptr<A> a(new A());

    typedef dbo::query_result_traits< dbo::ptr<A> > A_traits;

    std::vector<boost::any> values;
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

    dbo::ptr<D> d(new D());

    BOOST_REQUIRE(Wt::asString(boost::any(Coordinate(10, 4)))
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
     
    dbo::ptr<C> c1 = session_->add(new C("c1"));
    dbo::ptr<C> c2 = session_->add(new C("c2"));
    dbo::ptr<C> c3 = session_->add(new C("c3"));

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

    dbo::ptr<D> d(new D());

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

      dbo::ptr<D> d(new D());

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

    session_->add(new C("c1"));

    dbo::Query< dbo::ptr<C> > query = session_->find<C>();
    dbo::QueryModel< dbo::ptr<C> > *model
      = new dbo::QueryModel< dbo::ptr<C> >();

    model->setQuery(query);

    t.commit();

    boost::any d;

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

    session_->add(new D(Coordinate(5, 6), "yes"));
    dbo::ptr<D> d1 = session_->find<D>();

    BOOST_REQUIRE(d1->name == "yes");
    BOOST_REQUIRE(d1->id == Coordinate(5, 6));

    session_->add(new C("c1"));

    dbo::Query< dbo::ptr<C> > query = session_->find<C>();
    dbo::QueryModel< dbo::ptr<C> > *model
      = new dbo::QueryModel< dbo::ptr<C> >();

    model->setQuery(query);

    boost::any d;

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

    dbo::ptr<B> b1 = session_->add(new B("b1", B::State1));
    dbo::ptr<B> b2 = session_->add(new B("b2", B::State2));
    dbo::ptr<B> b3 = session_->add(new B("b3", B::State1));


    {
      dbo::collection<dbo::ptr<B> > c;
      c = session_->query< dbo::ptr<B> >
	("select B from " SCHEMA "\"table_b\" B ")
	.where("B.\"state\" = ?").orderBy("B.\"name\"")
	.limit(1).bind(0);

      BOOST_REQUIRE(c.size() == 1);
    }

    dbo::ptr<B> d = session_->query< dbo::ptr<B> >
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
  a1.wstring = "Hello";
  a1.string = "There";
  a1.checked = false;
  a1.i = 42;
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
    dbo::ptr<A> a = session_->add(new A(a1));    
    dbo::ptr<B> b = session_->add(new B(b1));

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

    dbo::ptr<A> a = session_->add(new A());
    dbo::ptr<B> b = session_->add(new B("b", B::State1));
    a.modify()->b = b;

    {
      dbo::collection<dbo::ptr<A> > c = session_->query< dbo::ptr<A> >
	("select A from " SCHEMA "\"table_a\" A ").where("\"b_id\" = ?").bind(b);

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
    a1.ptime = boost::posix_time::ptime
      (boost::gregorian::date(2005,boost::gregorian::Jan,1),
       boost::posix_time::time_duration(1,2,3));
    a1.pduration = boost::posix_time::hours(1) + boost::posix_time::seconds(10);
    a1.i = 42;
    a1.i64 = 9223372036854775805LL;
    a1.ll = 6066005651767221LL;
    a1.checked = true;
    a1.f = (float)42.42;
    a1.d = 42.424242;

    dbo::ptr<A> a = session->add(new A());

    t.commit();

    {
      dbo::Transaction t(*session);

      dbo::Query< dbo::ptr<A> > query = session->find<A>();
      dbo::QueryModel< dbo::ptr<A> > *model
	= new dbo::QueryModel< dbo::ptr<A> >();
      model->setQuery(query);
      model->addColumn ("date");
      model->addColumn ("time");
      model->addColumn ("binary");
      model->addColumn ("datetime");
      model->addColumn ("wstring");
      model->addColumn ("string");
      model->addColumn ("ptime");
      model->addColumn ("pduration");
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
      boost::posix_time::ptime p_time 
	(boost::gregorian::date(2010,boost::gregorian::Sep,9),
	 boost::posix_time::time_duration(3,2,1));
      boost::posix_time::time_duration p_duration 
        = boost::posix_time::hours(1) + boost::posix_time::seconds(10);
      int i = 50;
      ::int64_t i64 = 8223372036854775805LL;;
      long long ll = 7066005651767221LL;
      float f = (float)53.53;
      double d = 53.5353;
      bool checked = false;
      
      model->setData(0, 0, boost::any(date));
      model->setData(0, 1, boost::any(time));
      model->setData(0, 2, boost::any(bin));
      model->setData(0, 3, boost::any(Wt::WDateTime(date, time)));
      model->setData(0, 4, boost::any(ws));
      model->setData(0, 5, boost::any(s));
      model->setData(0, 6, boost::any(p_time));
      model->setData(0, 7, boost::any(p_duration));
      model->setData(0, 8, boost::any(i));
      model->setData(0, 9, boost::any(i64));
      model->setData(0, 10, boost::any(ll));
      model->setData(0, 11, boost::any(checked));
      model->setData(0, 12, boost::any(f));
      model->setData(0, 13, boost::any(d));

      //TODO, also set data using strings to test string to any value conversion

      dbo::ptr<A> aa = session->find<A>().resultValue();
      BOOST_REQUIRE(aa->date == date);
      BOOST_REQUIRE(aa->time == time);
      BOOST_REQUIRE(aa->binary == bin);
      BOOST_REQUIRE(aa->datetime == Wt::WDateTime(date, time));
      BOOST_REQUIRE(aa->wstring == ws);
      BOOST_REQUIRE(aa->string == s);
      BOOST_REQUIRE(aa->ptime == p_time);
      BOOST_REQUIRE(aa->pduration == p_duration);
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

    dbo::ptr<A> a = session_->add(new A());
    dbo::ptr<B> b = session_->add(new B("b", B::State1));
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

    dbo::ptr<A> a = session_->add(new A());
    dbo::ptr<C> c = session_->add(new C());

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

    dbo::ptr<A> a = session_->add(new A());
    dbo::ptr<B> b = session_->add(new B("b", B::State1));
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

    dbo::ptr<A> a1(new A());
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

    dbo::ptr<A> a1(new A());
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

    dbo::ptr<A> a1(new A());
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

    dbo::ptr<A> a1(new A());
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

    A *a1 = new A();
    a1->ll = 123456L;
    dbo::ptr<const A> aPtr(a1);

    session_->add(aPtr);
    t.commit();
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<const A> a1 = session_->find<const A>();
    dbo::ptr<A> a2 = session_->find<A>();
    dbo::collection<dbo::ptr<const A> > as1 = session_->find<const A>();
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

    A *a1 = new A();
    a1->ll = 123456L;
    dbo::ptr<A> aPtr1(a1);

    C *c1 = new C();
    c1->name = "Jos";
    dbo::ptr<C> cPtr1(c1);

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

    D *d = new D();
    d->id = Coordinate(2, 4);

    dbo::ptr<D> dPtr = session_->add(d);
    dbo::ptr<C> cPtr = session_->add(new C());

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

    dbo::ptr<E> e1(new E("e1"));
    dbo::ptr<C> c1(new C("c1"));
    session_->add(e1);
    session_->add(c1);

    typedef dbo::ptr_tuple<E, C>::type EC;
    EC ec = session_->query< EC >
      ("select E, C from  " SCHEMA "\"table_e\" E, " SCHEMA "\"table_c\" C");

    BOOST_REQUIRE(ec.get<0>()->name == "e1");
    BOOST_REQUIRE(ec.get<1>()->name == "c1");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test24b )
{
#ifndef SQLITE3
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<E> e1(new E("e1"));
    session_->add(e1);

    typedef dbo::ptr_tuple<E, E>::type EE;
    EE ee = session_->query< EE >
      ("select \"E1\", \"E2\" from " SCHEMA "\"table_e\" \"E1\" "
       "right join " SCHEMA "\"table_e\" \"E2\" on \"E1\".\"id\" != \"E2\".\"id\"");

    BOOST_REQUIRE(ee.get<1>()->name == "e1");
  }
#endif // SQLITE3
}

BOOST_AUTO_TEST_CASE( dbo_test24c )
{
  DboFixture f;
  dbo::Session *session_ = f.session_;
  {
    dbo::Transaction t(*session_);

    dbo::ptr<D> d1(new D(Coordinate(42, 43), "d1"));
    dbo::ptr<C> c1(new C("c1"));
    session_->add(d1);
    session_->add(c1);

    typedef dbo::ptr_tuple<D, C>::type DC;
    DC dc = session_->query< DC >
      ("select D, C from " SCHEMA "\"table_d\" D, " SCHEMA "\"table_c\" C");

    BOOST_REQUIRE(dc.get<0>()->name == "d1");
    BOOST_REQUIRE(dc.get<1>()->name == "c1");
  }
}

BOOST_AUTO_TEST_CASE( dbo_test25 )
{
#ifndef FIREBIRD // Cannot order by on blobs in Firebird...
  DboFixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    dbo::ptr<F> e1 = session_->add(new F("Alice",  "Kramden", "Female"));
    dbo::ptr<F> e2 = session_->add(new F("Ralph",  "Kramden", "Male"));
    dbo::ptr<F> e3 = session_->add(new F("Trixie", "Norton", "Female"));
    dbo::ptr<F> e4 = session_->add(new F("Ed",     "Norton", "Male"));

    t.commit();
  }

  {
    dbo::Query< dbo::ptr<F> > query = session_->find<F>().orderBy("\"id\"");
    dbo::QueryModel< dbo::ptr<F> > *model
      = new dbo::QueryModel< dbo::ptr<F> >();

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
    model->sort(2, Wt::AscendingOrder);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    delete model;
  }

  {
    class custom_sort_model : public dbo::QueryModel< dbo::ptr<F> > {
    public:
      custom_sort_model() : dbo::QueryModel< dbo::ptr<F> >() { }

      virtual std::string createOrderBy(int column, Wt::SortOrder order)
      {
        std::string dir = (order == Wt::AscendingOrder ? "asc" : "desc");
        return "\"" + fieldInfo(column).name() + "\" " + dir +
          ((column != 3) ? ", \"last_name\"" : "") +
          ((column != 2) ? ", \"first_name\"" : "");
      }
    };

    dbo::Query< dbo::ptr<F> > query = session_->find<F>().orderBy("\"id\"");

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
    model->sort(3, Wt::AscendingOrder);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    // sort on last name descending, then first name ascending
    model->sort(3, Wt::DescendingOrder);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Trixie");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Ralph");

    // sort on first name, then last name ascending
    model->sort(2, Wt::AscendingOrder);
    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "Alice");
    BOOST_REQUIRE(Wt::asString(model->data(1, 2)) == "Ed");
    BOOST_REQUIRE(Wt::asString(model->data(2, 2)) == "Ralph");
    BOOST_REQUIRE(Wt::asString(model->data(3, 2)) == "Trixie");

    // sort on gender, then last name, then first name
    model->sort(4, Wt::AscendingOrder);
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

    session_->add(new F("Alice",  "Kramden", "Female"));

    dbo::Query< dbo::ptr<F> > query = session_->find<F>();
    dbo::QueryModel< dbo::ptr<F> > *model
      = new dbo::QueryModel< dbo::ptr<F> >();

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
    model->dataChanged().connect(boost::bind<bool>(boost::ref(checkExpected),
          boost::ref(ExpectedFirstName)));

    /*
     * The setItemData() convenience method commits
     * a transaction prior to emitting dataChanged()
     */
    ExpectedFirstName = "AliceTwo";
    Wt::WAbstractItemModel::DataMap map;
    map[Wt::EditRole] = ExpectedFirstName;
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

    dbo::ptr<B> b(new B());
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
