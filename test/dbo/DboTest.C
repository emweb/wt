/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifdef WTDBO

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Firebird>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>
#include <Wt/Dbo/QueryModel>

//#define SCHEMA "test."
#define SCHEMA ""

namespace dbo = Wt::Dbo;

class A;
class B;
class C;
class D;

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
      field(action, coordinate.x, name + "_x");
      field(action, coordinate.y, name + "_y");
    }
  }
}

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

  std::vector<unsigned char> binary;
  Wt::WDate date;
  Wt::WTime time;
  Wt::WDateTime datetime;
  Wt::WString wstring;
  std::string string;
  bool checked;
  int i;
  ::int64_t i64;
  long long ll;
  float f;
  double d;

  bool operator== (const A& other) const {
    if (binary.size() != other.binary.size())
      return false;

    for (unsigned j = 0; j < binary.size(); ++j)
      if (binary[j] != other.binary[j])
	return false;

    return date == other.date
      && time == other.time
      && datetime == other.datetime
      && wstring == other.wstring
      && string == other.string
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
    dbo::field(a, string, "string", 50);
    dbo::field(a, i, "i");
    dbo::field(a, i64, "i64");
    dbo::field(a, ll, "ll");
    dbo::field(a, checked, "checked");
    dbo::field(a, f, "f");
    dbo::field(a, d, "d");

    dbo::belongsTo(a, b, "b", dbo::OnUpdateCascade | dbo::OnDeleteCascade);

    if (a.session()) {
      dbo::belongsTo(a, dthing, a.session()->template tableName<D>());

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

  B() { }

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
    dbo::field(a, name, "name");

    dbo::hasMany(a, asManyToOne, dbo::ManyToOne, "b");
    dbo::hasMany(a, csManyToMany, dbo::ManyToMany, SCHEMA "b_c", "the_b",
		   dbo::NotNull
		 | dbo::OnDeleteCascade
		 | dbo::OnUpdateCascade);
  }
};

class C {
public:
  std::string name;
  
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
    dbo::field(a, name, "name");

    dbo::hasMany(a, bsManyToMany, dbo::ManyToMany, SCHEMA "b_c", "the_c",
		   dbo::NotNull
		 | dbo::OnDeleteCascade
		 | dbo::OnUpdateCascade );
    dbo::hasMany(a, dsManyToMany, dbo::ManyToMany, SCHEMA "c_d");
  }
};

class D {
public:
  Coordinate id;
  std::string name;

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
    dbo::field(a, name, "name");

    dbo::hasMany(a, asManyToOne, dbo::ManyToOne, "d");
    dbo::hasMany(a, csManyToMany, dbo::ManyToMany, SCHEMA "c_d");
  }
};

struct DboFixture
{
  DboFixture()
  {
    static bool logged = false;
    dbo::SqlConnection *connection;

#ifdef SQLITE3
    if (!logged) {
      std::cerr << "DboTest.C created a Sqlite3 connector" << std::endl;
      logged = true;
    }

    dbo::backend::Sqlite3 *sqlite3 = new dbo::backend::Sqlite3(":memory:");
    sqlite3->setDateTimeStorage(dbo::SqlDate,
				dbo::backend::Sqlite3::JulianDaysAsReal);
    connection = sqlite3;
#endif // SQLITE3

#ifdef POSTGRES
    if (!logged) {
      std::cerr << "DboTest.C created a Postgres connector" << std::endl;
      logged = true;
    }

    connection = new dbo::backend::Postgres
      ("user=postgres_test password=postgres_test port=5432 dbname=wt_test");
#endif // POSTGRES

#ifdef FIREBIRD
    std::string file;
#ifdef WIN32
    file = "C:\\opt\\db\\firebird\\wt_test.fdb";
#else
    file = "/opt/db/firebird/wt_test.fdb";
#endif

    if (!logged) {
      std::cerr << "DboTest.C created a Firebird connector" << std::endl;
      logged = true;
    }

    connection = new dbo::backend::Firebird ("localhost", 
					     file, 
					     "test_user", "test_pwd", 
					     "", "", "");
#endif // FIREBIRD

    connection->setProperty("show-queries", "true");

    connectionPool_ = new dbo::FixedSqlConnectionPool(connection, 5);

    session_ = new dbo::Session();
    session_->setConnectionPool(*connectionPool_);

    session_->mapClass<A>(SCHEMA "table_a");
    session_->mapClass<B>(SCHEMA "table_b");
    session_->mapClass<C>(SCHEMA "table_c");
    session_->mapClass<D>(SCHEMA "table_d");

    session_->createTables();

    Wt::registerType<Coordinate>();
  }

