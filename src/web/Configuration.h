// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <exception>
#include <iostream>
#include <string>

#if defined(WT_THREADED) && !defined(WT_CONF_NO_SHARED_LOCK)
#define WT_CONF_LOCK
#endif

#ifdef WT_CONF_LOCK
#include <boost/thread.hpp>
#endif // WT_CONF_LOCK

#include "Wt/WApplication"

#include "WebSession.h"
#include "Wt/WRandom"

namespace rapidxml {
  template<class Ch> class xml_node;
}

namespace boost {
  namespace program_options {
    class variables_map;
  }
}

namespace Wt {
  class WLogger;
  class WServer;

#ifndef WT_TARGET_JAVA

class WT_API EntryPoint {
public:
  EntryPoint(EntryPointType type, ApplicationCreator appCallback,
	     const std::string& path, 
             const std::string& favicon);
  EntryPoint(WResource *resource, const std::string& path);
  ~EntryPoint();

  void setPath(const std::string& path);

  EntryPointType type() const { return type_; }
  WResource *resource() const { return resource_; }
  ApplicationCreator appCallback() const { return appCallback_; }
  const std::string& path() const { return path_; }
  const std::string& favicon() const { return favicon_; }

private:
  EntryPointType type_;
  WResource *resource_;
  ApplicationCreator appCallback_;
  std::string path_;
  std::string favicon_;
};

typedef std::vector<EntryPoint> EntryPointList;

#endif // WT_TARGET_JAVA

class WT_API Configuration
{
public:
  enum SessionPolicy {
    DedicatedProcess,
    SharedProcess
  };

  enum SessionTracking { 
    CookiesURL,
    URL
  };

  enum ErrorReporting {
    NoErrors,
    ErrorMessage,
    ErrorMessageWithStack
  };

  typedef std::map<std::string, std::string> PropertyMap;
  typedef std::vector<std::string> AgentList;

  Configuration(const std::string& applicationPath,
		const std::string& appRoot,
		const std::string& configurationFile,
		WServer *server);

  void rereadConfiguration();

  // finds an approot from the environment
  static std::string locateAppRoot();

  // finds a config file from the environment, in the approot, or the default
  // location
  static std::string locateConfigFile(const std::string& appRoot);

  /*
   * Override the sessionIdPrefix setting in the config file
   */
  void setSessionIdPrefix(const std::string& prefix);

#ifndef WT_TARGET_JAVA
  void addEntryPoint(const EntryPoint& entryPoint);
  void setDefaultEntryPoint(const std::string& path);
  const EntryPointList& entryPoints() const { return entryPoints_; }
#endif // WT_TARGET_JAVA

  SessionPolicy sessionPolicy() const;
  int numProcesses() const;
  int numThreads() const;
  int maxNumSessions() const;
  ::int64_t maxRequestSize() const;
  ::int64_t isapiMaxMemoryRequestSize() const;
  SessionTracking sessionTracking() const;
  bool reloadIsNewSession() const;
  int sessionTimeout() const;
  int bootstrapTimeout() const;
  int indicatorTimeout() const;
  int serverPushTimeout() const;
  std::string valgrindPath() const;
  ErrorReporting errorReporting() const;
  bool debug() const;
  std::string runDirectory() const;
  int sessionIdLength() const;
  std::string sessionIdPrefix() const;

#ifndef WT_TARGET_JAVA
  bool readConfigurationProperty(const std::string& name, std::string& value)
    const;
#else
  const std::string *property(const std::string& name) const;
#endif

  std::string appRoot() const;
  bool sendXHTMLMimeType() const;
  bool behindReverseProxy() const;
  std::string redirectMessage() const;
  bool serializedEvents() const;
  bool webSockets() const;
  bool inlineCss() const;
  bool persistentSessions() const;
  bool progressiveBoot() const;
  bool splitScript() const;
  float maxPlainSessionsRatio() const;
  bool ajaxPuzzle() const;
  bool sessionIdCookie() const;
  bool useSlashExceptionForInternalPaths() const;
  bool needReadBodyBeforeResponse() const;

  bool agentIsBot(const std::string& agent) const;
  bool agentSupportsAjax(const std::string& agent) const;

  // Things which are overridden by the connector
  void setSessionTimeout(int sessionTimeout);
  void setWebSockets(bool enabled);
  void setRunDirectory(const std::string& path);
  void setUseSlashExceptionForInternalPaths(bool enabled);
  void setNeedReadBodyBeforeResponse(bool needed);

  std::string generateSessionId();
  bool registerSessionId(const std::string& oldId, const std::string& newId);

  std::string sessionSocketPath(const std::string& sessionId);

private:
#ifdef WT_CONF_LOCK
  mutable boost::shared_mutex mutex_;
#endif // WT_CONF_LOCK

  WServer *server_;
  std::string applicationPath_;
  std::string appRoot_;
  std::string configurationFile_;

#ifndef WT_TARGET_JAVA
  EntryPointList entryPoints_;
#endif // WT_TARGET_JAVA

  SessionPolicy   sessionPolicy_;
  int             numProcesses_;
  int             numThreads_;
  int             maxNumSessions_;
  ::int64_t       maxRequestSize_;
  ::int64_t       isapiMaxMemoryRequestSize_;
  SessionTracking sessionTracking_;
  bool            reloadIsNewSession_;
  int             sessionTimeout_;
  int             bootstrapTimeout_;
  int		  indicatorTimeout_;
  int             serverPushTimeout_;
  std::string     valgrindPath_;
  ErrorReporting  errorReporting_;
  std::string     runDirectory_;
  int             sessionIdLength_;
  PropertyMap     properties_;
  bool            xhtmlMimeType_;
  bool            behindReverseProxy_;
  std::string     redirectMsg_;
  bool            serializedEvents_;
  bool		  webSockets_;
  bool            inlineCss_;
  AgentList       ajaxAgentList_, botList_;
  bool            ajaxAgentWhiteList_;
  bool            persistentSessions_;
  bool            progressiveBoot_;
  bool            splitScript_;
  float           maxPlainSessionsRatio_;
  bool            ajaxPuzzle_;
  bool            sessionIdCookie_;

  bool connectorSlashException_;
  bool connectorNeedReadBody_;
  bool connectorWebSockets_;
  std::string connectorSessionIdPrefix_;

  void reset();
  void readApplicationSettings(rapidxml::xml_node<char> *app);
  void readConfiguration(bool silent);
  WLogEntry log(const std::string& type) const;
};

}

#endif // HTTP_CONFIGURATION_HPP
