#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTableView.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WComboBox.h>
#include <Wt/WAny.h>

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
    ComboDelegate(std::shared_ptr<Wt::WAbstractItemModel> items)
	: items_(items)
    { }

    virtual void setModelData(const Wt::cpp17::any &editState, Wt::WAbstractItemModel* model,
                      const Wt::WModelIndex &index) const override
    {
      int stringIdx = (int)Wt::asNumber(editState);
        model->setData(index, stringIdx, Wt::ItemDataRole::User);
        model->setData(index, items_->data(stringIdx, 0), Wt::ItemDataRole::Display);
    }

    virtual Wt::cpp17::any editState(Wt::WWidget *editor, const Wt::WModelIndex& index) const override
    {
        Wt::WComboBox* combo = dynamic_cast<Wt::WComboBox*>
            (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
	return combo->currentIndex();
    }

    virtual void setEditState(Wt::WWidget *editor, const Wt::WModelIndex& index,
                  const Wt::cpp17::any& value) const override
    {
        Wt::WComboBox* combo = dynamic_cast<Wt::WComboBox*>
            (dynamic_cast<Wt::WContainerWidget*>(editor)->widget(0));
        combo->setCurrentIndex((int)Wt::asNumber(value));
    }

protected:
    virtual std::unique_ptr<Wt::WWidget> createEditor(const Wt::WModelIndex &index,
                                      Wt::WFlags<Wt::ViewItemRenderFlag> flags) const override
    {
        auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
        auto combo = container->addNew<Wt::WComboBox>();
	combo->setModel(items_);
	combo->setCurrentIndex((int)Wt::asNumber(index.data(Wt::ItemDataRole::User)));

	combo->changed().connect(std::bind(&ComboDelegate::doCloseEditor, this,
					   container.get(), true));
	combo->enterPressed().connect(std::bind(&ComboDelegate::doCloseEditor,
						this, container.get(), true));
	combo->escapePressed().connect(std::bind(&ComboDelegate::doCloseEditor,
						 this, container.get(), false));

        return std::move(container);
    }

private:
    std::shared_ptr<Wt::WAbstractItemModel> items_;

    virtual void doCloseEditor(Wt::WWidget *editor, bool save) const
    {
        closeEditor().emit(editor, save);
    }
};

SAMPLE_BEGIN(ComboDelegateTable)

auto table = Wt::cpp14::make_unique<Wt::WTableView>();

// create model
#ifndef WT_TARGET_JAVA
std::vector<WString> options { "apples", "pears", "bananas", "cherries" };
#else // WT_TARGET_JAVA
std::vector<Wt::WString> options;
options.push_back("apples");
options.push_back("pears");
options.push_back("bananas");
options.push_back("cherries");
#endif // WT_TARGET_JAVA

auto model = std::make_shared<Wt::WStandardItemModel>();
for (unsigned i=0; i < 2; i++) {
  for (unsigned j=0; j < 2; j++) {
    auto item = Wt::cpp14::make_unique<Wt::WStandardItem>();
    item->setData(0, Wt::ItemDataRole::User);
    item->setData(options[0], Wt::ItemDataRole::Display);
    item->setFlags(Wt::ItemFlag::Editable);
    model->setItem(i, j, std::move(item));
  }
}

// create table
table->setModel(model);
table->setEditTriggers(Wt::EditTrigger::SingleClicked);
auto slModel = std::make_shared<Wt::WStringListModel>();
slModel->setStringList(options);
std::shared_ptr<ComboDelegate> customdelegate =
    std::make_shared<ComboDelegate>(slModel);
table->setItemDelegate(customdelegate);

table->setSortingEnabled(false);
table->setColumnResizeEnabled(false);
table->setRowHeight(40);
table->setHeaderHeight(0);

const int WIDTH = 120;
for (int i = 0; i < table->model()->columnCount(); ++i)
    table->setColumnWidth(i, WIDTH);
table->setWidth((WIDTH + 7) * table->model()->columnCount() + 2);

SAMPLE_END(return std::move(table))
