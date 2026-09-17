/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WGlobal.h>

#include <type_traits>

using namespace Wt;

BOOST_AUTO_TEST_SUITE( rtl_side_test )

BOOST_AUTO_TEST_CASE( values )
{
  static_assert(std::is_same<decltype(RtlSide::Left), const Side>::value,
                "RtlSide::Left must have type const Side");
  static_assert(std::is_same<decltype(RtlSide::Right), const Side>::value,
                "RtlSide::Right must have type const Side");
  static_assert(std::is_same<decltype(RtlSide::Top), const Side>::value,
                "RtlSide::Top must have type const Side");
  static_assert(std::is_same<decltype(RtlSide::Bottom), const Side>::value,
                "RtlSide::Bottom must have type const Side");
  static_assert(std::is_same<decltype(RtlSide::CenterX), const Side>::value,
                "RtlSide::CenterX must have type const Side");
  static_assert(std::is_same<decltype(RtlSide::CenterY), const Side>::value,
                "RtlSide::CenterY must have type const Side");

  static_assert(RtlSide::Left == Side::Right, "Left must alias Right");
  static_assert(RtlSide::Right == Side::Left, "Right must alias Left");
  static_assert(RtlSide::Top == Side::Top, "Top must alias Top");
  static_assert(RtlSide::Bottom == Side::Bottom, "Bottom must alias Bottom");
  static_assert(RtlSide::CenterX == Side::CenterX, "CenterX must alias CenterX");
  static_assert(RtlSide::CenterY == Side::CenterY, "CenterY must alias CenterY");

  const Side aliases[] = {
    RtlSide::Left, RtlSide::Right, RtlSide::Top,
    RtlSide::Bottom, RtlSide::CenterX, RtlSide::CenterY
  };
  const Side sides[] = {
    Side::Right, Side::Left, Side::Top,
    Side::Bottom, Side::CenterX, Side::CenterY
  };
  const int values[] = { 0x8, 0x4, 0x1, 0x2, 0x10, 0x20 };

  for (unsigned i = 0; i < 6; ++i) {
    Side side = Side::Top;
    side = aliases[i];
    BOOST_CHECK(side == sides[i]);
    BOOST_CHECK_EQUAL(static_cast<int>(side), values[i]);

    WFlags<Side> flags(aliases[i]);
    BOOST_CHECK(flags == WFlags<Side>(sides[i]));
    BOOST_CHECK(flags.test(aliases[i]));
  }
}

BOOST_AUTO_TEST_CASE( flags )
{
  static_assert(std::is_same<decltype(RtlSide::Left | RtlSide::Top), WFlags<Side> >::value,
                "Alias combinations must have type WFlags<Side>");
  static_assert(std::is_same<decltype(RtlSide::Left | Side::Top), WFlags<Side> >::value,
                "Mixed combinations must have type WFlags<Side>");

  WFlags<Side> aliases = RtlSide::Left | RtlSide::Top;
  BOOST_CHECK(aliases == (Side::Right | Side::Top));
  BOOST_CHECK(!aliases.test(Side::Left));

  WFlags<Side> mixed = RtlSide::Right | Side::Bottom;
  BOOST_CHECK(mixed == (Side::Left | Side::Bottom));
  BOOST_CHECK(!mixed.test(Side::Right));
  BOOST_CHECK((Side::Top | RtlSide::Left) == (Side::Top | Side::Right));

  aliases |= RtlSide::Bottom;
  aliases |= Side::Left;
  BOOST_CHECK(aliases == AllSides);
  BOOST_CHECK((RtlSide::Left | RtlSide::Right | RtlSide::Top | RtlSide::Bottom) == AllSides);
  BOOST_CHECK((RtlSide::Left | RtlSide::Right | RtlSide::CenterX) == AllHorizontalSide);
  BOOST_CHECK((RtlSide::Top | RtlSide::Bottom | RtlSide::CenterY) == AllVerticalSides);
  BOOST_CHECK((RtlSide::CenterX | RtlSide::CenterY) == (Side::CenterX | Side::CenterY));
  BOOST_CHECK_EQUAL(AllSides.value(), 0x0f);
  BOOST_CHECK_EQUAL(AllHorizontalSide.value(), 0x1c);
  BOOST_CHECK_EQUAL(AllVerticalSides.value(), 0x23);
}

BOOST_AUTO_TEST_CASE( widget_apis )
{
  for (LayoutDirection direction : { LayoutDirection::LeftToRight, LayoutDirection::RightToLeft }) {
    BOOST_TEST_CONTEXT("Layout direction: " << static_cast<int>(direction)) {
      Test::WTestEnvironment environment;
      WApplication app(environment);
      app.setLayoutDirection(direction);
      BOOST_REQUIRE(app.layoutDirection() == direction);

      WContainerWidget aliases;
      WContainerWidget sides;

      aliases.setFloatSide(RtlSide::Left);
      sides.setFloatSide(Side::Right);
      BOOST_CHECK(aliases.floatSide() == sides.floatSide());
      BOOST_CHECK(aliases.floatSide() == Side::Right);
      aliases.setFloatSide(RtlSide::Right);
      sides.setFloatSide(Side::Left);
      BOOST_CHECK(aliases.floatSide() == sides.floatSide());
      BOOST_CHECK(aliases.floatSide() == Side::Left);

      aliases.setClearSides(RtlSide::Left);
      sides.setClearSides(Side::Right);
      BOOST_CHECK(aliases.clearSides() == sides.clearSides());
      BOOST_CHECK(aliases.clearSides() == Side::Right);
      aliases.setClearSides(RtlSide::Left | RtlSide::Right);
      sides.setClearSides(Side::Right | Side::Left);
      BOOST_CHECK(aliases.clearSides() == sides.clearSides());

      aliases.setMargin(10, RtlSide::Left | RtlSide::Top);
      sides.setMargin(10, Side::Right | Side::Top);
      aliases.setMargin(20, RtlSide::Right | Side::Bottom);
      sides.setMargin(20, Side::Left | Side::Bottom);
      aliases.setOffsets(30, RtlSide::Right | RtlSide::Top);
      sides.setOffsets(30, Side::Left | Side::Top);
      aliases.setOffsets(40, Side::Bottom | RtlSide::Left);
      sides.setOffsets(40, Side::Bottom | Side::Right);
      aliases.setPadding(50, RtlSide::Left | RtlSide::Top);
      sides.setPadding(50, Side::Right | Side::Top);
      aliases.setPadding(60, RtlSide::Right | Side::Bottom);
      sides.setPadding(60, Side::Left | Side::Bottom);

      for (Side side : { Side::Left, Side::Right, Side::Top, Side::Bottom }) {
        BOOST_CHECK(aliases.margin(side) == sides.margin(side));
        BOOST_CHECK(aliases.offset(side) == sides.offset(side));
        BOOST_CHECK(aliases.padding(side) == sides.padding(side));
      }
      BOOST_CHECK(aliases.margin(RtlSide::Left) == sides.margin(Side::Right));
      BOOST_CHECK(aliases.margin(RtlSide::Right) == sides.margin(Side::Left));
      BOOST_CHECK(aliases.offset(RtlSide::Left) == sides.offset(Side::Right));
      BOOST_CHECK(aliases.offset(RtlSide::Right) == sides.offset(Side::Left));
      BOOST_CHECK(aliases.padding(RtlSide::Left) == sides.padding(Side::Right));
      BOOST_CHECK(aliases.padding(RtlSide::Right) == sides.padding(Side::Left));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
