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
    TARGET_LINK_LIBRARIES(wtfcgi wt ${FCGI_LIB} ${FCGIPP_LIB})

    SET_TARGET_PROPERTIES(
      wtfcgi
    PROPERTIES
      VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
      SOVERSION ${WTFCGI_SOVERSION}
      DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )

    INSTALL(TARGETS wtfcgi
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  ENDIF(NOT FCGI_FOUND)
ELSE(CONNECTOR_FCGI)
  MESSAGE("** Disabling FastCGI connector.")
ENDIF(CONNECTOR_FCGI)

