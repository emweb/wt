Wt Jenkinsfiles and Dockerfiles
-------------------------------

This directory contains Jenkinsfiles and Dockerfiles used for testing Wt
at Emweb. These are included in the main Wt repository since they may be
of interest to external contributors or users of Wt as well.

- `minimal`: a Wt build with a minimum of features enabled on Ubuntu, testing
  once with threading support enabled, and once with threading support
  disabled. This is run often to provide immediate feedback on new commits.
- `freebsd`: same as `minimal`, but on FreeBSD (this machine is provisioned manually)
- `minver`: same as `minimal`, but on an older version of Ubuntu using the minimum GCC
   and Boost versions we're claiming to support at the moment
- `full`: a Wt build with almost all features enabled
- `db`: uses the Dockerfile of `full`. Builds Wt with all database backends
   except for Firebird, and runs the database tests using sidecar containers.
