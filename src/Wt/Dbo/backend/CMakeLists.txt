IF(ENABLE_SQLITE)

  OPTION(USE_SYSTEM_SQLITE3 "Use system-wide Sqlite3 instead of Wt's version" OFF)
  OPTION(USE_SQLITE3_BDB "Use BerkeleyDB storage with Sqlite3 API" OFF)

  IF(USE_SYSTEM_SQLITE3)

    FIND_PACKAGE( Sqlite3 REQUIRED)

    IF (USE_SQLITE3_BDB)
      ADD_DEFINITIONS(-DSQLITE3_BDB)
    ENDIF (USE_SQLITE3_BDB)

  ELSE(USE_SYSTEM_SQLITE3)

    SET(Sqlite3_SRCS amalgamation/sqlite3.c)

  ENDIF(USE_SYSTEM_SQLITE3)

  FIND_PACKAGE(Threads)

  SET (WT_THREAD_LIB "")
  IF (NOT CMAKE_HAVE_THREADS_LIBRARY)
    ADD_DEFINITIONS(-DSQLITE3_THREAD_SAFE=0)
  ELSE (NOT CMAKE_HAVE_THREADS_LIBRARY)
    SET(WT_THREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(NOT CMAKE_HAVE_THREADS_LIBRARY)

  SET(HAVE_SQLITE ON CACHE INTERNAL "building sqlite3 backend" FORCE)

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbosqlite3-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbosqlite3-version.rc)
      SET(Sqlite3_SRCS ${Sqlite3_SRCS} 
          ${CMAKE_CURRENT_BINARY_DIR}/wtdbosqlite3-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbosqlite3
    Sqlite3.C
    ${Sqlite3_SRCS}
    )

  MESSAGE("** Wt::Dbo: building SQLite3 backend.")

  TARGET_LINK_LIBRARIES(wtdbosqlite3 wtdbo
      ${SQLITE3_LIBRARIES}
      ${BOOST_DT_LIB}
      ${WT_THREAD_LIB}
      ${WT_MATH_LIBRARY}
      )

  IF(NOT WIN32)
    FIND_LIBRARY(DL_LIB
      NAMES
      dl
      )
    IF(DL_LIB)
      TARGET_LINK_LIBRARIES(wtdbosqlite3 ${DL_LIB})
    ENDIF(DL_LIB)
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
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbosqlite3
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )

  ENDIF(MSVC)

  INSTALL_FILES(/include/Wt/Dbo/backend "^Sqlite3$")
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*Sqlite3.*h$")

ELSE(ENABLE_SQLITE)
  SET(HAVE_SQLITE OFF CACHE INTERNAL "building sqlite3 backend" FORCE)
ENDIF(ENABLE_SQLITE)

IF(ENABLE_POSTGRES AND POSTGRES_FOUND)

  SET(HAVE_POSTGRES ON CACHE INTERNAL "building postgres backend" FORCE)

  MESSAGE("** Wt::Dbo: building Postgres backend.")

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbopostgres-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbopostgres-version.rc)
      SET(postgres_SRCS ${postgres_SRCS} 
          ${CMAKE_CURRENT_BINARY_DIR}/wtdbopostgres-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbopostgres
    Postgres.C
    ${postgres_SRCS}
    )
  TARGET_LINK_LIBRARIES(wtdbopostgres wtdbo ${POSTGRES_LIBRARIES} ${BOOST_DT_LIB})

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
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )

  INSTALL_FILES(/include/Wt/Dbo/backend "^Postgres$")
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*Postgres.*h$")
ELSE(ENABLE_POSTGRES AND POSTGRES_FOUND)
  MESSAGE("** Wt::Dbo: not building Postgres backend.")

  IF(ENABLE_POSTGRES)
    MESSAGE("    Indicate the location of your postgres installation using ")
    MESSAGE("    -DPOSTGRES_PREFIX=...")
  ENDIF(ENABLE_POSTGRES)

  SET(HAVE_POSTGRES OFF CACHE INTERNAL "building postgres backend" FORCE)
ENDIF(ENABLE_POSTGRES AND POSTGRES_FOUND)

IF(ENABLE_FIREBIRD)
  OPTION(USE_SYSTEM_IBPP "Use a custom IBPP library instead of Wt's version" OFF)

  IF(NOT USE_SYSTEM_IBPP)
    SET(IBPP_SRC_DIRECTORY ibpp/)
  ENDIF(NOT USE_SYSTEM_IBPP)
  INCLUDE(../../../../cmake/WtFindFirebirdSql.txt)
