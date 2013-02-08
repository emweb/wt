#include <Wt/WIconPair>
#include <Wt/WText>
#include <Wt/WTree>
#include <Wt/WTreeNode>

SAMPLE_BEGIN(Tree)
Wt::WTree *tree = new Wt::WTree();

tree->setSelectionMode(Wt::ExtendedSelection);

Wt::WIconPair *folderIcon 
    = new Wt::WIconPair("icons/yellow-folder-closed.png",
			"icons/yellow-folder-open.png", false);

Wt::WTreeNode *node = new Wt::WTreeNode("Furniture", folderIcon);

tree->setTreeRoot(node);
node->label()->setTextFormat(Wt::PlainText);
node->setLoadPolicy(Wt::WTreeNode::NextLevelLoading);
node->addChildNode(new Wt::WTreeNode("Table"));
node->addChildNode(new Wt::WTreeNode("Cupboard"));

Wt::WTreeNode *three = new Wt::WTreeNode("Chair");
node->addChildNode(three);
node->addChildNode(new Wt::WTreeNode("Coach"));
node->expand();

three->addChildNode(new Wt::WTreeNode("Doc"));
three->addChildNode(new Wt::WTreeNode("Grumpy"));
three->addChildNode(new Wt::WTreeNode("Happy"));
three->addChildNode(new Wt::WTreeNode("Sneezy"));
three->addChildNode(new Wt::WTreeNode("Dopey"));
three->addChildNode(new Wt::WTreeNode("Bashful"));
three->addChildNode(new Wt::WTreeNode("Sleepy"));

SAMPLE_END(return tree)
