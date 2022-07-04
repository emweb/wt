# Minimum dependency versions

We base the minimum versions of dependencies on a roughly 5-year sliding
window and a selection of commonly used operating systems.

We will look at:

- Debian oldstable
- The previous LTS version of Ubuntu
- The previous release of Red Hat Enterprise Linux (and its derivatives)
- The previous release of FreeBSD
- What was the latest version of Visual Studio 5 years ago

Note that this doesn't mean that you can't compile Wt on older versions
of these operating systems. We will just not go out of our way to
maintain compatibility with them.

You may be able to compile Wt on older operating systems if you
use a different toolset. For example, Red Hat supplies toolsets that
allow you to use a more recent compiler on older RHEL releases.

## Example: September 2022

At the time of writing, in September 2022, this means that we'll look at:

- Debian 10
- Ubuntu 20.04
- Red Hat Enterprise Linux 8
- FreeBSD 12
- Visual Studio 2017

## Wt's current core dependencies

- CMake 3.13 or higher
- Boost 1.66 or higher
- GCC 8 or higher
- Clang 7 or higher
- MSVC toolset 14.1 or higher
