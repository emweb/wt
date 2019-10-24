/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/Dbo/QueryColumn.h>

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

   Idea of the night for foreign keys and combo-box editing
    - two more ItemDataRoles:
      - OptionsModel
        a WAbstractItemModel: column 0 has entries with data of two roles:
        - ItemDataRole::Edit: the primary key
	- ItemDataRole::Display: the value to display
      - Validator
        a WValidator: used to validate the data

    typedef boost::function<WString (const Result&)> ResultValueFunction;

    addColumn(const ResultValueFunction& function,
              const std::string& editField = "",
	      WAbstractItemModel *editOptions = 0);

    Let WItemDelegate support better editing:
     - if validator is a WDateValidator, then display a date picker
     - if OptionsModel data is not empty, then display a combo box
 */

  }
}
