This script builds Wt as an OSX Framework, suitable for iPhone/iPad
development using XCode.

The framework contains Wt, the built-in httpd, Wt::Dbo and the sqlite3
backend. It does contain any user-interface components. You will
typically use it in conjunction with a UIWebView.

The only dependency for building Wt.framework is boost.framework,
which can be built using the script provided by Pete Goodliffe:

http://gitorious.org/boostoniphone

To build the framework, create a build directory, and from within that
directory, invoke the build-framework.sh script:

$ mkdir build-osx
$ cd build-osx
$ ../target/osx/build-framework.sh

Some settings may need to modified in the script:

- the location of the boost framework: BOOST_FRAMEWORK_PATH
- the SDK version
