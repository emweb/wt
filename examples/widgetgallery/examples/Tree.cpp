#include <Wt/WIconPair.h>
#include <Wt/WText.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>

SAMPLE_BEGIN(Tree)
std::unique_ptr<Wt::WTree> tree = std::make_unique<Wt::WTree>();

tree->setSelectionMode(Wt::SelectionMode::Extended);

auto folderIcon
    = std::make_unique<Wt::WIconPair>("icons/yellow-folder-closed.png",
			"icons/yellow-folder-open.png", false);

auto node =
    std::make_unique<Wt::WTreeNode>("Furniture",std::move(folderIcon));
tree->setTreeRoot(std::move(node));


tree->treeRoot()->label()->setTextFormat(Wt::TextFormat::Plain);
tree->treeRoot()->setLoadPolicy(Wt::ContentLoading::NextLevel);
tree->treeRoot()->addChildNode(std::make_unique<Wt::WTreeNode>("Table"));
tree->treeRoot()->addChildNode(std::make_unique<Wt::WTreeNode>("Cupboard"));

auto subtree = std::make_unique<Wt::WTreeNode>("Chair");
auto subtree_ = tree->treeRoot()->addChildNode(std::move(subtree));
tree->treeRoot()->addChildNode(std::make_unique<Wt::WTreeNode>("Coach"));
tree->treeRoot()->expand();

subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Doc"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Grumpy"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Happy"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Sneezy"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Dopey"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Bashful"));
subtree_->addChildNode(std::make_unique<Wt::WTreeNode>("Sleepy"));

SAMPLE_END(return std::move(tree))
