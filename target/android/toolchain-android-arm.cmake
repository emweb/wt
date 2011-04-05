# Adapt the pathnames in this file to suit your needs

# The path to the directory that contains your cross-compiled dependency
# libraries (boost, and optionally others such as graphicsmagick, ssl, ...)
SET(ANDROID_STAGING_DIR /home/pieter/soft/android/ndk-r5b-api8-staging)

# The path to your Android SDK installation
SET(ANDROID_SDK_DIR /home/pieter/soft/android/android-sdk-linux_x86)

# The ID of the target must be compatible with the NDK used to build Wt.
# Target ID's are specific to your installation; to discover your target ID,
# use 'android list target'. The default value of 99 is chosen to ensure
# that you will verify this.
SET(ANDROID_SDK_TARGET_ID 99) 

# The path to your standalone toolchain directory.
# See docs/STANDALONE-TOOLCHAIN.html in your ndk's installation dir
SET(ANDROID_NDK_TOOLS_DIR /home/pieter/soft/android/ndk-r5b-api8)

# Below is the normal contents of a cmake toolchain file, you'll
# probably not need to modify it.

SET(TARGET_CC ${ANDROID_NDK_TOOLS_DIR}/bin/arm-linux-androideabi-gcc)
SET(TARGET_CXX ${ANDROID_NDK_TOOLS_DIR}/bin/arm-linux-androideabi-g++)
SET(ANDROID_STRIP ${ANDROID_NDK_TOOLS_DIR}/bin/arm-linux-androideabi-strip)

SET(CMAKE_CXX_FLAGS "-fpic -ffunction-sections -funwind-tables -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__  -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -DANDROID  -Wa,--noexecstack")

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)

#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER ${TARGET_CC})

SET(CMAKE_CXX_COMPILER ${TARGET_CXX})

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${ANDROID_NDK_TOOLS_DIR} ${ANDROID_STAGING_DIR})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

