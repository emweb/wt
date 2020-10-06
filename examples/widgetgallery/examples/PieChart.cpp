#include <Wt/Chart/WPieChart.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WTableView.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
using namespace Wt::Chart;
#endif // WT_TARGET_JAVA

namespace {
    /*
     * A standard item which converts text edits to numbers
     */
    class NumericItem : public WStandardItem {
    public:
        virtual std::unique_ptr<WStandardItem> clone() const {
            return std::make_unique<NumericItem>();
	}

        virtual void setData(const cpp17::any &data, ItemDataRole role = ItemDataRole::User) {
    if (role == ItemDataRole::Edit) {
      cpp17::any dt;

      double d = asNumber(data);

      if (d != d)
        dt = data;
      else
        dt = cpp17::any(d);
      WStandardItem::setData(dt, role);

    } else {
      WStandardItem::setData(data, role);
    }
	}
    };
}

SAMPLE_BEGIN(PieChart)
auto container = std::make_unique<WContainerWidget>();

auto model = std::make_shared<WStandardItemModel>();
model->setItemPrototype(std::make_unique<NumericItem>());

// Configure the header.
model->insertColumns(model->columnCount(), 2);
model->setHeaderData(0, WString("Item"));
model->setHeaderData(1, WString("Sales"));

// Set data in the model.
model->insertRows(model->rowCount(), 6);
int row = 0;

model->setData(  row, 0, WString("Blueberry"));
model->setData(  row, 1, WString("Blueberry"), ItemDataRole::ToolTip);
model->setData(  row, 1, 120);

model->setData(++row, 0, WString("Cherry"));
model->setData(  row, 1, 30);

model->setData(++row, 0, WString("Apple"));
model->setData(  row, 1, 260);

model->setData(++row, 0, WString("Boston Cream"));
model->setData(  row, 1, 160);

model->setData(++row, 0, WString("Other"));
model->setData(  row, 1, 40);

model->setData(++row, 0, WString("Vanilla Cream"));
model->setData(  row, 1, 120);

// Set all items to be editable.
for (row = 0; row < model->rowCount(); ++row)
    for (int col = 0; col < model->columnCount(); ++col)
        model->item(row, col)->setFlags(ItemFlag::Editable);

WTableView* table = container->addNew<WTableView>();

table->setMargin(10, Side::Top | Side::Bottom);
table->setMargin(WLength::Auto, Side::Left | Side::Right);
table->setSortingEnabled(true);
table->setModel(model);
table->setColumnWidth(1, 100);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setWidth(150 + 100 + 14 + 2);

if (WApplication::instance()->environment().ajax())
    table->setEditTriggers(EditTrigger::SingleClicked);
else
    table->setEditTriggers(EditTrigger::None);

/*
 * Create the pie chart.
 */
Chart::WPieChart *chart = container->addNew<Chart::WPieChart>();
#ifndef WT_TARGET_JAVA
chart->setModel(model);       // Set the model.
#else // WT_TARGET_JAVA
chart->setModel(std::shared_ptr<WAbstractItemModel>(model));       // Set the model.
#endif // WT_TARGET_JAVA
chart->setLabelsColumn(0);    // Set the column that holds the labels.
chart->setDataColumn(1);      // Set the column that holds the data.

// Configure location and type of labels.
chart->setDisplayLabels(Chart::LabelOption::Outside |
                        Chart::LabelOption::TextLabel |
                        Chart::LabelOption::TextPercentage);

// Enable a 3D and shadow effect.
chart->setPerspectiveEnabled(true, 0.2);
chart->setShadowEnabled(true);

chart->setExplode(0, 0.3);  // Explode the first item.
chart->resize(800, 300);    // WPaintedWidget must be given an explicit size.
chart->setMargin(10, Side::Top | Side::Bottom); // Add margin vertically.
chart->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally.

SAMPLE_END(return std::move(container))
