/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/Dbo/QueryColumn>

namespace Wt {
  namespace Dbo {

  /*
   Editing ramblings

   Scenario:
    - query a Post, join its author User, and the number of comments

   Three different kinds of edits:
    - change a Post property
    - change a User property
    - change the author
    - cannot change the number of comments

   You want to display the author as its name (but that may be a
   combination of multiple fields, do we need to customize -- needs
   custom data()). Otherwise it is its primary key.... Unless we have a
   model used for editing, then we can lookup the key in that model and
   use the display value for editing ? In that way you need only once
   the translation from entity to display value.

    Even better, use the database to generate the display value in the map
    table / join clause ? We should at least support this: differentiate between
    edit and display field indexes ?
      "author_id" = editFieldIdx
      'u.name' || u."first_name" = display FieldIdx
      select ..., 'u.name' || u."first_name" from post join user u on author_id == u.id

   The query model item delegate puts a combo box, and populates
     its values with:
      - QueryModel from session.find<User>() with two fields added as columns:
        - display column = ...
        - value column = column 0
      - This information 
 */

QueryColumn::QueryColumn(const std::string& field, WFlags<ItemFlag> flags)
  : field_(field),
    flags_(flags),
    fieldIdx_(-1)
{ }

  }
}
