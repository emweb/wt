OPTION( USE_SYSTEM_SQLITE3 "Use system-wide Sqlite3 instead of Wt's version" OFF)

IF(USE_SYSTEM_SQLITE3)
  FIND_PACKAGE( Sqlite3 REQUIRED)
ELSE(USE_SYSTEM_SQLITE3)
  SET(Sqlite3_SRCS amalgamation/sqlite3.c)
ENDIF(USE_SYSTEM_SQLITE3)

ADD_LIBRARY(wtdbosqlite3
  Sqlite3.C
  ${Sqlite3_SRCS}
)

MESSAGE("** Wt::Dbo: building SQLite3 backend.")

TARGET_LINK_LIBRARIES(wtdbosqlite3 wtdbo ${SQLITE3_LIBRARIES} ${BOOST_WT_DT_LIB})

IF(NOT WIN32)
  # Note, on FreeBsd, the dl library should be removed
  TARGET_LINK_LIBRARIES(wtdbosqlite3 dl)
ENDIF(NOT WIN32)

INSTALL(TARGETS wtdbosqlite3
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

SET_TARGET_PROPERTIES(
  wtdbosqlite3
PROPERTIES
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  SOVERSION ${WTDBOSQLITE3_SOVERSION}
  DEBUG_POSTFIX "d"
)

INSTALL_FILES(/include/Wt/Dbo/backend "^Sqlite3$")
INSTALL_FILES(/include/Wt/Dbo/backend "^.*Sqlite3.*h$")

IF(POSTGRES_FOUND)
  MESSAGE("** Wt::Dbo: building Postgres backend.")
  ADD_LIBRARY(wtdbopostgres
    Postgres.C
    )
  TARGET_LINK_LIBRARIES(wtdbopostgres wtdbo ${POSTGRES_LIBRARIES} ${BOOST_WT_DT_LIB})

  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE})

  INSTALL(TARGETS wtdbopostgres
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  SET_TARGET_PROPERTIES(
    wtdbopostgres
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTDBOPOSTGRES_SOVERSION}
    DEBUG_POSTFIX "d"
  )

  INSTALL_FILES(/include/Wt/Dbo/backend "^Postgres$")
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*Postgres.*h$")
ELSE(POSTGRES_FOUND)
  MESSAGE("** Wt::Dbo: not building Postgres backend.")
  MESSAGE("    Indicate the location of your postgres installation using -DPOSTGRES_PREFIX=...")
ENDIF(POSTGRES_FOUND)

