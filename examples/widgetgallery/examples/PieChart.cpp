#include <Wt/Chart/WPieChart>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>

SAMPLE_BEGIN(PieChart)

//Wt::WStandardItemModel *model = new Wt::WStandardItemModel();

////headers
//model->insertColumns(model->columnCount(), 2);
//model->setHeaderData(0, Wt::WString("Item"));
//model->setHeaderData(1, Wt::WString("Sales"));

////data
//model->insertRows(model->rowCount(), 6);
//int row = 0;
//model->setData(row, 0, Wt::WString("Blueberry"));
//model->setData(row, 1, 120);
//// model->setData(row, 1, WString("Blueberry"), ToolTipRole);
//row++;
//model->setData(row, 0, WString("Cherry"));
//model->setData(row, 1, 30);
//row++;
//model->setData(row, 0, WString("Apple"));
//model->setData(row, 1, 260);
//row++;
//model->setData(row, 0, WString("Boston Cream"));
//model->setData(row, 1, 160);
//row++;
//model->setData(row, 0, WString("Other"));
//model->setData(row, 1, 40);
//row++;
//model->setData(row, 0, WString("Vanilla Cream"));
//model->setData(row, 1, 120);
//row++;

////set all items to be editable and selectable
//for (int row = 0; row < model->rowCount(); ++row)
//  for (int col = 0; col < model->columnCount(); ++col)
//    model->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsEditable);

Wt::WContainerWidget *container = new Wt::WContainerWidget();
//Wt::WTableView* table = new Wt::WTableView(container);

//table->setMargin(10, Wt::Top | Wt::Bottom);
//table->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);
//table->setSortingEnabled(true);
//table->setModel(model);
//table->setColumnWidth(1, 100);
//table->setRowHeight(22);

//if (Wt::WApplication::instance()->environment().ajax()) {
//  table->resize(150 + 100 + 14, 20 + 6 * 22);
//  table->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
//} else {
//  table->resize(150 + 100 + 14, WLength::Auto);
//  table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);
//}

///*
// * Create the pie chart.
// */
////WPieChart *chart = new WPieChart(this);
//Wt::Chart::WPieChart *chart = new Wt::Chart::WPieChart(container);
//chart->setModel(model);       // set the model
//chart->setLabelsColumn(0);    // set the column that holds the labels
//chart->setDataColumn(1);      // set the column that holds the data

//// configure location and type of labels
//chart->setDisplayLabels(Wt::Chart::Outside |
//                        Wt::Chart::TextLabel |
//                        Wt::Chart::TextPercentage);

//// enable a 3D and shadow effect
//chart->setPerspectiveEnabled(true, 0.2);
//chart->setShadowEnabled(true);

//// explode the first item
//chart->setExplode(0, 0.3);

//chart->resize(800, 300); // WPaintedWidget must be given an explicit size

//// add margin vertically
//chart->setMargin(10, Wt::Top | Wt::Bottom);
//// center horizontally
//chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

SAMPLE_END(return container)
