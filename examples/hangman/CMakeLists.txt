WT_ADD_EXAMPLE(hangman.wt
  hangman.C
  User.C
  Session.C
  HangmanGame.C
  HangmanWidget.C
  WordWidget.C
  LettersWidget.C
  ImagesWidget.C
  HighScoresWidget.C
  LoginWidget.C
  Dictionary.C
)
TARGET_LINK_LIBRARIES(hangman.wt wtdbo wtdbosqlite3)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
