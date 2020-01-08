/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Http/Request.h"

using namespace Wt::Http;

BOOST_AUTO_TEST_CASE( http_rangeTest1 )
{
  // These are all tests wo filesize
  Request::ByteRangeSpecifier ranges;
  
  // Basic tests
  ranges = Request::getRanges("bytes=0-24", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 24);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=1-2, 15-18, 600-900", -1);
  BOOST_REQUIRE(ranges.size() == 3);
  BOOST_REQUIRE(ranges[0].firstByte() == 1);
  BOOST_REQUIRE(ranges[0].lastByte() == 2);
  BOOST_REQUIRE(ranges[1].firstByte() == 15);
  BOOST_REQUIRE(ranges[1].lastByte() == 18);
  BOOST_REQUIRE(ranges[2].firstByte() == 600);
  BOOST_REQUIRE(ranges[2].lastByte() == 900);
  BOOST_REQUIRE(ranges.isSatisfiable());
  // Till end-of-file...
  ranges = Request::getRanges("bytes=100-", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 100);
  BOOST_REQUIRE(ranges[0].lastByte() == std::numeric_limits< ::uint64_t>::max());
  BOOST_REQUIRE(ranges.isSatisfiable());
  // suffix-byte-range-spec. When filesize is unknown, this does
  // not make sense
  ranges = Request::getRanges("bytes=-100", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  // examples from the RFC
  ranges = Request::getRanges("bytes=0-499", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 499);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-999", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-500", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=9500-", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 9500);
  BOOST_REQUIRE(ranges[0].lastByte() == std::numeric_limits< ::uint64_t>::max());
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=0-0,-1", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-600,601-999", -1);
  BOOST_REQUIRE(ranges.size() == 2);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 600);
  BOOST_REQUIRE(ranges[1].firstByte() == 601);
  BOOST_REQUIRE(ranges[1].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-700,601-999", -1);
  BOOST_REQUIRE(ranges.size() == 2);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 700);
  BOOST_REQUIRE(ranges[1].firstByte() == 601);
  BOOST_REQUIRE(ranges[1].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());


  // Whitespace test
  ranges = Request::getRanges(" bytes = 0 - 2 ", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 2);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("\tbytes\t=\t0\t-\t2\t", -1);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 2);
  BOOST_REQUIRE(ranges.isSatisfiable());

  // 'Syntax error' tests
  // Starting with some obvious ones
  ranges = Request::getRanges("bytes==1,2", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("1-2=bytes", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=1-2;", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=1-2-3", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=1,2,3", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("octets=1-2", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=0-1,a-b", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  // Explicitly mentioned in the RFC as 'syntactically invalid'
  ranges = Request::getRanges("bytes=2-0", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());
  // One invalid range = ignore the complete Range header
  ranges = Request::getRanges("bytes=1-2,2-0", -1);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(ranges.isSatisfiable());

}

BOOST_AUTO_TEST_CASE( http_rangeTest2 )
{
  Request::ByteRangeSpecifier ranges;

  // Queries past end of file
  ranges = Request::getRanges("bytes=10-2000", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 10);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=999-2000", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 999);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  // To the end of file
  ranges = Request::getRanges("bytes=100-", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 100);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  //cornercase queries
  ranges = Request::getRanges("bytes=10-999", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 10);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-1000", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());

  // suffix-byte-range-spec. When filesize is unknown, this does
  // not make sense
  ranges = Request::getRanges("bytes=-100", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 900);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-2000", 1000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-2000", 0);
  BOOST_REQUIRE(ranges.size() == 0); //??
  BOOST_REQUIRE(!ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-0", 0);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(!ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-0", 100);
  BOOST_REQUIRE(ranges.size() == 0);
  BOOST_REQUIRE(!ranges.isSatisfiable());

  // examples from the RFC
  ranges = Request::getRanges("bytes=0-499", 10000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 499);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-999", 10000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=-500", 10000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 9500);
  BOOST_REQUIRE(ranges[0].lastByte() == 9999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=9500-", 10000);
  BOOST_REQUIRE(ranges.size() == 1);
  BOOST_REQUIRE(ranges[0].firstByte() == 9500);
  BOOST_REQUIRE(ranges[0].lastByte() == 9999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=0-0,-1", 10000);
  BOOST_REQUIRE(ranges.size() == 2);
  BOOST_REQUIRE(ranges[0].firstByte() == 0);
  BOOST_REQUIRE(ranges[0].lastByte() == 0);
  BOOST_REQUIRE(ranges[1].firstByte() == 9999);
  BOOST_REQUIRE(ranges[1].lastByte() == 9999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-600,601-999", 10000);
  BOOST_REQUIRE(ranges.size() == 2);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 600);
  BOOST_REQUIRE(ranges[1].firstByte() == 601);
  BOOST_REQUIRE(ranges[1].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
  ranges = Request::getRanges("bytes=500-700,601-999", 10000);
  BOOST_REQUIRE(ranges.size() == 2);
  BOOST_REQUIRE(ranges[0].firstByte() == 500);
  BOOST_REQUIRE(ranges[0].lastByte() == 700);
  BOOST_REQUIRE(ranges[1].firstByte() == 601);
  BOOST_REQUIRE(ranges[1].lastByte() == 999);
  BOOST_REQUIRE(ranges.isSatisfiable());
}
