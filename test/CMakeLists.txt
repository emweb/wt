ADD_EXECUTABLE(test
  test.C
  chart/WChartTest.C
  dbo/DboTest.C
  dbo/DboTest2.C
  wdatetime/WDateTimeTest.C
)

# SQLITE3 test
TARGET_LINK_LIBRARIES(test wt wtdbo wtdbosqlite3)
ADD_DEFINITIONS(-DSQLITE3)

# POSTGRES test
#TARGET_LINK_LIBRARIES(test wt wtdbo wtdbopostgres)
#ADD_DEFINITIONS(-DPOSTGRES)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
  SUBDIRS(interactive)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
