/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WModelIndex.h>
#include <Wt/WStandardItemModel.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( modelindex_test1 )
{
  std::shared_ptr<WStandardItemModel> model = std::make_shared<WStandardItemModel>(10, 10);

  WModelIndex invalid;
  WModelIndex i1 = model->index(0, 0);
  WModelIndex i2 = model->index(0, 1);
  WModelIndex i3 = model->index(1, 0);
  WModelIndex i4 = model->index(1, 1);

  BOOST_REQUIRE(invalid == model->index(10, 0));

  BOOST_REQUIRE(invalid < i1);
  BOOST_REQUIRE(invalid < i2);
  BOOST_REQUIRE(invalid < i3);
  BOOST_REQUIRE(invalid < i4);
  BOOST_REQUIRE(i1 < i2);
  BOOST_REQUIRE(i1 < i3);
  BOOST_REQUIRE(i1 < i4);
  BOOST_REQUIRE(i2 < i3);
  BOOST_REQUIRE(i2 < i4);
  BOOST_REQUIRE(i3 < i4);

  // when flipped around, it should be false
  BOOST_REQUIRE(!(i1 < invalid));
  BOOST_REQUIRE(!(i2 < invalid));
  BOOST_REQUIRE(!(i3 < invalid));
  BOOST_REQUIRE(!(i4 < invalid));
  BOOST_REQUIRE(!(i2 < i1));
  BOOST_REQUIRE(!(i3 < i1));
  BOOST_REQUIRE(!(i4 < i1));
  BOOST_REQUIRE(!(i3 < i2));
  BOOST_REQUIRE(!(i4 < i2));
  BOOST_REQUIRE(!(i4 < i3));

  WModelIndex ri1 = i1;
  ri1.encodeAsRawIndex();
  WModelIndex ri2 = i2;
  ri2.encodeAsRawIndex();
  WModelIndex ri3 = i3;
  ri3.encodeAsRawIndex();
  WModelIndex ri4 = i4;
  ri4.encodeAsRawIndex();

  BOOST_REQUIRE(invalid < ri1);
  BOOST_REQUIRE(invalid < ri2);
  BOOST_REQUIRE(invalid < ri3);
  BOOST_REQUIRE(invalid < ri4);
  BOOST_REQUIRE(ri1 < i1);
  BOOST_REQUIRE(ri1 < i2);
  BOOST_REQUIRE(ri1 < i3);
  BOOST_REQUIRE(ri1 < i4);
  BOOST_REQUIRE(ri2 < i1);
  BOOST_REQUIRE(ri2 < i2);
  BOOST_REQUIRE(ri2 < i3);
  BOOST_REQUIRE(ri2 < i4);
  BOOST_REQUIRE(ri3 < i1);
  BOOST_REQUIRE(ri3 < i2);
  BOOST_REQUIRE(ri3 < i3);
  BOOST_REQUIRE(ri3 < i4);
  BOOST_REQUIRE(ri4 < i1);
  BOOST_REQUIRE(ri4 < i2);
  BOOST_REQUIRE(ri4 < i3);
  BOOST_REQUIRE(ri4 < i4);

  // when flipped around, it should be false
  BOOST_REQUIRE(!(ri1 < invalid));
  BOOST_REQUIRE(!(ri2 < invalid));
  BOOST_REQUIRE(!(ri3 < invalid));
  BOOST_REQUIRE(!(ri4 < invalid));
  BOOST_REQUIRE(!(i1 < ri1));
  BOOST_REQUIRE(!(i2 < ri1));
  BOOST_REQUIRE(!(i3 < ri1));
  BOOST_REQUIRE(!(i4 < ri1));
  BOOST_REQUIRE(!(i1 < ri2));
  BOOST_REQUIRE(!(i2 < ri2));
  BOOST_REQUIRE(!(i3 < ri2));
  BOOST_REQUIRE(!(i4 < ri2));
  BOOST_REQUIRE(!(i1 < ri3));
  BOOST_REQUIRE(!(i2 < ri3));
  BOOST_REQUIRE(!(i3 < ri3));
  BOOST_REQUIRE(!(i4 < ri3));
  BOOST_REQUIRE(!(i1 < ri4));
  BOOST_REQUIRE(!(i2 < ri4));
  BOOST_REQUIRE(!(i3 < ri4));
  BOOST_REQUIRE(!(i4 < ri4));

  // there should be some order in the raw indexes, but
  // it's not defined
  BOOST_REQUIRE(ri1 < ri2 || ri2 < ri1);
  BOOST_REQUIRE(ri1 < ri3 || ri3 < ri1);
  BOOST_REQUIRE(ri1 < ri4 || ri4 < ri1);
  BOOST_REQUIRE(ri2 < ri1 || ri1 < ri2);
  BOOST_REQUIRE(ri2 < ri3 || ri3 < ri2);
  BOOST_REQUIRE(ri2 < ri4 || ri4 < ri2);
  BOOST_REQUIRE(ri3 < ri1 || ri1 < ri3);
  BOOST_REQUIRE(ri3 < ri2 || ri2 < ri3);
  BOOST_REQUIRE(ri3 < ri4 || ri4 < ri3);
  BOOST_REQUIRE(ri4 < ri1 || ri1 < ri4);
  BOOST_REQUIRE(ri4 < ri2 || ri2 < ri4);
  BOOST_REQUIRE(ri4 < ri3 || ri3 < ri4);
}
