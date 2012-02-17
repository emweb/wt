Simplechat example
------------------

The simplechat example is a simple multi-user client + server. It is
simple in many ways, including the constraint that all sessions run in
the same process (it does not use inter-process communication), and
that the client does not have any advanced functions.

The example can be run as a plain Wt application, but also as a widget
set widget, which is a popup chat box that sits in the bottom right of
the browser window, and can be integrated easily in another
website. See the [blog post](http://www.webtoolkit.eu/wt/blog/2010/12/17/widgetset_mode_and_cross_origin_requests) for more information.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

- how to use server push, and post events to sessions using
  WServer::post() how to reuse a single widget to have two different
- UI experiences (as a normal widget, or as a compact real-estate conserving
  widget for the chat popup)
- how Wt supports Cross-Origin requests to implement widget set mode
- the use of layout managers (`WVBoxLayout` and `WHBoxLayout`)
- how to play small UI sounds using `WSound`
- how to connect client-side JavaScript to a signal, to react to certain events
- how to implement a method in client-side JavaScript using
  `implementJavaScript()`
- how to safely render user-entered HTML without risking XSS 