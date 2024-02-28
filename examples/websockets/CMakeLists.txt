if(BOOST_WT_FOUND)
#
# The ADD_EXAMPLE macro (defined in examples/CMakeLists.txt) ensures that
# the example is correctly built for the requested connector. It is equivalent
# to the following two lines:
#   add_executable(hello.wt hello.C)
#   target_link_libraries(hello.wt ${EXAMPLES_CONNECTOR})
# except when the ISAPI (for Microsoft IIS) connector is used, where it will
# build a DLL with the proper symbols exported.
#
WT_ADD_EXAMPLE(websocketstatic.wt static/StaticResource.C static/WebSocketApplication.h static/WebSocketApplication.C)
if(TARGET Boost::headers)
  target_link_libraries(websocketstatic.wt Boost::headers)
endif()
if(HAVE_SSL)
  target_link_libraries(websocketstatic.wt ${OPENSSL_LIBRARIES})
endif()
WT_ADD_EXAMPLE(websocketdynamic.wt mywebsocket.h dynamic/DynamicResource.C dynamic/WebSocketApplication.h dynamic/WebSocketApplication.C)
if(TARGET Boost::headers)
  target_link_libraries(websocketdynamic.wt Boost::headers)
endif()
if(HAVE_SSL)
  target_link_libraries(websocketdynamic.wt ${OPENSSL_LIBRARIES})
endif()

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. include_directories(/usr/local/include)
# instead of the following:
#
include_directories(${WT_SOURCE_DIR}/src)
else()
  MESSAGE(STATUS "** Not building WebSocket examples: requires boost headers.")
endif(BOOST_WT_FOUND)
