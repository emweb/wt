// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#if defined(WT_THREADED) && !defined(WT_CONF_NO_SHARED_LOCK)
#if _MSC_VER >= 1900 || __cplusplus >= 201703L
// we're using Visual Studio 2015 or higher, so we can use std::shared_mutex
#define WT_STD_CONF_LOCK
#define WT_CONF_LOCK
#else
#define WT_BOOST_CONF_LOCK
#define WT_CONF_LOCK
#endif
#endif

#ifdef WT_CONF_LOCK
#include <thread>
#endif // WT_CONF_LOCK

#ifdef WT_STD_CONF_LOCK
#include <shared_mutex>
#endif  // WT_STD_CONF_LOCK

#ifdef WT_BOOST_CONF_LOCK
#include <boost/thread/shared_mutex.hpp>
#endif // WT_BOOST_CONF_LOCK

#include "Wt/WApplication.h"

#include "WebSession.h"
#include "Wt/WRandom.h"

#ifndef WT_TARGET_JAVA
#include "Wt/AsioWrapper/asio.hpp"
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
#include "EntryPoint.h"
#endif // WT_TARGET_JAVA

namespace boost {
  namespace program_options {
    class variables_map;
  }
}

namespace Wt {
  namespace rapidxml {
    template<class Ch> class xml_node;
  }

  class WLogger;
  class WServer;

#ifndef WT_TARGET_JAVA

/// A segment in the deployment path of an entry point,
/// used for routing.
struct WT_API PathSegment {
  PathSegment()
    : parent(nullptr),
      entryPoint(nullptr)
  { }

  PathSegment(const std::string &s,
              PathSegment *p)
    : parent(p),
      entryPoint(nullptr),
      segment(s)
  { }

  PathSegment *parent;
  const EntryPoint *entryPoint;
  std::vector<std::unique_ptr<PathSegment>> children; // Static path segments
  std::unique_ptr<PathSegment> dynamicChild; // Dynamic path segment, lowest priority
  std::string segment;
};

#endif // WT_TARGET_JAVA

class WT_API HeadMatter {
public:
  HeadMatter(std::string contents,
             std::string userAgent);

  const std::string& contents() const { return contents_; }
  const std::string& userAgent() const { return userAgent_; }

private:
  std::string contents_;
  std::string userAgent_;
};

class WT_API Configuration
{
public:
  enum SessionPolicy {
    DedicatedProcess,
    SharedProcess
  };

  enum SessionTracking { 
    CookiesURL, // Use cookies if available, or fallback to URL-based,
		// does not support multiple sessions in same browser when
		// using cookies
    URL, // Use URL-based session tracking, support multiple sessions in the same
	 // browser
    Combined // Use a combination of multi-session cookie + URL-based session tracking
	     // Will error if cookies are not available (no fallback)
	     // This should be the most secure option, and supports multiple sessions
	     // in the same browser.
  };

  enum ErrorReporting {
    NoErrors, /* do not even catch them */
    ServerSideOnly,
    ErrorMessage
  };

  enum BootstrapMethod {
    DetectAjax,
    Progressive
  };

  typedef std::map<std::string, std::string> PropertyMap;
  typedef std::vector<std::string> AgentList;

  Configuration(const std::string& applicationPath,
		const std::string& appRoot,
		const std::string& configurationFile,
		WServer *server);

  void rereadConfiguration();

  // finds a configured approot based on the environment
  static std::string locateAppRoot();

  // finds a config file based on the environment, in the approot,
  // or the default location
  static std::string locateConfigFile(const std::string& appRoot);

  /*
   * Override the sessionIdPrefix setting in the config file
   */
  void setSessionIdPrefix(const std::string& prefix);

#ifndef WT_TARGET_JAVA
  void addEntryPoint(const EntryPoint& entryPoint);
  bool tryAddResource(const EntryPoint& entryPoint); // Returns bool indicating success:
						     // false if entry point existed already
  void removeEntryPoint(const std::string& path);
  void setDefaultEntryPoint(const std::string& path);
  // Returns matching entry point and match length
  EntryPointMatch matchEntryPoint(const std::string &scriptName,
                                  const std::string &path,
                                  bool matchAfterSlash) const;
  static bool matchesPath(const std::string &path,
                          const std::string &prefix,
		          bool matchAfterSlash);
  void setNumThreads(int threads);
#endif // WT_TARGET_JAVA

  const std::vector<MetaHeader>& metaHeaders() const { return metaHeaders_; }
  const std::vector<HeadMatter>& headMatter() const { return headMatter_; }
  SessionPolicy sessionPolicy() const;
  int numProcesses() const;
  int numThreads() const;
  int maxNumSessions() const;
  ::int64_t maxRequestSize() const;
  ::int64_t maxFormDataSize() const;
  int maxPendingEvents() const;
  ::int64_t isapiMaxMemoryRequestSize() const;
  SessionTracking sessionTracking() const;
  bool reloadIsNewSession() const;
  int sessionTimeout() const;
  int idleTimeout() const;
  int keepAlive() const; // sessionTimeout() / 2, or if sessionTimeout == -1, 1000000
  int multiSessionCookieTimeout() const; // sessionTimeout() * 2
  int bootstrapTimeout() const;
  int indicatorTimeout() const;
  int doubleClickTimeout() const;
  int serverPushTimeout() const;
  std::string valgrindPath() const;
  ErrorReporting errorReporting() const;
  bool debug() const;
  std::string runDirectory() const;
  int sessionIdLength() const;
  std::string sessionIdPrefix() const;
  int fullSessionIdLength() const; // length of prefix + session id
  int numSessionThreads() const;

