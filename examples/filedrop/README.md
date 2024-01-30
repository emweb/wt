File drop example
------------

This is an example that implements a file drop widget.

It also illustrates how to customize the transfer mechanism of the
widget. Define preprocessor macro `USE_CUSTOM_TRANSFER` to activate
this code.

How to run
----------

See the README in the parent directory.

Uploaded files will be put inside the `uploaded` directory.

What it illustrates
-------------------

- the use of `WFileDropWidget` to transfer files
- the customization of the client-side upload logic
- the customization of the server-side upload handling through `WResource`
