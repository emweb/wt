/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "Wt/WLogger.h"
#include "Wt/WServer.h"
#include "Wt/WString.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WTime.h"

#include "WebUtils.h"
#include "StringUtils.h"
#include "WebSession.h"

namespace Wt {

  namespace {
    WLogger defaultLogger;
  }

LOGGER("WLogger");

WLogEntry::WLogEntry(WLogEntry&& other)
  : impl_(std::move(other.impl_))
{ }

WLogEntry::~WLogEntry()
{
  if (impl_) {
    impl_->finish();
    impl_->logger_.addLine(impl_->type_, impl_->scope_, impl_->line_);
  }
}

WLogEntry& WLogEntry::operator<< (const WLogger::Sep&)
{
  if (impl_)
    impl_->nextField();

  return *this;
}

WLogEntry& WLogEntry::operator<< (const WLogger::TimeStamp&)
{
  std::string dt = WLocalDateTime::currentServerDateTime()
    .toString("yyyy-MMM-dd hh:mm:ss.zzz").toUTF8();

  return *this << '[' << dt << ']';
}

WLogEntry& WLogEntry::operator<< (const char *s)
{
  return *this << std::string(s);
}

WLogEntry& WLogEntry::operator<< (const WString& s)
{
  return *this << s.toUTF8();
}

WLogEntry& WLogEntry::operator<< (const std::string& s)
{
  if (impl_) {
    if (impl_->quote()) {
      startField();

      std::string ss(s);
      Wt::Utils::replace(ss, '"', "\"\"");

      impl_->line_ << ss;
    } else {
      if (!s.empty()) {
	startField();
	impl_->line_ << s;
      }
    }

    if (impl_->field_ == (int)impl_->logger_.fields().size() - 1
	&& impl_->scope_.empty())
      impl_->scope_ = s;
  }

  return *this;
}

WLogEntry& WLogEntry::operator<< (char v)
{
  startField();

  if (impl_)
    impl_->line_ << v;

  return *this;
}

WLogEntry& WLogEntry::operator<< (int v)
{
  startField();

  if (impl_)
    impl_->line_ << v;

  return *this;
}

WLogEntry& WLogEntry::operator<< (long long v)
{
  startField();

  if (impl_)
    impl_->line_ << v;

  return *this;
}

WLogEntry& WLogEntry::operator<< (double v)
{
  startField();

  if (impl_)
    impl_->line_ << v;

  return *this;
}

WLogEntry::WLogEntry(const WLogger& logger, const std::string& type,
		     bool mute)
{
  if (!mute)
    impl_ = cpp14::make_unique<Impl>(logger, type);
}

void WLogEntry::startField()
{
  if (impl_)
    impl_->startField();
}

WLogEntry::Impl::Impl(const WLogger& logger, const std::string& type)
  : logger_(logger),
    type_(type),
    field_(0),
    fieldStarted_(false)
{ }

void WLogEntry::Impl::startField()
{
  if (!fieldStarted_) {
    if (quote())
      line_ << '"';
    fieldStarted_ = true;
  }
}

void WLogEntry::Impl::finishField()
{
  if (fieldStarted_) {
    if (quote())
      line_ << '"';
  } else
    line_ << '-';
}

void WLogEntry::Impl::nextField()
{
  finishField();

  line_ << ' ';
  fieldStarted_ = false;
  ++field_;
}

void WLogEntry::Impl::finish()
{
  while (field_ < (int)logger_.fields().size() - 1)
    nextField();

  finishField();
}

bool WLogEntry::Impl::quote() const
{
  if (field_ < (int)logger_.fields().size())
    return logger_.fields()[field_].isString();
  else
    return false;
}

const WLogger::Sep WLogger::sep = WLogger::Sep();
const WLogger::TimeStamp WLogger::timestamp = WLogger::TimeStamp();

WLogger::Field::Field(const std::string& name, bool isString)
  : name_(name),
    string_(isString)
{ }

WLogger::WLogger()
  : o_(&std::cerr),
    ownStream_(false)
{
  Rule r;
  r.type = "*";
  r.scope = "*";
  r.include = true;
  rules_.push_back(r);
  r.type = "debug";
  r.include = false;
  rules_.push_back(r);
}

WLogger::~WLogger()
{ 
  if (ownStream_)
    delete o_;
}

void WLogger::setStream(std::ostream& o)
{
  if (ownStream_)
    delete o_;

  o_ = &o;
  ownStream_ = false;
}

void WLogger::setFile(const std::string& path)
{
  if (ownStream_) {
    delete o_;
    o_ = &std::cerr;
    ownStream_ = false;
  }

  std::ofstream *ofs;
#ifdef _MSC_VER
  FILE *file = _fsopen(path.c_str(), "at", _SH_DENYNO);
  if (file) {
    ofs = new std::ofstream(file);
  } else {
    ofs = new std::ofstream(path.c_str(), std::ios_base::out | std::ios_base::ate | std::ios_base::app);
  }
#else
  ofs = new std::ofstream(path.c_str(), 
			  std::ios_base::out | std::ios_base::ate | std::ios_base::app);
  if (!ofs->is_open()) {
    // maybe a special file (pipe, /dev/null) that does not support ate?
    delete ofs;
    ofs = new std::ofstream(path.c_str(), std::ios_base::out);
  }
#endif
  
  if (ofs->is_open()) {
    LOG_INFO("Opened log file (" << path << ").");
    o_ = ofs;
    ownStream_ = true;
  } else {
    delete ofs;

    LOG_ERROR("Could not open log file (" << path << "). "
              "We will be logging to std::cerr again.");
    o_ = &std::cerr;
    ownStream_ = false;
  }
}

void WLogger::addField(const std::string& name, bool isString)
{
  fields_.push_back(Field(name, isString));
}

WLogEntry WLogger::entry(const std::string& type) const
{
  return WLogEntry(*this, type, !logging(type));
}

void WLogger::addLine(const std::string& type,
		      const std::string& scope, const WStringStream& s) const
{
  if (logging(type, scope))
    if (o_)
      *o_ << s.str() << std::endl;
}

void WLogger::configure(const std::string& config)
{
  rules_.clear();

  Utils::SplitVector rules;
  boost::split(rules, config, boost::algorithm::is_space(),
	       boost::algorithm::token_compress_on);

  for (unsigned i = 0; i < rules.size(); ++i) {
    Utils::SplitVector type_scope;
    boost::split(type_scope, rules[i], boost::is_any_of(":"));

    Rule r;
    r.type = std::string(type_scope[0].begin(), type_scope[0].end());

    if (type_scope.size() == 1)
      r.scope = "*";
    else
      r.scope = std::string(type_scope[1].begin(), type_scope[1].end());

    r.include = true;

    if (r.type[0] == '-') {
      r.include = false;
      r.type = r.type.substr(1);
    } else if (r.type[0] == '+')
      r.type = r.type.substr(1);

    rules_.push_back(r);
  }
}

bool WLogger::logging(const std::string& type) const
{
  return logging(type.c_str());
}

bool WLogger::logging(const char *type) const
{
  bool result = false;

  for (unsigned i = 0; i < rules_.size(); ++i)
    if (rules_[i].type == "*" || rules_[i].type == type) {
      if (rules_[i].scope == "*")
	result = rules_[i].include;
      else if (rules_[i].include)
	result = true;
    }

  return result;
}

bool WLogger::logging(const std::string& type, const std::string& scope) const
{
  bool result = false;

  for (unsigned i = 0; i < rules_.size(); ++i)
    if (rules_[i].type == "*" || rules_[i].type == type)
      if (rules_[i].scope == "*" || rules_[i].scope == scope)
	result = rules_[i].include;

  return result;
}

WLogger& logInstance()
{ 
  WebSession *session = WebSession::instance();

  if (session)
    return session->logInstance();
  else {
    WServer *server = WServer::instance();

    if (server)
      return server->logger();
    else
      return defaultLogger;
  }
}

WLogEntry log(const std::string& type)
{
  WebSession *session = WebSession::instance();

  if (session)
    return session->log(type);
  else {
    WServer *server = WServer::instance();

    if (server)
      return server->log(type);
    else
      return defaultLogger.entry(type);
  }
}

}
