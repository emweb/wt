Socket-notifier feature example
-------------------------------

This is an example that illustrates the use of the `WSocketNotifier` API
to monitor one or more sockets within a Wt application.

If you need only to monitor networking sockets, then you can also use
asio, using the asio service available from
`WServer::ioService()`.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

- the use of the `WSocketNotifier` API
- the use of server-push to provide updates on socket activity to the user
