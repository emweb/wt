IF(NOT MULTI_THREADED_BUILD)
  MESSAGE(STATUS "** Not building codingview example: requires a multi-threaded build.")
ELSE(NOT MULTI_THREADED_BUILD)

 ADD_DEFINITIONS(-DTHREADED)

 WT_ADD_EXAMPLE(codingview.wt
   CoderApplication.C CodeSession.C CoderWidget.C ObserverWidget.C
 )

 TARGET_LINK_LIBRARIES(codingview.wt ${BOOST_THREAD_LIB} ${WT_THREAD_LIB})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF(NOT MULTI_THREADED_BUILD)
