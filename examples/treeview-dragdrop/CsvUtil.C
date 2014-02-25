#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <Wt/WAbstractItemModel>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WString>

#include "CsvUtil.h"

/*
 * A standard item which converts text edits to numbers
 */
class NumericItem : public Wt::WStandardItem {
public:
  virtual NumericItem *clone() const {
    return new NumericItem();
  }

  virtual void setData(const boost::any &data, int role = Wt::UserRole) {
    boost::any dt;

    if (role == Wt::EditRole) {
      std::string s = Wt::asString(data).toUTF8();

      char *end;
      double d = std::strtod(s.c_str(), &end);
      if (*end == 0)
	dt = boost::any(d);
      else
	dt = data;
    }

    Wt::WStandardItem::setData(dt, role);
  }
};

Wt::WStandardItemModel *csvToModel(const std::string& csvFile,
				   Wt::WObject *parent,
				   bool firstLineIsHeaders)
{
  std::ifstream f(csvFile.c_str());

  if (f) {
    Wt::WStandardItemModel *result = new Wt::WStandardItemModel(0, 0, parent);
    result->setItemPrototype(new NumericItem());
    readFromCsv(f, result, -1, firstLineIsHeaders);
    return result;
  } else
    return 0;
}

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

	  boost::any data(Wt::WString::fromUTF8(*i));
	  model->setData(dataRow, col, data);
	}
      }
    }

    ++csvRow;
  }
}
