IF(BOOST_FS_LIB_MT)
  SET(FS_LIB ${BOOST_FS_LIB_MT})
ELSE(BOOST_FS_LIB_MT)
  IF (BOOST_FS_LIB)
    SET(FS_LIB ${BOOST_FS_LIB})
  ENDIF(BOOST_FS_LIB)
ENDIF(BOOST_FS_LIB_MT)

IF(NOT WIN32 AND NOT FS_LIB)
  MESSAGE(STATUS "** Not building filetreetable example: requires boost_filesystem library.")
ELSE(NOT WIN32 AND NOT FS_LIB)

  ADD_EXECUTABLE(filetreetable.wt
    FileTreeTable.C
    FileTreeTableNode.C
    FileTreeExample.C
  )

  TARGET_LINK_LIBRARIES(filetreetable.wt
    ${EXAMPLES_CONNECTOR}
    ${FS_LIB}
  )

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

  #
  # Create a deploy script
  #
  SET(APPNAME filetreetable)
  SET(APPBIN filetreetable.wt)
  SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.css ${CMAKE_CURRENT_SOURCE_DIR}/icons")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

  ADD_DEPENDENCIES(filetreetable.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT WIN32 AND NOT FS_LIB)

