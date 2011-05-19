IF(NOT WT_NO_STD_WSTRING)

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

ADD_DEPENDENCIES(composer.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT WT_NO_STD_WSTRING)
