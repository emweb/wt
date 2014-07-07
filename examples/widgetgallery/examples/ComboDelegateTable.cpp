#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WStringListModel>
#include <Wt/WTableView>
#include <Wt/WItemDelegate>
#include <Wt/WContainerWidget>
#include <Wt/WComboBox>

/*
 * This delegate demonstrates how to override the editing behaviour of a
 * table cell.
 *
 * It takes a list of possible items on construction and, when edited, saves
 * the selected item from the list to the Wt::DisplayRole in the model for
 * Wt::WItemDelegate to render.
 * It also saves the items index for future editing (rather than each time
 * searching the item in the list). This is done using the general purpose
 * Wt::UserRole in the model.
 */
class ComboDelegate : public Wt::WItemDelegate {
public:
    ComboDelegate(Wt::WAbstractItemModel* items)
	: items_(items)
    { }

    void setModelData(const boost::any &editState, Wt::WAbstractItemModel* model,
		      const Wt::WModelIndex &index) const
    {
      int stringIdx = (int)Wt::asNumber(editState);
	model->setData(index, stringIdx, Wt::UserRole);
	model->setData(index, items_->data(stringIdx, 0), Wt::DisplayRole);
    }

    boost::any editState(Wt::WWidget* editor) const
    {
	Wt::WComboBox* combo = dynamic_cast<Wt::WComboBox*>
	    (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
	return combo->currentIndex();
    }

    void setEditState(Wt::WWidget* editor, const boost::any &value) const
    {
	Wt::WComboBox* combo = dynamic_cast<Wt::WComboBox*>
	    (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
	combo->setCurrentIndex((int)Wt::asNumber(value));
    }

protected:
    virtual Wt::WWidget* createEditor(const Wt::WModelIndex &index,
				      Wt::WFlags<Wt::ViewItemRenderFlag> flags) const
    {
	Wt::WContainerWidget *const container = new Wt::WContainerWidget();
	Wt::WComboBox* combo = new Wt::WComboBox(container);
	combo->setModel(items_);
	combo->setCurrentIndex((int)Wt::asNumber(index.data(Wt::UserRole)));

	combo->changed().connect(boost::bind(&ComboDelegate::doCloseEditor, this,
					     container, true));
	combo->enterPressed().connect(boost::bind(&ComboDelegate::doCloseEditor,
    					      this, container, true));
	combo->escapePressed().connect(boost::bind(&ComboDelegate::doCloseEditor,
						   this, container, false));

	return container;
    }

private:
    Wt::WAbstractItemModel* items_;

    void doCloseEditor(Wt::WWidget *editor, bool save) const
    {
	closeEditor().emit(editor, save);
    }
};

SAMPLE_BEGIN(ComboDelegateTable)

Wt::WTableView *table = new Wt::WTableView();

// create model
std::vector<Wt::WString> options;
options.push_back("apples");
options.push_back("pears");
options.push_back("bananas");
options.push_back("cherries");

Wt::WStandardItemModel *model = new Wt::WStandardItemModel(table);
for (unsigned i=0; i < 2; i++) {
  for (unsigned j=0; j < 2; j++) {
    Wt::WStandardItem *item = new Wt::WStandardItem();
    item->setData(0, Wt::UserRole);
    item->setData(options[0], Wt::DisplayRole);
    item->setFlags(Wt::ItemIsEditable);
    model->setItem(i, j, item);
  }
}

// create table
table->setModel(model);
table->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
Wt::WStringListModel* slModel = new Wt::WStringListModel(table);
slModel->setStringList(options);
ComboDelegate* customdelegate = new ComboDelegate(slModel);
table->setItemDelegate(customdelegate);

table->setSortingEnabled(false);
table->setColumnResizeEnabled(false);
table->setRowHeight(40);
table->setHeaderHeight(0);

const int WIDTH = 120;
for (int i = 0; i < table->model()->columnCount(); ++i)
    table->setColumnWidth(i, WIDTH);
table->setWidth((WIDTH + 7) * table->model()->columnCount() + 2);

SAMPLE_END(return table)
