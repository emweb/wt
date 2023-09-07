#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>

class OrderEdit final : public Wt::WTableView
{
public:
  explicit OrderEdit(std::vector<std::string> things)
    : Wt::WTableView()
  {
      auto model = std::make_shared<Wt::WStandardItemModel>();
      model->insertColumns(0, 2);
      model->setHeaderData(0, Wt::Orientation::Horizontal, "Thing name");
      model->setHeaderData(1, Wt::Orientation::Horizontal, "Thing index");

      int index = 1;
      for (auto& thing : things) {
          std::vector<std::unique_ptr<Wt::WStandardItem>> items;
          auto item = std::make_unique<Wt::WStandardItem>(thing);
          item->setData(thing);
          item->setFlags(item->flags() | Wt::ItemFlag::DragEnabled | Wt::ItemFlag::DropEnabled); // "append" flags, the default should be ItemFlag::Selectable.
          items.push_back(std::move(item));
          item = std::make_unique<Wt::WStandardItem>(std::to_string(index));
          item->setData(index);
          item->setFlags(item->flags() | Wt::ItemFlag::DragEnabled | Wt::ItemFlag::DropEnabled); // "append" flags, the default should be ItemFlag::Selectable.
          items.push_back(std::move(item));
          model->appendRow(std::move(items));
          ++index;
      }

      setSortingEnabled(false);
      setSelectionMode(Wt::SelectionMode::Single);
      setSelectionBehavior(Wt::SelectionBehavior::Rows);

      setDragEnabled(true);
      setEnabledDropLocations(Wt::DropLocation::BetweenRows);

      setModel(model);
  }

  void dropEvent(const Wt::WDropEvent& event, const Wt::WModelIndex& target, Wt::Side side) final
  {
    auto data = Wt::asString(model()->itemData(target)[Wt::ItemDataRole::User]);
    auto sideString = (side == Wt::Side::Bottom ? "bottom" : "other");
    std::cout << "Drop at row " << target.row() << " and colum " << target.column() << " at side " << sideString << " with data " << data << "\n";

    Wt::WTableView::dropEvent(event, target, side);

    for (int i = 0; i < model()->rowCount(); i++) {
      auto idx0 = model()->index(i, 0);
      auto idx1 = model()->index(i, 1);
      auto data0 = Wt::asString(model()->itemData(idx0)[Wt::ItemDataRole::User]);
      auto data1 = Wt::asString(model()->itemData(idx1)[Wt::ItemDataRole::User]);

      std::cout << data0 << ":::" << data1 << "\n";
    }
  }
};

std::unique_ptr<OrderEdit> makeOrderEdit()
{
  return std::make_unique<OrderEdit>(std::vector<std::string>{
    "AAA",
    "BBB",
    "CCC",
    "DDD",
    "EEE",
  });
}
