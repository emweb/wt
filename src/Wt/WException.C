/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException.h"

#ifdef WT_WITH_UNWIND
#define UNW_LOCAL_ONLY

#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace Wt {

std::string backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  std::stringstream ss;
  
  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }

    ss << "0x" << std::hex << pc;
    
    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }

      ss << " (" << nameptr << "+0x" << std::hex << offset << ")" << std::endl;
      
      std::free(demangled);
    } else {
      ss << " -- error: unable to obtain symbol name for this frame" << std::endl;
    }
  }

  return ss.str();
}

}
#else
namespace Wt {
std::string backtrace() {
  return "N/A";
}
}
#endif

namespace Wt {

WException::WException(const std::string& what)
#ifdef WT_WITH_UNWIND
  : what_(what + " at\n" + backtrace())
#else
  : what_(what)
#endif
{ }

WException::WException(const std::string& what,
		       const std::exception& wrapped)
#ifdef WT_WITH_UNWIND
  : what_(what + " at\n" + backtrace() + "Caused by: " + wrapped.what())
#else
  : what_(what + "\nCaused by: " + wrapped.what())
#endif
{ }

WException::~WException() throw()
{ }

void WException::setMessage(const std::string& message)
{
  what_ = message;
}

const char *WException::what() const throw()
{
  return what_.c_str();
}

}
