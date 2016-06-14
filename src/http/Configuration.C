/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WConfig.h"
#include "Wt/WLogger"
#include "Wt/WServer"

#include "Configuration.h"
#include "WebUtils.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WT_WIN32
#include <unistd.h>
#endif
#ifdef WT_WIN32
#include <process.h> // for getpid()
#include <winsock2.h> // for gethostname()
#endif
#include <iostream>
#include <fstream>

#ifdef __CYGWIN__
#include <winsock2.h> // for gethostname()
#endif

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

Configuration::Configuration(Wt::WLogger& logger, bool silent)
  : logger_(logger),
    silent_(silent),
    threads_(-1),
    docRoot_(),
    defaultStatic_(true),
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
    sslEnableV3_(false),
    sslClientVerification_("none"),
    sslVerifyDepth_(1),
    sslCaCertificates_(),
    sslCipherList_(),
    sessionIdPrefix_(),
    accessLog_(),
    parentPort_(-1),
    maxMemoryRequestSize_(128*1024)
{
  char buf[100];
  if (gethostname(buf, 100) == 0)
    serverName_ = buf;

#ifndef WTHTTP_WITH_ZLIB
  compression_ = false;
#endif
}

Configuration::~Configuration()
{
  unlink(pidPath_.c_str());
}

void Configuration::createOptions(po::options_description& options,
			          po::options_description &visibleOptions)
{
  po::options_description general("General options");
  general.add_options()
    ("help,h", "produce help message")

    ("threads,t",
     po::value<int>(&threads_)->default_value(threads_),
     "number of threads (-1 indicates that num_threads from wt_config.xml "
     "is to be used, which defaults to 10)")

    ("servername",
     po::value<std::string>(&serverName_)->default_value(serverName_),
     "servername (IP address or DNS name)")

    ("docroot",
     po::value<std::string>()->default_value(docRoot_),
     "document root for static files, optionally followed by a "
     "comma-separated list of paths with static files (even if they "
     "are within a deployment path), after a ';' \n\n"
     "e.g. --docroot=\".;/favicon.ico,/resources,/style\"\n")

    ("approot",
     po::value<std::string>(&appRoot_)->default_value(appRoot_),
     "application root for private support files; if unspecified, the value "
     "of the environment variable $WT_APP_ROOT is used, "
     "or else the current working directory")

    ("errroot",
     po::value<std::string>(&errRoot_)->default_value(errRoot_),
     "root for error pages")

    ("accesslog",
     po::value<std::string>(&accessLog_),
     "access log file (defaults to stdout), "
     "to disable access logging completely, use --accesslog=-")

    ("no-compression",
     "do not use compression")

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
     ("location of wt_config.xml; if unspecified, the value of the environment "
      "variable $WT_CONFIG_XML is used, or else the built-in default "
      "(" + std::string(WT_CONFIG_XML) + ") is tried, or else built-in "
      "defaults are used").c_str())

    ("max-memory-request-size",
     po::value< ::int64_t >(&maxMemoryRequestSize_)
       ->default_value(maxMemoryRequestSize_),
     "threshold for request size (bytes), for spooling the entire request to "
     "disk, to avoid DoS")

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
    ("ssl-enable-v3",
     "Switch on SSLv3 support (not recommended; disabled by default)")
    ("ssl-client-verification",
     po::value<std::string>(&sslClientVerification_)
       ->default_value(sslClientVerification_),
     "The verification mode for client certificates.\n"
     "This is either 'none', 'optional' or 'required'. When 'none', the server "
     "will not request a client certificate. When 'optional', the server will "
     "request a certificate, but the client does not have to supply one. With "
     "'required', the connection will be terminated if the client does not "
     "provide a valid certificate.")
    ("ssl-verify-depth",
     po::value<int>(&sslVerifyDepth_)->default_value(sslVerifyDepth_),
     "Specifies the maximum length of the server certificate chain.\n")
    ("ssl-ca-certificates",
     po::value<std::string>(&sslCaCertificates_)
       ->default_value(sslCaCertificates_),
     "Path to a file containing the concatenated trusted CA certificates, "
     "which can be used to authenticate the client. The file should contains a "
     "a number of PEM-encoded certificates.\n")
    ("ssl-cipherlist",
     po::value<std::string>(&sslCipherList_)
       ->default_value(sslCipherList_),
     "List of acceptable ciphers for SSL. This list is passed as-is to the SSL "
     "layer, so see openssl for the proper syntax. When empty, the default "
     "acceptable cipher list will be used. Example cipher list string: "
     "\"TLSv1+HIGH:!SSLv2\"\n")
    ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("parent-port", po::value<int>(&parentPort_)->default_value(parentPort_))
  ;

  options.add(general).add(http).add(https).add(hidden);
  visibleOptions.add(general).add(http).add(https);
}

