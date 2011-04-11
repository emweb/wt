This builds Wt and the examples as self-contained and stand-alone
applications for android.

WARNING: THIS IS STILL SOMEWHAT EXPERIMENTAL...

Download and unpack the latest Android NDK r5b (from
http://developer.android.com/sdk/ndk/index.html) and SDK r10 (from
http://developer.android.com/sdk/index.html). Install one or more
targets in the SDK.

Get the source distribution of wt-3.1.9 and unpack it somewhere.

Export an NDK and ANDROID_HOME variable.

$ export ANDROID_HOME=~/soft/android       # Where you installed things android
$ export NDK=$ANDROID_HOME/android-ndk-r5b

In order to use the Android NDK, you will need to prepare a
cross-tailchain for a specific Android target, more information on
this procedure can be found in docs/STANDALONE-TOOLCHAIN.html in the Android 
NDK directory.

To build a cross-toolchain for android API8 to $ANDROID_HOME/ndk-api8-arm,
next commands can be used.

$ export SYSROOT=$NDK/platforms/android-8/arch-arm
$ $NDK/build/tools/make-standalone-toolchain.sh
       --platform=android-8
       --install-dir=$ANDROID_HOME/ndk-api8-arm

Boost -> wimpie, patch + config files in platform/android/boost???

Go to the Wt directory and create a build directory (eg.:
/home/pieter/projects/wt/build-android-r5b-api8) Create an android
toolchain file by copying the toolchain example file
($WT_DIR/target/android/toolchain-android-arm.cmake) to your build
directory and changing the next variables:

ANDROID_STAGING_DIR : the directory where boost is installed to

ANDROID_SDK_DIR : e.g. $ANDROID_HOME/android-sdk-linux_x86

ANDROID_SDK_TARGET_ID : the SDK's target to be used, identified by its
id (note, when the toolchain's target and SDK's target differ this
will result in errors!)

ANDROID_NDK_TOOLS_DIR : e.g. $ANDROID_HOME/ndk-api8-arm

Run cmake:

cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain-android-arm.cmake -DANDROID=1
      -DBOOST_PREFIX=/ -DBOOST_VERSION=1_45 -DBOOST_COMPILER=gcc
      -DCMAKE_BUILD_TYPE=Debug ../

When running make -C examples, an .apk file (android package) is
generated for each example. This file can be installed directly on an
android device (executables can only be installed on a rooted device
or emulator). The package contains java code to start a C++ Wt server
(the C++ code is linked into an .so file and loaded using JNI) and a
WebView user interface component connecting directly to the Wt server.
