IF(NOT WIN32)
  ADD_EXECUTABLE(gitview.wt
    Git.C
    GitModel.C
    ../wt-homepage/SourceView.C
    GitView.C
  )
  TARGET_LINK_LIBRARIES(gitview.wt ${EXAMPLES_CONNECTOR})

  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
  )

  #
  # Create a deploy script
  #
  SET(APPNAME gitview)
  SET(APPBIN gitview.wt)
  SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/*.css")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

  ADD_DEPENDENCIES(gitview.wt wt ${EXAMPLES_CONNECTOR})
ENDIF(NOT WIN32)
