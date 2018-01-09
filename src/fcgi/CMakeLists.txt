IF(CONNECTOR_FCGI)
  IF(NOT FCGI_FOUND)

    MESSAGE("** Disabling FCGI connector: requires libfcgi ")
    MESSAGE("   Indicate the location of libfcgi using -DFCGI_PREFIX=..., or omit this connector using -DCONNECTOR_FCGI=OFF")

  ELSE(NOT FCGI_FOUND)
    MESSAGE("** Enabling FastCGI connector.")

    SET(libfcgisources
      FCGIRecord.C
      FCGIStream.C
      Server.C
      SessionInfo.C
      WServer.C
    )

    INCLUDE_DIRECTORIES(${FCGI_INCLUDE_DIRS})
    ADD_LIBRARY(wtfcgi ${libfcgisources})
    TARGET_LINK_LIBRARIES(wtfcgi PUBLIC wt PRIVATE ${FCGI_LIB} ${FCGIPP_LIB})

    IF(WT_WITH_SSL)
      TARGET_LINK_LIBRARIES(wtfcgi PRIVATE ${SSL_LIBRARIES})
    ENDIF(WT_WITH_SSL)

    IF(MULTI_THREADED_BUILD)
      TARGET_LINK_LIBRARIES(wtfcgi PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    ENDIF(MULTI_THREADED_BUILD)

    SET_TARGET_PROPERTIES(
      wtfcgi
    PROPERTIES
      EXPORT_NAME FCGI
      VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
      SOVERSION ${WTFCGI_SOVERSION}
      DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )

    INSTALL(TARGETS wtfcgi
      EXPORT wt-target-fcgi
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

    INSTALL(EXPORT wt-target-fcgi
            DESTINATION ${CMAKE_INSTALL_DIR}/wt
            NAMESPACE Wt::)

  ENDIF(NOT FCGI_FOUND)
ELSE(CONNECTOR_FCGI)
  MESSAGE("** Disabling FastCGI connector.")
ENDIF(CONNECTOR_FCGI)

