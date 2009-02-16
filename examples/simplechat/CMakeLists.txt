IF(NOT BOOST_WT_MT_FOUND)
  MESSAGE(STATUS "** Serverpush example needs boost with thread support... Skipping.")
ELSE(NOT BOOST_WT_MT_FOUND)

 ADD_DEFINITIONS(-DTHREADED)

 ADD_EXECUTABLE(simplechat.wt
   simpleChat.C SimpleChatWidget.C SimpleChatServer.C
 )

 TARGET_LINK_LIBRARIES(simplechat.wt ${EXAMPLES_CONNECTOR}
   ${BOOST_WT_THREAD_LIBRARY})

 #
 # If you have Wt installed somehwere, you should use the
 # installed Wt header files for your own Wt projects.
 # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
 # instead of the following:
 #
 INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

 #
 # Create a deploy script
 #
 SET(APPNAME simplechat)
 SET(APPBIN simplechat.wt)
 SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/simplechat.css ${CMAKE_CURRENT_SOURCE_DIR}/simplechat.xml")
 CONFIGURE_FILE(
   ${WT_SOURCE_DIR}/deploy.sh
   ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
 )

ENDIF(NOT BOOST_WT_MT_FOUND)

