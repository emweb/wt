/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 * In addition to these terms, permission is also granted to use and
 * modify these two files (CgiParser.C and CgiParser.h) so long as the
 * copyright above is maintained, modifications are documented, and
 * credit is given for any use of the library.
 *
 * CGI parser modelled after the PERL implementation cgi-lib.pl 2.18 by
 * Steven E. Brenner with the following original copyright:

# Perl Routines to Manipulate CGI input
# cgi-lib@pobox.com
#
# Copyright (c) 1993-1999 Steven E. Brenner  
# Unpublished work.
# Permission granted to use and modify this library so long as the
# copyright above is maintained, modifications are documented, and
# credit is given for any use of the library.
#
# Thanks are due to many people for reporting bugs and suggestions

# For more information, see:
#     http://cgi-lib.stanford.edu/cgi-lib/  

 */

//#define DEBUG

#include <iostream>
#include <fstream>
#include <stdlib.h>

#ifdef HAVE_GNU_REGEX
#include <regex.h>
#else
#include <boost/regex.hpp>
#endif // HAVE_GNU_REGEX

#include <boost/tokenizer.hpp>

#include "CgiParser.h"
#include "WebRequest.h"
#include "WtException.h"

using std::memmove;
using std::strcpy;
using std::strtol;

namespace {
#ifndef HAVE_GNU_REGEX
  const boost::regex boundary_e("\\bboundary=(?:(?:\"([^\"]+)\")|(\\S+))",
			       boost::regex::perl|boost::regex::icase);
  const boost::regex name_e("\\bname=(?:(?:\"([^\"]+)\")|([^\\s:;]+))",
			       boost::regex::perl|boost::regex::icase);
  const boost::regex filename_e("\\bfilename=(?:(?:\"([^\"]*)\")|([^\\s:;]+))",
			       boost::regex::perl|boost::regex::icase);
  const boost::regex content_e("^\\s*Content-type:"
			       "\\s*(?:(?:\"([^\"]+)\")|([^\\s:;]+))",
			       boost::regex::perl|boost::regex::icase);
  const boost::regex content_disposition_e("^\\s*Content-Disposition:",
			       boost::regex::perl|boost::regex::icase);
  const boost::regex content_type_e("^\\s*Content-Type:",
			       boost::regex::perl|boost::regex::icase);

  bool fishValue(const std::string& text,
		 const boost::regex& e, std::string& result)
  {
    boost::smatch what;

    if (boost::regex_search(text, what, e)) {
      result = what[1] + what[2];
      return true;
    } else
      return false;
  }

  bool regexMatch(const std::string& text, const boost::regex& e)
  {
    return boost::regex_search(text, e);
  }

#else
  regex_t boundary_e, name_e, filename_e, content_e,
    content_disposition_e, content_type_e;

  const char *boundary_ep = "\\bboundary=((\"([^\"]*)\")|([^ \t]*))";
  const char *name_ep = "\\bname=((\"([^\"]*)\")|([^ \t:;]*))";
  const char *filename_ep = "\\bfilename=((\"([^\"]*)\")|([^ \t:;]*))";
  const char *content_ep = "^[ \t]*Content-type:"
    "[ \t]*((\"([^\"]*)\")|([^ \t:;]*))";
  const char *content_disposition_ep = "^[ \t]*Content-Disposition:";
  const char *content_type_ep = "^[ \t]*Content-Type:";

  bool fishValue(const std::string& text,
		 regex_t& e1, std::string& result)
  {
    regmatch_t pmatch[5];
    int res = regexec(&e1, text.c_str(), 5, pmatch, 0);

    if (res == 0) {      
      if (pmatch[3].rm_so != -1)
	result = text.substr(pmatch[3].rm_so,
			     pmatch[3].rm_eo - pmatch[3].rm_so);
      if (pmatch[4].rm_so != -1)
	result = text.substr(pmatch[4].rm_so,
			     pmatch[4].rm_eo - pmatch[4].rm_so);

      return true;
    } else
      return false;
  }

  bool regexMatch(const std::string& text, regex_t& e)
  {
    regmatch_t pmatch[1];

    return regexec(&e, text.c_str(), 1, pmatch, 0) == 0;
  }

  class RegInitializer {
  protected:
    static bool regInitialized_;

  public:
    RegInitializer()
    {}

    ~RegInitializer() {
      cleanup();
    }

    static void setup() {
      if (!regInitialized_) {
	regcomp(&boundary_e, boundary_ep, REG_ICASE | REG_EXTENDED);
	regcomp(&name_e, name_ep, REG_ICASE | REG_EXTENDED);
	regcomp(&filename_e, filename_ep, REG_ICASE | REG_EXTENDED);
	regcomp(&content_e, content_ep, REG_ICASE | REG_EXTENDED);
	regcomp(&content_disposition_e, content_disposition_ep,
		REG_ICASE | REG_EXTENDED);
	regcomp(&content_type_e, content_type_ep, REG_ICASE | REG_EXTENDED);
	regInitialized_ = true;
      }
    }

