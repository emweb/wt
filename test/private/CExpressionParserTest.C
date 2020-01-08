/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WMessageResources.h>
#include <boost/test/unit_test.hpp>

namespace {
  int eval(std::string expression, ::uint64_t n) 
  {
    return Wt::WMessageResources::evalPluralCase(expression, n);
  }
}

BOOST_AUTO_TEST_CASE( cexpression_basic_expression_test )
{
  {
    std::string e = "1 + 2";
    BOOST_REQUIRE(eval(e, -1) == 3);
  }

  {
    std::string e = "1 + n";
    BOOST_REQUIRE(eval(e, 2) == 3);
  }

  {
    std::string e = "3 - n";
    BOOST_REQUIRE(eval(e, 2) == 1);
  }

  {
    std::string e = "3 * n";
    BOOST_REQUIRE(eval(e, 2) == 6);
  }

  {
    std::string e = "4 / n";
    BOOST_REQUIRE(eval(e, 2) == 2);
  }

  {
    std::string e = "5 % n";
    BOOST_REQUIRE(eval(e, 2) == 1);
  }

  {
    std::string e = "(5 + n) * (n + 2) + (n * n)";
    BOOST_REQUIRE(eval(e, 3) == 49);
  }

  {
    std::string e = "n == 4";
    BOOST_REQUIRE(eval(e, 4) == 1);
  }

  {
    std::string e = "n == 3";
    std::string e2 = "n != 3";
    BOOST_REQUIRE(eval(e, 4) == 0);
    BOOST_REQUIRE(eval(e2, 4) == 1);
    BOOST_REQUIRE(eval(e + " && " + e2, 4) == 0);
    BOOST_REQUIRE(eval(e + " || " + e2, 4) == 1);

    std::string te = e + " ? n + 3 : n * n";
    BOOST_REQUIRE(eval(te, 3) == 6);
    BOOST_REQUIRE(eval(te, 4) == 16);
  }

  {
    std::string lt_e = "n < 3";
    BOOST_REQUIRE(eval(lt_e, 2) == 1);
    BOOST_REQUIRE(eval(lt_e, 3) == 0);
    BOOST_REQUIRE(eval(lt_e, 4) == 0);

    std::string lte_e = "n <= 3";
    BOOST_REQUIRE(eval(lte_e, 2) == 1);
    BOOST_REQUIRE(eval(lte_e, 3) == 1);
    BOOST_REQUIRE(eval(lte_e, 4) == 0);

    std::string gt_e = "n > 3";
    BOOST_REQUIRE(eval(gt_e, 2) == 0);
    BOOST_REQUIRE(eval(gt_e, 3) == 0);
    BOOST_REQUIRE(eval(gt_e, 4) == 1);

    std::string gte_e = "n >= 3";
    BOOST_REQUIRE(eval(gte_e, 2) == 0);
    BOOST_REQUIRE(eval(gte_e, 3) == 1);
    BOOST_REQUIRE(eval(gte_e, 4) == 1);

    std::string combined = 
      lt_e + " || " + lte_e + " && " + gt_e + " && " + gte_e;
    BOOST_REQUIRE(eval(combined, 2) == 1);
    BOOST_REQUIRE(eval(combined, 3) == 0);
    BOOST_REQUIRE(eval(combined, 4) == 0);
  }

  {
    std::string e = "2 + 3 * n";
    BOOST_REQUIRE(eval(e, 2) == 8);
  }

  {
    std::string e = "2 < 3 == n";
    BOOST_REQUIRE(eval(e, 2) == 0);

    e += " || 1";
    BOOST_REQUIRE(eval(e, 2) == 1);

    e += " ? 2 : 4";
    BOOST_REQUIRE(eval(e, 2) == 2);
  }
}

BOOST_AUTO_TEST_CASE( cexpression_basic_languagesTest )
{
  //Polish language expression
  {
    std::string e 
      = "n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2";
    BOOST_REQUIRE(eval(e, 1) == 0);
    BOOST_REQUIRE(eval(e, 3) == 1);
    BOOST_REQUIRE(eval(e, 22) == 1);
    BOOST_REQUIRE(eval(e, 6) == 2);
    BOOST_REQUIRE(eval(e, 30) == 2);
  }

  //Japanese/Vietnamese/Korean language expression
  {
    std::string e = "0";
    BOOST_REQUIRE(eval(e, 0) == 0);
    BOOST_REQUIRE(eval(e, 1) == 0);
    BOOST_REQUIRE(eval(e, 3) == 0);
    BOOST_REQUIRE(eval(e, 22) == 0);
  }

  //English, German, Dutch, Swedish, Danish, Norwegian, Faroese,
  //Spanish, Portuguese, Italian, Bulgarian 
  //Greek
  //Finnish, Estonian, Hungarian 
  //Hebrew
  //Esperanto
  //Turkish
  //language expression
  {
    std::string e = "n != 1";
    BOOST_REQUIRE(eval(e, 0) == 1);
    BOOST_REQUIRE(eval(e, 1) == 0);
    BOOST_REQUIRE(eval(e, 3) == 1);
    BOOST_REQUIRE(eval(e, 22) == 1);
  }

  //Brazilian Portuguese, French language expression
  {
    std::string e = "n > 1";
    BOOST_REQUIRE(eval(e, 0) == 0);
    BOOST_REQUIRE(eval(e, 1) == 0);
    BOOST_REQUIRE(eval(e, 3) == 1);
    BOOST_REQUIRE(eval(e, 22) == 1);
  }
  
  //Russian, Ukrainian, Serbian, Croatian language expression
  {
    std::string e = "n%10==1 && n%100!=11 ? 0 :"
      "n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2";

    BOOST_REQUIRE(eval(e, 0) == 2);
    BOOST_REQUIRE(eval(e, 1) == 0);
    BOOST_REQUIRE(eval(e, 2) == 1);
    BOOST_REQUIRE(eval(e, 3) == 1);
    BOOST_REQUIRE(eval(e, 4) == 1);

    BOOST_REQUIRE(eval(e, 11) == 2);
    BOOST_REQUIRE(eval(e, 12) == 2);
    BOOST_REQUIRE(eval(e, 13) == 2);
    BOOST_REQUIRE(eval(e, 14) == 2);
    
    BOOST_REQUIRE(eval(e, 211) == 2);
    BOOST_REQUIRE(eval(e, 212) == 2);
    BOOST_REQUIRE(eval(e, 213) == 2);
    BOOST_REQUIRE(eval(e, 214) == 2);

    BOOST_REQUIRE(eval(e, 201) == 0);
    BOOST_REQUIRE(eval(e, 202) == 1);
    BOOST_REQUIRE(eval(e, 203) == 1);
    BOOST_REQUIRE(eval(e, 204) == 1);
  }
}
