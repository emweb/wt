if(ENABLE_LIBWTTEST AND ENABLE_SELENIUM_TESTS)
  message(STATUS "Configuring Selenium tests")

  # Check for Python3
  find_package(Python3 COMPONENTS Development Interpreter)
  if(NOT Python3_FOUND)
    message(WARNING "Python3 or Python3-dev not found - Selenium tests will be disabled")
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

  if (NOT Python3_Interpreter_FOUND)
    message(WARNING "Could not locate Python3 interpreter executable")
    return()
  endif()

  # Create venv to install Selenium
  if (NOT EXISTS ${CMAKE_SOURCE_DIR}/test/selenium/python-env)
    execute_process(COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_SOURCE_DIR}/test/selenium/python-env)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/test/selenium/python-env/bin/python3 -m pip install selenium)
  endif()

  # Add Selenium test source files
  set(SELENIUM_TEST_SOURCES
    ../test.C # Have correct boost headers included
    ElementTest.C
    InteractionTest.C
    LocatorTest.C
    SetupTest.C
  )

  # Create test executable
  add_executable(test.selenium ${SELENIUM_TEST_SOURCES})
  set_target_properties(test.selenium PROPERTIES FOLDER "test/selenium")
  target_link_libraries(test.selenium PRIVATE wt wthttp ${BOOST_FS_LIB} ${BOOST_TEST_LIBRARIES} ${WT_THREAD_LIB} Python3::Python)
  target_include_directories(test.selenium PRIVATE ${Python3_INCLUDE_DIRS})

  if(TARGET Boost::headers)
    target_link_libraries(test.selenium PRIVATE Boost::headers)
  endif()

  # Select correct activation script
  target_compile_definitions(test.selenium PRIVATE -DPYTHON_VENV_PATH=${CMAKE_SOURCE_DIR}/test/selenium/python-env)

  message(STATUS "Selenium tests configured successfully")
else()
  message(STATUS "Selenium tests disabled (set ENABLE_SELENIUM_TESTS=ON to enable)")
endif()
