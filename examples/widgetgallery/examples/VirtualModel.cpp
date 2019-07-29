#include <Wt/WAbstractTableModel.h>

class VirtualModel : public Wt::WAbstractTableModel
{
public:
  VirtualModel(int rows, int columns)
    : WAbstractTableModel(),
      rows_(rows),
      columns_(columns)
  { }

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
  {
    if (!parent.isValid())
      return rows_;
    else
      return 0;
  }

  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
  {
    if (!parent.isValid())
      return columns_;
    else
      return 0;
  }

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const
  {
    if (role == Wt::ItemDataRole::Display) {
      if (index.column() == 0)
        return Wt::WString("Row {1}").arg(index.row());
      else
        return Wt::WString("Item row {1}, col {2}")
	  .arg(index.row()).arg(index.column());
    } else {
      return Wt::cpp17::any();
    }
  }

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const
  {
    if (orientation == Wt::Orientation::Horizontal) {
      if (role == Wt::ItemDataRole::Display) {
        return Wt::WString("Column {1}").arg(section);
      } else {
        return Wt::cpp17::any();
      }
    } else
      return Wt::cpp17::any();
  }

private:
  int rows_, columns_;
};

SAMPLE_BEGIN(virtualModel)
SAMPLE_END(return nullptr)
