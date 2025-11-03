#include <Wt/cpp17/any.hpp>

#include <Wt/WString.h>

#include <boost/test/unit_test.hpp>

#include <memory>

namespace Wt {
  class WAbstractItemModel;
  class WStandardItemModel;
  class WStringListModel;
}

using namespace Wt;

const int MODEL_DIMENSION = 5;

class SourceModelWrapper
{
public:
  SourceModelWrapper() = default;

  void createStandardModel();
  void createListModel();

  int getModelRows();
  int getModelColumns();

  std::shared_ptr<WAbstractItemModel> getModel();
  bool isListModel();

  WString get(int i, int j);
  WString createValue(int i, int j);

  template<class Model>
  void testUnchanged(int i, int j, Model* model)
  {
    WString value = createValue(i, j);
    BOOST_TEST(get(i, j) == value);
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))) == value);
  }

  template<class Model>
  void testUnchangedRange(int toRow, int toCol, Model* model)
  {
    for (int i = 0; i < toRow; ++i) {
      for (int j = 0; j < toCol; ++j) {
        testUnchanged(i, j, model);
      }
    }
  }

private:
  std::shared_ptr<WStandardItemModel> standardItemModel_;
  std::shared_ptr<WStringListModel> stringListModel_;
};
