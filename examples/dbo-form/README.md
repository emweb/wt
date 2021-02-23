Dbo Form example
----------------

This example demonstrates how the Wt::Form module can be used to
automatically create a UI form for database objects.

How to run
----------

```
../../build/examples/dbo-form/dbo-form.wt --docroot . --approot approot --http-listen 0.0.0.0:8080 -c approot/wt_config.xml --resources-dir=../../resources
```

Note: this example requires TinyMCE to be located in the resources folder.
See `WTextEdit` for more information.

What it illustrates
-------------------

- the use of `Wt::Form` to create custom form delegates for data types used in the dbo class
- how a form view can be automatically constructed for a dbo class
