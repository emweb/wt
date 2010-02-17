IF(NOT WIN32 AND DEFINED BOOST_FS_LIB_MT)
  ADD_EXECUTABLE(gitview.wt
    Git.C
    GitModel.C
    ../wt-homepage/SourceView.C
    GitView.C
  )
  TARGET_LINK_LIBRARIES(gitview.wt ${EXAMPLES_CONNECTOR} ${BOOST_FS_LIB_MT})

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

ELSE(NOT WIN32 AND DEFINED BOOST_FS_LIB_MT)

  MESSAGE(STATUS "** Git view example needs boost filesystem library... Skipping.")

ENDIF(NOT WIN32 AND DEFINED BOOST_FS_LIB_MT)
