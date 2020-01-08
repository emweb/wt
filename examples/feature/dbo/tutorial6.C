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
 * Dbo tutorial section 7.4
 *  Specifying a natural primary key that is also a foreign key
 *****/

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

namespace dbo = Wt::Dbo;

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
	       int /*size*/ = -1)
    {
      field(action, coordinate.x, name + "_x");
      field(action, coordinate.y, name + "_y");
    }
  }
}

class GeoTag;

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<GeoTag> : public dbo_default_traits
    {
      typedef Coordinate IdType;
      static IdType invalidId() { return Coordinate(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

class GeoTag {
public:
  Coordinate  position;
  std::string name;

  template <class Action>
  void persist(Action& a)
  {
    dbo::id(a, position, "position");
    dbo::field(a, name, "name");
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

  session.mapClass<GeoTag>("geotag");

  /*
   * Try to create the schema (will fail if already exists).
   */
  session.createTables();

  {
    dbo::Transaction transaction(session);

    std::unique_ptr<GeoTag> tag{new GeoTag()};
    tag->position = Coordinate(5091, 315);
    tag->name = "Oekene";

    dbo::ptr<GeoTag> tagPtr = session.add(std::move(tag));

    transaction.commit();

    std::cerr << tagPtr.id() << std::endl;
  }
}

int main(int argc, char **argv)
{
  run();
}
