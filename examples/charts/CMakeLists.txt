IF(BOOST_WT_FOUND)
WT_ADD_EXAMPLE(charts.wt
	PanelList.C
	ChartConfig.C
	ChartsExample.C
	CsvUtil.C
	ChartsApplication.C
)
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(charts.wt Boost::headers)
ENDIF()

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building charts example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
