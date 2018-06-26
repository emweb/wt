WidgetGallery example
---------------------

This is the [widget gallery](https://www.webtoolkit.eu/widgets), which serves more as a directory of widgets available in Wt.

How to run
----------

This application illustrates a recommended deployment setup for a more involved application, separating:
- docroot: these are publicly available files for the browser to download
  through the web server (e.g. css, resources, ...)
- approot: these are private application files opened by the application (e.g.
  resource files, config files, etc...) and not deployed in your document root

To run the application using the built-in httpd, you thus need to:

    cd widgetgallery/docroot
    ln -s ../../../resources . # include standard Wt resource files in docroot
    cd ..
    ../../build/examples/widgetgallery/widgetgallery.wt \
          --docroot docroot --approot approot \
          --http-address 0.0.0.0 --http-port 8080

What it illustrates
-------------------

- most widgets in Wt in some form or another, in combination with the bootstrap
  theme (you can actually trigger the use of another theme using
  ?theme=default or ?theme=polished parameters)
- lots of small examples
- internal path handling and cross referencing
