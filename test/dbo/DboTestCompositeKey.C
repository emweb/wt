/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>

#include "DboFixture.h"

#include <string>
namespace dbo = Wt::Dbo;

//----------------------------------- DBOs -------------------------------------
//Fwd dec
class Page;
class Module;
struct PageKeys;
typedef Wt::Dbo::collection< Wt::Dbo::ptr<Page> > PageCollections;

//Module Dbo
class Module
{
public:
  PageCollections PageCollection;

  template<class Action>void persist(Action &a)
  {
    Wt::Dbo::hasMany(a, PageCollection, Wt::Dbo::ManyToOne, "Module");
  }
};

//Page Dbo
struct PageKeys
{
  long long id;
  Wt::Dbo::ptr<Module> ModulePtr;

  PageKeys();
  PageKeys(long long id, Wt::Dbo::ptr<Module> ModulePtr);

  bool operator< (const PageKeys &other) const;
  bool operator== (const PageKeys &other) const;
};
std::ostream &operator<< (std::ostream &o, const PageKeys &c);


//Mapping for PageKeys composite key
namespace Wt
{
  namespace Dbo
  {
    template<class Action>
      void field(Action &action, PageKeys &Keys, const std::string &name, int size = -1)
      {
        field(action, Keys.id, name + "_page_id");
        belongsTo(action, Keys.ModulePtr, name + "_Module", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
      }
    template<>
      struct dbo_traits<Page> : public dbo_default_traits
      {
        typedef PageKeys IdType;
        static IdType invalidId();
        static const char *surrogateIdField();
      };
  }
}

class Page
{
public:
  PageCollections ChildrenPages;
  Wt::Dbo::ptr<Page> ParentPage;

  Page();
  Page(long long id, Wt::Dbo::ptr<Module> ModulePtr = Wt::Dbo::ptr<Module>());

  template<class Action>void persist(Action &a)
  {
    Wt::Dbo::id(a, _Id, "Page");

    Wt::Dbo::hasMany(a, ChildrenPages, Wt::Dbo::ManyToOne, "Parent_Page");
#ifndef MSSQLSERVER
    Wt::Dbo::belongsTo(a, ParentPage, "Parent_Page", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade);
#else
    Wt::Dbo::belongsTo(a, ParentPage, "Parent_Page");
#endif
  }

private:
  PageKeys _Id;
};

PageKeys::PageKeys()
  : id(-1)
{ }
PageKeys::PageKeys(long long id, Wt::Dbo::ptr<Module> ModulePtr)
  : id(id), ModulePtr(ModulePtr)
{ }
bool PageKeys::operator== (const PageKeys &other) const
{
  return id == other.id && ModulePtr == other.ModulePtr;
}
bool PageKeys::operator< (const PageKeys &other) const
{
  if(id < other.id)
  {
    return true;
  }
  else if(id == other.id)
  {
    return ModulePtr < other.ModulePtr;
  }
  else
  {
    return false;
  }
}
std::ostream &operator<< (std::ostream &o, const PageKeys &c)
{
  return o << "(" << c.id << ", " << c.ModulePtr << ")";
}

Wt::Dbo::dbo_traits<Page>::IdType Wt::Dbo::dbo_traits<Page>::invalidId()
{
  return Wt::Dbo::dbo_traits<Page>::IdType();
}
const char* Wt::Dbo::dbo_traits<Page>::surrogateIdField()
{
  return 0;
}

Page::Page()
{ }
Page::Page(long long id, Wt::Dbo::ptr<Module> ModulePtr)
  : _Id(id, ModulePtr)
{ }
//------------------------------- End of DBOs ----------------------------------

struct DboCompositeKeyFixture : DboFixtureBase
{
  DboCompositeKeyFixture() :
    DboFixtureBase()
  {
    session_->mapClass<Module>("modules");
    session_->mapClass<Page>("pages");

    //Drop/Create
    try
    {
      session_->dropTables(); //todo:remove
    }
    catch(...)
    { }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;
    session_->createTables();
  }
};

BOOST_AUTO_TEST_SUITE( DBO_TEST_SUITE_NAME )

BOOST_AUTO_TEST_CASE( dbo_test_composite_concat )
{
  DboCompositeKeyFixture f;
}

BOOST_AUTO_TEST_SUITE_END()
