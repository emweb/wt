WT_ADD_EXAMPLE(qrlogin.wt 
        QRAuth.C
	QRAuthWidget.C
	model/QRAuthService.C
	model/QRTokenDatabase.C
	model/Session.C
	model/User.C
)

TARGET_LINK_LIBRARIES(qrlogin.wt ${EXAMPLES_DBO_LIBS})

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
