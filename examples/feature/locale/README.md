Locale feature example
----------------------

This is an example that illustrates how one may use the local date/time API.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

Timezones are not straitforward to deal with; the browser does not provide
information on the user's time zone, but only on his current UTC offset.

The UTC offset is sufficient to show the current time in the user's time zone,
but, because of daylight savings rules cannot be used to translate other UTC
dates in the user's time zone. If these dates are critical for your application
you will need to let the user configure his time zone.

This example shows one way of doing it by letting the user select a time zone
from a combo-box. Obviously, this is information that you would save in the
user's preferences.

- the use of WEnvironment::timeOffset() to get the user's current UTC offset
- the use of boost's time zone database to configure a time zone
- how one may make an educated guess of the time zone based on the user's
  current UTC offset
- the use of WLocalDateTime to display a time point in the user's locale
