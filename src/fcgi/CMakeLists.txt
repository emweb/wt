IF(CONNECTOR_FCGI)
  MESSAGE("** Enabling FastCGI.")
  IF(NOT FCGI_FOUND)
    MESSAGE("** Error finding Wt required library: ")
    MESSAGE(FATAL_ERROR "** Wt FCGI connector requires libfcgi, libfcgi++, and
    the libfcgi include files (the FastCGI client library)")
  ENDIF(NOT FCGI_FOUND)

  SET(libfcgisources
    FCGIRecord.C
    FCGIStream.C
    Server.C
    SessionInfo.C
  )

  INCLUDE_DIRECTORIES(${FCGI_INCLUDE_DIRS})
  ADD_LIBRARY(wtfcgi ${libfcgisources})
  TARGET_LINK_LIBRARIES(wtfcgi wt ${FCGI_LIB} ${FCGIPP_LIB})

  SET_TARGET_PROPERTIES(
    wtfcgi
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTFCGI_SOVERSION}
    DEBUG_POSTFIX "d"
  )

  INSTALL(TARGETS wtfcgi
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
ELSE(CONNECTOR_FCGI)
  MESSAGE("** Disabling FastCGI.")
ENDIF(CONNECTOR_FCGI)

