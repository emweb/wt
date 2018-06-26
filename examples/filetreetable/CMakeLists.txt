IF(NOT BOOST_FS_LIB)
  MESSAGE(STATUS "** Not building filetreetable example: requires boost_filesystem library.")
ELSE(NOT BOOST_FS_LIB)

  WT_ADD_EXAMPLE(filetreetable.wt
    FileTreeTable.C
    FileTreeTableNode.C
    FileTreeExample.C
  )

  TARGET_LINK_LIBRARIES(filetreetable.wt
    ${BOOST_FS_LIB}
    ${BOOST_SYSTEM_LIB}
  )

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF(NOT BOOST_FS_LIB)

