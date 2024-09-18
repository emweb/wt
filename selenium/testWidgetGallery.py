#!/usr/bin/env python3

import asyncio
import sys
import subprocess
import trio

from selenium import webdriver
from selenium.common.exceptions import NoSuchElementException, TimeoutException, UnexpectedAlertPresentException, WebDriverException

from selenium.webdriver.common.by import By

from selenium.webdriver.support.wait import WebDriverWait

from selenium.webdriver.firefox.firefox_binary import FirefoxBinary
from selenium.webdriver.firefox.firefox_profile import FirefoxProfile

# Generally speaking Selenium discourages the use of XPATH: https://www.selenium.dev/documentation/test_practices/encouraged/locators/
# In this example though, the whole page is essentially replaced on navigation, making this a better choice.
# One would normally approach this with Page object models: https://www.selenium.dev/documentation/test_practices/encouraged/page_object_models/
TITLE_XPATH = '//div[@class="wg-contents"]/div/div[not(contains(@style, "display: none"))]/div/div[@class="contents"]/h2'
CHROME_LOG = 'chromedriver.log'
FIREFOX_LOG = 'geckodriver.log'


def perform_chrome_test(url, binary=''):
  """
  Start a Selenium session for Chrome at the specified URL.
  The session is headless, so it can work without a screen.
  All logging will be done on stdout.

  :Args:
   - url - The URL accessed by Selenium
   - binary - (optional) The Chrome binary used to set up the Selenium WebDriver

  :Returns:
    The result will be 0 if successful or 1 on failure.
  """
  options = webdriver.ChromeOptions()
  options.page_load_strategy = 'normal'
  # This is a list of various Chrome "fixes", to avoid, rendering and rendering timout issues:
  # See: https://stackoverflow.com/a/52340526
  options.add_argument('-headless=new') # Run headless
  options.add_argument('--disable-gpu') # Disable GPU to avoid display lookup, render issues, and GL failures
  options.add_argument('--no-sandbox') # Disable sandbox
  options.add_argument('--disable-setuid-sandbox') # Disable SUID for sandbox as well
  options.add_argument('--disable-dev-shm-usage') # Disable /dev/shm and switch to /tmp
  options.add_argument('start-maximized'); # Try to avoid renderer timeout
  options.add_argument('enable-automation'); # Try to avoid renderer timeout

  if binary:
    service = webdriver.ChromeService(executable_path=binary, log_output=subprocess.STDOUT, service_args=['--verbose', '--log-path=' + CHROME_LOG])
  else:
    service = webdriver.ChromeService(log_output=subprocess.STDOUT, service_args=['--verbose', '--log-path=' + CHROME_LOG])

  driver = webdriver.Chrome(options=options, service=service)

  return execute_test(driver, url)


def perform_firefox_test(url, binary=''):
  """
  Start a Selenium session for Firefox at the specified URL.
  The session is headless, so it can work without a screen.
  All logging will be done on stdout.

  :Args:
   - url - The URL accessed by Selenium.
   - binary - (optional) The Gecko Driver binary used to set up the Selenium WebDriver.

  :Returns:
    The result will be 0 if successful or 1 on failure.
  """
  options = webdriver.FirefoxOptions()
  options.page_load_strategy = 'normal'
  options.log.level = 'trace' # Set trace level logging to find issues
  options.add_argument('-headless')
  options.add_argument('disable-gpu') # Disable GPU to avoid display lookup, render issues, and GL failures

  firefox_profile = FirefoxProfile()
  firefox_profile.set_preference('devtools.console.stdout.content', True)
  options.profile = firefox_profile

  if binary:
    service = webdriver.FirefoxService(executable_path=binary, log_output=FIREFOX_LOG)
  else:
    service = webdriver.FirefoxService(log_output=FIREFOX_LOG)

  driver = webdriver.Firefox(options=options, service=service)

  return execute_test(driver, url)


def execute_test(driver, url):
  """
  Performs the actual test.

  :Args:
   - driver - The WebDriver (either Chrome or Firefox) in its current state.
   - url - The URL accessed by Selenium.

  :Returns:
    The result will be 0 if successful or 1 on failure.
  """
  returnValue = 0

  try:
    driver.get(url)
    returnValue = trio.run(go_over_menu, driver)

  finally:
    driver.quit()

  return returnValue


