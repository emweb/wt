WT_ADD_EXAMPLE(extkitchen.wt ExtKitchenApplication.C CsvUtil.C)
TARGET_LINK_LIBRARIES(extkitchen.wt wtext)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ADD_DEPENDENCIES(extkitchen.wt wt wtext ${EXAMPLES_CONNECTOR})
