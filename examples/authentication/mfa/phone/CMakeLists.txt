WT_ADD_EXAMPLE(phone-login.wt
  authentry.h
  authentry.cpp
  myauthmodel.h
  myauthmodel.cpp
  myauthwidget.h
  myauthwidget.cpp
  mysession.h
  mysession.cpp
  myuser.h
  myuser.cpp
  phonewidget.h
  phonewidget.cpp
  phone-login.cpp
  qrcodehandler.h
  qrcodehandler.cpp
  qrcodepainter.h
  qrcodepainter.cpp
  ${WT_SOURCE_DIR}/src/thirdparty/qrcodegen/QrCode.cpp
)

target_link_libraries(phone-login.wt ${EXAMPLES_DBO_LIBS})

configure_file(approot/template.xml template.xml COPYONLY)

include_directories(${WT_SOURCE_DIR}/src)
