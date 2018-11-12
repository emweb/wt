#include <Wt/WIconPair.h>
#include <Wt/WText.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>

SAMPLE_BEGIN(Tree)
std::unique_ptr<Wt::WTree> tree = Wt::cpp14::make_unique<Wt::WTree>();

tree->setSelectionMode(Wt::SelectionMode::Extended);

auto folderIcon
    = Wt::cpp14::make_unique<Wt::WIconPair>("icons/yellow-folder-closed.png",
			"icons/yellow-folder-open.png", false);

auto node =
    Wt::cpp14::make_unique<Wt::WTreeNode>("Furniture",std::move(folderIcon));
tree->setTreeRoot(std::move(node));


tree->treeRoot()->label()->setTextFormat(Wt::TextFormat::Plain);
tree->treeRoot()->setLoadPolicy(Wt::ContentLoading::NextLevel);
tree->treeRoot()->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Table"));
tree->treeRoot()->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Cupboard"));

auto subtree = Wt::cpp14::make_unique<Wt::WTreeNode>("Chair");
auto subtree_ = tree->treeRoot()->addChildNode(std::move(subtree));
tree->treeRoot()->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Coach"));
tree->treeRoot()->expand();

subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Doc"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Grumpy"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Happy"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Sneezy"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Dopey"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Bashful"));
subtree_->addChildNode(Wt::cpp14::make_unique<Wt::WTreeNode>("Sleepy"));

SAMPLE_END(return std::move(tree))
