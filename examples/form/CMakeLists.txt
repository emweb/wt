#
# The form example needs the additional boost_date_time library,
# therefore we check first that it exists before trying to build
# the example.
#

IF(NOT BOOST_WT_FOUND)
  MESSAGE(STATUS "** Form example needs boost_date_time library... Skipping.")
ELSE(NOT BOOST_WT_FOUND)

  ADD_EXECUTABLE(formexample.wt DateValidator.C Form.C FormExample.C)
  TARGET_LINK_LIBRARIES(formexample.wt
    ${EXAMPLES_CONNECTOR}
    ${BOOST_WT_DT_LIBRARY}
    )

  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

  #
  # Create a deploy script
  #
  SET(APPNAME form)
  SET(APPBIN formexample.wt)
  SET(APPRESOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/*.xml")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

  ADD_DEPENDENCIES(formexample.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT BOOST_WT_FOUND)

