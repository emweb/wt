WT_ADD_EXAMPLE(auth1.wt 
        Auth1.C
	model/Session.C
	model/User.C
)

TARGET_LINK_LIBRARIES(auth1.wt wtdbo wtdbosqlite3)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
