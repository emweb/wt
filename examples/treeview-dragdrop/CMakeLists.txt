WT_ADD_EXAMPLE(treeviewdragdrop.wt
  CsvUtil.C
  FolderView.C
  TreeViewDragDrop.C
)

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)

ADD_DEPENDENCIES(treeviewdragdrop.wt wt ${EXAMPLES_CONNECTOR})

