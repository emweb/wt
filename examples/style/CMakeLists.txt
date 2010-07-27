IF(NOT GM_FOUND)

  MESSAGE(STATUS "** Not building style example: requires WRasterImage.")

ELSE(NOT GM_FOUND)

  ADD_EXECUTABLE(styleexample.wt CornerImage.C RoundedWidget.C StyleExample.C)
  TARGET_LINK_LIBRARIES(styleexample.wt ${EXAMPLES_CONNECTOR})

  INCLUDE_DIRECTORIES(
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

ENDIF(NOT GM_FOUND)

