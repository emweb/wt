ADD_EXECUTABLE(treeviewdragdrop.wt
  CsvUtil.C
  FolderView.C
  TreeViewDragDrop.C
)
TARGET_LINK_LIBRARIES(treeviewdragdrop.wt ${EXAMPLES_CONNECTOR})

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)

#
# Create a deploy script
#
SET(APPNAME treeviewdragdrop)
SET(APPBIN treeviewdragdrop.wt)
SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${CMAKE_CURRENT_SOURCE_DIR}/*.css ${CMAKE_CURRENT_SOURCE_DIR}/data")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(treeviewdragdrop.wt wt ${EXAMPLES_CONNECTOR})

