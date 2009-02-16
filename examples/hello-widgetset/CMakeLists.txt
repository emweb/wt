IF (${EXAMPLES_CONNECTOR} MATCHES "wthttp")

  ADD_EXECUTABLE(hello-widgetset.wt hello.C)
  TARGET_LINK_LIBRARIES(hello-widgetset.wt ${EXAMPLES_CONNECTOR})

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
  SET(APPNAME hello-widgetset)
  SET(APPBIN hello-widgetset.wt)
  SET(APPRESOURCES "")
  CONFIGURE_FILE(
    ${WT_SOURCE_DIR}/deploy.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
    )

ELSE (${EXAMPLES_CONNECTOR} MATCHES "wthttp")

  MESSAGE(STATUS "** hello-widget set example requires the built-in http... Skipping.")

ENDIF (${EXAMPLES_CONNECTOR} MATCHES "wthttp")
