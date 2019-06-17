IF(BOOST_WT_FOUND)
WT_ADD_EXAMPLE(widgetfunction.wt Example.C WidgetFunction.C)
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(widgetfunction.wt Boost::headers)
ENDIF()

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building template-fun example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
