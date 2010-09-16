/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

#include "Wt/WLogger"

#include "WtException.h"
#include "Utils.h"

namespace Wt {

WLogEntry::WLogEntry(const WLogEntry& other)
  : impl_(other.impl_)
{
  other.impl_ = 0;
}

WLogEntry::~WLogEntry()
{
  if (impl_) {
    impl_->finish();
    impl_->logger_.addLine(impl_->currentLine_.str());
  }

  delete impl_;
}

WLogEntry& WLogEntry::operator<< (const WLogger::Sep&)
{
  checkImpl();

  impl_->nextField();

  return *this;
}

WLogEntry& WLogEntry::operator<< (const WLogger::TimeStamp&)
{
  std::string dt = to_simple_string(microsec_clock::local_time());

  return *this << '[' << dt << ']';
}

WLogEntry& WLogEntry::operator<< (const char *s)
{
  return *this << std::string(s);
}

WLogEntry& WLogEntry::operator<< (const std::string& s)
{
  checkImpl();

  if (impl_->quote()) {
    startField();

    std::string ss(s);
    Wt::Utils::replace(ss, '"', "\"\"");

    impl_->currentLine_ << ss;
  } else
    if (!s.empty()) {
      startField();
      impl_->currentLine_ << s;
    }

  return *this;
}

WLogEntry::WLogEntry(const WLogger& logger)
{ 
  impl_ = new Impl(logger);
}

void WLogEntry::checkImpl()
{
  if (!impl_)
    throw WtException("WLogger: cannot use copied WLogEntry");
}

void WLogEntry::startField()
{
  checkImpl();

  impl_->startField();
}

WLogEntry::Impl::Impl(const WLogger& logger)
  : logger_(logger),
    currentField_(0),
    fieldStarted_(false)
{ }

void WLogEntry::Impl::startField()
{
  if (!fieldStarted_) {
    if (quote())
      currentLine_ << '"';
    fieldStarted_ = true;
  }
}

void WLogEntry::Impl::finishField()
{
  if (fieldStarted_) {
    if (quote())
      currentLine_ << '"';
  } else
    currentLine_ << '-';
}

void WLogEntry::Impl::nextField()
{
  finishField();

  currentLine_ << ' ';
  fieldStarted_ = false;
  ++currentField_;
}

void WLogEntry::Impl::finish()
{
  while (currentField_ < (int)logger_.fields().size() - 1)
    nextField();

  finishField();
}

bool WLogEntry::Impl::quote() const
{
  return logger_.fields()[currentField_].isString();
}

const WLogger::Sep WLogger::sep = WLogger::Sep();
const WLogger::TimeStamp WLogger::timestamp = WLogger::TimeStamp();

WLogger::Field::Field(const std::string& name, bool isString)
  : name_(name),
    string_(isString)
{ }

WLogger::WLogger()
  : o_(0),
    ownStream_(false)
{ }

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
  if (ownStream_)
    delete o_;

#ifdef _MSC_VER
  FILE *file = _fsopen(path.c_str(), "at", _SH_DENYNO);
  if (file) {
    o_ = new std::ofstream(file);
  } else {
    o_ = new std::ofstream(path.c_str(), std::ios_base::out | std::ios_base::ate);
  }
#else
  o_ = new std::ofstream(path.c_str(), std::ios_base::out | std::ios_base::ate);
#endif
  ownStream_ = true;
}

void WLogger::addField(const std::string& name, bool isString)
{
  fields_.push_back(Field(name, isString));
}

WLogEntry WLogger::entry() const
{
  return WLogEntry(*this);
}

void WLogger::addLine(const std::string& s) const
{
  if (o_)
    *o_ << s << std::endl;
}

}
