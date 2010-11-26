/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WConfig.h"
#include "Wt/WLogger"
#include "Wt/WServer"

#include "Configuration.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef WIN32
#include <process.h> // for getpid()
#include <Winsock2.h> // for gethostname()
#endif
#include <iostream>
#include <fstream>

#ifdef __CYGWIN__
#include <Winsock2.h> // for gethostname()
#endif

namespace http {
namespace server {

Configuration *Configuration::instance_ = 0;

Configuration::Configuration(Wt::WLogger& logger, bool silent)
  : logger_(logger),
    silent_(silent),
    threads_(10),
    docRoot_(),
    errRoot_(),
    deployPath_("/"),
    pidPath_(),
    serverName_(),
    compression_(true),
    gdb_(false),
    configPath_(),
    httpPort_("80"),
    httpsPort_("443"),
    sslCertificateChainFile_(),
    sslPrivateKeyFile_(),
    sslTmpDHFile_(),
    sessionIdPrefix_(),
    accessLog_(),
    maxMemoryRequestSize_(128*1024)
{
  if (instance_)
    throw Wt::WServer::Exception("Internal error: two Configuration instances?");
  instance_ = this;

  char buf[100];
  if (gethostname(buf, 100) == 0)
    serverName_ = buf;

#ifndef WTHTTP_WITH_ZLIB
  compression_ = false;
#endif
}

Configuration::~Configuration()
{
  instance_ = 0;
  unlink(pidPath_.c_str());
}

void Configuration::createOptions(po::options_description& options)
{
  po::options_description general("General options");
  general.add_options()
    ("help,h", "produce help message")

    ("threads,t",
     po::value<int>(&threads_)->default_value(threads_),
     "number of threads")

    ("servername",
     po::value<std::string>(&serverName_)->default_value(serverName_),
     "servername (IP address or DNS name)")

    ("docroot",
     po::value<std::string>()->default_value(docRoot_),
     "document root for static files")

    ("errroot",
     po::value<std::string>(&errRoot_)->default_value(errRoot_),
     "root for error pages")

    ("accesslog",
     po::value<std::string>(&accessLog_),
     "access log file (defaults to stdout)")

    ("no-compression",
     "do not compress dynamic text/html and text/plain responses")

    ("deploy-path",
     po::value<std::string>(&deployPath_)->default_value(deployPath_),
     "location for deployment")

    ("session-id-prefix",
     po::value<std::string>(&sessionIdPrefix_)->default_value(sessionIdPrefix_),
     "prefix for session-id's (overrides wt_config.xml setting)")

    ("pid-file,p",
     po::value<std::string>(&pidPath_)->default_value(pidPath_),
     "path to pid file (optional)")

    ("config,c",
     po::value<std::string>(&configPath_),
     ("location of wt_config.xml. If unspecified, WT_CONFIG_XML is searched "
     "in the environment, if it does not exist then the compiled-in default ("
     + std::string(WT_CONFIG_XML) + ") is tried. If the default does not "
      "exist, we revert to default values for all parameters.").c_str())

    ("max-memory-request-size",
     po::value< ::int64_t >(&maxMemoryRequestSize_),
     "Requests are usually read in memory before being processed. To avoid "
     "DOS attacks where large requests take up all RAM, use this parameter "
     "to force requests that are larger than the specified size (bytes) to "
     "be spooled to disk. This will also spool file uploads to disk.")

    ("gdb",
     "do not shutdown when receiving Ctrl-C (and let gdb break instead)")
     ;

  po::options_description http("HTTP/WebSocket server options");
  http.add_options()
    ("http-address", po::value<std::string>(),
     "IPv4 (e.g. 0.0.0.0) or IPv6 Address (e.g. 0::0)")
    ("http-port", po::value<std::string>(&httpPort_)->default_value(httpPort_),
     "HTTP port (e.g. 80)")
    ;

  po::options_description https("HTTPS/Secure WebSocket server options");
  https.add_options()
    ("https-address", po::value<std::string>(),
     "IPv4 (e.g. 0.0.0.0) or IPv6 Address (e.g. 0::0)")
    ("https-port",
     po::value<std::string>(&httpsPort_)->default_value(httpsPort_),
     "HTTPS port (e.g. 443)")
    ("ssl-certificate",
     po::value<std::string>()->default_value(sslCertificateChainFile_),
     "SSL server certificate chain file\n"
     "e.g. \"/etc/ssl/certs/vsign1.pem\"")
    ("ssl-private-key", po::value<std::string>()->default_value(sslPrivateKeyFile_),
     "SSL server private key file\n"
     "e.g. \"/etc/ssl/private/company.pem\"")
    ("ssl-tmp-dh", po::value<std::string>()->default_value(sslTmpDHFile_),
     "File for temporary Diffie-Hellman parameters\n"
     "e.g. \"/etc/ssl/dh512.pem\"")
    ;

  options.add(general).add(http).add(https);
}

void Configuration::setOptions(int argc, char **argv,
			       const std::string& configurationFile)
{
  po::options_description all_options("Allowed options");
  createOptions(all_options);

  try {
    po::variables_map vm;

    if (argc)
      po::store(po::parse_command_line(argc, argv, all_options), vm);

    if (!configurationFile.empty()) {
      std::ifstream cfgFile(configurationFile.c_str(),
	std::ios::in | std::ios::binary);
      if (cfgFile) {
	if (!silent_)
	  log("notice") << "Reading wthttpd configuration from: "
			<< configurationFile;
	po::store(po::parse_config_file(cfgFile, all_options), vm);
      }
    }

    po::notify(vm);

    if (vm.count("help")) {
      std::cout << all_options << std::endl;

      if (!configurationFile.empty())
	std::cout << "Settings may be set in the configuration file "
		  << configurationFile << std::endl;

      std::cout << std::endl;

      throw Wt::WServer::Exception("");
    }

    readOptions(vm);
  } catch (Wt::WServer::Exception& e) {
    throw;
  } catch (std::exception& e) {
    throw Wt::WServer::Exception(std::string("Error: ") + e.what());
  } catch (...) {
    throw Wt::WServer::Exception("Exception of unknown type!\n");
  }
}

void Configuration::readOptions(const po::variables_map& vm)
{
  if (!pidPath_.empty()) {
    std::ofstream pidFile(pidPath_.c_str());

    if (!pidFile)
      throw Wt::WServer::Exception("Cannot write to '" + pidPath_ + "'");

    pidFile << getpid() << std::endl;
  }

  gdb_ = vm.count("gdb");

  compression_ = !vm.count("no-compression");
#ifndef WTHTTP_WITH_ZLIB
  if(compression_) {
    std::cout << "Option no-compression is implied because wthttp was built "
	      << "without zlib support.\n";
    compression_ = false;
  }
#endif

  checkPath(vm, "docroot", "Document root", docRoot_, Directory);

  if (vm.count("http-address"))
    httpAddress_ = vm["http-address"].as<std::string>();

  if (errRoot_.empty()) {
    errRoot_ = docRoot_;
    if (!errRoot_.empty()) {
      if (errRoot_[errRoot_.length()-1] != '/')
	errRoot_+= '/';
    }
    errRoot_ += "error/";
  }
  if (errRoot_[errRoot_.length()-1] != '/')
    errRoot_+= '/';

  if (deployPath_.empty())
    deployPath_ = "/";
  else
    if (deployPath_[0] != '/')
      throw Wt::WServer::Exception("Deployment root must start with '/'");

  if (vm.count("https-address")) {
    httpsAddress_ = vm["https-address"].as<std::string>();

    checkPath(vm, "ssl-certificate", "SSL Certificate chain file",
	      sslCertificateChainFile_, RegularFile);
    checkPath(vm, "ssl-private-key", "SSL Private key file",
	      sslPrivateKeyFile_, RegularFile | Private);
    checkPath(vm, "ssl-tmp-dh", "SSL Temporary Diffie-Hellman file",
	      sslTmpDHFile_, RegularFile);
  }

  if (httpAddress_.empty() && httpsAddress_.empty()) {
    throw Wt::WServer::Exception
      ("Specify http-address and/or https-address "
       "to run a HTTP and/or HTTPS server.");
  } 
}

Wt::WLogEntry Configuration::log(const std::string& type) const
{
  Wt::WLogEntry e = logger_.entry();

  e << Wt::WLogger::timestamp << Wt::WLogger::sep
    << getpid() << Wt::WLogger::sep
    << /* sessionId << */ Wt::WLogger::sep
    << '[' << type << ']' << Wt::WLogger::sep;

  return e;
}

#ifdef _MSC_VER
static inline bool S_ISREG(unsigned short mode)
{
   return (mode & S_IFREG) != 0;
}

static inline bool S_ISDIR(unsigned short mode)
{
   return (mode & S_IFDIR) != 0;
}
#endif

void Configuration::checkPath(const po::variables_map& vm,
			      std::string varName,
			      std::string varDescription,
			      std::string& result,
			      int options)
{
  if (vm.count(varName)) {
    result = vm[varName].as<std::string>();

    struct stat t;
    if (stat(result.c_str(), &t) != 0) {
      std::perror("stat");
      throw Wt::WServer::Exception(varDescription
				   + " (\"" + result + "\") not valid.");
    } else {
      if (options & Directory) {
	while (result[result.length()-1] == '/')
	  result = result.substr(0, result.length() - 1);

	if (!S_ISDIR(t.st_mode)) {
	  throw Wt::WServer::Exception(varDescription + " (\"" + result
				       + "\") must be a directory.");
	}
      }

      if (options & RegularFile) {
	if (!S_ISREG(t.st_mode)) {
	  throw Wt::WServer::Exception(varDescription + " (\"" + result
				       + "\") must be a regular file.");
	}
      }
#ifndef WIN32
      if (options & Private) {
	if (t.st_mode & (S_IRWXG | S_IRWXO)) {
	  throw Wt::WServer::Exception(varDescription + " (\"" + result
			     + "\") must be unreadable for group and others.");
	}
      }
#endif
    }
  } else {
    throw Wt::WServer::Exception(varDescription + " (--" + varName
				 + ") was not set.");
  }
}

} // namespace server
} // namespace http
