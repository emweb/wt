#include <Wt/WContainerWidget.h>
#include <Wt/WIconPair.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>

SAMPLE_BEGIN(Tree)
std::unique_ptr<Wt::WContainerWidget> container = std::make_unique<Wt::WContainerWidget>();
Wt::WTree *tree = container->addWidget(std::make_unique<Wt::WTree>());

tree->setSelectionMode(Wt::SelectionMode::Extended);

auto folderIcon
    = std::make_unique<Wt::WIconPair>("icons/yellow-folder-closed.png",
                        "icons/yellow-folder-open.png", false);

auto node =
    std::make_unique<Wt::WTreeNode>("Furniture",std::move(folderIcon));
auto furnitureNode = node.get();
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

auto imageButton = container->addWidget(std::make_unique<Wt::WPushButton>("Use Image Icons"));
imageButton->clicked().connect([=](){
    auto icon
    = std::make_unique<Wt::WIconPair>("icons/yellow-folder-closed.png",
                        "icons/yellow-folder-open.png", false);
    furnitureNode->setLabelIcon(std::move(icon));
});

auto FAButton = container->addWidget(std::make_unique<Wt::WPushButton>("Use Font-Awesome Icons"));
FAButton->clicked().connect([=](){
    auto icon
    = std::make_unique<Wt::WIconPair>("folder",
                        "folder-open", false);
    icon->setIconsType(Wt::WIconPair::IconType::IconName);
    furnitureNode->setLabelIcon(std::move(icon));
});

SAMPLE_END(return std::move(container))
