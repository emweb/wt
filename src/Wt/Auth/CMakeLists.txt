SUBDIRS(Dbo)

FILE(GLOB AUTH_H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
LIST(REMOVE_ITEM AUTH_H_FILES MailUtils.h passwdqc.h)
INSTALL(FILES ${AUTH_H_FILES}
        DESTINATION include/Wt/Auth)

IF (ASCIIDOC_FOUND)
  ASCIIDOC_FILE(auth-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.html)
ENDIF (ASCIIDOC_FOUND)

if(WT_HAS_SAML)
  add_subdirectory(Saml)
endif()
