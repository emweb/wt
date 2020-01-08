/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <boost/version.hpp>
#include <Wt/Dbo/Dbo.h>

namespace dbo = Wt::Dbo;

#define SQL(...) #__VA_ARGS__

namespace {

void parseSql(const std::string& sql,  int listsCount,
	      int fieldsCount)
{
  dbo::Impl::SelectFieldLists result;

  dbo::Impl::parseSql(sql, result);

  BOOST_REQUIRE(result.size() == (unsigned)listsCount);

  int fields = 0;
  for (unsigned i = 0; i < result.size(); ++i) {
    fields += static_cast<int>(result[i].size());
    for (unsigned j = 0; j < result[i].size(); ++j) {
      dbo::Impl::SelectField& f = result[i][j];
      std::cerr << "Field: '" << sql.substr(f.begin, f.end - f.begin)
		<< "'" << std::endl;
    }
  }

  BOOST_REQUIRE(fields == fieldsCount);
}

}

BOOST_AUTO_TEST_CASE( DboImplTest_test1 )
{
  parseSql("select 1", 1, 1);
  parseSql("select a, b from foo", 1, 2);
  parseSql("select distinct a, b from foo", 1, 2);
  parseSql("select '1'", 1, 1);
  parseSql("select distinct '1'", 1, 1);
  parseSql("select 'Barts'' car'", 1, 1);

#if BOOST_VERSION >= 104100
  // These ones only work correctly with our new spirit-based parser

  parseSql("select 'Barts'', car', bike from depot", 1, 2);

  parseSql("SELECT cast(round(number, 2) as text) AS column_number FROM table",
	   1, 1);

  parseSql
    (SQL
     (WITH
      regional_sales AS 
      (
       SELECT region, SUM(amount) AS total_sales 
       FROM orders 
       GROUP BY region 
       ),
      top_regions AS
      ( 
       SELECT region 
       FROM regional_sales 
       WHERE total_sales > (SELECT SUM(total_sales)/10 FROM 
			    regional_sales)
      )
      SELECT region,
             product,
             SUM(quantity) AS product_units,
             SUM(amount) AS product_sales 
      FROM orders 
      WHERE region IN (SELECT region FROM top_regions)
      GROUP BY region, product, result, simpleSelectCount
      ),
     1, 4
     );

  parseSql
    (SQL
     (select a from foo
      intersect
      select b, c from bar),
     2, 3);

  parseSql("select from_a, from_b from table",1,2);
  parseSql("select a_from, b_from from table",1,2);

  parseSql("select from_a, from_b",1,2);
  parseSql("select a_from, b_from",1,2);

  parseSql("select from_a, SUM(SELECT from_b from table) from other_table",1,2);

#endif // BOOST_VERSION
}
