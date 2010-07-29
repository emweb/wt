IF(BOOST_FS_LIB_MT)
  SET(FS_LIB ${BOOST_FS_LIB_MT})
ELSE(BOOST_FS_LIB_MT)
  IF (BOOST_FS_LIB)
    SET(FS_LIB ${BOOST_FS_LIB})
  ENDIF(BOOST_FS_LIB)
ENDIF(BOOST_FS_LIB_MT)

IF(FS_LIB)

INCLUDE(CheckFunctionExists)

ADD_EXECUTABLE(Home.wt
	main.C
	Home.C
	WtHome.C
	JWtHome.C
	FileItem.C
	SourceView.C
	ExampleSourceViewer.C
	../blog/BlogSession.C
	../blog/BlogRSSFeed.C
        ../blog/view/BlogView.C
	../blog/view/CommentView.C
	../blog/view/PostView.C
	../blog/model/Comment.C
	../blog/model/Post.C
	../blog/model/Tag.C
	../blog/model/User.C
	../blog/asciidoc/asciidoc.C
)

TARGET_LINK_LIBRARIES(Home.wt ${EXAMPLES_CONNECTOR} wtdbo wtdbosqlite3 ${FS_LIB})

# Test whether crypt(3) is provided by libc. If it's not, check if
# libcrypt exists and if it provides crypt(3).
CHECK_FUNCTION_EXISTS(crypt CRYPT_EXISTS)
IF(NOT CRYPT_EXISTS)
  CHECK_LIBRARY_EXISTS(crypt crypt "" CRYPT_LIB_EXISTS)
  IF(CRYPT_LIB_EXISTS)
    SET(CMAKE_REQUIRED_LIBRARIES "crypt")
    CHECK_FUNCTION_EXISTS(crypt CRYPT_EXISTS)
  ENDIF(CRYPT_LIB_EXISTS)
ENDIF(NOT CRYPT_EXISTS)

IF(CRYPT_LIB_EXISTS)
  TARGET_LINK_LIBRARIES(Home.wt crypt)
ENDIF(CRYPT_LIB_EXISTS)

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ${WT_SOURCE_DIR}/examples/blog
  )

#
# Create a deploy script
#
SET(APPNAME wt-homepage)
SET(APPBIN Home.wt)

SET(APPRESOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_SOURCE_DIR}/css ${CMAKE_CURRENT_SOURCE_DIR}/*.xml ${CMAKE_CURRENT_SOURCE_DIR}/*.css")

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
  )

ADD_DEPENDENCIES(Home.wt wt ${EXAMPLES_CONNECTOR})

ELSE(FS_LIB)
  
  MESSAGE(STATUS "** Not building wt-homepage example: requires boost_filesystem library.")

ENDIF(FS_LIB)
