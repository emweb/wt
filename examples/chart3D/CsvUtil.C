/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>
#include <boost/tokenizer.hpp>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WString.h>

#include "CsvUtil.h"

void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
		 int numRows, bool firstLineIsHeaders)
{
  int csvRow = 0;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
	CsvTokenizer;
      CsvTokenizer tok(line);

      int col = 0;
      for (CsvTokenizer::iterator i = tok.begin();
	   i != tok.end(); ++i, ++col) {

	if (col >= model->columnCount())
	  model->insertColumns(model->columnCount(),
			       col + 1 - model->columnCount());

	if (firstLineIsHeaders && csvRow == 0)
          model->setHeaderData(col, Wt::cpp17::any(Wt::WString(*i)));
	else {
	  int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

	  if (numRows != -1 && dataRow >= numRows)
	    return;

	  if (dataRow >= model->rowCount())
	    model->insertRows(model->rowCount(),
			      dataRow + 1 - model->rowCount());

          Wt::cpp17::any data;
	  std::string s = *i;

	  char *endptr;

	  if (s.empty())
            data = Wt::cpp17::any();
	  else {
	    double d = strtod(s.c_str(), &endptr);
	    if (*endptr == 0)
              data = Wt::cpp17::any(d);
	    else
              data = Wt::cpp17::any(Wt::WString(s));
	  }

	  model->setData(dataRow, col, data);
	}
      }
    }

    ++csvRow;
  }
}
