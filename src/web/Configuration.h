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

#include "Wt/WApplication"
#include "Wt/WLogger"

#include "WebSession.h"
#include "WtRandom.h"

namespace rapidxml {
  template<class Ch> class xml_node;
}

namespace boost {
  namespace program_options {
    class variables_map;
  }
}

namespace Wt {

#ifndef WT_TARGET_JAVA

class WT_API EntryPoint {
 public:
  EntryPoint(EntryPointType type, ApplicationCreator appCallback,
	     const std::string& path, 
             const std::string& favicon);
  EntryPoint(WResource *resource, const std::string& path);
  ~EntryPoint();

  void setPath(const std::string& path);

  EntryPointType     type() const { return type_; }
  WResource         *resource() const { return resource_; }
  ApplicationCreator appCallback() const { return appCallback_; }
  const std::string& path() const { return path_; }
  const std::string& favicon() const { return favicon_; }

 private:
  EntryPointType     type_;
  WResource         *resource_;
  ApplicationCreator appCallback_;
  std::string        path_;
  std::string        favicon_;
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
  enum ServerType {
    WtHttpdServer,
    FcgiServer,
    IsapiServer
  };

  typedef std::map<std::string, std::string> PropertyMap;
  typedef std::vector<std::string> AgentList;

  Configuration(const std::string& applicationPath,
                const std::string& approot,
		const std::string& configurationFile,
		ServerType serverType,
		const std::string& startupMessage);

  /*
   * Override the sessionIdPrefix setting in the config file
   */
  void               setSessionIdPrefix(const std::string& prefix);

#ifndef WT_TARGET_JAVA
  void               addEntryPoint(const EntryPoint& entryPoint);
  void               setDefaultEntryPoint(const std::string& path);
  const EntryPointList& entryPoints() const { return entryPoints_; }
#endif // WT_TARGET_JAVA

  SessionPolicy      sessionPolicy() const { return sessionPolicy_; }
  int                numProcesses() const { return numProcesses_; }
  int                numThreads() const { return numThreads_; }
  int                maxNumSessions() const { return maxNumSessions_; }
  ::int64_t          maxRequestSize() const { return maxRequestSize_; }
  ::int64_t          isapiMaxMemoryRequestSize() const { return isapiMaxMemoryRequestSize_; }
  SessionTracking    sessionTracking() const { return sessionTracking_; }
  bool               reloadIsNewSession() const { return reloadIsNewSession_; }
  int                sessionTimeout() const { return sessionTimeout_; }
  int		     indicatorTimeout() const { return indicatorTimeout_; }
  int                serverPushTimeout() const { return serverPushTimeout_; }
  std::string        valgrindPath() const { return valgrindPath_; }
  bool               debug() const { return debug_; }
  std::string        runDirectory() const { return runDirectory_; }
  ServerType         serverType() const { return serverType_; }
  int                sessionIdLength() const { return sessionIdLength_; }
  std::string        sessionIdPrefix() const { return sessionIdPrefix_; }
  const PropertyMap& properties() const { return properties_; }
  const std::string* property(const std::string& name) const;
  bool               readConfigurationProperty(const std::string& name,
					       std::string& value) const;
  std::string        appRoot() const;
  bool               sendXHTMLMimeType() const { return xhtmlMimeType_; }
  bool               behindReverseProxy() const { return behindReverseProxy_; }
  std::string        redirectMessage() const { return redirectMsg_; }
  bool               serializedEvents() const { return serializedEvents_; }
  bool               webSockets() const { return webSockets_; }
  bool               inlineCss() const { return inlineCss_; }
  bool               ajaxAgentWhiteList() const { return ajaxAgentWhiteList_; }
  const AgentList&   ajaxAgentList() const { return ajaxAgentList_; }
  const AgentList&   botList() const { return botList_; }
  bool               persistentSessions() const { return persistentSessions_; }
  bool               progressiveBoot() const { return progressiveBoot_; }

  WLogger&           logger() { return logger_; }
  WLogEntry          log(const std::string& type) const;

  int                pid() const { return pid_; }

  /*
   * Generate a unique session Id.
   *
   * For a FastCGI server, this also creates a session file.
   */
  std::string generateSessionId();

  std::string sessionSocketPath(const std::string& sessionId);

private:
  std::string     applicationPath_;
  std::string     approot_;

#ifndef WT_TARGET_JAVA
  EntryPointList     entryPoints_;
#endif // WT_TARGET_JAVA

  ServerType      serverType_;
  SessionPolicy   sessionPolicy_;
  int             numProcesses_;
  int             numThreads_;
  int             maxNumSessions_;
  ::int64_t       maxRequestSize_;
  ::int64_t       isapiMaxMemoryRequestSize_;
  SessionTracking sessionTracking_;
  bool            reloadIsNewSession_;
  int             sessionTimeout_;
  int		  indicatorTimeout_;
  int             serverPushTimeout_;
  std::string     valgrindPath_;
  bool            debug_;
  std::string     runDirectory_;
  int             sessionIdLength_;
  std::string     sessionIdPrefix_;
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


  int		  pid_;
  WtRandom        random_;
  WLogger         logger_;

  void readApplicationSettings(rapidxml::xml_node<char> *app);
  void readConfiguration(const std::string& configurationFile,
			 const std::string& startupMessage);
  void setupLogger(const std::string& logFile);
};

}

#endif // HTTP_CONFIGURATION_HPP
