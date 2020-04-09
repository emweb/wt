#define BOOST_TEST_MODULE Foreign Key Constraint

#include <boost/test/included/unit_test.hpp>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/Dbo.h>
#include <algorithm>

namespace dbo = Wt::Dbo;

class TableSrc1
{
    public:

        template<typename Action>
        void persist([[maybe_unused]] Action &a)
        {
        }
};

class TableSrc2
{
    public:

        template<typename Action>
        void persist([[maybe_unused]] Action &a)
        {
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

BOOST_AUTO_TEST_CASE(foreign_key_constraints_test)
{
    dbo::Session session;
    auto conn = std::make_unique<dbo::backend::Sqlite3>(":memory:");
    session.setConnection(std::move(conn));

    session.mapClass<TableSrc1>("table_src1");
    session.mapClass<TableSrc2>("table_src2");
    session.mapClass<TableDst>("table_dst");

    auto sql = session.tableCreationSql();

    std::string needle_del_restrict = "on delete restrict";
    const auto it_del_restrict = std::search(begin(sql), end(sql), begin(needle_del_restrict), end(needle_del_restrict));
    BOOST_REQUIRE((it_del_restrict != cend(sql)));

    std::string needle_update_restrict = "on update restrict";
    const auto it_update_restrict = std::search(begin(sql), end(sql), begin(needle_update_restrict), end(needle_update_restrict));
    BOOST_REQUIRE((it_update_restrict != cend(sql)));

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
