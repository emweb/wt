IF(CONNECTOR_HTTP)
  MESSAGE("** Enabling built-in httpd.")

  # Wt uses strstrcase for case insensitive ASCII string
  # comparisons on Linux for performance reasons. This function is however
  # not supported on many platforms (windows, solaris)
  INCLUDE(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(strcasestr HAVE_STRCASESTR)
  IF (HAVE_STRCASESTR)
    ADD_DEFINITIONS(-DHAVE_STRCASESTR)
  ENDIF (HAVE_STRCASESTR)
  CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
  IF (HAVE_STRCASECMP)
    ADD_DEFINITIONS(-DHAVE_STRCASECMP)
  ENDIF (HAVE_STRCASECMP)
  CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
  IF (HAVE_STRNCASECMP)
    ADD_DEFINITIONS(-DHAVE_STRNCASECMP)
  ENDIF (HAVE_STRNCASECMP)

  SET(libhttpsources
    Android.h Android.C
    Configuration.h Configuration.C
    Connection.h Connection.C
    ConnectionManager.h ConnectionManager.C
    HTTPRequest.h HTTPRequest.C
    MimeTypes.h MimeTypes.C
    ProxyReply.h ProxyReply.C
    Reply.h Reply.C
    Request.h Request.C
    RequestHandler.h RequestHandler.C
    RequestParser.h RequestParser.C
    Server.h Server.C
    SessionProcess.h SessionProcess.C
    SessionProcessManager.h SessionProcessManager.C
    SslConnection.h SslConnection.C
    StaticReply.h StaticReply.C
    StockReply.h StockReply.C
    TcpConnection.h TcpConnection.C
    WServer.C
    WtReply.h WtReply.C
  )

 OPTION(HTTP_WITH_ZLIB "Support for zlib (http compression)" ${ZLIB_FOUND})

 IF(WIN32)
   IF(SHARED_LIBS)
     CONFIGURE_FILE(wthttp-version.rc.in
                    ${CMAKE_CURRENT_BINARY_DIR}/wthttp-version.rc)
     SET(libhttpsources ${libhttpsources}
         ${CMAKE_CURRENT_BINARY_DIR}/wthttp-version.rc)
   ENDIF(SHARED_LIBS)

# boost asio has a serious bug causing subtle data corruption
# https://svn.boost.org/trac/boost/ticket/8935
# https://svn.boost.org/trac/boost/ticket/8933
  IF (Boost_VERSION EQUAL 105400 OR Boost_VERSION STREQUAL "1.54.0")
    MESSAGE(FATAL_ERROR "** boost 1.54 is blacklisted for httpd on Windows (boost ticket #8935)")
  ENDIF ()
 ENDIF(WIN32)

 IF (HAVE_SSL)
    SET(MY_SSL_LIBS ${OPENSSL_LIBRARIES})
    ADD_DEFINITIONS(-DHTTP_WITH_SSL)
 ELSE (HAVE_SSL)
    SET(MY_SSL_LIBS "")
 ENDIF (HAVE_SSL)

 IF(NOT BOOST_WTHTTP_FOUND)

    MESSAGE(FATAL "** Could not find all boost libraries required to build the httpd connector (thread, filesystem, programoptions, datetime)")

  ENDIF(NOT BOOST_WTHTTP_FOUND)
  IF(HTTP_WITH_ZLIB)
    ADD_DEFINITIONS(-DWTHTTP_WITH_ZLIB ${ZLIB_DEFINITIONS})
    SET(MY_ZLIB_LIBS ${ZLIB_LIBRARIES})
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
  ELSE(HTTP_WITH_ZLIB)
    SET(MY_ZLIB_LIBS "")
  ENDIF(HTTP_WITH_ZLIB)

  INCLUDE_DIRECTORIES(
    ${BOOST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../web
    ${CMAKE_CURRENT_SOURCE_DIR}/../wt
    ${CMAKE_CURRENT_BINARY_DIR}/
  )

  ADD_DEFINITIONS(${ZLIB_DEFINITIONS})
  ADD_LIBRARY(wthttp ${libhttpsources})

  SET_PROPERTY(TARGET wthttp PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wthttp PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wthttp
    PUBLIC
      wt
    PRIVATE
      ${MY_ZLIB_LIBS}
      ${MY_SSL_LIBS}
      ${BOOST_WTHTTP_LIBRARIES}
      ${WT_SOCKET_LIBRARY}
      ${WIN32_STRSTRI_LIB}
  )

  IF(BOOST_WT_MT_FOUND)
    TARGET_LINK_LIBRARIES(wthttp PRIVATE ${WT_THREAD_LIB})
  ENDIF(BOOST_WT_MT_FOUND)

  INSTALL(TARGETS wthttp
    EXPORT wt-target-http
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-http
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wthttp
  PROPERTIES
    EXPORT_NAME HTTP
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
        wthttp
      PROPERTIES
        COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267 /wd4267"
    )
  ENDIF(MSVC)

  ADD_DEPENDENCIES(wthttp wt ${EXAMPLES_CONNECTOR})

ELSE(CONNECTOR_HTTP)
  MESSAGE("** Disabling built-in httpd.")
ENDIF(CONNECTOR_HTTP)

