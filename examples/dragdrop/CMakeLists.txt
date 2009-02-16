ADD_EXECUTABLE(dragdrop.wt 
  Character.C
  DragExample.C
  DragApplication.C
)
TARGET_LINK_LIBRARIES(dragdrop.wt ${EXAMPLES_CONNECTOR})

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

#
# Create a deploy script
#
SET(APPNAME dragdrop)
SET(APPBIN dragdrop.wt)
SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dragdrop.css ${CMAKE_CURRENT_SOURCE_DIR}/icons")

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(dragdrop.wt wt ${EXAMPLES_CONNECTOR})
