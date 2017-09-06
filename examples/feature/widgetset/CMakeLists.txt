IF (EXAMPLES_CONNECTOR MATCHES "wthttp|Wt::HTTP")

  WT_ADD_EXAMPLE(hellowidgetset.wt hello.C)

  #
  # If you have Wt installed somehwere, you should use the
  # installed Wt header files for your own Wt projects.
  # e.g. INCLUDE_DIRECTORIES(/usr/local/include)
  # instead of the following:
  #
  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ELSE (EXAMPLES_CONNECTOR MATCHES "wthttp")

  MESSAGE(STATUS "** hello-widget set example requires the built-in http... Skipping.")

ENDIF (EXAMPLES_CONNECTOR MATCHES "wthttp|Wt::HTTP")
