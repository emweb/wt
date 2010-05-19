/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/Dbo/QueryColumn>

namespace Wt {
  namespace Dbo {

QueryColumn::QueryColumn(const std::string& displayField)
  : displayField_(displayField),
    displayFieldIdx_(-1),
    editFieldIdx_(-1),
    editValuesModel_(0)
{ }

  }
}
