IF(NOT MYSQLPP_FOUND)

  MESSAGE(STATUS "** hangman example needs mysql++-2.x library... Skipping.")

ELSE(NOT MYSQLPP_FOUND)

  ADD_EXECUTABLE(hangman.wt
    hangman.C
    HangmanGame.C
    HangmanWidget.C
    HighScoresWidget.C
    LoginWidget.C
    Dictionary.C
    HangmanDb.C
  )
  TARGET_LINK_LIBRARIES(hangman.wt ${EXAMPLES_CONNECTOR} ${MYSQLPP_LIBRARIES})

  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
    ${MYSQLPP_INCLUDE_DIRS}
  )

  # Static linkage of mysqlpp, hence we don't want __declspecs
  ADD_DEFINITIONS(${MYSQLPP_DEFINITIONS})

  #
  # Create a deploy script
  #
  SET(APPNAME hangman)
  SET(APPBIN hangman.wt)
  SET(APPRESOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/dict*.txt")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

  ADD_DEPENDENCIES(hangman.wt wt ${EXAMPLES_CONNECTOR})
  
ENDIF(NOT MYSQLPP_FOUND)

