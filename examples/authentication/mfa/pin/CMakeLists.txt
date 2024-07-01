WT_ADD_EXAMPLE(pin-login.wt
  myauthwidget.h
  myauthwidget.cpp
  mysession.h
  mysession.cpp
  myuser.h
  myuser.cpp
  pinwidget.h
  pinwidget.cpp
  pin-login.cpp
)

target_link_libraries(pin-login.wt ${EXAMPLES_DBO_LIBS})

configure_file(approot/template.xml template.xml COPYONLY)

include_directories(${WT_SOURCE_DIR}/src)