    static void cleanup() {
      if (regInitialized_) {
	regfree(&boundary_e);
	regfree(&name_e);
	regfree(&filename_e);
	regfree(&content_e);
	regfree(&content_disposition_e);
	regfree(&content_type_e);
	regInitialized_ = false;
      }
    }
  };

  bool RegInitializer::regInitialized_ = false;

  static RegInitializer regInitializer;
#endif
}

namespace Wt {

void CgiParser::init()
{
#ifdef HAVE_GNU_REGEX
  RegInitializer::setup();
#endif
}

#ifndef WIN32
const int CgiParser::BUFSIZE;
const int CgiParser::MAXBOUND;
#endif
char CgiParser::buf_[BUFSIZE + MAXBOUND];

CgiParser::CgiParser(int maxPostData)
  : maxPostData_(maxPostData)
{
}

CgiParser::~CgiParser()
{
  for (EntryMap::const_iterator i = entries_.begin();
       i != entries_.end(); ++i)
    delete i->second;
}

CgiEntry *CgiParser::getEntry(const std::string key) const
{
  EntryMap::const_iterator i = entries_.find(key);
  if (i != entries_.end())
    return i->second;
  else
    return 0;
}

void CgiParser::parse(WebRequest& request)
{
  unsigned len = request.contentLength();
  std::string type = request.contentType();
  std::string meth = request.requestMethod();

  postDataExceeded_ = (len > maxPostData_ ? len : 0);

  std::string queryString = request.queryString();

  if (meth == "POST"
      && type.find("application/x-www-form-urlencoded") == 0) {
    char *buf = new char[len + 1];

    request.in().read(buf, len);
    if (request.in().gcount() != (int)len) {
      delete[] buf;
      throw WtException("Unexpected short read.");
    }
    buf[len] = 0;

    // This is a special Wt feature, I do not think it standard.
    // For POST, parameters in url-encoded URL are still parsed.
    if (!queryString.empty())
      queryString += '&';

    queryString += buf;
    delete[] buf;
  }

  if (!queryString.empty()) {
    typedef boost::tokenizer<boost::char_separator<char> > amp_tok;
    amp_tok tok(queryString, boost::char_separator<char>("&"));

#ifdef DEBUG
    std::cerr << queryString << std::endl;
#endif // DEBUG

    for (amp_tok::iterator i = tok.begin(); i != tok.end(); ++i) {
      std::string pair = *i;

#ifdef DEBUG
      std::cerr << pair << std::endl;
#endif // DEBUG

      // convert plus to space
      replaceAll(pair, '+', ' ');

      // split into key and value
      std::string::size_type equalPos = pair.find('=');
      std::string key = pair.substr(0, equalPos);
      std::string value  = (equalPos != std::string::npos) 
	? pair.substr(equalPos + 1) : "";

      // convert %XX from hex numbers to alphanumeric
      replaceHexTokens(key);
      replaceHexTokens(value);

#ifdef DEBUG
      std::cerr << key << ": \"" << value << "\"" << std::endl;
#endif // DEBUG

      CgiEntry *e = createEntry(key);
      e->setValue(value);
    }
  }

  if (type.find("multipart/form-data") == 0) {
    if (meth != "POST") {
      throw WtException("Invalid method for multipart/form-data: " + meth);
    }

    readMultipartData(request, type, len);
  }
}

void CgiParser::readMultipartData(WebRequest& request,
				  const std::string type, int len)
{
  std::string boundary;
    
  if (!fishValue(type, boundary_e, boundary))
    throw WtException("Could not find a boundary for multipart data.");
    
  boundary = "--" + boundary;

  buflen_ = 0;
  left_ = len;
  spoolStream_ = 0;
  entry_ = 0;

  parseBody(request, boundary);
  for (;;) {
    if (!parseHead(request))
      break;
    if (!parseBody(request,boundary)) 
      break;
  }
}

/*
 * Read until finding the boundary, saving to resultString or
 * resultFile. The boundary itself is not consumed.
 *
 * tossAtBoundary controls how many characters extra (<0)
 * or few (>0) are saved at the start of the boundary in the result.
 */
void CgiParser::readUntilBoundary(WebRequest& request,
				  const std::string boundary,
				  int tossAtBoundary,
				  std::string *resultString,
				  std::ostream *resultFile)
{
  int bpos;

  while ((bpos = index(boundary)) == -1) {
    if ((left_ == 0) && (buflen_ == 0))
      throw WtException("CgiParser: reached end of input while seeking end of "
			"headers or content. Format of CGI input is wrong");

    /* save BUFSIZE from buffer to file or value string*/
    int save = std::min(buflen_, BUFSIZE);
    if (resultString)
      *resultString += std::string(buf_, save);
    if (resultFile) 
      resultFile->write(buf_, save);

    /* wind buffer */
    windBuffer(save);

    unsigned amt = std::min(left_, BUFSIZE + MAXBOUND - buflen_);

    request.in().read(buf_ + buflen_, amt);    
    if (request.in().gcount() != (int)amt)
      throw WtException("CgiParser: short read");

    left_ -= amt;
    buflen_ += amt;
  }

  if (resultString)
    *resultString += std::string(buf_, bpos - tossAtBoundary);
  if (resultFile)
    resultFile->write(buf_, bpos - tossAtBoundary);

  /* wind buffer */
  windBuffer(bpos);
}

void CgiParser::windBuffer(int offset)
{
  if (offset < buflen_) {
    memmove(buf_, buf_ + offset, buflen_ - offset);
    buflen_ -= offset;
  } else
    buflen_ = 0;
}

int CgiParser::index(const std::string search)
{
  std::string bufS = std::string(buf_, buflen_);

  std::string::size_type i = bufS.find(search);

  if (i == std::string::npos)
    return -1;
  else
    return i;
}

bool CgiParser::parseHead(WebRequest& request)
{
  std::string head;
  readUntilBoundary(request, "\r\n\r\n", -2, &head, 0);

  std::string name;
  std::string fn;
  std::string ctype;

  for (unsigned current = 0; current < head.length();) {
    /* read line by line */
    std::string::size_type i = head.find("\r\n", current);
    const std::string text = head.substr(current, (i == std::string::npos
						   ? std::string::npos
						   : i - current));

    if (regexMatch(text, content_disposition_e)) {
      fishValue(text, name_e, name);
      fishValue(text, filename_e, fn);
    }

    if (regexMatch(text, content_type_e)) {
      fishValue(text, content_e, ctype);
    }

    current = i + 2;
  }

#ifdef DEBUG
  std::cerr << "name: " << name 
	    << " ct: " << ctype 
	    << " fn: " << fn << std::endl;
#endif

  entry_ = createEntry(name);

  if (!fn.empty()) {
    if (!postDataExceeded_) {
      /*
       * It is not easy to create a std::ostream pointing to a
       * temporary file name.
       */
#ifndef WIN32
      char spool[20];
      strcpy(spool, "/tmp/wtXXXXXX");

      int i = mkstemp(spool);
      close(i);
#else
      char spool[2 * L_tmpnam];
      // FIXME: check for error retval for tmpnam
      // FIXME: where is this tmp file created ? cwd ? is that ok ?
      tmpnam(spool);
#endif

      spoolStream_ = new std::ofstream(spool, std::ios::out | std::ios::binary);

      entry_->setStream(spool, fn, ctype);
    } else {
      spoolStream_ = 0;
    }
  }

  windBuffer(4);

  return true;
}

bool CgiParser::parseBody(WebRequest& request, const std::string boundary)
{
  std::string value;

  readUntilBoundary(request, boundary, 2,
		    spoolStream_ ? 0 : (entry_ ? &value : 0),
		    spoolStream_);

  if (spoolStream_) {
    delete spoolStream_;
    spoolStream_ = 0;
  } else
    if (entry_) {
#ifdef DEBUG
      std::cerr << "value: \"" << value << "\"" << std::endl;
#endif
      entry_->setValue(value);
    }

  entry_ = 0;

  if (std::string(buf_ + boundary.length(), 2) == "--")
    return false;

  windBuffer(boundary.length() + 2);

  return true;
}

void CgiParser::replaceAll(std::string& v, char from, char to)
{
  for (std::string::size_type i = v.find(from);
       i != std::string::npos;
       i = v.find(from, i+1))
    v[i] = to;
}

void CgiParser::replaceHexTokens(std::string& v)
{
  for (unsigned i = 0; i < (unsigned)std::max(0, (int)v.length() - 2); ++i) {
    if (v[i] == '%') {
      std::string h = v.substr(i + 1, 2);
      char *e = 0;
      int hval = strtol(h.c_str(), &e, 16);

      if (*e != 0)
	continue; // not a proper %XX with XX hexadecimal format

      v.replace(i, 3, 1, (char)hval);
    }
  }
}

CgiEntry *CgiParser::createEntry(const std::string param)
{
  CgiEntry *result = new CgiEntry();

  if (entries_.find(param) != entries_.end()) {
    CgiEntry *l = entries_[param];

    for (; l->next_; l = l->next_)
      ;

    l->next_ = result;
  } else
    entries_[param] = result;

  return result;
}

CgiEntry::CgiEntry()
  : isFile_(false),
    fileIsStolen_(false),
    next_(0)
{ }

CgiEntry::~CgiEntry()
{
  if (next_)
    delete next_;

  if (isFile_ && !fileIsStolen_)
    unlink(value_.c_str());
}

void CgiEntry::stealFile()
{
  fileIsStolen_ = true;
}

void CgiEntry::setValue(const std::string value)
{
  value_ = value;
  isFile_ = false;
}

void CgiEntry::setStream(const std::string spoolName,
			 const std::string clientFilename,
			 const std::string clientType)
{
  value_ = spoolName;
  cfn_ = clientFilename;
  ct_ = clientType;
  isFile_ = true;
}

} // namespace Wt
