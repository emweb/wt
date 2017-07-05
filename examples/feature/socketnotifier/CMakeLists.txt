IF(NOT MULTI_THREADED_BUILD)
  MESSAGE(STATUS "** Not building socketnotifier feature example: requires a multi-threaded build.")
ELSE(NOT MULTI_THREADED_BUILD)

  IF (MINGW)
    MESSAGE(STATUS "** Not building socketnotifier example for lack of getaddrinfo freeaddrinfo.")
  ELSE (MINGW)

    WT_ADD_EXAMPLE(socketnotifier.wt SocketNotifier.C)

    #
    # If you have Wt installed somehwere, you should use the
    # installed Wt header files for your own Wt projects.
    # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
    # instead of the following:
    #
    INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

    TARGET_LINK_LIBRARIES(socketnotifier.wt ${WT_SOCKET_LIBRARY})
  ENDIF (MINGW)

ENDIF(NOT MULTI_THREADED_BUILD)
