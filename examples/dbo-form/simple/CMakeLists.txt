WT_ADD_EXAMPLE(dbo-form-simple.wt
  DboFormSimple.C
)

TARGET_LINK_LIBRARIES(dbo-form-simple.wt ${EXAMPLES_DBO_LIBS})
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)
