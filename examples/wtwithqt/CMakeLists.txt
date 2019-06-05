IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(BUILD_WTWITHQT4 true)
IF (NOT MULTI_THREADED_BUILD)
  SET(BUILD_WTWITHQT4 false)
ENDIF (NOT MULTI_THREADED_BUILD)
IF (NOT QT_FOUND)
  SET(BUILD_WTWITHQT4 false)
ENDIF (NOT QT_FOUND)

SET(BUILD_WTWITHQT5 true)
IF (NOT MULTI_THREADED_BUILD)
  SET(BUILD_WTWITHQT5 false)
ENDIF (NOT MULTI_THREADED_BUILD)
IF (NOT Qt5Core_FOUND)
  SET(BUILD_WTWITHQT5 false)
ENDIF (NOT Qt5Core_FOUND)

SUBDIRS(lib)

IF (NOT BUILD_WTWITHQT4)

  MESSAGE(STATUS "** Not building wtwithqt example.")
  MESSAGE(STATUS "   wtwithqt example requires a Qt4 installation.")

ELSE (NOT BUILD_WTWITHQT4)

  QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/QtObject.h
                   ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject.C)

  WT_ADD_EXAMPLE(helloqt.wt
    hello.C
    QtObject.C
    ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject.C
  )

  TARGET_COMPILE_DEFINITIONS(
    helloqt.wt
  PRIVATE
    -DWT_NO_SLOT_MACROS
  )

  TARGET_LINK_LIBRARIES(helloqt.wt
    wtwithqt
    ${QT_LIBRARIES}
  )

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/wt/include)
  # instead of the following:
  #
  TARGET_INCLUDE_DIRECTORIES(
    helloqt.wt
  PRIVATE
    ${WT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR}
  )

ENDIF (NOT BUILD_WTWITHQT4)

IF (NOT BUILD_WTWITHQT5)

  MESSAGE(STATUS "** Not building wtwithqt5 example.")
  MESSAGE(STATUS "   wtwithqt5 example requires a Qt5 installation.")

ELSE (NOT BUILD_WTWITHQT5)

  QT5_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/QtObject.h
                   ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject5.C)

  WT_ADD_EXAMPLE(helloqt5.wt
    hello.C
    QtObject.C
    ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject5.C
  )

  TARGET_LINK_LIBRARIES(helloqt5.wt
    wtwithqt5
  )

  TARGET_COMPILE_DEFINITIONS(
    helloqt5.wt
  PRIVATE
    -DWT_NO_SLOT_MACROS
  )

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/wt/include)
  # instead of the following:
  #
  TARGET_INCLUDE_DIRECTORIES(
    helloqt5.wt
  PRIVATE
    ${WT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
  )

ENDIF (NOT BUILD_WTWITHQT5)
