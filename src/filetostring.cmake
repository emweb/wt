# This CMake script encodes a text file as a char array, that is then interpreted as a C string
# This allows us to embed files into our binaries.
# An older version would use string literals, but the standard allows compilers to limit the length
# of string literals, only requiring the limit to be at least 65536 characters, and some compilers like MSVC 2017
# enforce this. It also causes a -Woverlength-strings warning on Clang. The MSVC compiler also limits the
# maximum size of a string literal token.
# This code was inspired by, but is heavily adapted from:
# https://stackoverflow.com/a/27206982
# See N4140, annex B, 2.15
# (https://timsong-cpp.github.io/cppwp/n4140/implimits#2.15)

file(READ ${infile} filedata HEX)
# Change every pair of hex characters into a 0x literal
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
# Add line breaks every 16 characters
string(REGEX REPLACE "([^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,)" "\\1\n    " filedata ${filedata})
set(outstr "")
string(APPEND outstr "// This is automatically generated code -- do not edit!\n")
string(APPEND outstr "// Generated from ${infile}\n")
string(APPEND outstr "\n")
string(APPEND outstr "namespace {\n")
string(APPEND outstr "  const unsigned char ${var}_data[] = {\n")
string(APPEND outstr "    ${filedata}0x00\n")
string(APPEND outstr "  };\n")
string(APPEND outstr "}\n")
string(APPEND outstr "\n")
string(APPEND outstr "namespace skeletons {\n")
string(APPEND outstr "  const char* ${var} = reinterpret_cast<const char*>(${var}_data);\n")
string(APPEND outstr "}\n")
file(WRITE ${outfile} "${outstr}")
