/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WServer"
#include "Wt/WLogger"
#include "Wt/WRegExp"
#include "Wt/WResource"

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

#include "3rdparty/rapidxml/rapidxml.hpp"

#ifdef WT_WIN32
#include <io.h>
#include <process.h>
#endif

#ifdef WT_CONF_LOCK
#define READ_LOCK boost::shared_lock<boost::shared_mutex> lock(mutex_)
#define WRITE_LOCK boost::lock_guard<boost::shared_mutex> lock(mutex_)
#else
#define READ_LOCK
#define WRITE_LOCK
#endif // WT_CONF_LOCK

using namespace Wt::rapidxml;

namespace {

using namespace Wt;

bool regexMatchAny(const std::string& agent,
		   const std::vector<std::string>& regexList) {
  WT_USTRING s = WT_USTRING::fromUTF8(agent);
  for (unsigned i = 0; i < regexList.size(); ++i) {
    WRegExp expr(WT_USTRING::fromUTF8(regexList[i]));

    if (expr.exactMatch(s))
      return true;
  }

  return false;
}

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

void setInt(xml_node<> *element, const char *tagName, int& result)
{
  std::string v = singleChildElementValue(element, tagName, "");

  if (!v.empty()) {
    try {
      result = boost::lexical_cast<int>(v);
    } catch (boost::bad_lexical_cast& e) {
      throw WServer::Exception("<" + std::string(tagName)
			       + ">: expecting integer value");
    }
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

LOGGER("config");

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
			     const std::string& appRoot,
			     const std::string& configurationFile,
			     WServer *server)
  : server_(server),
    applicationPath_(applicationPath),
    appRoot_(appRoot),
    configurationFile_(configurationFile),
    runDirectory_(RUNDIR),
    singleSession_(false),
    connectorSlashException_(false), // need to use ?_=
    connectorNeedReadBody_(false),
    connectorWebSockets_(true)
{
  reset();
  readConfiguration(false);
}

void Configuration::reset()
{
  sessionPolicy_ = SharedProcess;
  numProcesses_ = 1;
  numThreads_ = 10;
  maxNumSessions_ = 100;
  maxRequestSize_ = 128 * 1024;
  isapiMaxMemoryRequestSize_ = 128 * 1024;
  sessionTracking_ = URL;
  reloadIsNewSession_ = true;
  sessionTimeout_ = 600;
  bootstrapTimeout_ = 10;
  indicatorTimeout_ = 500;
  doubleClickTimeout_ = 200;
  serverPushTimeout_ = 50;
  valgrindPath_ = "";
  errorReporting_ = ErrorMessage;
  if (!runDirectory_.empty()) // disabled by connector
    runDirectory_ = RUNDIR;
  sessionIdLength_ = 16;
  properties_.clear();
  xhtmlMimeType_ = false;
  behindReverseProxy_ = false;
  redirectMsg_ = "Load basic HTML";
  serializedEvents_ = false;
  webSockets_ = false;
  inlineCss_ = true;
  ajaxAgentList_.clear();
  botList_.clear();
  ajaxAgentWhiteList_ = false;
  persistentSessions_ = false;
  splitScript_ = false;
  maxPlainSessionsRatio_ = 1;
  ajaxPuzzle_ = false;
  sessionIdCookie_ = false;
  cookieChecks_ = true;
  webglDetection_ = true;
  bootstrapConfig_.clear();

  if (!appRoot_.empty())
    properties_["appRoot"] = appRoot_;
}

Configuration::SessionPolicy Configuration::sessionPolicy() const
{
  READ_LOCK;
  return sessionPolicy_;
}

int Configuration::numProcesses() const
{
  READ_LOCK;
  return numProcesses_;
}

int Configuration::numThreads() const
{
  READ_LOCK;
  return numThreads_;
}

int Configuration::maxNumSessions() const
{
  READ_LOCK;
  return maxNumSessions_;
}

::int64_t Configuration::maxRequestSize() const
{
  return maxRequestSize_;
}

::int64_t Configuration::isapiMaxMemoryRequestSize() const
{
  READ_LOCK;
  return isapiMaxMemoryRequestSize_;
}

Configuration::SessionTracking Configuration::sessionTracking() const
{
  READ_LOCK;
  return sessionTracking_;
}

bool Configuration::reloadIsNewSession() const
{
  READ_LOCK;
  return reloadIsNewSession_;
}

int Configuration::sessionTimeout() const
{
  READ_LOCK;
  return sessionTimeout_;
}

int Configuration::bootstrapTimeout() const
{
  READ_LOCK;
  return bootstrapTimeout_;
}

int Configuration::indicatorTimeout() const
{
  READ_LOCK;
  return indicatorTimeout_;
}

int Configuration::doubleClickTimeout() const
{
  READ_LOCK;
  return doubleClickTimeout_;
}

int Configuration::serverPushTimeout() const
{
  READ_LOCK;
  return serverPushTimeout_;
}

std::string Configuration::valgrindPath() const
{
  READ_LOCK;
  return valgrindPath_;
}

Configuration::ErrorReporting Configuration::errorReporting() const
{
  READ_LOCK;
  return errorReporting_;
}

bool Configuration::debug() const
{
  READ_LOCK;
  return errorReporting_ != ErrorMessage;
}

std::string Configuration::runDirectory() const
{
  READ_LOCK;
  return runDirectory_;
}

int Configuration::sessionIdLength() const
{
  READ_LOCK;
  return sessionIdLength_;
}

std::string Configuration::sessionIdPrefix() const
{
  READ_LOCK;
  return connectorSessionIdPrefix_;
}

bool Configuration::behindReverseProxy() const
{
  READ_LOCK;
  return behindReverseProxy_;
}

std::string Configuration::redirectMessage() const
{
  READ_LOCK;
  return redirectMsg_;
}

bool Configuration::serializedEvents() const
{
  READ_LOCK;
  return serializedEvents_;
}

bool Configuration::webSockets() const
{
  return webSockets_;
}

bool Configuration::inlineCss() const
{
  READ_LOCK;
  return inlineCss_;
}

bool Configuration::persistentSessions() const
{
  READ_LOCK;
  return persistentSessions_;
}

bool Configuration::progressiveBoot(const std::string& internalPath) const
{
  READ_LOCK;
  bool result = false;

  for (unsigned i = 0; i < bootstrapConfig_.size(); ++i) {
    const BootstrapEntry& e = bootstrapConfig_[i];
    if (e.prefix) {
      if (internalPath == e.path ||
	  boost::starts_with(internalPath, e.path + '/'))
	result = e.method == Progressive;
    } else
      if (internalPath == e.path)
	result = e.method == Progressive;
  }

  return result;
}

bool Configuration::splitScript() const
{
  READ_LOCK;
  return splitScript_;
}

float Configuration::maxPlainSessionsRatio() const
{
  READ_LOCK;
  return maxPlainSessionsRatio_;
}

bool Configuration::ajaxPuzzle() const
{
  READ_LOCK;
  return ajaxPuzzle_;
}

std::string Configuration::uaCompatible() const
{
  READ_LOCK;
  return uaCompatible_;
}

bool Configuration::sessionIdCookie() const
{
  READ_LOCK;
  return sessionIdCookie_;
}

bool Configuration::cookieChecks() const
{
  READ_LOCK;
  return cookieChecks_;
}

bool Configuration::useSlashExceptionForInternalPaths() const
{
  return connectorSlashException_;
}

bool Configuration::needReadBodyBeforeResponse() const
{
  return connectorNeedReadBody_;
}

bool Configuration::webglDetect() const
{
  READ_LOCK;
  return webglDetection_;
}

bool Configuration::singleSession() const
{
  return singleSession_;
}

bool Configuration::agentIsBot(const std::string& agent) const
{
  READ_LOCK;

  return regexMatchAny(agent, botList_);
}

bool Configuration::agentSupportsAjax(const std::string& agent) const
{
  READ_LOCK;

  bool matches = regexMatchAny(agent, ajaxAgentList_);
  if (ajaxAgentWhiteList_)
    return matches;
  else
    return !matches;
}

std::string Configuration::appRoot() const
{
  READ_LOCK;

  std::string approot;

  if (!readConfigurationProperty("appRoot", approot)) {
    return "";
  }

  if (!approot.empty() && approot[approot.length() - 1] != '/'
#ifdef WT_WIN32
      && approot[approot.length() - 1] != '\\'
#endif
     ) {
    approot += "/";
  }

  return approot;
}

std::string Configuration::locateAppRoot()
{
  char *value;

  if ((value = ::getenv("WT_APP_ROOT")))
    return value;
  else
    return std::string();
}

std::string Configuration::locateConfigFile(const std::string& appRoot)
{
  char *value;

  if ((value = ::getenv("WT_CONFIG_XML")))
    return value;
  else {
    // Configuration file could be $WT_APP_ROOT/wt_config.xml
    if (!appRoot.empty()) {
      std::string result = appRoot + "/wt_config.xml";
      std::ifstream s(result.c_str(), std::ios::in | std::ios::binary);
      if (s)
	return result;
    }

    return WT_CONFIG_XML;
  }
}

void Configuration::addEntryPoint(const EntryPoint& ep)
{
  if (ep.type() == StaticResource)
    ep.resource()->currentUrl_ = ep.path();

  entryPoints_.push_back(ep);
}

void Configuration::removeEntryPoint(const std::string& path)
{
  for (unsigned i = 0; i < entryPoints_.size(); ++i) {
    EntryPoint &ep = entryPoints_[i];
    if (ep.path() == path) {
      entryPoints_.erase(entryPoints_.begin() + i);
      break;
    }
  }
}

void Configuration::setDefaultEntryPoint(const std::string& path)
{
  for (unsigned i = 0; i < entryPoints_.size(); ++i)
    if (entryPoints_[i].path().empty())
      entryPoints_[i].setPath(path);
}

void Configuration::setSessionTimeout(int sessionTimeout)
{
  sessionTimeout_ = sessionTimeout;
}

void Configuration::setSessionIdPrefix(const std::string& prefix)
{
  connectorSessionIdPrefix_ = prefix;
}

void Configuration::setWebSockets(bool enabled)
{
  connectorWebSockets_ = enabled;
  if (!enabled)
    webSockets_ = false;
}

void Configuration::setNeedReadBodyBeforeResponse(bool needed)
{
  connectorNeedReadBody_ = needed;
}

void Configuration::setUseSlashExceptionForInternalPaths(bool use)
{
  connectorSlashException_ = use;
}

void Configuration::setRunDirectory(const std::string& path)
{
  runDirectory_ = path;
}

void Configuration::setNumThreads(int threads)
{
  numThreads_ = threads;
}

void Configuration::setBehindReverseProxy(bool enabled)
{
  behindReverseProxy_ = enabled;
}

void Configuration::setSingleSession(bool singleSession)
{
  singleSession_ = singleSession;
}

void Configuration::readApplicationSettings(xml_node<> *app)
{
  xml_node<> *sess = singleChildElement(app, "session-management");

  if (sess) {
    xml_node<> *dedicated = singleChildElement(sess, "dedicated-process");
    xml_node<> *shared = singleChildElement(sess, "shared-process");
    std::string tracking = singleChildElementValue(sess, "tracking", "");

    if (dedicated && shared)
      throw WServer::Exception("<application-settings> requires either "
			       "<dedicated-process> or <shared-process>, "
			       "not both");

    if (dedicated) {
      sessionPolicy_ = DedicatedProcess;
      setInt(dedicated, "max-num-sessions", maxNumSessions_);
    }

    if (shared) {
      sessionPolicy_ = SharedProcess;

      setInt(shared, "num-processes", numProcesses_);
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

    setInt(sess, "timeout", sessionTimeout_);
    setInt(sess, "bootstrap-timeout", bootstrapTimeout_);
    setInt(sess, "server-push-timeout", serverPushTimeout_);
    setBoolean(sess, "reload-is-new-session", reloadIsNewSession_);
  }

  std::string maxRequestStr
    = singleChildElementValue(app, "max-request-size", "");
  if (!maxRequestStr.empty())
    maxRequestSize_ = boost::lexical_cast< ::int64_t >(maxRequestStr) * 1024;

  std::string debugStr = singleChildElementValue(app, "debug", "");

  if (!debugStr.empty()) {
    if (debugStr == "stack" || debugStr == "false")
      errorReporting_ = ErrorMessage;
    else if (debugStr == "naked")
      errorReporting_ = NoErrors;
    else if (debugStr == "true")
      errorReporting_ = ServerSideOnly;
    else
      throw WServer::Exception("<debug>: expecting 'true', 'false',"
			       "'naked', or 'stack'");
  }

  setInt(app, "num-threads", numThreads_);

  xml_node<> *fcgi = singleChildElement(app, "connector-fcgi");
  if (!fcgi)
    fcgi = app; // backward compatibility

  valgrindPath_ = singleChildElementValue(fcgi, "valgrind-path",
					  valgrindPath_);
  runDirectory_ = singleChildElementValue(fcgi, "run-directory",
					  runDirectory_);

  setInt(fcgi, "num-threads", numThreads_); // backward compatibility < 3.2.0

  xml_node<> *isapi = singleChildElement(app, "connector-isapi");
  if (!isapi)
    isapi = app; // backward compatibility

  setInt(isapi, "num-threads", numThreads_); // backward compatibility < 3.2.0

  std::string maxMemoryRequestSizeStr =
    singleChildElementValue(isapi, "max-memory-request-size", "");
  if (!maxMemoryRequestSizeStr.empty()) {
    isapiMaxMemoryRequestSize_ = boost::lexical_cast< ::int64_t >
      (maxMemoryRequestSizeStr) * 1024;
  }

  setInt(app, "session-id-length", sessionIdLength_);

  /*
   * If a session-id-prefix is defined in the configuration file, then
   * we loose the prefix defined by the connector (e.g. wthttpd), but who
   * would do such a thing ? */
  connectorSessionIdPrefix_
    = singleChildElementValue(app,"session-id-prefix",
			      connectorSessionIdPrefix_);

  setBoolean(app, "send-xhtml-mime-type", xhtmlMimeType_);
  if (xhtmlMimeType_)
    LOG_WARN("ignoring send-xhtml-mime-type setting: HTML5 is now always used");
  redirectMsg_ = singleChildElementValue(app, "redirect-message", redirectMsg_);

  setBoolean(app, "behind-reverse-proxy", behindReverseProxy_);
  setBoolean(app, "strict-event-serialization", serializedEvents_);
  setBoolean(app, "web-sockets", webSockets_);

  setBoolean(app, "inline-css", inlineCss_);
  setBoolean(app, "persistent-sessions", persistentSessions_);

  uaCompatible_ = singleChildElementValue(app, "UA-Compatible", "");

  bool progressive = false;
  setBoolean(app, "progressive-bootstrap", progressive);

  xml_node<> *bootstrap = singleChildElement(app, "bootstrap-method");
  if (bootstrap) {
    progressive = std::string(bootstrap->value()) == "progressive";

    std::vector<xml_node<> *> entries = childElements(bootstrap, "for");
    for (unsigned i = 0; i < entries.size(); ++i) {
      xml_node<> *entry = entries[i];

      std::string path;
      if (!attributeValue(entry, "path", path) || path.empty())
	throw WServer::Exception("<for> requires attribute 'path'");

      bootstrapConfig_.push_back(BootstrapEntry());
      BootstrapEntry& e = bootstrapConfig_.back();
      
      e.prefix = path[path.length() - 1] == '*';
      e.method = std::string(entry->value()) == "progressive"
	? Progressive : DetectAjax;
      if (e.prefix) {
	e.path = path.substr(0, path.length() - 1);
	if (!e.path.empty() && e.path[e.path.length() - 1] == '/')
	  e.path.erase(e.path.length() - 1);
      } else
	e.path = path;
    }
  }

  if (progressive) {
    bootstrapConfig_.insert(bootstrapConfig_.begin(), BootstrapEntry());
    bootstrapConfig_.front().prefix = true;
    bootstrapConfig_.front().method = Progressive;
  }

  if (progressive)
    setBoolean(app, "split-script", splitScript_);
  setBoolean(app, "session-id-cookie", sessionIdCookie_);
  setBoolean(app, "cookie-checks", cookieChecks_);
  setBoolean(app, "webgl-detection", webglDetection_);

  std::string plainAjaxSessionsRatioLimit
    = singleChildElementValue(app, "plain-ajax-sessions-ratio-limit", "");

  if (!plainAjaxSessionsRatioLimit.empty())
    maxPlainSessionsRatio_
      = boost::lexical_cast<float>(plainAjaxSessionsRatioLimit);

  setBoolean(app, "ajax-puzzle", ajaxPuzzle_);
  setInt(app, "indicator-timeout", indicatorTimeout_);
  setInt(app, "double-click-timeout", doubleClickTimeout_);

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

      if (name == "appRoot" && !appRoot_.empty())
	LOG_WARN("ignoring configuration property 'appRoot' ("
		 << value
		 << ") because was already set to " << appRoot_);
      else
        properties_[name] = value;
    }
  }

  std::vector<xml_node<> *> metaHeaders = childElements(app, "meta-headers");
  for (unsigned i = 0; i < metaHeaders.size(); ++i) {
    xml_node<> *metaHeader = metaHeaders[i];

    std::string userAgent;
    attributeValue(metaHeader, "user-agent", userAgent);

    std::vector<xml_node<> *> metas = childElements(metaHeader, "meta");
    for (unsigned j = 0; j < metas.size(); ++j) {
      xml_node<> *meta = metas[j];
      
      std::string name, property, httpEquiv, content;
      attributeValue(meta, "name", name);
      attributeValue(meta, "http-equiv", httpEquiv);
      attributeValue(meta, "property", property);
      attributeValue(meta, "content", content);

      MetaHeaderType type;
      if (!name.empty())
	type = MetaName;
      else if (!httpEquiv.empty()) {
	type = MetaHttpHeader;
	name = httpEquiv;
      } else if (!property.empty()) {
	type = MetaProperty;
	name = property;
      } else {
	throw WServer::Exception
	  ("<meta> requires attribute 'name', 'property' or 'http-equiv'");
      }

      metaHeaders_.push_back(MetaHeader(type, name, content, "", userAgent));
    }
  }
}

void Configuration::rereadConfiguration()
{
  WRITE_LOCK;

  try {
    LOG_INFO("Rereading configuration...");
    Configuration conf(applicationPath_, appRoot_, configurationFile_, 0);
    reset();
    readConfiguration(true);
    LOG_INFO("New configuration read.");
  } catch (WException& e) {
    LOG_ERROR("Error reading configuration: " << e.what());
  }
}

void Configuration::readConfiguration(bool silent)
{
  std::ifstream s(configurationFile_.c_str(), std::ios::in | std::ios::binary);

  if (!s) {
    if (configurationFile_ != WT_CONFIG_XML)
      throw WServer::Exception
	("Error reading '" + configurationFile_ + "': could not open file.");
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
    std::string logConfig;
    for (unsigned i = 0; i < applications.size(); ++i) {
      xml_node<> *app = applications[i];

      std::string appLocation;
      if (!attributeValue(app, "location", appLocation))
	throw WServer::Exception("<application-settings> requires attribute "
				 "'location'");

      if (appLocation == "*" || appLocation == applicationPath_) {
	logFile = singleChildElementValue(app, "log-file", logFile);
	logConfig = singleChildElementValue(app, "log-config", logConfig);
      }
    }

    if (server_)
      server_->initLogger(logFile, logConfig);

    if (!silent)
      LOG_INFO("reading Wt config file: " << configurationFile_
	       << " (location = '" << applicationPath_ << "')");

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
    throw WServer::Exception("Error reading: " + configurationFile_ + ": "
			     + e.what());
  } catch (...) {
    throw WServer::Exception("Exception of unknown type!\n");
  }
}

bool Configuration::registerSessionId(const std::string& oldId,
				      const std::string& newId)
{
  if (!runDirectory_.empty()) {

    if (!newId.empty()) {
      std::string socketPath = sessionSocketPath(newId);

      struct stat finfo;
      if (stat(socketPath.c_str(), &finfo) != -1)
	return false;

      if (oldId.empty()) {
	if (sessionPolicy_ == SharedProcess) {
	  std::ofstream f(socketPath.c_str());
	  f << getpid() << std::endl;
	  f.flush();
	}
      }
    }

    if (!oldId.empty()) {
      if (newId.empty())
	unlink(sessionSocketPath(oldId).c_str());
      else
	std::rename(sessionSocketPath(oldId).c_str(),
	            sessionSocketPath(newId).c_str());
    }
  }

  return true;
}

std::string Configuration::generateSessionId()
{
  std::string sessionId = sessionIdPrefix();
  sessionId += WRandom::generateId(sessionIdLength() - sessionId.length());
  return sessionId;
}

std::string Configuration::sessionSocketPath(const std::string& sessionId)
{
  return runDirectory_ + "/" + sessionId;
}

bool Configuration::readConfigurationProperty(const std::string& name,
                                              std::string& value) const
{
  PropertyMap::const_iterator i = properties_.find(name);

  if (i != properties_.end()) {
    value = i->second;
    return true;
  } else
    return false;
}

WLogEntry Configuration::log(const std::string& type) const
{
  if (server_)
    return server_->log(type);
  else
    return Wt::log(type);
}

}
