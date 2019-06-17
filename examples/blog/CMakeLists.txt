IF(BOOST_WT_FOUND)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)

IF(ASCIIDOC_EXECUTABLE)
  ADD_DEFINITIONS(-DASCIIDOC_EXECUTABLE="${ASCIIDOC_EXECUTABLE}")
ENDIF(ASCIIDOC_EXECUTABLE)

WT_ADD_EXAMPLE(blog.wt
  model/BlogSession.C
  model/BlogUserDatabase.C
  model/Post.C
  model/Comment.C
  model/User.C
 model/Tag.C
  model/Token.C
  view/PostView.C
  view/CommentView.C
  view/BlogLoginWidget.C
  view/BlogView.C
  view/EditUsers.C
  asciidoc/asciidoc.C
  BlogRSSFeed.C
  blog.C
)

IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(blog.wt Boost::headers)
ENDIF()

TARGET_LINK_LIBRARIES(blog.wt ${EXAMPLES_DBO_LIBS})

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
  TARGET_LINK_LIBRARIES(blog.wt crypt)
ENDIF(CRYPT_LIB_EXISTS)

INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building blog example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
