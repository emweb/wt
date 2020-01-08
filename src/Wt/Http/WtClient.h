// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_WT_CLIENT_H_
#define WT_HTTP_WT_CLIENT_H_

#include <Wt/WFlags.h>
#include <string>

namespace Wt {

  namespace Http {

/*! \class WtClient Wt/Http/WtClient.h Wt/Http/WtClient.h
 *  \brief Support for a %Wt web application client.
 *
 * \note This class provides a utility to bootstrap a %Wt session, but does not
 *       deal with new developments related to the bootstrap procedure such
 *       as progressive bootstrap.
 *
 * \ingroup http
 */
class WT_API WtClient
{
public:
  /*! \brief Enumeration for client user-agent options.
   */
  enum class ClientOption {
    SupportsAjax = 0x1 //!< Flag that indicates supports for AJAX
  };

  /*! \brief Starts a %Wt session.
   *
   * A %Wt application uses a bootstrap procedure during which it
   * collects information on user agent capabilities (unless the
   * progressive bootstrap method is enabled). Therefore a session is
   * not spawned after the first request, but only after these
   * capabilities have been collected by a second request.
   *
   * This function starts a session on the specific \p host, \p port,
   * \p path, and optional \p query. These correspond to the different
   * parts of a url:
   * 
   * <tt>http://</tt><i>host</i><tt>:</tt><i>port</i>/<i>path</i><tt>?</tt>\p query.
   */
  static void startWtSession(const std::string& host,
			     const std::string& port,
			     const std::string& path,
			     const std::string& query = std::string(),
			     WFlags<ClientOption> flags
			       = ClientOption::SupportsAjax);
};

W_DECLARE_OPERATORS_FOR_FLAGS(WtClient::ClientOption)

  }
}

#endif // WT_HTTP_WtCLIENT_H_
