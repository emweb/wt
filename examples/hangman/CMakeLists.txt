INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
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
  Dictionary.C
)
TARGET_LINK_LIBRARIES(hangman.wt ${EXAMPLES_DBO_LIBS})

# Test whether crypt(3) is provided by libc. If it's not, check if
# libcrypt exists and if it provides crypt(3).
CHECK_FUNCTION_EXISTS(crypt CRYPT_EXISTS)
IF(NOT CRYPT_EXISTS)
  CHECK_LIBRARY_EXISTS(crypt crypt "" CRYPT_LIB_EXISTS)
  IF(CRYPT_LIB_EXISTS)
    SET(CMAKE_REQUIRED_LIBRARIES "crypt")
    CHECK_FUNCTION_EXISTS(crypt CRYPT_EXISTS)
  ENDIF(CRYPT_LIB_EXISTS)
ENDIF(NOT CRYPT_EXISTS)

IF(CRYPT_LIB_EXISTS)
  TARGET_LINK_LIBRARIES(hangman.wt crypt)
ENDIF(CRYPT_LIB_EXISTS)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
