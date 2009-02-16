ADD_EXECUTABLE(charts.wt
	PanelList.C
	ChartConfig.C
	ChartsExample.C
	CsvUtil.C
	ChartsApplication.C
)
TARGET_LINK_LIBRARIES(charts.wt wtext ${EXAMPLES_CONNECTOR})

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

#
# Create a deploy script
#
SET(APPNAME charts)
SET(APPBIN charts.wt)
SET(APPRESOURCES "")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(charts.wt wt wtext ${EXAMPLES_CONNECTOR})
