// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLOGGER_H_
#define WLOGGER_H_

#include <Wt/WStringStream.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifndef WT_TARGET_JAVA

namespace Wt {

class WLogEntry;
class WString;

/*! \class WLogger Wt/WLogger.h Wt/WLogger.h
 *  \brief A simple logging class.
 *
 * This class logs events to a stream in a flexible way. It allows to
 * create log files using the commonly used <a
 * href="http://www.w3.org/Daemon/User/Config/Logging.html#common-logfile-format">Common
 * Log Format</a> or <a
 * href="http://httpd.apache.org/docs/1.3/logs.html#combined">Combined
 * Log Format</a>, but provides a general way for logging entries that
 * consists of a fixed number of fields.
 *
 * It is used by %Wt to create the application log
 * (WApplication::log()), and built-in httpd access log.
 *
 * To use this class for custom logging, you should instantiate a
 * logger, add one or more field definitions using addField(), and set
 * an output stream using setStream() or setFile(). To stream data to
 * the logger, use entry() to start formatting a new entry.
 *
 * Usage example:
 * \code
 * // Setup the logger
 * Wt::WLogger logger;
 * logger.addField("datetime", false);
 * logger.addField("session", false);
 * logger.addField("type", false);
 * logger.addField("message", true);
 *
 * logger.setFile("/tmp/mylog.txt");
 *
 * // Add an entry
 * Wt::WLogEntry entry = logger.entry();
 * entry << Wt::WLogger::timestamp << Wt::WLogger::sep
 *       << '[' << wApp->sessionId() << ']' << Wt::WLogger::sep
 *       << '[' << "notice" << ']' << Wt::WLogger::sep
 *       << "Succesfully started.";
 * \endcode
 *
 * \sa WApplication::log()
 */
class WT_API WLogger
{
public:
  /*! \brief Class that indicates a field separator.
   *
   * \sa sep
   */
  struct Sep { };

  /*! \brief %Field separator constant.
   *
   * \sa WLogEntry::operator<<(const WLogger::Sep&)
   */
  static const Sep sep;

  /*! \brief Class that indicates a time stamp.
   *
   * \sa timestamp
   */
  struct TimeStamp { };

  /*! \brief Timestamp field constant.
   *
   * \sa WLogEntry::operator<<(const WLogger::TimeStamp&)
   */
  static const TimeStamp timestamp;

  /*! \brief Class that holds the configuration for a single field.
   *
   * \sa addField()
   */
  class Field {
  public:
    /*! \brief Returns the field name.
     */
    std::string name() const { return name_; }

    /*! \brief Returns if the field is a quoted string.
     *
     * String fields can contain white-space, and are therefore quoted
     * in the log.
     */
    bool isString() const { return string_; }

  private:
    std::string name_;
    bool string_;

    Field(const std::string& name, bool isString);

    friend class WLogger;
  };

  /*! \brief Creates a new logger.
   *
   * This creates a new logger, which defaults to logging to stderr.
   */
  WLogger();

  /*! \brief Destructor.
   */
  ~WLogger();

  /*! \brief Sets the output stream.
   *
   * The default logger outputs to stderr.
   *
   * \sa setFile()
   */
  void setStream(std::ostream& o);

  /*! \brief Sets the output file.
   *
   * Opens a file output stream for \p path.
   * The default logger outputs to stderr.
   *
   * This logs a message notifying the user whether
   * the file was successfully opened for writing to
   * the previous ostream (usually std::cerr).
   *
   * If you want to suppress the info message,
   * you can configure the logger with "-info:WLogger".
   * The error when the file was not successfully opened
   * is logged at the error log level.
   *
   * \note If the previous ostream was a file set with this
   * method, the message will be logged to std::cerr instead
   * of the previous file, because the previous file
   * is closed first.
   *
   * \sa setStream()
   */
  void setFile(const std::string& path);

  /*! \brief Configures what things are logged.
   *
   * The configuration is a string that defines rules for enabling or
   * disabling certain logging. It is a white-space delimited list of
   * rules, and each rule is of the form:
   *
   * - <tt>[-]level</tt> : enables (or disables) logging of messages of
   *   the given level; '*' is a wild-card that matches all levels
   *
   * - <tt>[-]level:scope</tt> : enables (or disables) logging of
   *   messages of the given level and scope; '*' is a wild-card that
   *   matches all levels or scopes.
   *
   * The default configuration is <tt>"* -debug"</tt>, i.e. by default
   * everything is logged, except for "debug" messages.
   *
   * Some other examples:
   *
   * - <tt>"* -debug debug:wthttp"</tt>: logs everything, including
   *   debugging messages of scope "wthttp", but no other debugging
   *   messages.
   *
   * - <tt>"* -info -debug"</tt>: disables logging of info messages in
   *   addition to debugging messages.
   *
   * \note The standard logging is typically configured in the configuration
   *       file, in the &lt;log-config&gt; block.
   */
  void configure(const std::string& config);

