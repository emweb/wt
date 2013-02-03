#include <Wt/WIconPair>
#include <Wt/WText>
#include <Wt/WTree>
#include <Wt/WTreeTable>
#include <Wt/WTreeTableNode>

namespace {
    Wt::WTreeTableNode *addNode(Wt::WTreeTableNode *parent, const char *name,
				const char *yuppie, const char *holidays,
				const char *favorite) {
	Wt::WTreeTableNode *node = new Wt::WTreeTableNode(name, 0, parent);
	node->setColumnWidget(1, new Wt::WText(yuppie));
	node->setColumnWidget(2, new Wt::WText(holidays));
	node->setColumnWidget(3, new Wt::WText(favorite));
	return node;
    }
}

SAMPLE_BEGIN(TreeTable)

Wt::WTreeTable *treeTable = new Wt::WTreeTable();

treeTable->resize(650, 200);
treeTable->tree()->setSelectionMode(Wt::ExtendedSelection);
treeTable->addColumn("Yuppie Factor", 125);
treeTable->addColumn("# Holidays", 125);
treeTable->addColumn("Favorite Item", 125);

Wt::WTreeTableNode *root = new Wt::WTreeTableNode("All Personnel");
treeTable->setTreeRoot(root, "Emweb Organigram");

Wt::WTreeTableNode *group;

group = new Wt::WTreeTableNode("Upper Management", 0, root);
addNode(group, "Chief Anything Officer", "-2.8", "20", "Scepter");
addNode(group, "Vice President of Parties", "13.57", "365", "Flag");
addNode(group, "Vice President of Staplery", "3.42", "27", "Perforator");

group = new Wt::WTreeTableNode("Middle management", 0, root);
addNode(group, "Boss of the house", "9.78", "35", "Happy Animals");
addNode(group, "Xena caretaker", "8.66", "10", "Yellow bag");

group = new Wt::WTreeTableNode("Actual Workforce", 0, root);
addNode(group, "The Dork", "9.78", "22", "Mojito");
addNode(group, "The Stud", "8.66", "46", "Toothbrush");
addNode(group, "The Ugly", "13.0", "25", "Paper bag");

root->expand();

SAMPLE_END(return treeTable)