ENDIF(ENABLE_FIREBIRD)

IF(ENABLE_FIREBIRD AND FIREBIRD_FOUND)

  SET(HAVE_FIREBIRD ON CACHE INTERNAL "building firebird backend" FORCE)

  IF(MSVC)
    ADD_DEFINITIONS(-DIBPP_WINDOWS)
  ENDIF(MSVC)
  IF(APPLE)
    ADD_DEFINITIONS(-DIBPP_DARWIN)
  ENDIF(APPLE)
  IF(UNIX)
    ADD_DEFINITIONS(-DIBPP_LINUX)
  ENDIF(UNIX)

  INCLUDE_DIRECTORIES(${FIREBIRD_INCLUDE_DIRS})

  MESSAGE("** Wt::Dbo: building FirebirdSQL backend.")

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbofirebird-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbofirebird-version.rc)
      SET(firebird_SRCS ${firebird_SRCS} 
          ${CMAKE_CURRENT_BINARY_DIR}/wtdbofirebird-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbofirebird
    Firebird.C
    ${IBPP_ALL_IN_ONE_FILE}
    ${firebird_SRCS}
  )
  TARGET_LINK_LIBRARIES(wtdbofirebird
    wtdbo
    ${FIREBIRD_LIBRARIES}
    ${BOOST_DT_LIB}
  )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbofirebird
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
  ENDIF(MSVC)

  INSTALL(TARGETS wtdbofirebird
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  SET_TARGET_PROPERTIES(
    wtdbofirebird
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTDBOFIREBIRD_SOVERSION}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )

  INSTALL_FILES(/include/Wt/Dbo/backend "^Firebird$")
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*Firebird.*h$")
ELSE(ENABLE_FIREBIRD AND FIREBIRD_FOUND)
  MESSAGE("** Wt::Dbo: not building Firebird backend.")

  IF(ENABLE_FIREBIRD)
    MESSAGE("    Indicate the location of your FirebirdSQL and IBPP installation using ")
    MESSAGE("    -DFIREBIRD_PREFIX=... -DIBPP_SRC_DIRECTORY=...")
  ENDIF(ENABLE_FIREBIRD)

  SET(HAVE_FIREBIRD OFF CACHE INTERNAL "building firebird backend" FORCE)
ENDIF(ENABLE_FIREBIRD AND FIREBIRD_FOUND)

IF(ENABLE_MYSQL AND MYSQL_FOUND)

  SET(HAVE_MYSQL ON CACHE INTERNAL "building mysql backend" FORCE)

  MESSAGE("** Wt::Dbo: building MySQL backend.")
  ADD_DEFINITIONS(${MYSQL_DEFINITIONS})

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbomysql-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbomysql-version.rc)
      SET(mysql_SRCS ${mysql_SRCS} 
          ${CMAKE_CURRENT_BINARY_DIR}/wtdbomysql-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbomysql
    MySQL.C
    ${mysql_SRCS}
    )
  TARGET_LINK_LIBRARIES(wtdbomysql wtdbo ${MYSQL_LIBRARIES} ${BOOST_DT_LIB})

  INCLUDE_DIRECTORIES(${MYSQL_INCLUDE})

  INSTALL(TARGETS wtdbomysql
    RUNTIME DESTINATION bin	
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  SET_TARGET_PROPERTIES(
    wtdbomysql
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTDBOMYSQL_SOVERSION}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
  INSTALL_FILES(/include/Wt/Dbo/backend "^MySQL$")
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*MySQL.*h$")
ELSE(ENABLE_MYSQL AND MYSQL_FOUND)
  MESSAGE("** Wt::Dbo: not building MySQL backend.")

  IF(ENABLE_MYSQL)
    MESSAGE("    Indicate the location of your mariadb or mysqlclient installation using ")
    MESSAGE("    -DMYSQL_PREFIX=... and the library to search for (e.g. mariadb or ")
    MESSAGE("    mysqlclient) using -DMYSQL_LIBRARY=...")
  ENDIF(ENABLE_MYSQL)

  SET(HAVE_MYSQL OFF CACHE INTERNAL "building mysql backend" FORCE)
ENDIF(ENABLE_MYSQL AND MYSQL_FOUND)

