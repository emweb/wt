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
class ComboDelegate : public WItemDelegate {
public:
    ComboDelegate(std::shared_ptr<WAbstractItemModel> items)
	: items_(items)
    { }

    virtual void setModelData(const cpp17::any &editState, WAbstractItemModel* model,
                      const WModelIndex &index) const override
    {
      int stringIdx = (int)asNumber(editState);
        model->setData(index, stringIdx, ItemDataRole::User);
        model->setData(index, items_->data(stringIdx, 0), ItemDataRole::Display);
    }

    virtual cpp17::any editState(WWidget *editor, const WModelIndex& index) const override
    {
        WComboBox* combo = dynamic_cast<WComboBox*>
            (dynamic_cast<WContainerWidget*>(editor)->widget(0));
	return combo->currentIndex();
    }

    virtual void setEditState(WWidget *editor, const WModelIndex& index,
                  const cpp17::any& value) const override
    {
        WComboBox* combo = dynamic_cast<WComboBox*>
            (dynamic_cast<WContainerWidget*>(editor)->widget(0));
        combo->setCurrentIndex((int)asNumber(value));
    }

protected:
    virtual std::unique_ptr<WWidget> createEditor(const WModelIndex &index,
                                      WFlags<ViewItemRenderFlag> flags) const override
    {
        auto container = cpp14::make_unique<WContainerWidget>();
        auto combo = container->addWidget(cpp14::make_unique<WComboBox>());
	combo->setModel(items_);
	combo->setCurrentIndex((int)asNumber(index.data(ItemDataRole::User)));

	combo->changed().connect(std::bind(&ComboDelegate::doCloseEditor, this,
					   container.get(), true));
	combo->enterPressed().connect(std::bind(&ComboDelegate::doCloseEditor,
						this, container.get(), true));
	combo->escapePressed().connect(std::bind(&ComboDelegate::doCloseEditor,
						 this, container.get(), false));

        return std::move(container);
    }

private:
    std::shared_ptr<WAbstractItemModel> items_;

    virtual void doCloseEditor(WWidget *editor, bool save) const
    {
        closeEditor().emit(editor, save);
    }
};

SAMPLE_BEGIN(ComboDelegateTable)

auto table = cpp14::make_unique<WTableView>();

// create model
std::vector<WString> options { "apples", "pears", "bananas", "cherries" };

auto model = std::make_shared<WStandardItemModel>();
for (unsigned i=0; i < 2; i++) {
  for (unsigned j=0; j < 2; j++) {
    auto item = cpp14::make_unique<WStandardItem>();
    item->setData(0, ItemDataRole::User);
    item->setData(options[0], ItemDataRole::Display);
    item->setFlags(ItemFlag::Editable);
    model->setItem(i, j, std::move(item));
  }
}

// create table
table->setModel(model);
table->setEditTriggers(EditTrigger::SingleClicked);
auto slModel = std::make_shared<WStringListModel>();
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
