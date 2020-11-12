IF(BOOST_FS_LIB)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)

SET(SRC
	main.C
	Home.C
	WtHome.C
	JWtHome.C
	FileItem.C
	SourceView.C
	ExampleSourceViewer.C
	../blog/BlogRSSFeed.C
	../blog/view/BlogLoginWidget.C
        ../blog/view/BlogView.C
	../blog/view/CommentView.C
	../blog/view/PostView.C
  	../blog/view/EditUsers.C
	../blog/model/BlogSession.C
	../blog/model/BlogUserDatabase.C
	../blog/model/Comment.C
	../blog/model/Post.C
	../blog/model/User.C
	../blog/model/Tag.C
	../blog/model/Token.C
	../blog/asciidoc/asciidoc.C
)

IF(WT_EMWEB_BUILD)
  SET(SRC ${SRC} ../../../emweb/QuoteForm.C)
  INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/../emweb/)
  ADD_DEFINITIONS(-DWT_EMWEB_BUILD)
ENDIF(WT_EMWEB_BUILD)

WT_ADD_EXAMPLE(Home.wt ${SRC})

TARGET_LINK_LIBRARIES(Home.wt ${EXAMPLES_DBO_LIBS} ${BOOST_FS_LIB} ${BOOST_SYSTEM_LIB})

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
  ../blog
  )

ELSE(BOOST_FS_LIB)
  
  MESSAGE(STATUS "** Not building wt-homepage example: requires boost_filesystem library.")

ENDIF(BOOST_FS_LIB)
