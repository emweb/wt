IF(NOT MULTI_THREADED_BUILD)
  MESSAGE(STATUS "** Not building simplechat example: requires a multi-threaded build.")
ELSE(NOT MULTI_THREADED_BUILD)

 ADD_DEFINITIONS(-DTHREADED)

 WT_ADD_EXAMPLE(simplechat.wt
   simpleChat.C
   PopupChatWidget.C
   SimpleChatWidget.C
   SimpleChatServer.C
)

 TARGET_LINK_LIBRARIES(simplechat.wt ${BOOST_THREAD_LIB} ${WT_THREAD_LIB})

 #
 # If you have Wt installed somehwere, you should use the
 # installed Wt header files for your own Wt projects.
 # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
 # instead of the following:
 #
 INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ENDIF(NOT MULTI_THREADED_BUILD)