  bool isAllowedOrigin(const std::string &origin) const;

#ifndef WT_TARGET_JAVA
  bool readConfigurationProperty(const std::string& name, std::string& value)
    const;
#else
  const std::string *property(const std::string& name) const;
#endif

#ifndef WT_TARGET_JAVA
  using IpAddress = AsioWrapper::asio::ip::address;
#else
  struct IpAddress {};
#endif

  struct WT_API Network {
      IpAddress address;
      unsigned char prefixLength;

#ifndef WT_TARGET_JAVA
      bool operator==(const Network &other) const noexcept
      {
        return address == other.address && prefixLength == other.prefixLength;
      }

      bool operator!=(const Network &other) const noexcept
      {
        return !operator==(other);
      }
#endif

      static Network fromString(const std::string &s);
      bool contains(const IpAddress &address) const;
  };

  void setAppRoot(const std::string& path);
  std::string appRoot() const;
  bool behindReverseProxy() const; // Deprecated
  std::string originalIPHeader() const;
  std::vector<Network> trustedProxies() const;
  bool isTrustedProxy(const std::string &ipAddress) const;
  std::string redirectMessage() const;
  bool serializedEvents() const;
  bool webSockets() const;
  bool inlineCss() const;
  bool persistentSessions() const;
  bool progressiveBoot(const std::string& internalPath) const;
  bool splitScript() const;
  float maxPlainSessionsRatio() const;
  bool ajaxPuzzle() const;
  bool sessionIdCookie() const;
  bool cookieChecks() const;
  bool useSlashExceptionForInternalPaths() const;
  bool needReadBodyBeforeResponse() const;
  bool webglDetect() const;

  bool agentIsBot(const std::string& agent) const;
  bool agentSupportsAjax(const std::string& agent) const;
  std::string uaCompatible() const;

  // Things which are overridden by the connector
  void setSessionTimeout(int sessionTimeout);
  void setWebSockets(bool enabled);
  void setRunDirectory(const std::string& path);
  void setUseSlashExceptionForInternalPaths(bool enabled);
  void setNeedReadBodyBeforeResponse(bool needed);
  void setBehindReverseProxy(bool enabled);
  void setOriginalIPHeader(const std::string &originalIPHeader);
  void setTrustedProxies(const std::vector<Network> &trustedProxies);

  std::string generateSessionId();
  bool registerSessionId(const std::string& oldId, const std::string& newId);

  std::string sessionSocketPath(const std::string& sessionId);
  const std::string &defaultEntryPoint() const { return defaultEntryPoint_; }
private:
  struct BootstrapEntry {
    bool prefix;
    std::string path;
    BootstrapMethod method;
  };

#ifdef WT_STD_CONF_LOCK
  mutable std::shared_mutex mutex_;
#endif // WT_STD_CONF_LOCK
#ifdef WT_BOOST_CONF_LOCK
  mutable boost::shared_mutex mutex_;
#endif // WT_BOOST_CONF_LOCK

  WServer *server_;
  std::string applicationPath_;
  std::string appRoot_;
  std::string configurationFile_;
  std::string uaCompatible_;

#ifndef WT_TARGET_JAVA
  EntryPointList entryPoints_;
  PathSegment rootPathSegment_; /// The toplevel path segment ('/') for routing,
                                /// root of the rounting tree.
#endif // WT_TARGET_JAVA

  SessionPolicy   sessionPolicy_;
  int             numProcesses_;
  int             numThreads_;
  int             maxNumSessions_;
  ::int64_t       maxRequestSize_;
  ::int64_t       maxFormDataSize_;
  int             maxPendingEvents_;
  ::int64_t       isapiMaxMemoryRequestSize_;
  SessionTracking sessionTracking_;
  bool            reloadIsNewSession_;
  int             sessionTimeout_;
  int             idleTimeout_;
  int             bootstrapTimeout_;
  int		  indicatorTimeout_;
  int             doubleClickTimeout_;
  int             serverPushTimeout_;
  std::string     valgrindPath_;
  ErrorReporting  errorReporting_;
  std::string     runDirectory_;
  int             sessionIdLength_;
  PropertyMap     properties_;
  bool            xhtmlMimeType_;
  bool            behindReverseProxy_;

  // trusted-proxy-config
  std::string     originalIPHeader_;
  std::vector<Network> trustedProxies_;

  std::string     redirectMsg_;
  bool            serializedEvents_;
  bool		  webSockets_;
  bool            inlineCss_;
  AgentList       ajaxAgentList_, botList_;
  bool            ajaxAgentWhiteList_;
  bool            persistentSessions_;
  bool            splitScript_;
  float           maxPlainSessionsRatio_;
  bool            ajaxPuzzle_;
  bool            sessionIdCookie_;
  bool            cookieChecks_;
  bool            webglDetection_;
  int             numSessionThreads_;

  std::vector<std::string> allowedOrigins_;

  std::vector<BootstrapEntry> bootstrapConfig_;
  std::vector<MetaHeader> metaHeaders_;
  std::vector<HeadMatter> headMatter_;

  bool connectorSlashException_;
  bool connectorNeedReadBody_;
  bool connectorWebSockets_;
  std::string connectorSessionIdPrefix_;
  std::string defaultEntryPoint_;

  void reset();
  void readApplicationSettings(Wt::rapidxml::xml_node<char> *app);
  void readConfiguration(bool silent);
  WLogEntry log(const std::string& type) const;

  // Add the given entryPoint to the routing tree
  // NOTE: Server may not be running, or WRITE_LOCK should
  // be grabbed before registerEntryPoint is invoked.
  // This is to be used by the other entry point functions
  // (addEntryPoint, tryAddResource, removeEntryPoint,...)
  void registerEntryPoint(const EntryPoint &entryPoint);
};

}

#endif // HTTP_CONFIGURATION_HPP
