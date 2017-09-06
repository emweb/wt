SET(tutorial_libs ${EXAMPLES_DBO_LIBS})

#IF (MULTI_THREADED)
#  SET(tutorial_libs ${tutorial_libs} ${BOOST_THREAD_LIB})
#ENDIF (MULTI_THREADED)

SUBDIRS(
    tutorial9
)

ADD_EXECUTABLE(dbo-tutorial1  tutorial1.C)
TARGET_LINK_LIBRARIES(dbo-tutorial1 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial2  tutorial2.C)
TARGET_LINK_LIBRARIES(dbo-tutorial2 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial3  tutorial3.C)
TARGET_LINK_LIBRARIES(dbo-tutorial3 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial4  tutorial4.C)
TARGET_LINK_LIBRARIES(dbo-tutorial4 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial5  tutorial5.C)
TARGET_LINK_LIBRARIES(dbo-tutorial5 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial6  tutorial6.C)
TARGET_LINK_LIBRARIES(dbo-tutorial6 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial7  tutorial7.C)
TARGET_LINK_LIBRARIES(dbo-tutorial7 ${tutorial_libs})

ADD_EXECUTABLE(dbo-tutorial8  tutorial8.C)
TARGET_LINK_LIBRARIES(dbo-tutorial8 ${tutorial_libs})

IF(MSVC)
	SET_TARGET_PROPERTIES(dbo-tutorial1 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial2 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial3 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial4 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial5 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial6 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial7 PROPERTIES FOLDER "dbo/examples")
	SET_TARGET_PROPERTIES(dbo-tutorial8 PROPERTIES FOLDER "dbo/examples")
ENDIF(MSVC)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF(INSTALL_EXAMPLES)
  INSTALL(TARGETS dbo-tutorial1 dbo-tutorial2 dbo-tutorial3
    dbo-tutorial4 dbo-tutorial5 dbo-tutorial6
    dbo-tutorial7 dbo-tutorial8
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXAMPLES_DESTINATION}/feature/dbo/)
ENDIF(INSTALL_EXAMPLES)
