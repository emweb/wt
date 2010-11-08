/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <Wt/WServer>
#include <Wt/WResource>

#include "Configuration.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "rapidxml/rapidxml.hpp"

#ifdef WIN32
#include <io.h>
#include <process.h>
#endif

using namespace rapidxml;

namespace {

using namespace Wt;

xml_node<> *singleChildElement(xml_node<> *element, const char* tagName)
{
  xml_node<> *result = element->first_node(tagName);
  if (result) {
    xml_node<> *next = result->next_sibling(tagName);

    if (next) {
      throw WServer::Exception
	(std::string("Expected only one child <") + tagName
	 + "> in <" + element->name() + ">");
    }
  }

  return result;
}

bool attributeValue(xml_node<> *element, const char *attributeName,
		    std::string& result)
{
  xml_attribute<> *attr = element->first_attribute(attributeName);

  if (attr) {
    result = attr->value();

    return true;
  } else
    return false;
}

std::string elementValue(xml_node<> *element, const char *elementName)
{
  for (xml_node<> *e = element->first_node(); e; e = e->next_sibling())
    if (e->type() != node_data && e->type() != node_cdata)
      throw WServer::Exception(std::string("<")
			       + elementName
			       + "> should only contain text.");

  return element->value();
}

std::string singleChildElementValue(xml_node<> *element, const char* tagName,
				    const std::string& defaultValue)
{
  xml_node<> *child = singleChildElement(element, tagName);

  if (!child)
    return defaultValue;
  else
    return elementValue(child, tagName);
}

void setBoolean(xml_node<> *element, const char *tagName, bool& result)
{
  std::string v = singleChildElementValue(element, tagName, "");

  if (!v.empty()) {
    if (v == "true")
      result = true;
    else if (v == "false")
      result = false;
    else
      throw WServer::Exception("<" + std::string(tagName)
			       + ">: expecting 'true' or 'false'");
  }
}

std::vector<xml_node<> *> childElements(xml_node<> *element,
					const char *tagName)
{
  std::vector<xml_node<> *> result;

  for (xml_node<> *r = element->first_node(tagName); r;
       r = r->next_sibling(tagName))
    result.push_back(r);
  
  return result;
}

}

