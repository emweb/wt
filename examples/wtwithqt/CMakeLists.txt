IF (NOT QT_FOUND)

  MESSAGE(STATUS "** Wt/Qt interopability example (wtwithqt) needs Qt4... Skipping.")

ELSE (NOT_QT_FOUND)

  SUBDIRS(lib)

  QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/QtObject.h
                   ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject.C)

  ADD_EXECUTABLE(helloqt.wt
    hello.C
    QtObject.C
    ${CMAKE_CURRENT_BINARY_DIR}/moccedQtObject.C
  )

  TARGET_LINK_LIBRARIES(helloqt.wt
    ${EXAMPLES_CONNECTOR}
    wtwithqt
    QtCore
    ${QT_LIBRARIES}
  )

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/wt/include)
  # instead of the following:
  #
  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR}
  )

  #
  # Create a deploy script
  #
  SET(APPNAME hello)
  SET(APPBIN helloqt.wt)
  SET(APPRESOURCES "")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

ENDIF (NOT QT_FOUND)
