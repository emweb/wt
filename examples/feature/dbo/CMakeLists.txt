ADD_EXECUTABLE(tutorial1  tutorial1.C)
TARGET_LINK_LIBRARIES(tutorial1 wtdbo wtdbosqlite3 ${BOOST_WT_DT_LIBRARY})

ADD_EXECUTABLE(tutorial2  tutorial2.C)
TARGET_LINK_LIBRARIES(tutorial2 wtdbo wtdbosqlite3 ${BOOST_WT_DT_LIBRARY})

ADD_EXECUTABLE(tutorial3  tutorial3.C)
TARGET_LINK_LIBRARIES(tutorial3 wtdbo wtdbosqlite3 ${BOOST_WT_DT_LIBRARY})

ADD_EXECUTABLE(tutorial4  tutorial4.C)
TARGET_LINK_LIBRARIES(tutorial4 wtdbo wtdbosqlite3 ${BOOST_WT_DT_LIBRARY})

ADD_EXECUTABLE(tutorial5  tutorial5.C)
TARGET_LINK_LIBRARIES(tutorial5 wtdbo wtdbosqlite3 ${BOOST_WT_DT_LIBRARY})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
