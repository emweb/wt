Blog example
------------

This is an example that implements a simple yet feature-complete blog.
This is the blog that is used on [the Wt
homepage](http://www.webtoolkit.eu/wt/blog).

Since this example is actually used in production code, the example is
a bit more elaborate compared to other more simple examples.

How to run
----------

See the README in the parent directory.

The example itself will be deployed at '/blog', and the RSS feed at
'/blog/feed'. A SQLite3 database will be created in the working
directory if it didn't yet exist.

What it illustrates
-------------------

- the use of `Wt::Dbo` to create a simple database-based model layer
- how you can use these database objects inside View widgets
- the use of `WTemplate` for recurisvely defining HTML-based views
- the use `Wt::Auth` for authentication.
- the use of a static `WResource` for an RSS feed