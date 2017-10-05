#include "Wt/Payment/Result.h"

namespace Wt {
  namespace Payment {

Result::Result(const WString& errorMessage)
  : errorMessage_(errorMessage)
{ }

void Result::setResponseMessage(const std::map<std::string, std::string> &msg)
{
  responseMessage_ = msg;
}

std::map<std::string, std::string> Result::responseMessage() const
{
  return responseMessage_;
}

void Result::setRequestMessage(const std::map<std::string, std::string> & msg)
{
  requestMessage_ = msg;
}

std::map<std::string, std::string> Result::requestMessage() const
{
  return requestMessage_;
}


Approval::Approval(ApprovalOutcome outcome)
  : outcome_(outcome)
{ }

Approval::Approval(const WString& errorMessage)
  : Result(errorMessage)
{ }

  }
}
