if(ENABLE_LIBWTTEST AND ENABLE_SELENIUM_TESTS)
  message(STATUS "Configuring Selenium tests")

  # Check for Python3
  find_package(Python3 COMPONENTS Development)
  if(NOT Python3_FOUND)
    message(WARNING "Python3-dev not found - Selenium tests will be disabled")
    return()
  endif()

  if (NOT CONNECTOR_HTTP)
    message(WARNING "Cannot build Selenium tests, requires the Wt HTTP connector")
    return()
  endif()

  if (NOT MULTI_THREADED)
    message(WARNING "Selenium tests require multi-threading support")
    return()
  endif()

  # Create test executable
  add_executable(test.selenium ${SELENIUM_TEST_SOURCES})
  set_target_properties(test.selenium PROPERTIES FOLDER "test/selenium")
  target_link_libraries(test.selenium PRIVATE wt wthttp ${BOOST_FS_LIB} ${BOOST_TEST_LIBRARIES} ${WT_THREAD_LIB} Python3::Python)
  target_include_directories(test.selenium PRIVATE ${Python3_INCLUDE_DIRS})

  if(TARGET Boost::headers)
    target_link_libraries(test.selenium PRIVATE Boost::headers)
  endif()

  message(STATUS "Selenium tests configured successfully")
else()
  message(STATUS "Selenium tests disabled (set ENABLE_SELENIUM_TESTS=ON to enable)")
endif()
