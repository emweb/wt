IF(NOT GD_FOUND)
  MESSAGE(STATUS "** Mandelbrot example needs gd library... Skipping.")
ELSE(NOT GD_FOUND)

  ADD_EXECUTABLE(mandelbrot.wt MandelbrotImage.C MandelbrotExample.C)
  TARGET_LINK_LIBRARIES(mandelbrot.wt ${EXAMPLES_CONNECTOR} ${GD_LIBRARIES})

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

ENDIF(NOT GD_FOUND)

