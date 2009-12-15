/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/SqlStatement"

#include <cassert>

namespace Wt {
  namespace Dbo {

SqlConnection::~SqlConnection()
{
  assert(statementCache_.empty());
}

void SqlConnection::clearStatementCache()
{
  for (StatementMap::iterator i = statementCache_.begin();
       i != statementCache_.end(); ++i)
    delete i->second;

  statementCache_.clear();
}

void SqlConnection::executeSql(const std::string& sql)
{
  SqlStatement *s = prepareStatement(sql);
  s->execute();
  delete s;
}

SqlStatement *SqlConnection::getStatement(const std::string& id) const
{
  StatementMap::const_iterator i = statementCache_.find(id);
  if (i != statementCache_.end())
    return i->second;
  else
    return 0;
}

void SqlConnection::saveStatement(const std::string& id,
				  SqlStatement *statement)
{
  statementCache_[id] = statement;
}

  }
}
