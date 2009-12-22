ADD_LIBRARY(wtdbo
  ptr.C
  DbAction.C
  Exception.C
  Session.C
  SqlConnection.C
  SqlStatement.C
  StdSqlTraits.C
  Transaction.C
  WtSqlTraits.C
)

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

SUBDIRS(backend)

INSTALL_FILES(/include/Wt/Dbo "^[^.]+\\.h$")
INSTALL_FILES(/include/Wt/Dbo "^[^b][^.]+$")

IF (ASCIIDOC_FOUND)
  ASCIIDOC_FILE(dbo-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo/tutorial.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo/tutorial.html)
ENDIF (ASCIIDOC_FOUND)

