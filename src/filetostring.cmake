file(READ ${infile} f0)
string(LENGTH "${f0}" f0_LEN)
set(f4 "// This is automatically generated code -- do not edit!\n// Generated from ${infile} \nnamespace skeletons {")

# Max length for MSVC string literals is 16380 (compiler error C2026), see:
# https://docs.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/compiler-error-c2026
# The old 64K limit that caused fatal error C1091 doesn't appear to matter anymore,
# so the length after string literal concatenation can be arbitrarily long.
# We're keeping it to 8190 (= 106380 / 2) to allow for some extra length
# due to the regex replaces
set(chunklength 8190)

string(APPEND f4 "\n  const char* ${var} = ")

if(f0_LEN GREATER 0)
  while(f0_LEN GREATER 0)
    if(f0_LEN GREATER ${chunklength})
      string(SUBSTRING "${f0}" 0 ${chunklength} f3_CHUNK)
      string(REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f3_CHUNK}")
      string(REGEX REPLACE "\"" "\\\\\"" f2 "${f1}")
      string(REGEX REPLACE "\r?\n" "\\\\r\\\\n\"\n    \"" f3 "${f2}")

      string(APPEND f4 "\n    \"${f3}\"")

      math(EXPR f0_LEN "${f0_LEN} - ${chunklength}")
      string(SUBSTRING "${f0}" ${chunklength} ${f0_LEN} f0)
    else()
      string(SUBSTRING "${f0}" 0 ${f0_LEN} f3_CHUNK)
      string(REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f3_CHUNK}")
      string(REGEX REPLACE "\"" "\\\\\"" f2 "${f1}")
      string(REGEX REPLACE "\r?\n" "\\\\r\\\\n\"\n    \"" f3 "${f2}")

      string(APPEND f4 "\n    \"${f3}\"")

      set(f0_LEN 0)
    endif()
  endwhile()
  string(APPEND f4 ";")
else()
  string(APPEND "\"\";")
endif()

string(APPEND f4 "\n}\n")
file(WRITE ${outfile} "${f4}")
