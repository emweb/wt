Codeview example
----------------

This is an example that illustrates the real-time interactivity between
multiple users, using server-push.

It shows an online editor, where multiple observers may observe another
person editing code.

How to run
----------

See the README in the parent directory.

When a user accesses the application, a new 'session' is generated
which is identified by a unique internal path, and the user will be
the 'Coder'. Other users which access this URL will become 'Observers'
for that session.

You may want to deploy the application at a particular path,
e.g. `/code`, because the application uses internal paths:
`--deploy-path=/code`, and you may also want to enable web sockets
using a custom wt_config.xml file.

What it illustrates
-------------------

- the use of server-push, using `WServer::post()` to post events to a session
- embedding server (`CodeSession`) and client (`CodeWidget`) functionality
  in a single process
- the use of internal paths