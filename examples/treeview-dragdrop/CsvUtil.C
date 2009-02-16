#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <Wt/WAbstractItemModel>
#include <Wt/WString>

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
	  model->setHeaderData(col, boost::any(Wt::WString::fromUTF8(*i)));
	else {
	  int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

	  if (numRows != -1 && dataRow >= numRows)
	    return;

	  if (dataRow >= model->rowCount())
	    model->insertRows(model->rowCount(),
			      dataRow + 1 - model->rowCount());

	  std::string s = *i;

	  boost::any data;

	  char *end;
	  int i = std::strtol(s.c_str(), &end, 10);
	  if (*end == 0)
	    data = boost::any(i);
	  else {
	    double d = std::strtod(s.c_str(), &end);
	    if (*end == 0)
	      data = boost::any(d);
	    else
	      data = boost::any(Wt::WString::fromUTF8(s));
	  }

	  model->setData(dataRow, col, data);
	}
      }
    }

    ++csvRow;
  }
}
