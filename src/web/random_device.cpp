#include <Wt/WConfig.h>

#ifdef WT_NO_BOOST_RANDOM
/* boost random_device.cpp implementation
 *
 * Copyright Jens Maurer 2000
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id: random_device.cpp,v 1.4 2008/05/14 09:34:04 kdf Exp $
 *
 */

#if WIN32
// Don't link to boost_random
#define BOOST_RANDOM_NO_LIB
// And when compiling, don't add declspec to the classnames.
// We have our own implementation of random_device.cpp (older boosts
// didn't support windows well)
#ifdef BOOST_ALL_DYN_LINK
#undef BOOST_ALL_DYN_LINK
#endif
#ifdef BOOST_RANDOM_DYN_LINK
#undef BOOST_RANDOM_DYN_LINK
#endif
#endif

#include <boost/lexical_cast.hpp>
#include <boost/nondet_random.hpp>
#include <string>
#include <cassert>

#ifndef WIN32
#  ifndef __CYGWIN__
#    define USE_URANDOM
#  endif
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#  define USE_WIN32_CRYPT
#endif

#ifdef USE_URANDOM
#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
const bool boost::random_device::has_fixed_range;
const boost::random_device::result_type boost::random_device::min_value;
const boost::random_device::result_type boost::random_device::max_value;
#endif

// the default is the unlimited capacity device, using some secure hash
// try "/dev/random" for blocking when the entropy pool has drained
const char * const boost::random_device::default_token = "/dev/urandom";

/*
 * This uses the POSIX interface for unbuffered reading.
 * Using buffered std::istream would consume entropy which may
 * not actually be used.  Entropy is a precious good we avoid
 * wasting.
 */

#if defined(__GNUC__) && defined(_CXXRT_STD_NAME)
// I have severe difficulty to get the POSIX includes to work with
// -fhonor-std and Dietmar K�hl's standard C++ library.  Hack around that
// problem for now.
extern "C" {
static const int O_RDONLY = 0;
extern int open(const char *__file, int __oflag, ...);
extern int read(int __fd, __ptr_t __buf, size_t __nbytes);
extern int close(int __fd);
}
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // open
#include <unistd.h>   // read, close
#endif

#include <errno.h>    // errno
#include <string.h>   // strerror
#include <stdexcept>  // std::invalid_argument


class boost::random_device::impl
{
public:
  impl(const std::string & token) : path(token) {
    fd = open(token.c_str(), O_RDONLY);
    if(fd < 0)
      error("cannot open");
  }

  ~impl() { if(close(fd) < 0) error("could not close"); }

  unsigned int next() {
    unsigned int result;
    long sz = read(fd, reinterpret_cast<char *>(&result), sizeof(result));
    if(sz == -1)
      error("error while reading");
    else if(sz != sizeof(result)) {
      errno = 0;
      error("EOF while reading");
    }
    return result;
  }

private:
  void error(const std::string & msg) {
    throw std::invalid_argument("boost::random_device: " + msg + 
                                " random-number pseudo-device " + path + 
                                ": " + strerror(errno));
  }
  const std::string path;
  int fd;
};

#endif

#ifdef USE_WIN32_CRYPT

#include <windows.h>
#include <wincrypt.h>
#include <stdexcept>
const char * const boost::random_device::default_token = "";

// Note about thread-safety: according to my reading of the MSDN page 
// 'Threading Issues with Cryptographic Service Providers', both the
// CryptAcquireContext and CryptGenRandom functions are thread-safe,
// as we specify CRYPT_VERIFYCONTEXT (IO operations will not be performed
// and aparently those are the only ones causing threading problems)

class boost::random_device::impl
{
public:
  impl(const std::string & ) {
     InitOk_ = CryptAcquireContext(&hProv_, 0, 0,
         PROV_RSA_FULL, CRYPT_VERIFYCONTEXT|CRYPT_SILENT) ? true : false;
     if (!InitOk_) error("error while initializing crypt random context");
  }

  ~impl() { if (InitOk_) CryptReleaseContext(hProv_, 0); }

  unsigned int next() {
    unsigned int result = 0;
    if((!InitOk_) ||
       (!CryptGenRandom(hProv_, sizeof(result), (BYTE*)&result))) {
      error("error while generating random number");
    }
    return result % RAND_MAX;
  }

private:
  void error(const std::string & msg) {
     throw std::invalid_argument(msg + " ; error code " + boost::lexical_cast<std::string>(GetLastError()));
  }
  bool InitOk_;
  HCRYPTPROV hProv_;
};
#endif

boost::random_device::random_device(const std::string& token)
  : pimpl(new impl(token))
{
  assert((std::numeric_limits<result_type>::max)() == max_value);
}

boost::random_device::~random_device()
{
  // the complete class impl is now visible, so we're safe
  // (see comment in random.hpp)
  delete pimpl;
}

double boost::random_device::entropy() const
{
  return 10;
}

unsigned int boost::random_device::operator()()
{
  return pimpl->next();
}

#endif
