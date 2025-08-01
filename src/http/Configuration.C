/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WConfig.h"
#include "Wt/WLogger.h"
#include "Wt/WServer.h"

#include "Configuration.h"
#include "WebUtils.h"
#include "StringUtils.h"
#include "MimeTypes.h"

#include "Wt/cpp17/filesystem.hpp"

#ifndef WT_WIN32
#include <unistd.h>
#endif
#ifdef WT_WIN32
#include <process.h> // for getpid()
#include <winsock2.h> // for gethostname()
#endif
#include <algorithm>
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
    fileExtMapPath_(),
    staticCacheControl_("max-age=3600"),
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
    sslPreferServerCiphers_(false),
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
  if (parentPort_ == -1)
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
     "number of threads (-1 indicates that num-threads from wt_config.xml "
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

    ("resources-dir",
     po::value<std::string>(&resourcesDir_)->default_value(resourcesDir_),
     "path to the Wt resources folder. By default, Wt will look for its resources "
     "in the resources subfolder of the docroot (see --docroot). If a file is not found "
     "in that resources folder, this folder will be checked instead as a fallback. "
     "If this option is omitted, then Wt will not use a fallback resources folder."
     )

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
     "access log file, "
     "if not specified, access logs are logged like other logs, "
     "to disable access logging completely, use --accesslog=-")

    ("no-compression",
     "do not use compression")

    ("deploy-path",
     po::value<std::string>(&deployPath_)->default_value(deployPath_),
     "location for deployment")

    ("session-id-prefix",
     po::value<std::string>(&sessionIdPrefix_)->default_value(sessionIdPrefix_),
     "prefix for session IDs (overrides wt_config.xml setting)")

    ("pid-file,p",
     po::value<std::string>(&pidPath_)->default_value(pidPath_),
     "path to pid file (optional)")

    ("config,c",
     po::value<std::string>(&configPath_),
     ("location of wt_config.xml; if unspecified, the value of the environment "
      "variable $WT_CONFIG_XML is used, or else the built-in default "
      "(" + std::string(WT_CONFIG_XML) + ") is tried, or else built-in "
      "defaults are used").c_str())

    ("mime-map-append",
     po::value<std::string>(&fileExtMapPath_),
     "location of wt_mimeMap.csv; the mappings in wt_mimeMap will be added to the default mappings.")

     ("mime-map-override",
     po::value<std::string>(&fileExtMapPath_),
     "location of wt_mimeMap.csv; if unspecified, the built-in default is used. "
     "The mappings in wt_mimeMap will be used instead of to the default mappings.")

    ("static-cache-control",
     po::value<std::string>(&staticCacheControl_)->default_value(staticCacheControl_),
     "Cache-Control header value for static files (defaults to max-age=3600)")

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
    ("http-listen", po::value<std::vector<std::string> >(&httpListen_)->multitoken(),
     "address/port pair to listen on. If no port is specified, 80 is used as the default, e.g. "
     "127.0.0.1:8080 will cause the server to listen on port 8080 of 127.0.0.1 (localhost). "
     "For IPv6, use square brackets, e.g. [::1]:8080 will cause the server to listen on port "
     "8080 of [::1] (localhost). This argument can be repeated, e.g. "
     "--http-listen 0.0.0.0:8080 --http-listen [0::0]:8080 will cause the server to listen on "
     "port 8080 of all interfaces using IPv4 and IPv6. You must specify this option or --https-listen "
     "at least once. The older style --http-address and --https-address can also be used for backwards "
     "compatibility."
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
     " "
     "If a hostname is provided instead of an IP address, the server "
     "will listen on all of the addresses (IPv4 and IPv6) that this hostname resolves to."
#endif // NO_RESOLVE_ACCEPT_ADDRESS
     )
    ("http-address", po::value<std::string>(),
     "IPv4 (e.g. 0.0.0.0) or IPv6 Address (e.g. 0::0). You must specify either "
     "--http-listen, --https-listen, --http-address, or --https-address.")
    ("http-port", po::value<std::string>(&httpPort_)->default_value(httpPort_),
     "HTTP port (e.g. 80)")
    ;

  po::options_description https("HTTPS/Secure WebSocket server options");
  https.add_options()
    ("https-listen", po::value<std::vector<std::string> >(&httpsListen_)->multitoken(),
     "address/port pair to listen on. If no port is specified, 80 is used as the default, e.g. "
     "127.0.0.1:8080 will cause the server to listen on port 8080 of 127.0.0.1 (localhost). "
     "For IPv6, use square brackets, e.g. [::1]:8080 will cause the server to listen on port "
     "8080 of [::1] (localhost). This argument can be repeated, e.g. "
     "--https-listen 0.0.0.0:8080 --https-listen [0::0]:8080 will cause the server to listen on "
     "port 8080 of all interfaces using IPv4 and IPv6."
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
     " "
     "If a hostname is provided instead of an IP address, the server "
     "will listen on all of the addresses (IPv4 and IPv6) that this hostname resolves to."
#endif // NO_RESOLVE_ACCEPT_ADDRESS
     )
    ("https-address", po::value<std::string>(),
     "IPv4 (e.g. 0.0.0.0) or IPv6 Address (e.g. 0::0). You must specify either "
     "--http-listen, --https-listen, --http-address, or --https-address.")
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
    ("ssl-prefer-server-ciphers",
     po::value<bool>(&sslPreferServerCiphers_)
       ->default_value(sslPreferServerCiphers_),
     "By default, the client's preference is used for determining the cipher "
     "that is choosen during a SSL or TLS handshake. By enabling this option, "
     "the server's preference will be used." )
    ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("parent-port", po::value<int>(&parentPort_)->default_value(parentPort_))
  ;

  options.add(general).add(http).add(https).add(hidden);
  visibleOptions.add(general).add(http).add(https);
}

