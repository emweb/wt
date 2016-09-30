# Force autoconf to be at least this version number.
AC_PREREQ([2.69])

# Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
# checks. -Wall and -Werror are instructions to Automake, not gcc.
AC_INIT([helo], [0.0.1])
AC_CONFIG_SRCDIR([hello/hello.C])
AM_INIT_AUTOMAKE

# These are the files to be generated.
AC_CONFIG_FILES([Makefile hello/Makefile composer/Makefile]) # blog/Makefile])

# Safety check - list a source file that wouldn't be in other directories.

# Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([config.h])

# Check for a C++ compiler.
AC_PROG_CXX

# Check for libraries, headers, data etc here.
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([memset])
#AC_CHECK_HEADERS([stdint.h])
#AC_CHECK_HEADERS([stdlib.h])
#AC_CHECK_HEADERS([sys/time.h])
#AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_FUNC_MALLOC
AC_PROG_CC
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_CHECK_LIB([boost_system], [main])
AC_CHECK_LIB([boost_filesystem], [main])
AC_CHECK_LIB([wt], [main])
AC_CHECK_LIB([wtfcgi], [main])

# Last line: actually generate all required output files.
AC_OUTPUT

# For debug porpose
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
      yes) debug=true ;;
      no)  debug=false ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")
