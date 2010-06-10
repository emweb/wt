ADD_LIBRARY(wtdbo
  ptr.C
  Call.C
  DbAction.C
  Exception.C
  FixedSqlConnectionPool.C
  Query.C
  QueryColumn.C
  Session.C
  SqlConnection.C
  SqlConnectionPool.C
  SqlStatement.C
  SqlTraits.C
  StdSqlTraits.C
  Transaction.C
  WtSqlTraits.C
)

TARGET_LINK_LIBRARIES(wtdbo ${BOOST_THREAD_LIB_MT})

INSTALL(TARGETS wtdbo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

SET_TARGET_PROPERTIES(
  wtdbo
PROPERTIES
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  SOVERSION ${WTDBO_SOVERSION}
  DEBUG_POSTFIX "d"
)
IF(MSVC)
  SET_TARGET_PROPERTIES(
    wtdbo
  PROPERTIES
    COMPILE_FLAGS "/wd4251 /wd4275"
  )
ENDIF(MSVC)

SUBDIRS(backend)

INSTALL_FILES(/include/Wt/Dbo "^[^.]+\\.h$")
INSTALL_FILES(/include/Wt/Dbo "^[^b.][^.]+$")

IF (ASCIIDOC_FOUND)
  ASCIIDOC_FILE(dbo-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo/tutorial.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo/tutorial.html)
ENDIF (ASCIIDOC_FOUND)

