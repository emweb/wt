IF(NOT WIN32)

IF(BOOST_FS_LIB_MT)
  SET(FS_LIB ${BOOST_FS_LIB_MT})
ELSE(BOOST_FS_LIB_MT)
  IF (BOOST_FS_LIB)
    SET(FS_LIB ${BOOST_FS_LIB})
  ENDIF(BOOST_FS_LIB)
ENDIF(BOOST_FS_LIB_MT)


IF(FS_LIB)
  ADD_EXECUTABLE(gitview.wt
    Git.C
    GitModel.C
    ../wt-homepage/SourceView.C
    GitView.C
  )
  TARGET_LINK_LIBRARIES(gitview.wt ${EXAMPLES_CONNECTOR} ${FS_LIB})

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

ELSE(FS_LIB)

  MESSAGE(STATUS "** Not building gitmodel example: requires boost_filesystem library.")

ENDIF(FS_LIB)

ELSE(NOT WIN32)

  MESSAGE(STATUS "** Not building gitmodel example: uses POSIX calls not available on win32")

ENDIF(NOT WIN32)
