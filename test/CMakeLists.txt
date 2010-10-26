SET(TEST_SOURCES
  test.C
  chart/WChartTest.C
  models/WBatchEditProxyModelTest.C
  private/HttpTest.C
  utf8/Utf8Test.C
  utf8/XmlTest.C
  wdatetime/WDateTimeTest.C
)

# HAVE_SQLITE does not work: why ?
IF(ENABLE_SQLITE)
  ADD_DEFINITIONS(-DWTDBO)

  SET(TEST_SOURCES ${TEST_SOURCES}
    dbo/DboTest.C
    dbo/DboTest2.C
    private/DboImplTest.C
  )

  # SQLITE3 test
  SET(TEST_LIBS wtdbo wtdbosqlite3)
  ADD_DEFINITIONS(-DSQLITE3)

  # POSTGRES test
  #SET(TEST_LIBS wtdbo wtdbopostgres)
  #ADD_DEFINITIONS(-DPOSTGRES)

ENDIF(ENABLE_SQLITE)

ADD_EXECUTABLE(test
  ${TEST_SOURCES}
)

TARGET_LINK_LIBRARIES(test wt ${TEST_LIBS})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
  SUBDIRS(interactive)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
