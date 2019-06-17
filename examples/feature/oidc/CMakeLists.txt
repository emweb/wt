IF(BOOST_WT_FOUND)
WT_ADD_EXAMPLE(oidc.wt
	Oidc.C
        OAuthWidget.C
        RegistrationView.C
        OAuthAuthorizationEndpoint.C
        model/User.C
	model/IssuedToken.C
	model/OAuthClient.C
        model/OidcUserDatabase.C
        model/Session.C
        model/UserDetailsModel.C
)

TARGET_LINK_LIBRARIES(oidc.wt ${EXAMPLES_DBO_LIBS})
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(oidc.wt Boost::headers)
ENDIF()

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building oidc example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
