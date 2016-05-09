#include <Wt/Chart/WPieChart>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WTableView>

namespace {
    /*
     * A standard item which converts text edits to numbers
     */
    class NumericItem : public Wt::WStandardItem {
    public:
	virtual NumericItem *clone() const {
	    return new NumericItem();
	}

	virtual void setData(const boost::any &data, int role = Wt::UserRole) {
    if (role == Wt::EditRole) {
      boost::any dt;

      double d = Wt::asNumber(data);

      if (d != d)
        dt = data;
      else
        dt = boost::any(d);
      Wt::WStandardItem::setData(dt, role);

    } else {
      Wt::WStandardItem::setData(data, role);
    }
	}
    };
}

SAMPLE_BEGIN(PieChart)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WStandardItemModel *model = new Wt::WStandardItemModel(container);
model->setItemPrototype(new NumericItem());

// Configure the header.
model->insertColumns(model->columnCount(), 2);
model->setHeaderData(0, Wt::WString("Item"));
model->setHeaderData(1, Wt::WString("Sales"));

// Set data in the model.
model->insertRows(model->rowCount(), 6);
int row = 0;

model->setData(  row, 0, Wt::WString("Blueberry"));
model->setData(  row, 1, Wt::WString("Blueberry"), Wt::ToolTipRole);
model->setData(  row, 1, 120);

model->setData(++row, 0, Wt::WString("Cherry"));
model->setData(  row, 1, 30);

model->setData(++row, 0, Wt::WString("Apple"));
model->setData(  row, 1, 260);

model->setData(++row, 0, Wt::WString("Boston Cream"));
model->setData(  row, 1, 160);

model->setData(++row, 0, Wt::WString("Other"));
model->setData(  row, 1, 40);

model->setData(++row, 0, Wt::WString("Vanilla Cream"));
model->setData(  row, 1, 120);

// Set all items to be editable.
for (row = 0; row < model->rowCount(); ++row)
    for (int col = 0; col < model->columnCount(); ++col)
	model->item(row, col)->setFlags(Wt::ItemIsEditable);

Wt::WTableView* table = new Wt::WTableView(container);

table->setMargin(10, Wt::Top | Wt::Bottom);
table->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);
table->setSortingEnabled(true);
table->setModel(model);
table->setColumnWidth(1, 100);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setWidth(150 + 100 + 14 + 2);

if (Wt::WApplication::instance()->environment().ajax())
    table->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
else
    table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);

/*
 * Create the pie chart.
 */
Wt::Chart::WPieChart *chart = new Wt::Chart::WPieChart(container);
chart->setModel(model);       // Set the model.
chart->setLabelsColumn(0);    // Set the column that holds the labels.
chart->setDataColumn(1);      // Set the column that holds the data.

// Configure location and type of labels.
chart->setDisplayLabels(Wt::Chart::Outside |
                        Wt::Chart::TextLabel |
                        Wt::Chart::TextPercentage);

// Enable a 3D and shadow effect.
chart->setPerspectiveEnabled(true, 0.2);
chart->setShadowEnabled(true);

chart->setExplode(0, 0.3);  // Explode the first item.
chart->resize(800, 300);    // WPaintedWidget must be given an explicit size.
chart->setMargin(10, Wt::Top | Wt::Bottom); // Add margin vertically.
chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally.

SAMPLE_END(return container)
