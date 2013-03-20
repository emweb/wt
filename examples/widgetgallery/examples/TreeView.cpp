#include <Wt/WTreeView>

#ifdef WT_EXAMPLE
#include "GitModel.cpp"
#define GIT_REPOSITORY "/path/to/repository/.git"
#endif // WT_EXAMPLE

SAMPLE_BEGIN(TreeView)
Wt::WTreeView *treeView = new Wt::WTreeView();
treeView->resize(600, 400);

Wt::WAbstractItemModel *model = new GitModel(GIT_REPOSITORY, treeView);
treeView->setModel(model);

treeView->setRowHeight(24);
treeView->setHeaderHeight(24);
treeView->setSortingEnabled(false);
treeView->setSelectionMode(Wt::SingleSelection);
treeView->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);

SAMPLE_END(return treeView)
