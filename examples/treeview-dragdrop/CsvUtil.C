#include <fstream>

#include <boost/tokenizer.hpp>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WString.h>

#include "CsvUtil.h"

/*
 * A standard item which converts text edits to numbers
 */
class NumericItem : public WStandardItem {
public:
  virtual std::unique_ptr<WStandardItem> clone() const {
    return std::unique_ptr<NumericItem>(std::make_unique<NumericItem>());
  }

  virtual void setData(const cpp17::any &data, ItemDataRole role = ItemDataRole::User) {
    cpp17::any dt;

    if (role == ItemDataRole::Edit) {
      std::string s = asString(data).toUTF8();

      char *end;
      double d = std::strtod(s.c_str(), &end);
      if (*end == 0)
        dt = cpp17::any(d);
      else
	dt = data;
    } else
      dt = data;

    WStandardItem::setData(dt, role);
  }
};

std::shared_ptr<WStandardItemModel> csvToModel(const std::string& csvFile,
				   bool firstLineIsHeaders)
{
  std::ifstream f(csvFile.c_str());

  if (f) {
    std::shared_ptr<WStandardItemModel> result = std::make_shared<WStandardItemModel>(0, 0);
    result->setItemPrototype(std::make_unique<NumericItem>());
    readFromCsv(f, result, -1, firstLineIsHeaders);
    return result;
  } else
    return nullptr;
}

void readFromCsv(std::istream& f, std::shared_ptr<WAbstractItemModel> model,
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
	  model->setHeaderData(col, cpp17::any{WString{*i}});
	else {
	  int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

	  if (numRows != -1 && dataRow >= numRows)
	    return;

	  if (dataRow >= model->rowCount())
	    model->insertRows(model->rowCount(),
			      dataRow + 1 - model->rowCount());

	  cpp17::any data{WString{*i}};
	  model->setData(dataRow, col, data);
	}
      }
    }

    ++csvRow;
  }
}