void Configuration::setOptions(const std::string &applicationPath,
                               const std::vector<std::string> &args,
                               const std::string &configurationFile)
{
  po::options_description all_options("Allowed options");
  po::options_description visible_options("Allowed options");
  createOptions(all_options, visible_options);

  try {
    po::variables_map vm;

    if (!args.empty())
      po::store(po::command_line_parser(args).options(all_options).allow_unregistered().run(), vm);

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

  options_.clear();
  options_.push_back(applicationPath);
  options_.insert(end(options_), begin(args), end(args));
}

std::vector<std::string> Configuration::options() const
{
  return options_;
}

void Configuration::readOptions(const po::variables_map& vm)
{
  if (!pidPath_.empty() && parentPort_ == -1) {
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
      staticPaths_.erase(std::remove(staticPaths_.begin(), staticPaths_.end(), ""), staticPaths_.end());
      defaultStatic_ = false;
    }

    if (parts.size() > 0)
      docRoot_ = std::string(parts[0].begin(), parts[0].end());

    checkPath(docRoot_, "Document root", Directory);
  } else
    throw Wt::WServer::Exception("Document root (--docroot) was not set.");

  if (vm.count("mime-map-append")) {
    mime_types::updateMapping(vm["mime-map-append"].as<std::string>());
  }

  if (vm.count("mime-map-override")) {
    mime_types::setMapping(vm["mime-map-override"].as<std::string>());
  }

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
  }

  if (vm.count("https-listen") || vm.count("https-address")) {
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

  if (httpListen_.empty() &&
      httpAddress_.empty() &&
      httpsListen_.empty() &&
      httpsAddress_.empty()) {
    throw Wt::WServer::Exception
      ("Specify http-listen, https-listen, http-address and/or https-address "
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
  namespace fs = Wt::cpp17::filesystem;
  Wt::cpp17::fs_error_code ec;
  const auto status = fs::status(result, ec);
  if (ec) {
    throw Wt::WServer::Exception(varDescription
                                 + " (\"" + result + "\") not valid: "
                                 + ec.message());
  } else if (!exists(status)) {
    throw Wt::WServer::Exception(varDescription
                                 + " (\"" + result + "\") does not exist.");
  } else {
    if (options & Directory) {
      while (result[result.length()-1] == '/')
        result = result.substr(0, result.length() - 1);

      if (!is_directory(status)) {
        throw Wt::WServer::Exception(varDescription + " (\"" + result
                                     + "\") must be a directory.");
      }
    }

    if (options & RegularFile) {
      if (!is_regular_file(status)) {
        throw Wt::WServer::Exception(varDescription + " (\"" + result
                                     + "\") must be a regular file.");
      }
    }
#ifndef WT_WIN32
    if (options & Private) {
      /* We want to compare the resulting permissions against
       * fs::perms::no_perms in case boost is used for
       * cpp17::filesystem but we want to instead compare it against
       * fs::perms::none in case std::filesystem is used for
       * cpp17::filesystem.
       *
       * Since both of those values are equivalent to 0, we just use
       * static_cast.
       */
      if (static_cast<unsigned>(status.permissions() & (fs::perms::group_all | fs::perms::others_all))) {
        throw Wt::WServer::Exception(varDescription + " (\"" + result
                            + "\") must be unreadable for group and others.");
      }
    }
#endif
  }
}

} // namespace server
} // namespace http
