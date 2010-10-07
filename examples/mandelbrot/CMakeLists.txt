IF(NOT HAVE_GM)
  MESSAGE(STATUS "** Not building mandelbrot example: requires Wt::WRasterImage.")
ELSE(NOT HAVE_GM)

  WT_ADD_EXAMPLE(mandelbrot.wt MandelbrotImage.C MandelbrotExample.C)

  INCLUDE_DIRECTORIES(
    ${GD_INCLUDE_DIRS}
    ${WT_SOURCE_DIR}/src
  )

  ADD_DEPENDENCIES(mandelbrot.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT HAVE_GM)

