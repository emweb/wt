IF (BUILD_WTWITHQT4)

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
TARGET_INCLUDE_DIRECTORIES(
  wtwithqt
PRIVATE
  ${WT_SOURCE_DIR}/src
  ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR}
)

ENDIF (BUILD_WTWITHQT4)

IF (BUILD_WTWITHQT5)

QT5_GENERATE_MOC (${CMAKE_CURRENT_SOURCE_DIR}/DispatchThread.h
                  ${CMAKE_CURRENT_BINARY_DIR}/moccedDispatchThread5.C)

ADD_LIBRARY(wtwithqt5 STATIC
  WQApplication.C
  DispatchThread.C

  ${CMAKE_CURRENT_BINARY_DIR}/moccedDispatchThread5.C
)

SET_PROPERTY(TARGET wtwithqt5 PROPERTY POSITION_INDEPENDENT_CODE ON)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/wt/include)
# instead of the following:
#
TARGET_INCLUDE_DIRECTORIES(
  wtwithqt5
PRIVATE
  ${WT_SOURCE_DIR}/src
  ${Qt5Core_INCLUDE_DIRS}
)

ENDIF (BUILD_WTWITHQT5)
