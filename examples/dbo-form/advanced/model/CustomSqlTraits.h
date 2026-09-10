// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CUSTOM_SQL_TRAITS_H_
#define CUSTOM_SQL_TRAITS_H_

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>

struct Text {
  Wt::WString content = "";
};

namespace Wt {
  namespace Dbo {
template<>
struct sql_value_traits<Text, void> {
  static std::string type(SqlConnection *conn, int size)
  {
    return conn->textType(size);
  }

  static void bind(Text value, SqlStatement *statement, int column, int)
  {
    statement->bind(column, value.content.toUTF8());
  }

  static bool read(Text& value, SqlStatement *statement, int column, int size)
  {
    std::string s;
    bool result = statement->getResult(column, &s, size);
    if (result) {
      value.content = Wt::WString::fromUTF8(s);
      return true;
    } else {
      value.content = Wt::WString::Empty;
      return false;
    }
  }
};
  }
}

#endif // CUSTOM_SQL_TRAITS_H_