  ~DboFixture()
  {
    session_->dropTables();

    delete session_;
    delete connectionPool_;
  }

  dbo::SqlConnectionPool *connectionPool_;
  dbo::Session *session_;
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
  a1.wstring = "Hello";
  a1.string = "There";
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
  }

  /* Remove the A, check it is no longer found during the same transaction */
  {
    dbo::Transaction t(*session_);

    {
      As allAs = session_->find<A>();
      BOOST_REQUIRE(allAs.size() == 1);
      dbo::ptr<A> a2 = *allAs.begin();

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
      = session_->query<std::string>("select \"name\" from \"table_b\"");

    for (dbo::collection<std::string>::const_iterator i = names.begin();
	 i != names.end(); ++i)
      std::cerr << *i << std::endl;
  }

  {
    dbo::Transaction t(*session_);

    dbo::ptr<B> b1 = session_->query< dbo::ptr<B> >
      ("select distinct B from \"table_b\" B ").where("B.\"name\" = ?").bind("b1");

    std::size_t count = session_->query< dbo::ptr<B> >
      ("select distinct B from \"table_b\" B ").where("B.\"name\" = ?").bind("b1")
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

//The query below is extended to 
//select count(1) from ( select B."id", B."name", A."id", A."date", A."b_id" 
//from "table_b" B join "table_a" A on A."b_id" = B."id"); 
//by Dbo when it is used to return the size of the collection.
//This is not valid SQL by the SQL standard definition,
//because 2 id fields are mentioned in the select clause.
//A valid alternative would be:
//select count(1) from ( select B."id", B."name", A."id" as id2, A."date", 
//A."b_id" from "table_b" B join "table_a" A on A."b_id" = B."id");
//Firebird is not able to execute this query.
#ifndef FIREBIRD
    BAs bas = session_->query<BA>
      ("select B, A "
       "from \"table_b\" B join \"table_a\" A on A.\"b_id\" = B.\"id\"")
      .orderBy("A.\"i\"");

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
#endif //FIREBIRD
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
  {
    dbo::Transaction t(*session_);

    std::string result = session_->query<std::string>("select 'dima '' ? '");
    BOOST_REQUIRE(result == "dima ' ? ");
  }
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
      ("select \"id\", \"id\" from \"table_a\"").resultValue();

    BOOST_REQUIRE(id1 == aId);
    BOOST_REQUIRE(id2 == aId);

#ifdef POSTGRES
    dbo::ptr<A> a;
    int id;
    boost::tie(a, id) = session_->query<boost::tuple<dbo::ptr<A>, int> >
      ("select (a), a.\"id\" from \"table_a\" a").resultValue();

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

    session_->execute("delete from \"table_a\"");
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
      t.commit();
    } catch (std::exception& e) {
      std::cerr << "Catching exception: " << e.what() << std::endl;
      caught = true;
    }

    BOOST_REQUIRE(caught);
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

    BOOST_REQUIRE(model->columnCount() == 3);
    BOOST_REQUIRE(model->rowCount() == 1);

    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "name");

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

    BOOST_REQUIRE(model->columnCount() == 3);
    BOOST_REQUIRE(model->rowCount() == 1);

    BOOST_REQUIRE(Wt::asString(model->headerData(0)) == "id");
    BOOST_REQUIRE(Wt::asString(model->headerData(1)) == "version");
    BOOST_REQUIRE(Wt::asString(model->headerData(2)) == "name");

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "c1");

    model->setData(0, 2, std::string("changed"));

    BOOST_REQUIRE(Wt::asString(model->data(0, 2)) == "changed");

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
      dbo::collection<dbo::ptr<B> > c = session_->query< dbo::ptr<B> >
	("select B from \"table_b\" B ")
	.where("B.\"state\" = ?").orderBy("B.\"name\"")
	.limit(1).bind(0);

      BOOST_REQUIRE(c.size() == 1);
    }

    dbo::ptr<B> d = session_->query< dbo::ptr<B> >
      ("select B from \"table_b\" B ")
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
	("select A from \"table_a\" A ").where("\"b_id\" = ?").bind(b);

      BOOST_REQUIRE(c.size() == 1);
    }
  }
}

#endif
