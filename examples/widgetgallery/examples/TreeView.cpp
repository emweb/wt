#include <Wt/WTreeView.h>

#ifdef WT_EXAMPLE
#include "GitModel.cpp"
#define GIT_REPOSITORY "/path/to/repository/.git"
#endif // WT_EXAMPLE

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(TreeView)
auto treeView = std::make_unique<WTreeView>();
treeView->resize(600, 400);

auto model = std::make_shared<GitModel>(GIT_REPOSITORY);
treeView->setModel(model);

treeView->setRowHeight(24);
treeView->setHeaderHeight(24);
treeView->setSortingEnabled(false);
treeView->setSelectionMode(SelectionMode::Single);
treeView->setEditTriggers(EditTrigger::None);

SAMPLE_END(return std::move(treeView))
