IF(NOT MULTI_THREADED_BUILD)
  MESSAGE(STATUS "** Not building broadcast feature example: requires a multi-threaded build.")
ELSE(NOT MULTI_THREADED_BUILD)

  WT_ADD_EXAMPLE(broadcast.wt BroadCast.C)

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
  # instead of the following:
  #
  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

  TARGET_LINK_LIBRARIES(broadcast.wt ${WT_THREAD_LIB})

ENDIF(NOT MULTI_THREADED_BUILD)
