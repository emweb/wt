#include <Wt/WIconPair.h>
#include <Wt/WText.h>
#include <Wt/WTree.h>
#include <Wt/WTreeTable.h>
#include <Wt/WTreeTableNode.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

namespace {
    WTreeTableNode *addNode(WTreeTableNode *parent, const char *name,
				const char *yuppie, const char *holidays,
				const char *favorite) {
	auto node = std::make_unique<WTreeTableNode>(name);
	auto node_ = node.get();
	parent->addChildNode(std::move(node));
	node_->setColumnWidget(1, std::make_unique<WText>(yuppie));
	node_->setColumnWidget(2, std::make_unique<WText>(holidays));
	node_->setColumnWidget(3, std::make_unique<WText>(favorite));
	return node_;
    }
}

SAMPLE_BEGIN(TreeTable)

auto treeTable = std::make_unique<WTreeTable>();

treeTable->resize(650, 200);
treeTable->tree()->setSelectionMode(SelectionMode::Extended);
treeTable->addColumn("Yuppie Factor", 125);
treeTable->addColumn("# Holidays", 125);
treeTable->addColumn("Favorite Item", 125);

auto root = std::make_unique<WTreeTableNode>("All Personnel");
treeTable->setTreeRoot(std::move(root), "Emweb Organigram");

auto group = std::make_unique<WTreeTableNode>("Upper Management");
WTreeTableNode *group_ = group.get();

treeTable->treeRoot()->addChildNode(std::move(group));
addNode(group_, "Chief Anything Officer", "-2.8", "20", "Scepter");
addNode(group_, "Vice President of Parties", "13.57", "365", "Flag");
addNode(group_, "Vice President of Staplery", "3.42", "27", "Perforator");

group = std::make_unique<WTreeTableNode>("Middle management");
group_ = group.get();

treeTable->treeRoot()->addChildNode(std::move(group));
addNode(group_, "Boss of the house", "9.78", "35", "Happy Animals");
addNode(group_, "Xena caretaker", "8.66", "10", "Yellow bag");

group = std::make_unique<WTreeTableNode>("Actual Workforce");
group_ = group.get();

treeTable->treeRoot()->addChildNode(std::move(group));
addNode(group_, "The Dork", "9.78", "22", "Mojito");
addNode(group_, "The Stud", "8.66", "46", "Toothbrush");
addNode(group_, "The Ugly", "13.0", "25", "Paper bag");

treeTable->treeRoot()->expand();

SAMPLE_END(return std::move(treeTable))