void Configuration::setOptions(int argc, char **argv,
			       const std::string& configurationFile)
{
  po::options_description all_options("Allowed options");
  po::options_description visible_options("Allowed options");
  createOptions(all_options, visible_options);

  try {
    po::variables_map vm;

    if (argc)
      po::store(po::command_line_parser(argc, argv).options(all_options).allow_unregistered().run(), vm);
      
    if (!configurationFile.empty()) {
      std::ifstream cfgFile(configurationFile.c_str(),
	std::ios::in | std::ios::binary);
      if (cfgFile) {
	if (!silent_)
	  LOG_INFO_S(this, "reading wthttpd configuration from: "
		     << configurationFile);
	po::store(po::parse_config_file(cfgFile, all_options), vm);
      }
    }

    po::notify(vm);

    if (vm.count("help")) {
      std::cout << visible_options << std::endl;

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

#ifndef WT_WIN32
  for (int i = 0; i < argc; ++i) {
    options_.push_back(argv[i]);
  }
#endif // !WT_WIN32
}

#ifndef WT_WIN32
std::vector<std::string> Configuration::options() const
{
  return options_;
}
#endif // !WT_WIN32

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

  if (vm.count("docroot")) {
    docRoot_ = vm["docroot"].as<std::string>();

    if (docRoot_ == "") {
      throw Wt::WServer::Exception(
        "Document root was not set, or was set to the empty path. "
        "Use --docroot to set the HTML root directory.");
    }
    Wt::Utils::SplitVector parts;
    boost::split(parts, docRoot_, boost::is_any_of(";"));

    if (parts.size() > 1) {
      if (parts.size() != 2)
	throw Wt::WServer::Exception("Document root (--docroot) should be "
				     "of format path[;./p1[,p2[,...]]]");
      boost::split(staticPaths_, parts[1], boost::is_any_of(","));
      defaultStatic_ = false;
    }

    if (parts.size() > 0)
      docRoot_ = std::string(parts[0].begin(), parts[0].end());

    checkPath(docRoot_, "Document root", Directory);
  } else
    throw Wt::WServer::Exception("Document root (--docroot) was not set.");

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

  sslEnableV3_ = vm.count("ssl-enable-v3");

  if (vm.count("https-address")) {
    httpsAddress_ = vm["https-address"].as<std::string>();

    checkPath(vm, "ssl-certificate", "SSL Certificate chain file",
	      sslCertificateChainFile_, RegularFile);
    checkPath(vm, "ssl-private-key", "SSL Private key file",
	      sslPrivateKeyFile_, RegularFile | Private);
    checkPath(vm, "ssl-tmp-dh", "SSL Temporary Diffie-Hellman file",
	      sslTmpDHFile_, RegularFile);
  }

  if (sslClientVerification_ != "none") {

    checkPath(vm, "ssl-ca-certificates",
	      "Client authentication SSL CA certificates file",
	      sslCaCertificates_, RegularFile);

    if (sslClientVerification_ != "optional" && 
	sslClientVerification_ != "once" && 
	sslClientVerification_ != "required") {
      throw Wt::WServer::Exception(
	      "ssl-client-verification must be \"none\", \"optional\", \"once\" or "
	      "\"required\"");
    }
  }

  if (httpAddress_.empty() && httpsAddress_.empty()) {
    throw Wt::WServer::Exception
      ("Specify http-address and/or https-address "
       "to run a HTTP and/or HTTPS server.");
  } 
}

Wt::WLogEntry Configuration::log(const std::string& type) const
{
  Wt::WLogEntry e = logger_.entry(type);

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

    checkPath(result, varDescription, options);
  } else {
    throw Wt::WServer::Exception(varDescription + " (--" + varName
				 + ") was not set.");
  }
}

void Configuration::checkPath(std::string& result,
			      std::string varDescription,
			      int options)
{
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
#ifndef WT_WIN32
    if (options & Private) {
      if (t.st_mode & (S_IRWXG | S_IRWXO)) {
	throw Wt::WServer::Exception(varDescription + " (\"" + result
			    + "\") must be unreadable for group and others.");
      }
    }
#endif
  }
}

} // namespace server
} // namespace http
