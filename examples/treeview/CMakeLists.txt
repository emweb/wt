ADD_EXECUTABLE(treeviewexample.wt
  TreeViewExample.C
  TreeViewApplication.C
)
TARGET_LINK_LIBRARIES(treeviewexample.wt ${EXAMPLES_CONNECTOR})

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)

#
# Create a deploy script
#
SET(APPNAME treeview)
SET(APPBIN treeviewexample.wt)
SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/*.C ${CMAKE_CURRENT_SOURCE_DIR}/*.h")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(treeviewexample.wt wt ${EXAMPLES_CONNECTOR})

