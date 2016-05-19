/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/Exception"

#include <boost/lexical_cast.hpp>

namespace Wt {
  namespace Dbo {

Exception::Exception(const std::string& error, const std::string& code)
  : std::runtime_error(error),
    code_(code)
{ }

Exception::~Exception() throw() { }

StaleObjectException::StaleObjectException(const std::string& id, 
					   const char *table,
					   int version)
  : Exception(std::string("Stale object, ") + table + ", id = " + id +
	      ", version = " + boost::lexical_cast<std::string>(version))
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
