Wt/Qt interopability library
============================

This library contains interopability classes for using Wt in
conjunction with Qt4 (http://trolltech.com/products/qt).

IMPORTANT! The Wt/Qt interopability library is licensed under the MIT
License. This license is different from the license that covers the Wt
libraries and other examples, and AND ONLY APPLIES TO FILES IN THIS
DIRECTORY. See below for the complete license terms.

The easiest way to use this library, is by linking the two source
files into your project.

Currently, this library provides:

1) Support for using QtCore library, and Qt's signal/slot mechanism
within a Wt application.

Atlhough from the surface, one should be able to simply use QtCore
within a Wt application, it turns out this is not possible. The reason
is that Qt makes a number of assumptions that are invalid within Wt's
multi-threaded event loop dispatching.

This is needed because Qt's object model, which defines a hierarchy of
QObjects, requires that every QObject in the hierarchy is created from
within the same thread. In addition, Qt's signal/slot system is
thread-aware and behaves very differently when a signal is emitted
from within a different thread than the thread in which the receiver
object lives.

Wt on the other hand does not guarantee that every event is dispatched
within the same thread. This is a side effect of the fact that Wt uses
thread pools in combination with asynchronous I/O to be able to serve
multiple connections simultaneously without requiring a high number of
threads.

Therefore, you cannot manipulate a QObject hierarchy, or propagate
events using Qt's signal/slot system, in a multi-threaded Wt
application server, since this is likely to violate Qt's thread/object
assumptions, without taking precautions (which are implemented by this
library).

To be able to QtCore objects within a Wt application, you should
reimplement WQApplication instead of WApplication. This application
class spawns a QThread that is dedicated to a single application
instance, and used for event handling, after your application is
constructed. You should not create any Qt objects from the
constructor, but rather from the WQApplication::create() method, which
runs within the context of this thread. Likewise, you should not
destroy Qt objects from the application destructor, but from the
WQApplication::destroy() method, which also runs in this thread.

You may enable a Qt event loop in this QThread, by setting the option
in the constructor. In this way, you can use QTcpSocket and other Qt
classes which rely on the presence of an event loop. Note that Qt
requires that you instantiate a QApplication object before you can use
a Qt event loop (only one is needed per process, so it may be shared
between multiple Wt sessions). You need to do this yourself, and a
convenient location could be within your main() function.

2) Conversion between QString and WString

The same file (WQApplication) provides toWString() and toQString()
functions for converting between WString and QString. Since both
classes fully support unicode, this conversion is lossless.

LICENSE
=======

Copyright (C) 2008 Emweb bv, Herent, Belgium.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
