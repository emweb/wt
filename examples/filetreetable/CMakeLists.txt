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

  WT_ADD_EXAMPLE(filetreetable.wt
    FileTreeTable.C
    FileTreeTableNode.C
    FileTreeExample.C
  )

  TARGET_LINK_LIBRARIES(filetreetable.wt
    ${FS_LIB}
  )

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

  ADD_DEPENDENCIES(filetreetable.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT WIN32 AND NOT FS_LIB)

