IF(NOT BOOST_WT_MT_FOUND)
  MESSAGE(STATUS "** Not building socketnotifier feature example: requires a multi-threaded build.")
ELSE(NOT BOOST_WT_MT_FOUND)

  WT_ADD_EXAMPLE(socketnotifier.wt SocketNotifier.C)

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
  # instead of the following:
  #
  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF(NOT BOOST_WT_MT_FOUND)
