WT_ADD_EXAMPLE(treeviewexample.wt
  TreeViewExample.C
  TreeViewApplication.C
)

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)

ADD_DEPENDENCIES(treeviewexample.wt wt ${EXAMPLES_CONNECTOR})

