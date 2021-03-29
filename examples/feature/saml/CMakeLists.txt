wt_add_example(saml.wt
  Saml.C
  SamlService.h SamlService.C
  model/Session.h model/Session.C
  model/User.h model/User.C
)

target_link_libraries(saml.wt ${EXAMPLES_DBO_LIBS})

include_directories(${WT_SOURCE_DIR}/src)
