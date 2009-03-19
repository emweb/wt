FILE(READ ${infile} f0)
STRING( REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f0}" )
STRING( REGEX REPLACE "\"" "\\\\\"" f2 "${f1}" )
STRING( REGEX REPLACE "\r?\n" "\\\\r\\\\n\"\n  \"" f3 "${f2}" )
SET( f4 "// This is automatically generated code -- do not edit!\n// Generated from ${file} \n\nnamespace skeletons {\n\n  const char * ${var} =\n  \"${f3}\";\n}\n" )
FILE(WRITE ${outfile} "${f4}")

