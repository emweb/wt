// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_RESULT_H
#define WT_PAYMENT_RESULT_H

#include <Wt/WString.h>
#include <map>

namespace Wt {
  namespace Payment {

/*! \class Result Wt/Payment/Result.h Wt/Payment/Result.h
 *  \brief A class that represents the result of a payment API call.
 * 
 * This class indicates the result of an asynchronous call: error()
 * indicates whether there was an error.
 *
 * \ingroup payment
 */
class WT_API Result
{
public:
  /*! \brief Default constructor.
   *
   * This creates a successful result.
   */
  Result() {}

  /*! \brief Constructor with an error message.
   *
   * This creates an unsuccessful result. The error message cannot be
   * empty.
   */
  Result(const WString& errorMessage);

  /*! \brief Returns whether the result indicates success.
   */
  bool error() const { return (!errorMessage_.empty()); }

  /*! \brief Returns the error message.
   */
  WString errorMessage() const { return errorMessage_; }

  /*! \brief Sets the request parameters.
   *
   * For an asynchronous API call that uses name value pairs, this sets
   * the underlying values that were sent in the request.
   */
  void setRequestMessage(const std::map<std::string, std::string> & msg);

  /*! \brief Returns the request parameters.
   *
   * \sa setRequestMessage()
   */
  std::map<std::string, std::string> requestMessage() const;

  /*! \brief Sets the response parameters.
   *
   * For an asynchronous API call that uses name value pairs, this sets
   * the underlying values that were returned in the response.
   */
  void setResponseMessage(const std::map<std::string, std::string> &msg);

  /*! \brief Returns the response parameters.
   *
   * \sa setResponseMessage()
   */
  std::map<std::string, std::string> responseMessage() const;

private:
  WString errorMessage_;
  std::map<std::string, std::string> responseMessage_;
  std::map<std::string, std::string> requestMessage_;

};

/*! \brief Enumeration for a payment approval outcome.
 */
enum class ApprovalOutcome {
  Denied = 0,      //!< The user denied the payment.
  Interrupted = 1, //!< The payment process was interrupted.
  Accepted = 2     //!< The user accepted the payment.
};

/*! \class Approval Wt/Payment/Result.h Wt/Payment/Result.h
 *  \brief A class that represents the result of a payment.
 *
 * \ingroup payment
 */
class Approval : public Result
{
public:
  /*! \brief Typedef for enum Wt::Payment::ApprovalOutcome */
  typedef ApprovalOutcome Outcome;

  /*! \brief Constructor.
   */
  Approval(ApprovalOutcome outcome);

  /*! \brief Constructor for an interrupted payment.
   */
  Approval(const WString& errorMessage);

  /*! \brief Returns the outcome.
   */
  ApprovalOutcome outcome() const { return outcome_; }

private:
  ApprovalOutcome outcome_;
};

  }
}

#endif // WT_PAYMENT_RESULT_H_