  /*! \brief Adds a field.
   *
   * Add a field to the logger. When \p isString is \p true, values
   * will be quoted.
   */
  void addField(const std::string& name, bool isString);

  /*! \brief Returns the field list.
   */
  const std::vector<Field>& fields() const { return fields_; }

  /*! \brief Starts a new log entry.
   *
   * Returns a new entry. The entry is logged in the destructor of
   * the entry (i.e. when the entry goes out of scope).
   *
   * The \p type reflects a logging level. You can freely choose a type,
   * but these are commonly used inside the library:
   * - "debug": debugging info (suppressed by default)
   * - "info": informational notices
   * - "warning": warnings (potentially wrong API use)
   * - "secure": security-related events
   * - "error": errors (wrong API use, unexpected protocol messages)
   * - "fatal": fatal errors (terminate the session)
   */
  WLogEntry entry(const std::string& type) const;

  /*! \brief Returns whether messages of a given type are logged.
   *
   * Returns \c true if messages of the given type are logged. It may be
   * that not messages of all scopes are logged.
   *
   * \sa configure()
   */
  bool logging(const std::string& type) const;

  /*! \brief Returns whether messages of a given type are logged.
   *
   * Returns \c true if messages of the given type are logged. It may be
   * that not messages of all scopes are logged.
   *
   * \sa configure()
   */
  bool logging(const char *type) const;

  /*! \brief Returns whether messages of a given type and scope are logged.
   *
   * \sa configure()
   */
  bool logging(const std::string& type, const std::string& scope) const;

private:
  std::ostream* o_;
  bool ownStream_;
  std::vector<Field> fields_;

  struct Rule {
    bool include;
    std::string type;
    std::string scope;
  };

  std::vector<Rule> rules_;

  void addLine(const std::string& type, const std::string& scope,
	       const WStringStream& s) const;

  friend class WLogEntry;
};

/*! \class WLogEntry Wt/WLogger.h Wt/WLogger.h
 *  \brief A stream-like object for creating an entry in a log file.
 *
 * This class is returned by WLogger::entry() and creates a log entry using
 * a stream-like interface.
 */
class WT_API WLogEntry
{
public:
  /*! \brief Move constructor.
   *
   * This is mostly for returning a (newly constructed) %WLogEntry from a function.
   *
   * Appending to the from object after move construction has no effect.
   */
  WLogEntry(WLogEntry&& from);

  WLogEntry(const WLogEntry&) = delete;
  WLogEntry& operator=(const WLogEntry&) = delete;
  WLogEntry& operator=(WLogEntry&&) = delete;

  /*! \brief Destructor.
   */
  ~WLogEntry();

  /*! \brief Writes a field separator.
   *
   * You must separate fields in a single entry using the WLogger::sep
   * constant.
   */
  WLogEntry& operator<< (const WLogger::Sep&);

  /*! \brief Writes a time stamp in the current field.
   *
   * Formats a timestamp (date+time) to the current field.
   */
  WLogEntry& operator<< (const WLogger::TimeStamp&);

  /*! \brief Writes a string in the current field.
   */
  WLogEntry& operator<< (const char *);

  /*! \brief Writes a string in the current field.
   */
  WLogEntry& operator<< (const std::string&);

  /*! \brief Writes a string in the current field.
   */
  WLogEntry& operator<< (const WString&);

  /*! \brief Writes a char value in the current field.
   */
  WLogEntry& operator<< (char);

  /*! \brief Writes a number value in the current field.
   */
  WLogEntry& operator<< (int);

  /*! \brief Writes a number value in the current field.
   */
  WLogEntry& operator<< (long long);

  /*! \brief Writes a number value in the current field.
   */
  WLogEntry& operator<< (double);

  /*! \brief Writes a pointer value in the current field.
   */
  template <typename T>
  WLogEntry& operator<< (T *t) {
    startField();
    if (impl_) {
      char buf[100];
      std::sprintf(buf, "%p", t);
      impl_->line_ << buf;
    }
    return *this;
  }

  /*! \brief Writes a value in the current field.
   */
  template <typename T>
  WLogEntry& operator<< (T t) {
    startField();
    if (impl_)
    {
      using std::to_string;
      impl_->line_ << to_string(t);
    }
    return *this;
  }

private:
  struct Impl {
    const WLogger& logger_;
    WStringStream line_;
    std::string type_, scope_;
    int field_;
    bool fieldStarted_;

    Impl(const WLogger& logger, const std::string& type);

    bool quote() const;

    void finish();
    void finishField();
    void nextField();
    void startField();
  };

  mutable std::unique_ptr<Impl> impl_;

  WLogEntry(const WLogger& logger, const std::string& type, bool mute);

  void startField();

