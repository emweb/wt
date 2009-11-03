#include "Wt/WTableView"
#include "Wt/WText"
#include "Wt/WItemDelegate"

namespace Wt {

  WTableView::WTableView(WContainerWidget *parent)
    : WCompositeWidget(parent)
  { 
    setImplementation(table_ = new WTable());
    itemDelegate_ = new WItemDelegate();
    model_ = 0;
  }

  WTableView::~WTableView()
  { }

  WTable* WTableView::table()
  {
    return table_;
  }

  void WTableView::setModel(WAbstractItemModel* model)
  {
    /* Allow the view to be attached to another model by clearing the internal
     * WTable, which was filled by the previous model.
     */
    table_->clear();

    if (!model)
      return;

    model_ = model;

    for (int r = 0; r < model_->rowCount(); r++) {
      table_->insertRow(r);
    }
  
    for (int c = 0; c < model->columnCount(); c++) {
      table_->insertColumn(c);
    }

    for (int c = 0; c < model->columnCount(); c++) {
      boost::any header = model->headerData(c);
      table_->elementAt(0, c)->addWidget(new WText(asString(header)));
    }
  
    table_->setHeaderCount(1);

    for (int r = 0; r < model->rowCount(); r++) {
      for (int c = 0; c < model->columnCount(); c++) {
	WWidget* w = getWidget(r, c);
	if (w)
	  table_->elementAt(r + table_->headerCount(), c)->addWidget(w);
      }
    }

    model_->columnsInserted().connect(SLOT(this, WTableView::columnsInserted));
    model_->columnsRemoved().connect(SLOT(this, WTableView::columnsRemoved));

    model_->rowsInserted().connect(SLOT(this, WTableView::rowsInserted));
    model_->rowsRemoved().connect(SLOT(this, WTableView::rowsRemoved));

    model_->dataChanged().connect(SLOT(this, WTableView::dataChanged));
    model_->headerDataChanged()
      .connect(SLOT(this, WTableView::headerDataChanged));  
  }

  void WTableView::columnsInserted(const WModelIndex& index, int firstColumn, int lastColumn) 
  {
    for (int i = firstColumn; i <= lastColumn; i++) {
      table_->insertColumn(i);
    }
  }

  void WTableView::columnsRemoved(const WModelIndex& index, int firstColumn, int lastColumn) 
  {
    for (int i = lastColumn; i >= firstColumn; i--) {
      table_->deleteColumn(i);
    }
  }

  void WTableView::rowsInserted(const WModelIndex& index, int firstRow, int lastRow)
  {
    for (int i = firstRow; i <= lastRow; i++) {
      table_->insertRow(i + table_->headerCount());
    }
  }
  
  void WTableView::rowsRemoved(const WModelIndex& index, int firstRow, int lastRow) 
  {
    for (int i = lastRow; i >= firstRow; i--) {
      table_->deleteRow(i - table_->headerCount());
    }
  }

  void WTableView::dataChanged(const WModelIndex& topLeft,
			       const WModelIndex& bottomRight)
  {
    for (int i = topLeft.row(); 
	 i <= bottomRight.row(); 
	 i++) {
      for (int j = topLeft.column(); j <= bottomRight.column(); j++) {
	table_->elementAt(i + table_->headerCount(),j)->clear();
	WWidget* w = getWidget(i, j);
	if (w)
	  table_->elementAt(i + table_->headerCount(),j)->addWidget(w);
      }
    }
  }

  void WTableView::headerDataChanged(Orientation orientation, int first, int last) 
  {
    for (int c = first; c <= last; c++) {
      boost::any header = model_->headerData(c);
      table_->elementAt(0, c)->clear();
      
      if (!header.empty())
	table_->elementAt(0, c)->addWidget(new WText(asString(header)));
    }
  }

  WTableView::ColumnInfo::ColumnInfo(const WTableView *view, 
				     WApplication *app,
				     int column)
    : itemDelegate_(0)
  {

  }

  void WTableView::setItemDelegate(WAbstractItemDelegate *delegate)
  {
    itemDelegate_ = delegate;
  }

  void WTableView::setItemDelegateForColumn(int column,
					   WAbstractItemDelegate *delegate)
  {
    columnInfo(column).itemDelegate_ = delegate;
  }

  WAbstractItemDelegate *WTableView::itemDelegateForColumn(int column) const
  {
    return columnInfo(column).itemDelegate_;
  }

  WAbstractItemDelegate *WTableView
  ::itemDelegate(const WModelIndex& index) const
  {
    WAbstractItemDelegate *result = itemDelegateForColumn(index.column());

    return result ? result : itemDelegate_;
  }

  WTableView::ColumnInfo& WTableView::columnInfo(int column) const
  {
    while (column >= (int)columns_.size())
      columns_.push_back(ColumnInfo(this, WApplication::instance(),
				    column));

    return columns_[column];
  }

  WWidget* WTableView::getWidget(const int row, const int column) 
  {
    WAbstractItemDelegate *itemDelegate = itemDelegateForColumn(column);
    if (!itemDelegate)
      itemDelegate = itemDelegate_;
    
    return itemDelegate->update(0, model_->index(row, column), 0);
  }
}
