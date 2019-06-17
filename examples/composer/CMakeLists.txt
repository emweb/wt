IF(BOOST_WT_FOUND)
WT_ADD_EXAMPLE(composer.wt 
  AddresseeEdit.C
  AttachmentEdit.C
  ComposeExample.C
  Composer.C
  ContactSuggestions.C
  Label.C
  Option.C
  OptionList.C
)
IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(composer.wt Boost::headers)
ENDIF()

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ${WT_SOURCE_DIR}/examples/treelist
)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building composer example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
