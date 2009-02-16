/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <Wt/WServer>

#include "Configuration.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <mxml.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#include <process.h>
#endif

namespace {

using namespace Wt;

mxml_node_t *singleChildElement(mxml_node_t *element, const char* tagName)
{
  mxml_node_t *result = mxmlFindElement(element, element, tagName,
					0, 0, MXML_DESCEND_FIRST);

  if (result) {
    mxml_node_t *next = mxmlFindElement(result, element, tagName,
					0, 0, MXML_NO_DESCEND);
    if (next) {
      throw WServer::Exception
	(std::string("Expected only one child <") + tagName
	 + "> in <" + element->value.element.name + ">");
    }
  }

  return result;
}

bool attributeValue(mxml_node_t *element, const char *attributeName,
		    std::string& result)
{
  const char *r = mxmlElementGetAttr(element, attributeName);

  if (r) {
    result = r;

    return true;
  } else
    return false;
}

std::string elementValue(mxml_node_t *element, const char *elementName)
{
  if (element->child) {
    for (mxml_node_t *e = element->child; e != element->last_child; e = e->next)
      if (e->type != MXML_TEXT)
	throw WServer::Exception(std::string("<")
				       + elementName
				       + "> should only contain text.");

    char *r = mxmlSaveAllocString(element->child, MXML_NO_CALLBACK);
    std::string result = r;
    free(r);
    boost::trim(result);
    return result;
  } else
    return std::string();
}

std::string singleChildElementValue(mxml_node_t *element, const char* tagName,
				    const std::string& defaultValue)
{
  mxml_node_t *child = singleChildElement(element, tagName);

  if (!child)
    return defaultValue;
  else
    return elementValue(child, tagName);
}

void setBoolean(mxml_node_t *element, const char *tagName, bool& result)
{
  std::string v = singleChildElementValue(element, tagName, "");

  if (!v.empty())
    if (v == "true")
      result = true;
    else if (v == "false")
      result = false;
    else
      throw WServer::Exception("<" + std::string(tagName)
				     + ">: expecting 'true' or 'false'");
}

std::vector<mxml_node_t *> 
childElements(mxml_node_t *element, const char *tagName)
{
  std::vector<mxml_node_t *> result;

  for (mxml_node_t *r = mxmlFindElement(element, element, tagName,
					0, 0, MXML_DESCEND_FIRST);
       r;
       r = mxmlFindElement(r, element, tagName, 0, 0, MXML_NO_DESCEND)) {
    result.push_back(r);
  }
  
  return result;
}

}

