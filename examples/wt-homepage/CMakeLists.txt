ADD_EXECUTABLE(Home.wt
	main.C
	Home.C
	WtHome.C
	JWtHome.C
	FileItem.C
	SourceView.C
	ExampleSourceViewer.C
)

TARGET_LINK_LIBRARIES(Home.wt ${EXAMPLES_CONNECTOR})

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ${WT_SOURCE_DIR}/examples/treelist
  )

#
# Create a deploy script
#
SET(APPNAME wt-homepage)
SET(APPBIN Home.wt)

SET(APPRESOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${CMAKE_CURRENT_SOURCE_DIR}/*.css")

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

ADD_DEPENDENCIES(Home.wt wt ${EXAMPLES_CONNECTOR})
