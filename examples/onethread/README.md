Wt legacy integration example
-----------------------------

As Wt is often used to upgrade existing desktop applications to the web,
it is common that these applications cannot handle the constant thread
switching done by Wt.

Wt uses a thread pool which handles incoming requests, and subsequent
requests to the same session can potentially be handled by different
threads. Although in most cases one can avoid this problem by simply
using Wt with a thread pool having a single thread, such a
configuration cannot handle recursive event loops (which block the
thread while waiting for a new event, but this new event then arrives
at a thread pool without threads ready for processing).

This example provides a specialized WApplication class
(SingleThreadedApplication) which will allocate a dedicated thread to the
session and which performs all event handling from within this thread.

This can be used in combination with a process-per-session deployment
model (as facilitated by the FastCGI connector) to isolate each
session in its own process and within a single thread.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

How a legacy application, which requires a single thread for the
entire duration of a session and still requires recursive event loops,
can be ported to Wt.