namespace Wt {

EntryPoint::EntryPoint(WebSession::Type type, ApplicationCreator appCallback,
		       const std::string& path)
  : type_(type),
    appCallback_(appCallback),
    path_(path)
{ }

void EntryPoint::setPath(const std::string& path)
{
  path_ = path;
}

Configuration::Configuration(const std::string& applicationPath,
			     const std::string& configurationFile,
			     ServerType serverType,
			     const std::string& startupMessage)
  : applicationPath_(applicationPath),
    serverType_(serverType),
    sessionPolicy_(DedicatedProcess),
    numProcesses_(10),
    numThreads_(10),
    maxNumSessions_(100),
    maxRequestSize_(128),
    sessionTracking_(URL),
    reloadIsNewSession_(false),
    sessionTimeout_(600),
    valgrindPath_(""),
    allowDebug_(false),
    runDirectory_(RUNDIR),
    sessionIdLength_(16),
    xhtmlMimeType_(false),
    behindReverseProxy_(false),
    redirectMsg_("Load basic HTML"),
    serializedEvents_(false),
    pid_(getpid())
{
  logger_.addField("datetime", false);
  logger_.addField("app", false);
  logger_.addField("session", false);
  logger_.addField("type", false);
  logger_.addField("message", true);

  setupLogger(std::string());

  std::string configFile = configurationFile;

  // If no config file was given as startup option, see if there is
  // a preference in the environment
  if (configFile.empty()) {
    if (::getenv("WT_CONFIG_XML")) {
      // Environment var must contain path to config file
      configFile = getenv("WT_CONFIG_XML");
    } else {
      // No config file so far, try the default location
      std::ifstream test(WT_CONFIG_XML);
      if (!test) {
	// Just use the default configuration
	return;
      }
      configFile = WT_CONFIG_XML;
    }
  }

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

static void error_cb(const char *message)
{
  throw WServer::Exception(message);
}

void Configuration::readApplicationSettings(mxml_node_t *app)
{
  mxml_node_t *sess = singleChildElement(app, "session-management");

  if (sess) {
    mxml_node_t *dedicated = singleChildElement(sess, "dedicated-process");
    mxml_node_t *shared = singleChildElement(sess, "shared-process");
    std::string tracking = singleChildElementValue(sess, "tracking", "");
    std::string timeoutStr = singleChildElementValue(sess, "timeout", "");

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

    setBoolean(sess, "reload-is-new-session", reloadIsNewSession_);
  }

  std::string maxRequestStr
    = singleChildElementValue(app, "max-request-size", "");

  if (!maxRequestStr.empty())
    maxRequestSize_ = boost::lexical_cast<int>(maxRequestStr);

  mxml_node_t *fcgi = singleChildElement(app, "connector-fcgi");
  if (!fcgi)
    fcgi = app; // backward compatibility

  setBoolean(fcgi, "enable-debug", allowDebug_);

  valgrindPath_ = singleChildElementValue(fcgi, "valgrind-path", valgrindPath_);
  runDirectory_ = singleChildElementValue(fcgi, "run-directory", runDirectory_);

  std::string numThreadsStr = singleChildElementValue(fcgi, "num-threads", "");
  if (!numThreadsStr.empty())
    numThreads_ = boost::lexical_cast<int>(numThreadsStr);

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

  mxml_node_t *properties = singleChildElement(app, "properties");

  if (properties) {
    std::vector<mxml_node_t *> nodes = childElements(properties, "property");

    for (unsigned i = 0; i < nodes.size(); ++i) {
      mxml_node_t *property = nodes[i];

      std::string name;
      if (!attributeValue(property, "name", name))
	throw WServer::Exception("<property> requires attribute 'name'");

      std::string value = elementValue(property, "property");
      properties_[name] = value;
    }
  }
}

void Configuration::readConfiguration(const std::string& configurationFile,
				      const std::string& startupMessage)
{
  try {
    FILE *fp = fopen(configurationFile.c_str(), "r");
    if (!fp) {
      throw WServer::Exception("Configuration file not found at " +
			       configurationFile);
    }

    mxml_node_t *top = mxmlNewElement(MXML_NO_PARENT, "top");

    mxmlSetErrorCallback(error_cb);

    mxml_node_t *first = mxmlLoadFile(top, fp, MXML_NO_CALLBACK);

    if (first) {
      mxml_node_t *root = singleChildElement(top, "server");

      std::vector<mxml_node_t *> applications
	= childElements(root, "application-settings");

      /*
       * Scan the config file first to determine the logFile, in order
       * to setup logging before parsing the other settings.
       */
      std::string logFile;
      for (unsigned i = 0; i < applications.size(); ++i) {
	mxml_node_t *app = applications[i];

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
	mxml_node_t *app = applications[i];

	std::string appLocation;
	attributeValue(app, "location", appLocation);

	if (appLocation == "*" || appLocation == applicationPath_)
	  readApplicationSettings(app);
      }
    } else
      throw WServer::Exception("Malformed XML file");

    if (fp)
      fclose(fp);

    mxmlDelete(top);

  } catch (std::exception& e) {
    throw WServer::Exception(std::string("Error: ") + e.what());
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
  entryPoints_.push_back(ep);
}

std::string Configuration::generateSessionId()
{
  std::string session_id = sessionIdPrefix();

  for (int i = session_id.length(); i < sessionIdLength(); ++i) {
    // use alphanumerical characters (big and small) and numbers
    int d = random_.rand() % (26 + 26 + 10);

    char c = (d < 10 ? ('0' + d)
	      : (d < 36 ? ('A' + d - 10)
		 : 'a' + d - 36));
    session_id.push_back(c);
  }

  if (serverType_ == FcgiServer) {
    std::string socketPath = runDirectory_ + "/" + session_id;
  
    struct stat finfo;
    if (stat(socketPath.c_str(), &finfo) != -1)
      // exists already -- try another one
      return generateSessionId();
    else {
      if (sessionPolicy_ == SharedProcess) {
	std::ofstream f(socketPath.c_str());
	f << pid_ << std::endl;
	f.flush();
      }
    }
  }

  return session_id;
}

}
