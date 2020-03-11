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
    ptr.h ptr_impl.h ptr.C
    Call.h Call_impl.h Call.C
    DbAction.h DbAction_impl.h DbAction.C
    Exception.h Exception.C
    FixedSqlConnectionPool.h FixedSqlConnectionPool.C
    Json.h Json.C
    Query.h Query_impl.h Query.C
    QueryColumn.h QueryColumn.C
    SqlQueryParse.C
    Session.h Session_impl.h Session.C
    SqlConnection.h SqlConnection.C
    SqlConnectionPool.h SqlConnectionPool.C
    SqlStatement.h SqlStatement.C
    SqlTraits.h SqlTraits_impl.h SqlTraits.C
    StdSqlTraits.h StdSqlTraits.C
    Transaction.h Transaction.C
    WtSqlTraits.h WtSqlTraits.C
    EscapeOStream.h EscapeOStream.C
    StringStream.h StringStream.C
    Logger.h Logger.C
  )

  SET_PROPERTY(TARGET wtdbo PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbo PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbo PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  IF(HAVE_UNWIND)
    TARGET_LINK_LIBRARIES(wtdbo PRIVATE ${UNWIND_LIBRARIES})
  ENDIF(HAVE_UNWIND)

  IF(TARGET Boost::headers)
    TARGET_LINK_LIBRARIES(wtdbo PRIVATE Boost::headers)
  ENDIF()

  IF(MULTI_THREADED_BUILD)
    TARGET_LINK_LIBRARIES(wtdbo
                          PRIVATE
                          ${WT_THREAD_LIB})
  ENDIF(MULTI_THREADED_BUILD)

  TARGET_INCLUDE_DIRECTORIES(wtdbo INTERFACE $<INSTALL_INTERFACE:include>)

  INSTALL(TARGETS wtdbo
      EXPORT wt-target-dbo
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbo
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbo
  PROPERTIES
    EXPORT_NAME Dbo
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbo
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
	SET_TARGET_PROPERTIES(wtdbo PROPERTIES FOLDER "dbo")
  ENDIF(MSVC)

  SUBDIRS(backend)

  INSTALL_FILES(/include/Wt/Dbo "^[^.]+\\.h$")
  INSTALL_FILES(/include/Wt FILES ${WCONFIG_H_PATH})

  IF (ASCIIDOC_FOUND)
    ASCIIDOC_FILE(dbo-tutorial
      ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo.doc
      ${CMAKE_SOURCE_DIR}/doc/tutorial/dbo.html)
  ENDIF (ASCIIDOC_FOUND)

ELSE (ENABLE_LIBWTDBO)
  MESSAGE("** libwtdbo disabled, not building Wt::Dbo.")
ENDIF (ENABLE_LIBWTDBO)
