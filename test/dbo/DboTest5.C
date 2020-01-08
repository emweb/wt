/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include "DboFixture.h"

namespace dbo = Wt::Dbo;

class Shard
{
public:
  std::string name;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
  }
};

class MixedId
{
public:
  dbo::ptr<Shard> shard;
  std::string value;

  template<class Action>
  void persist(Action& a)
  {
    dbo::auxId(a, shard, "shard");
    dbo::field(a, value, "value");
  }
};

struct Dbo5Fixture : DboFixtureBase
{
  Dbo5Fixture() :
    DboFixtureBase()
  {
    session_->mapClass<MixedId> ("mixed");
    session_->mapClass<Shard> ("shard");
    try {
      session_->dropTables(); //todo:remove
    } catch (...) {
    }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;

    session_->createTables();
  }
};

BOOST_AUTO_TEST_CASE( dbo5_test1 )
{
  Dbo5Fixture f;

  dbo::Session& session = *f.session_;

  long long id = -1;
  {
    dbo::Transaction transaction(session);
    std::unique_ptr<Shard> shard{new Shard()};
    shard->name = "shard1";
    dbo::ptr<Shard> s = session.add(std::move(shard));
    
    std::unique_ptr<MixedId> mixedId{new MixedId()};
    mixedId->shard = s;
    mixedId->value = "A lot";
    dbo::ptr<MixedId> m = session.add(std::move(mixedId));
    m.flush();
    id = m.id();
  }

  {
    dbo::Transaction transaction(session);
    dbo::ptr<MixedId> m = session.load<MixedId>(id);

    m.modify()->value = "Plenty";
  }

  {
    dbo::Transaction transaction(session);
    dbo::ptr<MixedId> m = session.load<MixedId>(id);

    m.remove();
  }
}

