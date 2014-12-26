IF(ENABLE_LIBWTDBO)

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbo-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbo-version.rc)
      SET(wtdboversionrc ${CMAKE_CURRENT_BINARY_DIR}/wtdbo-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbo
    ${wtdboversionrc}
    ptr.C
    Call.C
    DbAction.C
    Exception.C
    FixedSqlConnectionPool.C
    Json.C
    Query.C
    QueryColumn.C
    SqlQueryParse.C
    Session.C
    SqlConnection.C
    SqlConnectionPool.C
    SqlStatement.C
    SqlTraits.C
    StdSqlTraits.C
    Transaction.C
    WtSqlTraits.C
    EscapeOStream.C
    StringStream.C
  )

  IF(MSVC)
    #win32 links automatically against boost libs; specifying too much causes
    #trouble
    IF(MULTI_THREADED_BUILD)
      TARGET_LINK_LIBRARIES(wtdbo ${CMAKE_THREAD_LIBS_INIT})
    ENDIF(MULTI_THREADED_BUILD)
  ELSE(MSVC)
    IF(MULTI_THREADED_BUILD)
      TARGET_LINK_LIBRARIES(wtdbo ${BOOST_THREAD_LIB} ${BOOST_SYSTEM_LIB} ${CMAKE_THREAD_LIBS_INIT} ${BOOST_DT_LIB})
    ELSE(MULTI_THREADED_BUILD)
      TARGET_LINK_LIBRARIES(wtdbo ${BOOST_DT_LIB})
    ENDIF(MULTI_THREADED_BUILD)
  ENDIF(MSVC)

  INSTALL(TARGETS wtdbo
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  SET_TARGET_PROPERTIES(
    wtdbo
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTDBO_SOVERSION}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbo
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
  ENDIF(MSVC)

  SUBDIRS(backend)

  INSTALL_FILES(/include/Wt/Dbo "^[^.]+\\.h$")
  INSTALL_FILES(/include/Wt/Dbo "^[^b][^.]+$")
  INSTALL_FILES(/include/Wt/Dbo "^[^b.][^.]+$")
  INSTALL_FILES(/include/Wt FILES ${WCONFIG_H_PATH})

  IF (ASCIIDOC_FOUND)
    IF(NOT WIN32)
      ADD_CUSTOM_TARGET(doc)
    ENDIF(NOT WIN32)
    ASCIIDOC_FILE(dbo-tutorial
      ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo.doc
      ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo.html)
  ENDIF (ASCIIDOC_FOUND)

ELSE (ENABLE_LIBWTDBO)
  MESSAGE("** libwtdbo disabled, not building Wt::Dbo.")
ENDIF (ENABLE_LIBWTDBO)
