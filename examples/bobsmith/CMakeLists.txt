WT_ADD_EXAMPLE(bobsmith.wt bobsmith.C)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ADD_DEPENDENCIES(bobsmith.wt wt ${EXAMPLES_CONNECTOR})

