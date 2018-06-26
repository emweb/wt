IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

IF(NOT MULTI_THREADED_BUILD)

  MESSAGE(STATUS "** Not building onethread example: requires a (ironically) a multi-threaded build.")

ELSE(NOT MULTI_THREADED_BUILD)

  WT_ADD_EXAMPLE(hello1thread.wt
    hello.C
    SingleThreadedApplication.C
  )

  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
  )

  TARGET_LINK_LIBRARIES(hello1thread.wt ${WT_THREAD_LIB})

ENDIF(NOT MULTI_THREADED_BUILD)
