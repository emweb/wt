SET(tutorial_libs wtdbo wtdbosqlite3)

#IF (MULTI_THREADED)
#  SET(tutorial_libs ${tutorial_libs} ${BOOST_THREAD_LIB})
#ENDIF (MULTI_THREADED)

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

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
