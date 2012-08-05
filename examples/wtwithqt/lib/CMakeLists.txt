QT4_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/DispatchThread.h
		  ${CMAKE_CURRENT_BINARY_DIR}/moccedDispatchThread.C)

ADD_LIBRARY(wtwithqt STATIC
  WQApplication.C
  DispatchThread.C

  ${CMAKE_CURRENT_BINARY_DIR}/moccedDispatchThread.C
)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/wt/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src/wt
  ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR}
)
