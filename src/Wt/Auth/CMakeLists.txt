SUBDIRS(Dbo)

FILE(GLOB AUTH_H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
LIST(REMOVE_ITEM AUTH_H_FILES MailUtils.h passwdqc.h)
INSTALL(FILES ${AUTH_H_FILES}
        DESTINATION include/Wt/Auth)

if(ASCIIDOCTOR_FOUND)
  ASCIIDOC_FILE(auth-tutorial
    ${WT_SOURCE_DIR}/doc/tutorial/auth.adoc
    ${WT_SOURCE_DIR}/doc/tutorial/auth.html)
endif()

if(WT_HAS_SAML)
  add_subdirectory(Saml)
endif()
