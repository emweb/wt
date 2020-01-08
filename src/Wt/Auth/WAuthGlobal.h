// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WAUTHGLOBAL_H_
#define WAUTHGLOBAL_H_

#include <Wt/WGlobal.h>

namespace Wt {
  namespace Auth {

/*! \brief Enumeration of the OAuth client authorization method.
 *
 * Specifies how the OAuth client authorizes itself with the
 * auth provider. I.e. how it passes the client ID and secret to the
 * provider.
 *
 * \sa OAuthService::clientSecretMethod
 * \sa OAuthClient::authMethod
 *
 * \ingroup auth
 */
enum ClientSecretMethod
{
  /*! \brief Pass the client ID and secret to the auth provider with
   * a GET request with Basic auth.
   *
   */
  HttpAuthorizationBasic,

  /*! \brief Pass the client ID and secret to the auth provider as URL
   * parameters of a GET request.
   *
   *  This is not part of the standard but this is what Facebook does.
   */
  PlainUrlParameter, // not available in combination with Post method

  /*! \brief Pass the client ID and secret to the auth provider as
   * parameters of a POST request.
   *
   */
  RequestBodyParameter
};

  }
}

#endif // WAUTHGLOBAL_H_
