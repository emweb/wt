ADD_EXECUTABLE(paintexample.wt
  PaintApplication.C PaintExample.C ShapesWidget.C
)

TARGET_LINK_LIBRARIES(paintexample.wt ${EXAMPLES_CONNECTOR})

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
SET(APPNAME paintexample)
SET(APPBIN paintexample.wt)
SET(APPRESOURCES "")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

