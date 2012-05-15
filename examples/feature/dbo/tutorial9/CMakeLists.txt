SET(tutorial_libs wtdbo wtdbosqlite3)

#IF (MULTI_THREADED)
#  SET(tutorial_libs ${tutorial_libs} ${BOOST_THREAD_LIB})
#ENDIF (MULTI_THREADED)

ADD_EXECUTABLE(dbo-tutorial9 main.C Membership.C Organisation.C Person.C)
TARGET_LINK_LIBRARIES(dbo-tutorial9 ${tutorial_libs})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF(INSTALL_EXAMPLES)
  INSTALL(TARGETS dbo-tutorial9 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXAMPLES_DESTINATION}/feature/dbo/)
ENDIF(INSTALL_EXAMPLES)
