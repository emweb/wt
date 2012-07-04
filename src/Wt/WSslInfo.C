/*
 * Copyright (C) 2012 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSslInfo"

#include "Wt/WLogger"
#include "Wt/Utils"

#include <sstream>
#include <stdexcept>

#ifdef WT_WITH_SSL

namespace Wt {

LOGGER("WSslInfo");

WSslInfo::WSslInfo(const WSslCertificate &clientCertificate,
		   const std::vector<WSslCertificate> &clientCertificateChain,
		   WValidator::Result clientVerificationResult)
  : clientCertificate_(clientCertificate),
    clientCertificateChain_(clientCertificateChain),
    clientVerificationResult_(clientVerificationResult)
{
  LOG_DEBUG("WSslInfo fields: " <<  gdb());
}

std::string WSslInfo::gdb() const
{
  std::stringstream ss;
  ss << "client certificate:\n";
  clientCertificate_.gdb();
  
  for (unsigned i = 0; i < clientCertificateChain_.size(); ++i) {
    ss << "client cert chain " << i << " :\n";
    clientCertificateChain_[i].gdb();
  }

  ss
    << "valid: " << (clientVerificationResult_.state() == WValidator::Valid)
    << std::endl
    << "validity info: " << clientVerificationResult_.message() << std::endl;
  
  return ss.str();
}

}

#endif

