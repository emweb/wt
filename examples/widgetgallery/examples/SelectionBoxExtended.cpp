#include <Wt/WContainerWidget.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SelectionBoxExtended)

auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WSelectionBox *sb2 = container->addNew<Wt::WSelectionBox>();
sb2->addItem("Bacon");
sb2->addItem("Cheese");
sb2->addItem("Mushrooms");
sb2->addItem("Green peppers");
sb2->addItem("Ham");
sb2->addItem("Pepperoni");
sb2->addItem("Red peppers");
sb2->addItem("Turkey");
sb2->setSelectionMode(Wt::SelectionMode::Extended);
std::set<int> selection;    // By default select the items with index 1 and 4.
selection.insert(1);        // Index 1 corresponds to the 2nd item.
selection.insert(4);        // Index 4 corresponds to the 5th item.
sb2->setSelectedIndexes(selection);
sb2->setMargin(10, Wt::Side::Right);

Wt::WText *out = container->addNew<Wt::WText>();
out->addStyleClass("help-block");

sb2->activated().connect([=] {
    Wt::WString selected;

    std::set<int> newSelection = sb2->selectedIndexes();
    for (std::set<int>::iterator it = newSelection.begin();
	 it != newSelection.end(); ++it) {
	if (!selected.empty())
	    selected += ", ";

	selected += sb2->itemText(*it);
    }

    out->setText(Wt::WString("You choose {1}.").arg(selected));
});

SAMPLE_END(return std::move(container))
