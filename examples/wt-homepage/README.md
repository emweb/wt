Wt homepage
-----------

This is the [Wt homepage](http://www.webtoolkit.eu/wt).

How to run
----------

Copy the blog.xml file and css directory from the blog example to this directory
See the README in the parent directory.

Note that the little chat widget is a separate application (simplechat), which
is visible on the homepage through Wt's 'widgetset' mechanism. In order for
this to work, you must run this example in progressive bootstrap mode. You
can configure progressive bootstrap mode in wt_config.xml.

What it illustrates
-------------------

- using `WMenu` and `WTabWidget`
- the internal path API to react to navigation events
- internationalization and localization API, using message resource bundles
- the `WServer` API to deploy multiple applications within the same server
