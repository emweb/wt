#include <Wt/WAbstractTableModel.h>

class VirtualModel : public WAbstractTableModel
{
public:
  VirtualModel(int rows, int columns)
    : WAbstractTableModel(),
      rows_(rows),
      columns_(columns)
  { }

  virtual int rowCount(const WModelIndex& parent = WModelIndex()) const
  {
    if (!parent.isValid())
      return rows_;
    else
      return 0;
  }

  virtual int columnCount(const WModelIndex& parent = WModelIndex()) const
  {
    if (!parent.isValid())
      return columns_;
    else
      return 0;
  }

  virtual cpp17::any data(const WModelIndex& index, ItemDataRole role = ItemDataRole::Display) const
  {
    switch (role.value()) {
    case ItemDataRole::Display:
      if (index.column() == 0)
        return WString("Row {1}").arg(index.row());
      else
        return WString("Item row {1}, col {2}")
	  .arg(index.row()).arg(index.column());
    default:
      return cpp17::any();
    }
  }

  virtual cpp17::any headerData(int section,
                                Orientation orientation = Orientation::Horizontal,
                                ItemDataRole role = ItemDataRole::Display) const
  {
    if (orientation == Orientation::Horizontal) {
      switch (role.value()) {
      case ItemDataRole::Display:
        return WString("Column {1}").arg(section);
      default:
        return cpp17::any();
      }
    } else
      return cpp17::any();
  }

private:
  int rows_, columns_;
};

SAMPLE_BEGIN(virtualModel)
SAMPLE_END(return 0)
