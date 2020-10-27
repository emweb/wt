/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WFormModel.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( formmodel_test )
{
  WFormModel::Field field1 = "Field 1";
  WFormModel::Field field2 = "Field 2";
  const std::string field1s = field1;
  WFormModel::Field field1cp = field1s.c_str();

  // Different pointer
  BOOST_CHECK(field1 != field1cp);

  WFormModel model;

  model.addField(field1);
  model.addField(field2);

  {
    std::vector<WFormModel::Field> check{field1, field2};
    auto fields = model.fields();
    BOOST_CHECK_EQUAL_COLLECTIONS(fields.begin(), fields.end(),
                                  check.begin(), check.end());
  }

  // "Field 1" is already in the map, so it is not added
  model.addField(field1cp);

  {
    std::vector<WFormModel::Field> check{field1, field2};
    auto fields = model.fields();
    BOOST_CHECK_EQUAL_COLLECTIONS(fields.begin(), fields.end(),
                                  check.begin(), check.end());
  }

  model.setVisible(field1cp, false);

  BOOST_CHECK(!model.isVisible(field1));
  BOOST_CHECK(!model.isVisible(field1cp));

  model.setVisible(field1, true);

  BOOST_CHECK(model.isVisible(field1));
  BOOST_CHECK(model.isVisible(field1cp));

  // This should match field1, so it is removed
  model.removeField(field1cp);

  {
    std::vector<WFormModel::Field> check{field2};
    auto fields = model.fields();
    BOOST_CHECK_EQUAL_COLLECTIONS(fields.begin(), fields.end(),
                                  check.begin(), check.end());
  }
}