# CHECK BOTH TITLE AND ID
def check_moved_page(driver, previousValue):
  """
  Queries the page for its title and the ID of the title, so it can detect page changes.
  Both the title and its ID are used to ensure that for pages where the title is identical, the navigation change is captured.

  :Args:
   - driver - The WebDriver (either Chrome or Firefox) in its current state.
   - previousValue - A dictionary that contains a value for both "textContent" and "id".
                     Optionally entries can exist for "error" or "timeout" to indicate caught exceptions.
  """
  pageTitle = driver.find_element(By.XPATH, TITLE_XPATH)

  errors = [NoSuchElementException]
  wait = WebDriverWait(driver, timeout=2, poll_frequency=.2, ignored_exceptions=errors)

  def _refresh_page(d):
    pageTitle = driver.find_element(By.XPATH, TITLE_XPATH)
    return pageTitle.get_attribute('textContent') != previousValue['textContent'] or pageTitle.get_attribute('id') != previousValue['id']

  try:
    wait.until(_refresh_page)
  # After an exception occured on the previous page and we navigate.
  except UnexpectedAlertPresentException as upe:
    return { 'error' : upe.msg }
  # An exception occured on an interactive element on the current page.
  except WebDriverException as wde:
    return { 'error' : wde.msg }
  # Timeout occured, meaning no change was detected. This can be due to a slow request or an unresponsive application.
  except TimeoutException as te:
    return { 'timeout' : te.msg }

  pageTitle = driver.find_element(By.XPATH, TITLE_XPATH)
  print('Moved from page with title: "{0}" to page with title "{1}"'.format(previousValue['textContent'], pageTitle.get_attribute('textContent')))
  titleValue = {}
  titleValue['textContent'] = pageTitle.get_attribute('textContent');
  titleValue['id'] = pageTitle.get_attribute('id');
  return titleValue


async def go_over_menu(driver):
  """
  Retrieves the whole main menu structure and goes over its elements.

  This will simulate a user going through the menu and opening each page in order.

  :Args:
   - driver - The WebDriver (either Chrome or Firefox) in its current state.

  :Returns:
    The result will be 0 if successful or 1 on failure.
  """
  currentLogLine = 0
  if isinstance(driver.service, webdriver.FirefoxService):
    async with await trio.open_file(FIREFOX_LOG, mode='r') as log:
      currentLogLine = len(await log.readlines())
      print("SET LOG LINE: {0}".format(currentLogLine))

  titleValue = {
                 'textContent': '',
                 'id': ''
               }
  check_moved_page(driver, titleValue)

  main_menu = driver.find_element(By.CLASS_NAME, 'nav')
  main_menu_items = main_menu.find_elements(By.XPATH, 'child::li')
  for main_item in main_menu_items:
    sub_menu = main_item.find_element(By.CLASS_NAME, 'submenu')
    sub_menu_items = sub_menu.find_elements(By.XPATH, 'child::li')
    main_item.click();
    for sub_item in sub_menu_items:
      link = sub_item.find_element(By.TAG_NAME, 'a')
      print("=====\nGO TO: {0}".format(link.get_attribute("href")))
      sub_item.click();

      titleValue = check_moved_page(driver, titleValue)

      # Firefox log fix
      capturedProblem = []
      if isinstance(driver.service, webdriver.FirefoxService):
        async with await trio.open_file(FIREFOX_LOG, mode='r') as log:
          newLogLines = await log.readlines();

          if len(newLogLines) > currentLogLine:
            for i in range(currentLogLine, len(newLogLines)):
              capturedProblem.append(newLogLines[i])
            currentLogLine = len(newLogLines)

      # Retrieve the browser logs. This functions for Chrome, but not yet for Firefox
      # See: https://github.com/mozilla/geckodriver/issues/284
      try:
        browserLogs = driver.get_log('browser')

        if 'timeout' in titleValue:
          for entry in browserLogs:
            if entry['level'] == 'SEVERE' and entry['source'] == 'javascript' and 'Uncaught' in entry['message']:
              print(entry)

      # Firefox
      except WebDriverException:
        if len(capturedProblem) > 0:
          for line in capturedProblem:
            print("PROBLEM: {0}".format(line))

      if 'error' in titleValue or 'timeout' in titleValue:
        return 1

      print("=====\n")


browsers = {
  'chrome': perform_chrome_test,
  'firefox': perform_firefox_test
}


usage = """
FORMAT: {0} browser URL [browser binary]
Run this script with two arguments, either 'chrome' or 'firefox'.
Then as the second argument, the URL of the webpage.
This URL needs to be fully qualified, including the protocol.
Optionally, an additional argument can be passed, with the location of the Gecko Driver binary (for FireFox snap).
""".format(__file__)


def main():
  if len(sys.argv) > 2:
    browser = browsers.get(sys.argv[1])
    url = sys.argv[2]
    if browser is not None and url is not None:
      if (len(sys.argv) == 4):
        return browser(url, sys.argv[3])
      else:
        return browser(url)
    return 0
  else:
    print(usage)
    return 0


if __name__ == '__main__':
  quit(main())

