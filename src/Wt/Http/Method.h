// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_METHOD_H_
#define WT_HTTP_METHOD_H_

namespace Wt {
  namespace Http {

/*! \brief Enumeration for a HTTP method
 *
 * This enumeration is currently limited to only a few of the most
 * used HTTP methods.
 */
enum class Method {
  Get,    //!< a HTTP GET
  Post,   //!< a HTTP POST
  Put,    //!< a HTTP PUT
  Delete, //!< a HTTP DELETE
  Patch,  //!< a HTTP PATCH
  Head    //!< a HTTP HEAD
};

  }
}

#endif // WT_HTTP_METHOD_H_
