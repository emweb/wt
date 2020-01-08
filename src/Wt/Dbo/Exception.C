/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/Exception.h"

#ifdef WT_WITH_UNWIND
#define UNW_LOCAL_ONLY

#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace Wt {
  namespace Dbo {

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
}

#else

namespace Wt {
  namespace Dbo {

std::string backtrace() {
  return "N/A";
}

  }
}

#endif

namespace Wt {
  namespace Dbo {

Exception::Exception(const std::string& error, const std::string& code)
#ifdef WT_WITH_UNWIND
  : std::runtime_error(error + " at\n" + backtrace()),
#else
  : std::runtime_error(error),
#endif
    code_(code)
{
}

Exception::~Exception() throw() { }

StaleObjectException::StaleObjectException(const std::string& id, 
					   const char *table,
					   int version)
  : Exception(std::string("Stale object, ") + table + ", id = " + id +
              ", version = " + std::to_string(version))
{ }

ObjectNotFoundException::ObjectNotFoundException(const char *table,
						 const std::string& id)
  : Exception(std::string("Object not found in ") + table + ", id = " + id)
{ }

NoUniqueResultException::NoUniqueResultException()
  : Exception("Query: resultValue(): more than one result")
{ }
  }
}
