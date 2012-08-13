SET(TEST_SOURCES
  test.C
  auth/BCryptTest.C
  auth/SHA1Test.C
  chart/WChartTest.C
  json/JsonParserTest.C
  http/HttpClientTest.C
  mail/MailClientTest.C
  models/WBatchEditProxyModelTest.C
  models/WStandardItemModelTest.C
  private/HttpTest.C
  private/CExpressionParserTest.C
  private/I18n.C
  utf8/Utf8Test.C
  utf8/XmlTest.C
  utils/Base64Test.C
  wdatetime/WDateTimeTest.C
  length/WLengthTest.C
  color/WColorTest.C
  paintdevice/WSvgTest.C
  payment/MoneyTest.C
)

IF (WT_HAS_WRASTERIMAGE)
   SET(TEST_SOURCES ${TEST_SOURCES}
     paintdevice/WRasterTest.C
   )
ENDIF(WT_HAS_WRASTERIMAGE)

# HAVE_SQLITE does not work: why ?
IF(ENABLE_SQLITE)
  ADD_DEFINITIONS(-DWTDBO)

  SET(TEST_SOURCES ${TEST_SOURCES}
    dbo/DboTest.C
    dbo/DboTest2.C
    dbo/Benchmark.C
    private/DboImplTest.C
  )

IF(TEST_DBO_BACKEND MATCHES "postgres")
  SET(TEST_LIBS wtdbo wtdbopostgres)
  ADD_DEFINITIONS(-DPOSTGRES)
ENDIF(TEST_DBO_BACKEND MATCHES "postgres")

IF(TEST_DBO_BACKEND MATCHES "firebird")
  SET(TEST_LIBS wtdbo wtdbofirebird)
  ADD_DEFINITIONS(-DFIREBIRD)	
ENDIF(TEST_DBO_BACKEND MATCHES "firebird")

IF(TEST_DBO_BACKEND MATCHES "sqlite")
  SET(TEST_LIBS wtdbo wtdbosqlite3)
  ADD_DEFINITIONS(-DSQLITE3)
ENDIF(TEST_DBO_BACKEND MATCHES "sqlite")

ENDIF(ENABLE_SQLITE)

ADD_EXECUTABLE(test
  ${TEST_SOURCES}
)

TARGET_LINK_LIBRARIES(test wt wttest ${TEST_LIBS} ${BOOST_FS_LIB})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
  SUBDIRS(interactive)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
