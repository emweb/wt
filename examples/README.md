Running the examples
--------------------

The examples that come with the library will by default be linked
against the built-in httpd. In any case, we recommend this way of
exploring the examples and trying them out, even if you'll end up
deploying using FastCGI or ISAPI.

You typically need the following commands to run an example (`foobar`):

    cd foobar # source directory for example foobar
    ../../build/examples/foobar/foobar.wt --docroot . --http-address 0.0.0.0 --http-port 8080 --resources-dir=../../resources

By running the examples from within their source directory, in this way the
examples will find the auxiliary files in the expected places.

Some examples may need additional command line arguments, which are detailed
in the README.md for each example.

Some examples need third-party JavaScript libraries (ExtJS or TinyMCE).

- Download [ExtJS](http://yogurtearl.com/ext-2.0.2.zip), and install it according to [these instructions](http://www.webtoolkit.eu/wt/doc/reference/html/group__ext.html)
- Download [TinyMCE](http://tinymce.moxiecode.com/) and install its tiny_mce folder into the resources/ folder.

You will notice 404 File not Found errors for `ext/` or
`resources/tiny_mce/` if you are missing these JavaScript libraries.
