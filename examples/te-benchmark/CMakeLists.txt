IF(BOOST_WT_FOUND)
#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF(ENABLE_MYSQL AND MYSQL_FOUND)
WT_ADD_EXAMPLE(te-benchmark.wt benchmark.cpp)
TARGET_LINK_LIBRARIES(te-benchmark.wt ${EXAMPLES_WTDBO_LIB} ${EXAMPLES_WTDBOMYSQL_LIB})
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(te-benchmark.wt Boost::headers)
ENDIF()
ENDIF(ENABLE_MYSQL AND MYSQL_FOUND)

IF(ENABLE_POSTGRES AND POSTGRES_FOUND)
WT_ADD_EXAMPLE(te-benchmark-pg.wt benchmark.cpp)
TARGET_COMPILE_DEFINITIONS(te-benchmark-pg.wt PRIVATE BENCHMARK_USE_POSTGRES)
TARGET_LINK_LIBRARIES(te-benchmark-pg.wt ${EXAMPLES_WTDBO_LIB} ${EXAMPLES_WTDBOPOSTGRES_LIB})
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(te-benchmark-pg.wt Boost::headers)
ENDIF()
ENDIF(ENABLE_POSTGRES AND POSTGRES_FOUND)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building te-benchmark example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
