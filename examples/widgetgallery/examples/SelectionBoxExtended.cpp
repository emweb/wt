#include <Wt/WContainerWidget>
#include <Wt/WSelectionBox>
#include <Wt/WText>

SAMPLE_BEGIN(SelectionBoxExtended)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WSelectionBox *sb2 = new Wt::WSelectionBox(container);
sb2->addItem("Bacon");
sb2->addItem("Cheese");
sb2->addItem("Mushrooms");
sb2->addItem("Green peppers");
sb2->addItem("Ham");
sb2->addItem("Pepperoni");
sb2->addItem("Red peppers");
sb2->addItem("Turkey");
sb2->setSelectionMode(Wt::ExtendedSelection);
std::set<int> selection;    // By default select the items with index 1 and 4.
selection.insert(1);        // Index 1 corresponds to the 2nd item.
selection.insert(4);        // Index 4 corresponds to the 5th item.
sb2->setSelectedIndexes(selection);
sb2->setMargin(10, Wt::Right);

Wt::WText *out = new Wt::WText(container);
out->addStyleClass("help-block");

sb2->activated().connect(std::bind([=] () {
    Wt::WString selected;

    std::set<int> selection = sb2->selectedIndexes();
    for (std::set<int>::iterator it = selection.begin();
	 it != selection.end(); ++it) {
	if (!selected.empty())
	    selected += ", ";

	selected += sb2->itemText(*it);
    }

    out->setText(Wt::WString::fromUTF8("You choose {1}.").arg(selected));
}));

SAMPLE_END(return container)
