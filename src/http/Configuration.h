// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_CONFIGURATION_HPP
#define HTTP_CONFIGURATION_HPP

#include <exception>
#include <string>
#include <boost/program_options.hpp>
#include <boost/cstdint.hpp>

// For ::int64_t and ::uint64_t on Windows only
#include "Wt/WDllDefs.h"

namespace po = boost::program_options;

namespace boost {
  namespace program_options {
    class variables_map;
  }
}

namespace Wt {
  class WLogger;
  class WLogEntry;
}

namespace http {
namespace server {

class Configuration
{
public:
  Configuration(Wt::WLogger& logger, bool silent = false);
  ~Configuration();

  void setOptions(int argc, char **argv, const std::string& configurationFile);

  static Configuration& instance() { return *instance_; }

  int threads() const { return threads_; }
  const std::string& docRoot() const { return docRoot_; }
  const std::string& errRoot() const { return errRoot_; }
  const std::string& deployPath() const { return deployPath_; }
  const std::string& pidPath() const { return pidPath_; }
  const std::string& serverName() const { return serverName_; }
  bool compression() const { return compression_; }
  bool gdb() const { return gdb_; }
  const std::string& configPath() const { return configPath_; }

  const std::string& httpAddress() const { return httpAddress_; }
  const std::string& httpPort() const { return httpPort_; }

  const std::string& httpsAddress() const { return httpsAddress_; }
  const std::string& httpsPort() const { return httpsPort_; }
  const std::string& sslCertificateChainFile() const 
    { return sslCertificateChainFile_; }
  const std::string& sslPrivateKeyFile() const { return sslPrivateKeyFile_; }
  const std::string& sslTmpDHFile() const { return sslTmpDHFile_; }

  const std::string& sessionIdPrefix() const { return sessionIdPrefix_; }
  const std::string& accessLog() const { return accessLog_; }

  ::int64_t maxMemoryRequestSize() const { return maxMemoryRequestSize_; }

  Wt::WLogEntry log(const std::string& type) const;

private:
  Wt::WLogger& logger_;
  bool silent_;

  int threads_;
  std::string docRoot_;
  std::string errRoot_;
  std::string deployPath_;
  std::string pidPath_;
  std::string serverName_;
  bool compression_;
  bool gdb_;
  std::string configPath_;

  std::string httpAddress_;
  std::string httpPort_;

  std::string httpsAddress_;
  std::string httpsPort_;
  std::string sslCertificateChainFile_;
  std::string sslPrivateKeyFile_;
  std::string sslTmpDHFile_;

  std::string sessionIdPrefix_;
  std::string accessLog_;

  ::int64_t maxMemoryRequestSize_;

  static Configuration *instance_;

  void createOptions(po::options_description& options);
  void readOptions(const po::variables_map& vm);

  void checkPath(const boost::program_options::variables_map& vm, std::string varName,
		 std::string varDescription, std::string& result,
		 int options);

  enum PathOptions { RegularFile = 0x1,
		     Directory = 0x2,
		     Private = 0x4 };
};

} // namespace server
} // namespace http

#endif // HTTP_CONFIGURATION_HPP
