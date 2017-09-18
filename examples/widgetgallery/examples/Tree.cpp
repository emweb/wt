#include <Wt/WIconPair.h>
#include <Wt/WText.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>

SAMPLE_BEGIN(Tree)
std::unique_ptr<WTree> tree = cpp14::make_unique<WTree>();

tree->setSelectionMode(SelectionMode::Extended);

auto folderIcon
    = cpp14::make_unique<WIconPair>("icons/yellow-folder-closed.png",
			"icons/yellow-folder-open.png", false);

auto node =
    cpp14::make_unique<WTreeNode>("Furniture",std::move(folderIcon));
tree->setTreeRoot(std::move(node));


tree->treeRoot()->label()->setTextFormat(TextFormat::Plain);
tree->treeRoot()->setLoadPolicy(ContentLoading::NextLevel);
tree->treeRoot()->addChildNode(cpp14::make_unique<WTreeNode>("Table"));
tree->treeRoot()->addChildNode(cpp14::make_unique<WTreeNode>("Cupboard"));

auto subtree = cpp14::make_unique<WTreeNode>("Chair");
auto subtree_ = tree->treeRoot()->addChildNode(std::move(subtree));
tree->treeRoot()->addChildNode(cpp14::make_unique<WTreeNode>("Coach"));
tree->treeRoot()->expand();

subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Doc"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Grumpy"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Happy"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Sneezy"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Dopey"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Bashful"));
subtree_->addChildNode(cpp14::make_unique<WTreeNode>("Sleepy"));

SAMPLE_END(return std::move(tree))
