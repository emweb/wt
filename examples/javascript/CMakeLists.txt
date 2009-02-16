
ADD_EXECUTABLE(javascript.wt
  JavascriptExample.C
  Popup.C
)

TARGET_LINK_LIBRARIES(javascript.wt
  ${EXAMPLES_CONNECTOR}
)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

#
# Create a deploy script
#
SET(APPNAME javascript)
SET(APPBIN javascript.wt)
SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.css")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(javascript.wt wt ${EXAMPLES_CONNECTOR})