namespace Wt {

EntryPoint::EntryPoint(EntryPointType type, ApplicationCreator appCallback,
		       const std::string& path, const std::string& favicon)
  : type_(type),
    resource_(0),
    appCallback_(appCallback),
    path_(path),
    favicon_(favicon)
{ }

EntryPoint::EntryPoint(WResource *resource, const std::string& path)
  : type_(StaticResource),
    resource_(resource),
    appCallback_(0),
    path_(path)
{ }

EntryPoint::~EntryPoint()
{
}

void EntryPoint::setPath(const std::string& path)
{
  path_ = path;
}

Configuration::Configuration(const std::string& applicationPath,
                             const std::string& approot,
			     const std::string& configurationFile,
			     ServerType serverType,
			     const std::string& startupMessage)
  : applicationPath_(applicationPath),
    approot_(approot),
    serverType_(serverType),
    sessionPolicy_(SharedProcess),
    numProcesses_(1),
    numThreads_(serverType == WtHttpdServer ? 0 : 10),
    maxNumSessions_(100),
    maxRequestSize_(128 * 1024),
    isapiMaxMemoryRequestSize_(128 * 1024),
    sessionTracking_(URL),
    reloadIsNewSession_(true),
    sessionTimeout_(600),
    indicatorTimeout_(500),
    serverPushTimeout_(50),
    valgrindPath_(""),
    debug_(false),
    runDirectory_(RUNDIR),
    sessionIdLength_(16),
    xhtmlMimeType_(false),
    behindReverseProxy_(false),
    redirectMsg_("Load basic HTML"),
    serializedEvents_(false),
    webSockets_(false),
    inlineCss_(true),
    ajaxAgentWhiteList_(false),
    persistentSessions_(false),
    progressiveBoot_(false),
    pid_(getpid())
{
  char *value = 0;

  if ((value = ::getenv("WT_APP_ROOT"))) {
    // Configuration file could be $WT_APP_ROOT/wt_config.xml
    approot_ = value;
  }

  if (approot_ != "")
    properties_["appRoot"] = approot_;

  logger_.addField("datetime", false);
  logger_.addField("app", false);
  logger_.addField("session", false);
  logger_.addField("type", false);
  logger_.addField("message", true);

  setupLogger(std::string());

  std::string configFile;

  if (!configurationFile.empty())
    configFile = configurationFile;
  else if ((value = ::getenv("WT_CONFIG_XML")))
    configFile = value;
  else if (!approot_.empty()) {
    std::ifstream s((approot_ + "/wt_config.xml").c_str(),
		    std::ios::in | std::ios::binary);
    if (s)
      configFile = approot_ + "/wt_config.xml";
  }

  if (configFile.empty())
    configFile = WT_CONFIG_XML;

  std::cerr << "Reading: " << configFile << std::endl;

  readConfiguration(configFile, startupMessage);
}

void Configuration::setSessionIdPrefix(const std::string& prefix)
{
  sessionIdPrefix_ = prefix;
}

void Configuration::setDefaultEntryPoint(const std::string& path)
{
  for (unsigned i = 0; i < entryPoints_.size(); ++i)
    if (entryPoints_[i].path().empty())
      entryPoints_[i].setPath(path);
}

void Configuration::readApplicationSettings(xml_node<> *app)
{
  xml_node<> *sess = singleChildElement(app, "session-management");

  if (sess) {
    xml_node<> *dedicated = singleChildElement(sess, "dedicated-process");
    xml_node<> *shared = singleChildElement(sess, "shared-process");
    std::string tracking = singleChildElementValue(sess, "tracking", "");
    std::string timeoutStr = singleChildElementValue(sess, "timeout", "");
    std::string serverPushTimeoutStr
      = singleChildElementValue(sess, "server-push-timeout", "");

    if (dedicated && shared)
      throw WServer::Exception("<application-settings> requires either "
			       "<dedicated-process> or <shared-process>, "
			       "not both");

    if (dedicated) {
      sessionPolicy_ = DedicatedProcess;

      std::string maxnumStr
	= singleChildElementValue(dedicated, "max-num-sessions", "");

      if (!maxnumStr.empty())
	maxNumSessions_ = boost::lexical_cast<int>(maxnumStr);
    }

    if (shared) {
      sessionPolicy_ = SharedProcess;

      std::string numProcessesStr
	= singleChildElementValue(shared, "num-processes", "");

      if (!numProcessesStr.empty())
	numProcesses_ = boost::lexical_cast<int>(numProcessesStr);
    }

    if (!tracking.empty()) {
      if (tracking == "Auto")
	sessionTracking_ = CookiesURL;
      else if (tracking == "URL")
	sessionTracking_ = URL;
      else
	throw WServer::Exception("<session-tracking>: expecting 'Auto' "
				 "or 'URL'");
    }

    if (!timeoutStr.empty())
      sessionTimeout_ = boost::lexical_cast<int>(timeoutStr);

    if (!serverPushTimeoutStr.empty())
      serverPushTimeout_ = boost::lexical_cast<int>(serverPushTimeoutStr);

    setBoolean(sess, "reload-is-new-session", reloadIsNewSession_);
  }

  std::string maxRequestStr
    = singleChildElementValue(app, "max-request-size", "");

  if (!maxRequestStr.empty())
    maxRequestSize_ = boost::lexical_cast< ::int64_t >(maxRequestStr) * 1024;

  setBoolean(app, "debug", debug_);

  if (serverType_ == FcgiServer) {
    xml_node<> *fcgi = singleChildElement(app, "connector-fcgi");
    if (!fcgi)
      fcgi = app; // backward compatibility

    valgrindPath_ = singleChildElementValue(fcgi, "valgrind-path", valgrindPath_);
    runDirectory_ = singleChildElementValue(fcgi, "run-directory", runDirectory_);

    std::string numThreadsStr = singleChildElementValue(fcgi, "num-threads", "");
    if (!numThreadsStr.empty())
      numThreads_ = boost::lexical_cast<int>(numThreadsStr);
  }
  if (serverType_ == IsapiServer) {
    xml_node<> *isapi = singleChildElement(app, "connector-isapi");
    if (!isapi)
      isapi = app; // backward compatibility

    std::string numThreadsStr = singleChildElementValue(isapi, "num-threads", "");
    if (!numThreadsStr.empty())
      numThreads_ = boost::lexical_cast<int>(numThreadsStr);

    std::string maxMemoryRequestSizeStr =
      singleChildElementValue(isapi, "max-memory-request-size", "");
    if (!maxMemoryRequestSizeStr.empty()) {
      isapiMaxMemoryRequestSize_ = boost::lexical_cast< ::int64_t >(maxMemoryRequestSizeStr) * 1024;
    }
  }

  std::string sessionIdLength
    = singleChildElementValue(app, "session-id-length", "");
  if (!sessionIdLength.empty())
    sessionIdLength_ = boost::lexical_cast<int>(sessionIdLength);

  sessionIdPrefix_
    = singleChildElementValue(app,"session-id-prefix", sessionIdPrefix_);

  setBoolean(app, "send-xhtml-mime-type", xhtmlMimeType_);
  redirectMsg_ = singleChildElementValue(app, "redirect-message", redirectMsg_);

  setBoolean(app, "behind-reverse-proxy", behindReverseProxy_);
  setBoolean(app, "strict-event-serialization", serializedEvents_);
  setBoolean(app, "web-sockets", webSockets_);

  if (webSockets_ && serverType_ != WtHttpdServer)
    throw WServer::Exception("<web-sockets> only supported by built-in "
			     "httpd connector.");

  setBoolean(app, "inline-css", inlineCss_);
  setBoolean(app, "persistent-sessions", persistentSessions_);
  setBoolean(app, "progressive-bootstrap", progressiveBoot_);

  std::string indicatorTimeoutStr
    = singleChildElementValue(app, "indicator-timeout", "");

  if (!indicatorTimeoutStr.empty())
    indicatorTimeout_ = boost::lexical_cast<int>(indicatorTimeoutStr);

  std::vector<xml_node<> *> userAgents = childElements(app, "user-agents");

  for (unsigned i = 0; i < userAgents.size(); ++i) {
    xml_node<> *userAgentsList = userAgents[i];

    std::string type;
    if (!attributeValue(userAgentsList, "type", type))
      throw WServer::Exception("<user-agents> requires attribute 'type'");

    std::string mode;
    attributeValue(userAgentsList, "mode", mode);
    
    AgentList *list;
    if (type == "ajax") {
      list = &ajaxAgentList_;
      if (mode == "black-list")
	ajaxAgentWhiteList_ = false;
      else if (mode == "white-list")
	ajaxAgentWhiteList_ = true;
      else
	throw WServer::Exception
	  ("<user-agents type=\"ajax\" requires attribute 'mode' with value "
	   "\"white-list\" or \"black-list\"");
    } else if (type == "bot")
      list = &botList_;
    else
      throw WServer::Exception
	("<user-agents> requires attribute 'type' with value "
	 "\"ajax\" or \"bot\"");

    std::vector<xml_node<> *> agents
      = childElements(userAgentsList, "user-agent");

    for (unsigned j = 0; j < agents.size(); ++j)
      list->push_back(elementValue(agents[j], "user-agent"));
  }

  xml_node<> *properties = singleChildElement(app, "properties");

  if (properties) {
    std::vector<xml_node<> *> nodes = childElements(properties, "property");

    for (unsigned i = 0; i < nodes.size(); ++i) {
      xml_node<> *property = nodes[i];

      std::string name;
      if (!attributeValue(property, "name", name))
	throw WServer::Exception("<property> requires attribute 'name'");

      std::string value = elementValue(property, "property");

      if (name == "approot")
	name = "appRoot";

      if (name == "appRoot" && approot_ != "") {
        log("warning") << "Ignoring configuration property 'appRoot' (" << value
          << ") because the connector has set it to " << approot_;
      } else {
        properties_[name] = value;
      }
    }
  }
}

void Configuration::readConfiguration(const std::string& configurationFile,
				      const std::string& startupMessage)
{
  std::ifstream s(configurationFile.c_str(), std::ios::in | std::ios::binary);
  if (!s) {
    if (configurationFile != WT_CONFIG_XML)
      throw WServer::Exception("Error reading '"
			       + configurationFile + "': could not open file.");
    else
      return;
  }

  s.seekg(0, std::ios::end);
  int length = s.tellg();
  s.seekg(0, std::ios::beg);

  boost::scoped_array<char> text(new char[length + 1]);
  s.read(text.get(), length);
  s.close();
  text[length] = 0;

  try {
    xml_document<> doc;
    doc.parse<parse_normalize_whitespace
      | parse_trim_whitespace
      | parse_validate_closing_tags>(text.get());

    xml_node<> *root = doc.first_node();

    if (!root)
      throw WServer::Exception("<server> expected.");

    std::vector<xml_node<> *> applications
      = childElements(root, "application-settings");

    /*
     * Scan the config file first to determine the logFile, in order
     * to setup logging before parsing the other settings.
     */
    std::string logFile;
    for (unsigned i = 0; i < applications.size(); ++i) {
      xml_node<> *app = applications[i];

      std::string appLocation;
      if (!attributeValue(app, "location", appLocation))
	throw WServer::Exception("<application-settings> requires attribute "
				 "'location'");

      if (appLocation == "*" || appLocation == applicationPath_)
	logFile = singleChildElementValue(app, "log-file", logFile);
    }

    setupLogger(logFile);

    if (!startupMessage.empty())
      log("notice") << startupMessage;
    log("notice") << "Reading Wt config file: " << configurationFile
		  << " (location = '" << applicationPath_ << "')";

    /*
     * Now read application settings.
     */
    for (unsigned i = 0; i < applications.size(); ++i) {
      xml_node<> *app = applications[i];

      std::string appLocation;
      attributeValue(app, "location", appLocation);

      if (appLocation == "*" || appLocation == applicationPath_)
	readApplicationSettings(app);
    }
  } catch (std::exception& e) {
    throw WServer::Exception("Error reading: " + configurationFile + ": "
			     + e.what());
  } catch (...) {
    throw WServer::Exception("Exception of unknown type!\n");
  }
}

void Configuration::setupLogger(const std::string& logFile)
{
  if (logFile.empty())
    logger_.setStream(std::cerr);
  else
    logger_.setFile(logFile);
}

WLogEntry Configuration::log(const std::string& type) const
{
  WLogEntry e = logger_.entry();

  e << WLogger::timestamp << WLogger::sep
    << pid_ << WLogger::sep
    << /* sessionId << */ WLogger::sep
    << '[' << type << ']' << WLogger::sep;

  return e;
}

void Configuration::addEntryPoint(const EntryPoint& ep)
{
  if (ep.type() == StaticResource)
    ep.resource()->currentUrl_ = ep.path();

  entryPoints_.push_back(ep);
}

std::string Configuration::generateSessionId()
{
  std::string sessionId = sessionIdPrefix();

  for (int i = sessionId.length(); i < sessionIdLength(); ++i) {
    // use alphanumerical characters (big and small) and numbers
    int d = random_.rand() % (26 + 26 + 10);

    char c = (d < 10 ? ('0' + d)
	      : (d < 36 ? ('A' + d - 10)
		 : 'a' + d - 36));
    sessionId.push_back(c);
  }

  if (serverType_ == FcgiServer) {
    std::string socketPath = sessionSocketPath(sessionId);

    struct stat finfo;
    if (stat(socketPath.c_str(), &finfo) != -1)
      // exists already -- try another one
      return generateSessionId();
  }

  return sessionId;
}

std::string Configuration::sessionSocketPath(const std::string& sessionId)
{
  return runDirectory_ + "/" + sessionId;
}

const std::string* Configuration::property(const std::string& name) const 
{
  PropertyMap::const_iterator i = properties_.find(name);

  if (i != properties_.end()) 
    return &i->second;
  else
    return 0;
}

bool Configuration::readConfigurationProperty(const std::string& name,
                                              std::string& value) const
{
  const std::string* prop = property(name);

  if (prop) {
    value = *prop;
    return true;
  } else
    return false;
}

std::string Configuration::appRoot() const
{
  std::string approot;

  if (!readConfigurationProperty("appRoot", approot)) {
    return "";
  }

  if (!approot.empty() && approot[approot.length() - 1] != '/'
#ifdef WIN32
      && approot[approot.length() - 1] != '\\'
#endif
     ) {
    approot += "/";
  }

  return approot;
}

}
