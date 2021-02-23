WT_ADD_EXAMPLE(dbo-form.wt
  DboForm.C
  EnumCombo.h
  EnumFormDelegate.h
  PtrCollectionFormDelegate.h PtrCollectionFormDelegate.C
  PtrFormDelegate.h PtrFormDelegate.C
  QueryCombo.h
  QuerySelectionBox.h
  TextEditFormDelegate.h TextEditFormDelegate.C
  TextFormDelegate.h TextFormDelegate.C
  model/CustomSqlTraits.h
  model/TestDboObject.h model/TestDboObject.C
  model/TestSession.h model/TestSession.C
)

TARGET_LINK_LIBRARIES(dbo-form.wt ${EXAMPLES_DBO_LIBS})
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
