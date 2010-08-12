IF(NOT MYSQLPP_FOUND)

  MESSAGE(STATUS "** Not building hangman example: requires mysql++")
  MESSAGE(STATUS "   Indicate the path of your mysql++-2.x installation using -DMYSQL_PREFIX=...")

ELSE(NOT MYSQLPP_FOUND)

  WT_ADD_EXAMPLE(hangman.wt
    hangman.C
    HangmanGame.C
    HangmanWidget.C
    HighScoresWidget.C
    LoginWidget.C
    Dictionary.C
    HangmanDb.C
  )
  TARGET_LINK_LIBRARIES(hangman.wt ${MYSQLPP_LIBRARIES})

  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
    ${MYSQLPP_INCLUDE_DIRS}
  )

  # Static linkage of mysqlpp, hence we don't want __declspecs
  ADD_DEFINITIONS(${MYSQLPP_DEFINITIONS})

  ADD_DEPENDENCIES(hangman.wt wt ${EXAMPLES_CONNECTOR})
  
ENDIF(NOT MYSQLPP_FOUND)

