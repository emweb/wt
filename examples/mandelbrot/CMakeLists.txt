IF(NOT GM_FOUND)
  MESSAGE(STATUS "** Not building mandelbrot example: requires Wt::WRasterImage.")
ELSE(NOT GM_FOUND)

  ADD_EXECUTABLE(mandelbrot.wt MandelbrotImage.C MandelbrotExample.C)
  TARGET_LINK_LIBRARIES(mandelbrot.wt ${EXAMPLES_CONNECTOR})

  INCLUDE_DIRECTORIES(
    ${GD_INCLUDE_DIRS}
    ${WT_SOURCE_DIR}/src
  )

  #
  # Create a deploy script
  #
  SET(APPNAME mandelbrot)
  SET(APPBIN mandelbrot.wt)
  SET(APPRESOURCES "")

  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )
  ADD_DEPENDENCIES(mandelbrot.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT GM_FOUND)

