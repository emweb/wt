#
# The style example needs the additional gd library,
# therefore we check first that it exists before trying to build
# the example.
#

IF(NOT GD_FOUND)
  MESSAGE(STATUS "** Style example needs gd library... Skipping.")
ELSE(NOT GD_FOUND)

  ADD_EXECUTABLE(styleexample.wt CornerImage.C RoundedWidget.C StyleExample.C)
  TARGET_LINK_LIBRARIES(styleexample.wt ${EXAMPLES_CONNECTOR} ${GD_LIBRARIES})

  INCLUDE_DIRECTORIES(
    $(GD_INCLUDE_DIRS)
    ${WT_SOURCE_DIR}/src
  )
  #
  # Create a deploy script
  #
  SET(APPNAME style)
  SET(APPBIN styleexample.wt)
  SET(APPRESOURCES "")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

  ADD_DEPENDENCIES(styleexample.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT GD_FOUND)

