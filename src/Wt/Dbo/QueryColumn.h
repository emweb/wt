// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_COLUMN_H_
#define WT_DBO_QUERY_COLUMN_H_

#include <Wt/WModelIndex.h>
#include <Wt/WString.h>
#include <Wt/Dbo/Dbo.h>

namespace Wt {
class WAbstractTableModel;

  namespace Dbo {

// TODO figure out a nice syntax for qualified fields ?
class QueryColumn 
{
  QueryColumn(const std::string& field,
	      const WString& header,
	      WFlags<ItemFlag> flags);

  /*
  QueryColumn(const std::string& field,
	      WAbstractTableModel *editValuesModel);
  */

private:
  typedef std::map<ItemDataRole, cpp17::any> HeaderData;

  std::string field_;
  WFlags<ItemFlag> flags_;
  int fieldIdx_;
  HeaderData headerData_;

  WAbstractTableModel *editValuesModel_;

  template <class Result> friend class QueryModel;
};

/*
 * Defined in the header file to avoid a link-time dependency on
 * libwt.so
 */
inline QueryColumn::QueryColumn(const std::string& field,
				const WString& header,
				WFlags<ItemFlag> flags)
  : field_(field),
    flags_(flags),
    fieldIdx_(-1)
{ 
  headerData_[ItemDataRole::Display] = header;
}

  }
}

#endif // WT_DBO_QUERY_COLUMN_H_