  friend class WLogger;
};

WT_API WLogger& logInstance();

#ifdef DOXYGEN_ONLY
/*! \file */
/*! \brief Logging function
 *
 * This creates a new log entry, e.g.:
 *
 * \code
 * Wt::log("info") << "Doing something interesting now with " << appleCount() << " apples.";
 * \endcode
 *
 * \relates WLogger
 */
extern WLogEntry log(const std::string& type);
#else
WT_API extern WLogEntry log(const std::string& type);
#endif

}

#endif // WT_TARGET_JAVA

#ifdef WT_BUILDING
# ifndef WT_TARGET_JAVA
#  ifndef LOG4CPLUS
#   define LOGGER(s) static const char *logger = s

#   ifdef WT_DEBUG_ENABLED
#    define LOG_DEBUG_S(s,m) (s)->log("debug") << Wt::logger << ": " << m
#    define LOG_DEBUG(m) Wt::log("debug") << Wt::logger << ": " << m
#   else
#    define LOG_DEBUG_S(s,m)
#    define LOG_DEBUG(m)
#   endif

#   define LOG_INFO_S(s,m) (s)->log("info") << Wt::logger << ": " << m
#   define LOG_INFO(m) do { \
    if (Wt::logInstance().logging("info", Wt::logger))	\
      Wt::log("info") << Wt::logger << ": " << m;	\
    }  while(0)
#   define LOG_WARN_S(s,m) (s)->log("warning") << Wt::logger << ": " << m
#   define LOG_WARN(m) do { \
    if (Wt::logInstance().logging("warning", Wt::logger)) \
      Wt::log("warning") << Wt::logger << ": " << m; \
    } while(0)
#   define LOG_SECURE_S(s,m) (s)->log("secure") << Wt::logger << ": " << m
#   define LOG_SECURE(m) do { \
    if (Wt::logInstance().logging("secure", Wt::logger)) \
      Wt::log("secure") << Wt::logger << ": " << m; \
    } while(0)
#   define LOG_ERROR_S(s,m) (s)->log("error") << Wt::logger << ": " << m
#   define LOG_ERROR(m) do { \
    if (Wt::logInstance().logging("error", Wt::logger)) \
      Wt::log("error") << Wt::logger << ": " << m; \
    } while(0)

#  else // !LOG4CPLUS

#   define LOGGER(s) \
      static log4cplus::Logger logger = log4cplus::Logger::getInstance(s)

#   ifdef WT_DEBUG_ENABLED
#    define LOG_DEBUG(m) LOG4CPLUS_DEBUG(Wt::logger, m)
#    define LOG_DEBUG_S(s,m) LOG_DEBUG(m)
#   else
#    define LOG_DEBUG(m)
#    define LOG_DEBUG_S(s,m)
#   endif

#   define LOG_INFO(m) LOG4CPLUS_INFO(Wt::logger, m)
#   define LOG_INFO_S(s,m) LOG_NOTICE(m)
#   define LOG_WARN(m) LOG4CPLUS_WARN(Wt::logger, m)
#   define LOG_WARN_S(s,m) LOG_WARN(m)
#   define LOG_SECURE(m) LOG4CPLUS_ERROR(Wt::logger, "auth: " << m)
#   define LOG_SECURE_S(s,m) LOG_SECURE(m)
#   define LOG_ERROR(m) LOG4CPLUS_ERROR(Wt::logger, m)
#   define LOG_ERROR_S(s,m) LOG_ERROR(m)

#  endif // LOG4CPLUS
# else // WT_TARGET_JAVA

class Logger {
public:
  void debug(const std::ostream& s);
  void info(const std::ostream& s);
  void warn(const std::ostream& s);
  void error(const std::ostream& s);
};

# define LOGGER(s) Logger logger;

# define LOG_DEBUG_S(s,m) logger.debug(std::stringstream() << m)
# define LOG_DEBUG(m) logger.debug(std::stringstream() << m)
# define LOG_INFO_S(s,m) logger.info(std::stringstream() << m)
# define LOG_INFO(m) logger.info(std::stringstream() << m)
# define LOG_WARN_S(s,m) logger.warn(std::stringstream() << m)
# define LOG_WARN(m) logger.warn(std::stringstream() << m)
# define LOG_SECURE_S(s,m) logger.warn(std::stringstream() << "secure:" << m)
# define LOG_SECURE(m) logger.warn(std::stringstream() << "secure:" << m)
# define LOG_ERROR_S(s,m) logger.error(std::stringstream() << m)
# define LOG_ERROR(m) logger.error(std::stringstream() << m)

# endif // WT_TARGET_JAVA

# ifdef WT_DEBUG_ENABLED
#  ifndef WT_TARGET_JAVA
#   define WT_DEBUG(statement) do { \
      if (Wt::WApplication::instance()->debug()) statement; } \
    while(0)
#  else
#   define WT_DEBUG(statement)
#  endif
# else
#  define WT_DEBUG(statement)
# endif

#endif  // WT_BUILDING

#endif // WLOGGER_H_
