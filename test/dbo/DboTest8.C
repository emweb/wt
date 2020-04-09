/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>

#include "DboFixture.h"

#include <algorithm>

namespace dbo = Wt::Dbo;

class TableSrc1
{
public:

  template<typename Action>
  void persist(Action &a)
  {
    (void)a;
  }
};

class TableSrc2
{
public:

  template<typename Action>
  void persist(Action &a)
  {
    (void)a;
  }
};

class TableDst
{
public:
  dbo::ptr<TableSrc1> table_src1;
  dbo::ptr<TableSrc2> table_src2;

  template<typename Action>
  void persist(Action &a)
  {
    dbo::belongsTo(a, table_src1, dbo::OnDeleteRestrict);
    dbo::belongsTo(a, table_src2, dbo::OnUpdateRestrict);
  }
};

struct Dbo8Fixture : DboFixtureBase {
  Dbo8Fixture()
  {
    session_->mapClass<TableSrc1>("table_src1");
    session_->mapClass<TableSrc2>("table_src2");
    session_->mapClass<TableDst>("table_dst");
  }
};

BOOST_AUTO_TEST_CASE( dbo8_test1 )
{
  Dbo8Fixture f;
  dbo::Session &session = *f.session_;

  auto sql = session.tableCreationSql();

  std::string needle_del_restrict = "on delete restrict";
  const auto it_del_restrict = std::search(begin(sql), end(sql), begin(needle_del_restrict), end(needle_del_restrict));
  BOOST_REQUIRE((it_del_restrict != end(sql)));

  std::string needle_update_restrict = "on update restrict";
  const auto it_update_restrict = std::search(begin(sql), end(sql), begin(needle_update_restrict), end(needle_update_restrict));
  BOOST_REQUIRE((it_update_restrict != end(sql)));

  try
  {
    session.createTables();
    BOOST_REQUIRE(true);
  }
  catch(...)
  {
    BOOST_REQUIRE(false);
  }
}
