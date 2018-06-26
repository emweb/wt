Locale feature example
----------------------

This is an example that illustrates how one may use the local date/time API.

How to run
----------

See the README in the parent directory.

On POSIX systems (Linux, macOS, BSD,...), the time zone information in /usr/share/zoneinfo is
used, and you don't need to do anything.

On Windows, this example requires the uncompressed IANA time zone data to be installed.

On Windows, you can specify the tzdata directory with `date::set_install`.
This example sets it to `./tzdata`. The default on Windows is in the users's
Downloads folder: `Downloads/tzdata`. You will also need to add the windowsZones.xml
file (see http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml).

To summarize, on Windows:

 - In the directory that you will run the example from, create a `tzdata` directory
 - Download the latest time zone database from IANA: https://www.iana.org/time-zones
   (Choose the Data Only Distribution), and extract its contents into the `tzdata`
   directory you just created.
 - Download `windowsZones.xml` from
   http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml and
   add it to the `tzdata` directory
 

What it illustrates
-------------------

Timezones are not straightforward to deal with; the browser does not provide
information on the user's time zone, but only on his current UTC offset.

The UTC offset is sufficient to show the current time in the user's time zone,
but, because of daylight savings rules cannot be used to translate other UTC
dates in the user's time zone. If these dates are critical for your application
you will need to let the user configure his time zone.

This example shows one way of doing it by letting the user select a time zone
from a combo-box. Obviously, this is information that you would save in the
user's preferences.

- the use of WEnvironment::timeOffset() to get the user's current UTC offset
- the use of the time zone database to configure a time zone
- how one may make an educated guess of the time zone based on the user's
  current UTC offset
- the use of WLocalDateTime to display a time point in the user's locale
