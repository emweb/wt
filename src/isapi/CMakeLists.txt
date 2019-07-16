IF(CONNECTOR_ISAPI)
  IF(NOT MULTI_THREADED)
    MESSAGE("** Disabling ISAPI because MULTI_THREADED is disabled.")
  ELSE(NOT MULTI_THREADED)
    MESSAGE("** Enabling ISAPI.")

    ADD_DEFINITIONS(-D_WIN32_WINNT=0x0600 -DWTISAPI_STATIC)

    SET(libsapisources
      Server.h Server.C
      WServer.C
      Isapi.C
      IsapiRequest.h IsapiRequest.C
      IsapiStream.h IsapiStream.C
    )

    INCLUDE_DIRECTORIES(${FCGI_INCLUDE_DIRS})

    ADD_LIBRARY(wtisapi STATIC ${libsapisources})
    TARGET_LINK_LIBRARIES(wtisapi PUBLIC wt)

    IF(TARGET Boost::headers)
      TARGET_LINK_LIBRARIES(wtisapi PRIVATE Boost::headers)
    ENDIF()

    SET_TARGET_PROPERTIES(
      wtisapi
    PROPERTIES
      EXPORT_NAME Isapi
      VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
      DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )
    IF(MSVC)
      SET_TARGET_PROPERTIES(
          wtisapi
        PROPERTIES
          COMPILE_FLAGS "${BUILD_PARALLEL} /wd4267 /wd4251 /wd4275"
          COMPILE_PDB_NAME "wtisapi"
          COMPILE_PDB_NAME_DEBUG "wtisapi${DEBUG_LIB_POSTFIX}"
      )
    ENDIF(MSVC)

    INSTALL(TARGETS wtisapi
      EXPORT wt-target-isapi
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

    INSTALL(EXPORT wt-target-isapi
            DESTINATION ${CMAKE_INSTALL_DIR}/wt
            NAMESPACE Wt::)

  ENDIF(NOT MULTI_THREADED)

ELSE(CONNECTOR_ISAPI)
  MESSAGE("** Disabling ISAPI.")
ENDIF(CONNECTOR_ISAPI)

