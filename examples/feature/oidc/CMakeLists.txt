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

TARGET_LINK_LIBRARIES(oidc.wt wtdbo wtdbosqlite3)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
