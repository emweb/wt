/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

// This test ensures that the SeleniumFixture correctly sets up the
// server, it will call for the API initialization, and load the
// initial page.
SELENIUM_TEST(selenium_setup, Wt::WApplication)
END_SELENIUM_TEST
