#include <Wt/WTreeView>

SAMPLE_BEGIN(TreeView)
Wt::WTreeView *treeView = new Wt::WTreeView();
treeView->resize(600, 400);

Wt::WAbstractItemModel *model = new GitModel("../../.git", treeView);
treeView->setModel(model);

treeView->setRowHeight(24);
treeView->setHeaderHeight(24);
treeView->setSortingEnabled(false);
treeView->setSelectionMode(Wt::SingleSelection);
SAMPLE_END(return treeView)
