IF(NOT WIN32 AND NOT BOOST_FS_LIB)
  MESSAGE(STATUS "** Not building filetreetable example: requires boost_filesystem library.")
ELSE(NOT WIN32 AND NOT BOOST_FS_LIB)

  WT_ADD_EXAMPLE(filetreetable.wt
    FileTreeTable.C
    FileTreeTableNode.C
    FileTreeExample.C
  )

  TARGET_LINK_LIBRARIES(filetreetable.wt
    ${BOOST_FS_LIB}
  )

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

  ADD_DEPENDENCIES(filetreetable.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT WIN32 AND NOT BOOST_FS_LIB)

