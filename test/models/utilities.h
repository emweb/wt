#include <memory>

namespace Wt {
  class WAbstractItemModel;
  class WStandardItemModel;
  class WStringListModel;
  class WString;
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

private:
  std::shared_ptr<WStandardItemModel> standardItemModel_;
  std::shared_ptr<WStringListModel> stringListModel_;
};
