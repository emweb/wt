# Selenium Test Framework for Wt

This directory contains a test framework for running Selenium-based browser tests
against Wt web applications. To this end Python3 is used as the inbetween layer
for Selenium calls, as no C++ Selenium binding exists.

## Overview

The framework provides:

1. **C++ Test Framework** (`SeleniumTest.h`) - Sets up a local WServer and manages the test lifecycle.
2. **Python Translation** (`PythonInterpreter.h`) - Translates calls to Selenium from C++ to Python3.
3. **Self-tests** (`ElementTest.C`, `InteractionTest.C`, `LocatorTest.C`) - Demonstrate how to use the framework.

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                     C++ Test (Boost.Test)                │
│  ┌────────────────────────────────────────────────────┐  │
│  │  ServerTest                                        │  │
│  │  - Similar to Boost test philosophy                │  │
│  │  - Creates WServer with WApplication               │  │
│  │  - Starts server on localhost (on random port)     │  │
│  │  - Exposes several untility classes/functions      │  │
│  └────────────────────┬───────────────────────────────┘  │
│                       │ Creates                          │
│  ┌────────────────────▼───────────────────────────────┐  │
│  │  SeleniumAPI                                       │  │
│  │  - Sets up the connection                          │  │
│  │  - Passes URL and browser type                     │  │
│  │  - Can be used to query elements                   │  │
│  └────────────────────┬───────────────────────────────┘  │
│                       │ Uses                             │
│  ┌────────────────────▼───────────────────────────────┐  │
│  │  PythonInterpreter                                 │  │
│  │  - Handles Python virtual environment activation   │  │
│  │  - Imports the correct modules                     │  │
│  │  - Translates C++ calls and concepts to Python3    │  │
│  └────────────────────┬───────────────────────────────┘  │
└───────────────────────┼──────────────────────────────────┘
                        │
                        │ Python
                        │
                        ▼
┌──────────────────────────────────────────────────────────┐
│             Python3 Selenium Environment                 │
│  ┌────────────────────────────────────────────────────┐  │
│  │  WebDriver (Chrome/Firefox)                        │  │
│  │  - Connects to localhost                           │  │
│  │  - Retrieves/interacts with page elements          │  │
│  │  - Validates page behavior                         │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

## Structure

### C++ Framework

- **`SeleniumTest.h`** - Main framework header
  - `SELENIUM_TEST(name, app)/END_SELENIUM_TEST`:
      Test identifier and wrapper. A Boost unit test will be created carrying
      the `name` that is specified. A WServer is set up, and will define an
      entry point for the `app` (`WApplication`) type.
  - `SeleniumAPI`:
      This is the API that allows you to interact with Selenium (through Python3
      interpretation). It will set up the connection to the browser, and load
      the initial page. These calls happen within the `SELENIUM_TEST`.
      The function of note is `getElement(FindBy findBy, const std::string& value)`.
      This allows you to query Selenium, to find an element on the page.
      The `FindBy` dictates the strategy that it to be used for retrieval,
      and thus specified the `value` format. Supported types are:
      - `ID`: retrieve the element by its id (like `$("#...")`)
      - `XPATH`: retrieve the element by its XPATH (like `//div[...]/...`)
      - `CSS_SELECTOR`: retrieve the element by its CSS selector (like `div #...`)
      - `CLASS_NAME`: retrieve the element by its classname (like  `$("....")`)
      - `TAG_NAME`: retrieve the element by its tag (like  `div`)
      - `NAME`: retrieve the element by its name attribute (like  `<div name="..."`)
  - `SeleniumWait`:
      This is the API that allows you to wait for interactive elements on the
      page. If an element is suspected to change, or a certain condition needs
      to be true for the test to carry on, this class can be used. There are
      several functions here that can be used to this end.
      - `until(std::function<bool()>)`: will wait until the function returns
        `true`.
      - `until(Element, Element::method, std::string), `: will wait until the
         value returned by `Element`'s `method` equal `string`.
      - `until(Element, Element::method, std::string1, std::string2), `: will
         wait until the value returned by `Element`'s `method` equal `string2`,
         using the additional `string1` argument. This is useful for e.g. CSS
         values, where the method will also take `string1` as input.
      If the condition that is waited upon is never reached, a timeout will
      occur. By default this is set to `10 seconds`. Each of the above functions
      can take a `std::chrono::seconds` argument, so that this default can be
      overridden.

### Python interpretation

Inside the **`PythonInterpreter`** class, the actual translation to Python3
calls occur. This will interactively call its API to translate parts of the C++
calls to Python. More information on this API can be found here:
https://docs.python.org/3/c-api/

The interpretation happens inside the virtual environment, set up in the
[CMakeLists.txt](./CMakeLists.txt) file.

Not much information about this will be noted, as this is a little out of scope.
Just know that if you do add calls here, you will be required to do manual
reference counting (or write your own wrapper).

### Example Tests

- **`ElementTest.C`** - Demonstrates:
  - `Element` API
  - retrieving elements with the `SeleniumAPI`

- **`LocatorTest.C`** - Demonstrates:
  - retrieving elements with the `SeleniumAPI`

- **`InteractionTest.C`** - Demonstrates:
  - `Element` API to interact with keyboard and mouse
  - using waiting for page interaction results

## Accessing information

Inside the `SeleniumTest` the `SELENIUM_TEST` macro is defined. This will wrap
a Selenium test case. Inside this macro, several classes or functions are
exposed, allowing for easy access to information and functionality. These are:

- `api`: this is an instance of `SeleniumAPI`, this will be used to find
  elements on the page.
- `wait`: this is an instance of `SeleniumWait`, allowing for `until` calls, to
  pause execution until a certain condition holds.
- `updateApplication`: this function will allow you to update the
  `WApplication` instance that the `SeleniumTest` is using.

## Prerequisites

### Required

- **Python 3.x** with package:
  - `selenium` (install with: `pip3 install selenium`)

Strictly speaking `selenium` is not a requirement, as it is installed ad-hoc.

## Usage

### Building and Running Tests

1. **Enable Selenium tests in CMake:**
   ```bash
   cd build
   cmake -DENABLE_SELENIUM_TESTS=ON ..
   make test.selenium
   ```

2. **Run the tests:**
   ```bash
   # From build directory
   ./test/selenium/test.selenium
   ```

   If desired, specific test can be run with the `-t` flag. Do note that the
   filter will be of the format `selenium_{suite}/{name}`.

### Creating Your Own Tests

Creating you own test is very easy. You only need to have an instance of
a `WApplication`, and then call the `SELENIUM_TEST` macro, and you are set.

```cpp
#include "SeleniumTest.h"

class MyTestApp : public Wt::WApplication {
public:
  MyTestApp(const Wt::WEnvironment& env)
    : WApplication(env)
  {
    auto btn = root()->addNew<Wt::WPushButton>("Click me!");
    btn->setId("my-specific-button")

    // Other stuff
  }
};

BOOST_AUTO_TEST_SUITE(selenium_{suite})

SELENIUM_TEST({name}, MyTestApp)
  auto element = api.getElement(SeleniumAPI::FindBy::ID, "my-specific-button")
  BOOST_REQUIRE(element.has_value());

  // Do more...
END_SELENIUM_TEST
```
