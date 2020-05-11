IF(ENABLE_SQLITE)

  OPTION(USE_SYSTEM_SQLITE3 "Use system-wide Sqlite3 instead of Wt's version" OFF)
  OPTION(USE_SQLITE3_BDB "Use BerkeleyDB storage with Sqlite3 API" OFF)

  IF(USE_SYSTEM_SQLITE3)

    FIND_PACKAGE( Sqlite3 REQUIRED)

    IF (USE_SQLITE3_BDB)
      ADD_DEFINITIONS(-DSQLITE3_BDB)
    ENDIF (USE_SQLITE3_BDB)

  ELSE(USE_SYSTEM_SQLITE3)

    IF (NOT WIN32)
      ADD_DEFINITIONS(-DHAVE_USLEEP)
    ENDIF (NOT WIN32)
    SET(Sqlite3_SRCS amalgamation/sqlite3.c)

  ENDIF(USE_SYSTEM_SQLITE3)

  FIND_PACKAGE(Threads)

  set(WT_SQLITE3_THREAD_LIB "")
  IF (NOT Threads_FOUND)
    ADD_DEFINITIONS(-DSQLITE3_THREAD_SAFE=0)
  ELSE (NOT Threads_FOUND)
    set(WT_SQLITE3_THREAD_LIB Threads::Threads)
  ENDIF (NOT Threads_FOUND)

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
    Sqlite3.h Sqlite3.C
    ${Sqlite3_SRCS}
    )

  SET_PROPERTY(TARGET wtdbosqlite3 PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbosqlite3 PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbosqlite3 PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  MESSAGE("** Wt::Dbo: building SQLite3 backend.")

  TARGET_LINK_LIBRARIES(wtdbosqlite3
    PUBLIC
      wtdbo
    PRIVATE
      ${SQLITE3_LIBRARIES}
      ${WT_SQLITE3_THREAD_LIB}
      ${WT_MATH_LIBRARY}
      )

  IF(NOT WIN32)
    FIND_LIBRARY(DL_LIB
      NAMES
      dl
      )
    IF(DL_LIB)
      TARGET_LINK_LIBRARIES(wtdbosqlite3 PRIVATE ${DL_LIB})
    ENDIF(DL_LIB)
  ENDIF(NOT WIN32)

  INSTALL(TARGETS wtdbosqlite3
    EXPORT wt-target-dbosqlite3
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbosqlite3
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbosqlite3
  PROPERTIES
    EXPORT_NAME DboSqlite3
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbosqlite3
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
	SET_TARGET_PROPERTIES(wtdbosqlite3 PROPERTIES FOLDER "dbo")
  ENDIF(MSVC)

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
    Postgres.h Postgres.C
    ${postgres_SRCS}
    )

  SET_PROPERTY(TARGET wtdbopostgres PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbopostgres PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbopostgres PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wtdbopostgres
    PUBLIC
      wtdbo
    PRIVATE
      ${POSTGRES_LIBRARIES}
  )

  IF(TARGET Boost::headers)
    TARGET_LINK_LIBRARIES(wtdbopostgres PRIVATE Boost::headers)
  ENDIF()

  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE})

  IF(MSVC)
    SET_TARGET_PROPERTIES(wtdbopostgres PROPERTIES FOLDER "dbo")
  ENDIF(MSVC)

  INSTALL(TARGETS wtdbopostgres
    EXPORT wt-target-dbopostgres
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbopostgres
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbopostgres
  PROPERTIES
    EXPORT_NAME DboPostgres
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )

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
    Firebird.h Firebird.C
    ${IBPP_ALL_IN_ONE_FILE}
    ${firebird_SRCS}
  )

  SET_PROPERTY(TARGET wtdbofirebird PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbofirebird PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbofirebird PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wtdbofirebird
    PUBLIC
      wtdbo
    PRIVATE
      ${FIREBIRD_LIBRARIES}
  )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbofirebird
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
	SET_TARGET_PROPERTIES(wtdbofirebird PROPERTIES FOLDER "dbo")
  ENDIF(MSVC)

  INSTALL(TARGETS wtdbofirebird
    EXPORT wt-target-dbofirebird
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbofirebird
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbofirebird
  PROPERTIES
    EXPORT_NAME DboFirebird
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )

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
    MySQL.h MySQL.C
    ${mysql_SRCS}
    )

  SET_PROPERTY(TARGET wtdbomysql PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbomysql PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbomysql PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wtdbomysql
    PUBLIC
      wtdbo
    PRIVATE
      ${MYSQL_LIBRARIES}
  )

  INCLUDE_DIRECTORIES(${MYSQL_INCLUDE})

  INSTALL(TARGETS wtdbomysql
    EXPORT wt-target-dbomysql
    RUNTIME DESTINATION bin	
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbomysql
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbomysql
  PROPERTIES
    EXPORT_NAME DboMySQL
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
  INSTALL_FILES(/include/Wt/Dbo/backend "^.*MySQL.*h$")
  IF(MSVC)
    SET_TARGET_PROPERTIES(wtdbomysql PROPERTIES FOLDER "dbo")
  ENDIF(MSVC)
ELSE(ENABLE_MYSQL AND MYSQL_FOUND)
  MESSAGE("** Wt::Dbo: not building MySQL backend.")

  IF(ENABLE_MYSQL)
    MESSAGE("    Indicate the location of your mariadb or mysqlclient installation using ")
    MESSAGE("    -DMYSQL_PREFIX=... and the library to search for (e.g. mariadb or ")
    MESSAGE("    mysqlclient) using -DMYSQL_LIBRARY=...")
  ENDIF(ENABLE_MYSQL)

  SET(HAVE_MYSQL OFF CACHE INTERNAL "building mysql backend" FORCE)
ENDIF(ENABLE_MYSQL AND MYSQL_FOUND)

IF(ENABLE_MSSQLSERVER AND ODBC_FOUND)

  SET(HAVE_MSSQLSERVER ON CACHE INTERNAL "building Microsoft SQL Server backend" FORCE)

  MESSAGE("** Wt::Dbo: building Microsoft SQL Server backend.")

  IF(WIN32)
    IF(SHARED_LIBS)
      CONFIGURE_FILE(wtdbomssqlserver-version.rc.in
                     ${CMAKE_CURRENT_BINARY_DIR}/wtdbomssqlserver-version.rc)
      SET(mssqlserver_SRCS ${mssqlserver_SRCS}
          ${CMAKE_CURRENT_BINARY_DIR}/wtdbomssqlserver-version.rc)
    ENDIF(SHARED_LIBS)
  ENDIF(WIN32)

  ADD_LIBRARY(wtdbomssqlserver
    MSSQLServer.h MSSQLServer.C
    ${mssqlserver_SRCS}
  )

  SET_PROPERTY(TARGET wtdbomssqlserver PROPERTY C_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbomssqlserver PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wtdbomssqlserver PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wtdbomssqlserver
    PUBLIC
      wtdbo
    PRIVATE
      ${ODBC_LIBRARIES}
  )

  INCLUDE_DIRECTORIES(${ODBC_INCLUDE})

  INSTALL(TARGETS wtdbomssqlserver
    EXPORT wt-target-dbomssqlserver
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-dbomssqlserver
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  SET_TARGET_PROPERTIES(
    wtdbomssqlserver
  PROPERTIES
    EXPORT_NAME DboMSSQLServer
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
    )

  IF(MSVC)
    SET_TARGET_PROPERTIES(
      wtdbomssqlserver
    PROPERTIES
      COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4267"
    )
    SET_TARGET_PROPERTIES(wtdbomssqlserver PROPERTIES FOLDER "dbo")
  endif(MSVC)

  INSTALL_FILES(/include/Wt/Dbo/backend "^.*MSSQLServer.*h$")
ELSE(ENABLE_MSSQLSERVER AND ODBC_FOUND)
  MESSAGE("** Wt::Dbo: not building Microsoft SQL Server backend.")
  SET(HAVE_MSSQLSERVER OFF CACHE INTERNAL "building Microsoft SQL Server backend" FORCE)
ENDIF(ENABLE_MSSQLSERVER AND ODBC_FOUND)
