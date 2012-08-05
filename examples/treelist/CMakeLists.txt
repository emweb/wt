WT_ADD_EXAMPLE(demotreelist.wt DemoTreeList.C IconPair.C TreeNode.C)

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)

ADD_DEPENDENCIES(demotreelist.wt wt ${EXAMPLES_CONNECTOR})

