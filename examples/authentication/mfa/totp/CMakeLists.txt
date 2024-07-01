WT_ADD_EXAMPLE(totp-login.wt
  mysession.h
  mysession.cpp
  myuser.h
  myuser.cpp
  totp-login.cpp
)

TARGET_LINK_LIBRARIES(totp-login.wt ${EXAMPLES_DBO_LIBS})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
