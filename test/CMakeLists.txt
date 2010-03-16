ADD_EXECUTABLE(test
	test.C
	chart/WChartTest.C
	dbo/DboTest.C
	dbo/DboTest2.C
	wdatetime/WDateTimeTest.C
)

TARGET_LINK_LIBRARIES(test wt wtdbo wtdbosqlite3)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
  SUBDIRS(interactive)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
