IF(CONNECTOR_ISAPI)
  IF(NOT MULTI_THREADED)
    MESSAGE("** Disabling ISAPI because MULTI_THREADED is disabled.")
  ELSE(NOT MULTI_THREADED)
    IF(BUILD_SHARED_LIBS)
      MESSAGE("** Disabling ISAPI: ISAPI is not supported in SHARED_LIBS mode.")
    ELSE(BUILD_SHARED_LIBS)
      MESSAGE("** Enabling ISAPI.")

      ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501 -DWTISAPI_STATIC)

      SET(libsapisources
        Server.C
        Isapi.C
        IsapiRequest.C
        IsapiStream.C
      )

      INCLUDE_DIRECTORIES(${FCGI_INCLUDE_DIRS})
      ADD_LIBRARY(wtisapi ${libsapisources})
      TARGET_LINK_LIBRARIES(wtisapi wt)

      SET_TARGET_PROPERTIES(
        wtisapi
      PROPERTIES
        VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
        SOVERSION ${WTISAPI_SOVERSION}
        DEBUG_POSTFIX "d"
      )
      IF(MSVC)
        SET_TARGET_PROPERTIES(
            wtisapi
          PROPERTIES
            COMPILE_FLAGS "${BUILD_PARALLEL} /wd4267 /wd4251 /wd4275"
        )
      ENDIF(MSVC)

      INSTALL(TARGETS wtisapi
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

    ENDIF(BUILD_SHARED_LIBS)
  ENDIF(NOT MULTI_THREADED)

ELSE(CONNECTOR_ISAPI)
  MESSAGE("** Disabling ISAPI.")
ENDIF(CONNECTOR_ISAPI)

