/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <csignal>
#include <fstream>
#include <regex>
#include <exception>
#include <vector>

#include "fcgiapp.h"
#include "Configuration.h"
#include "FCGIRecord.h"
#include "Server.h"
#include "SessionInfo.h"
#include "WebUtils.h"
#include "WebController.h"

#include "Wt/WIOService.h"
#include "Wt/WServer.h"
#include "Wt/WLogger.h"

#include <boost/algorithm/string.hpp>

using std::exit;
using std::strcpy;
using std::strlen;
using std::memset;

namespace Wt {

  LOGGER("wtfcgi");
 
/*
 * From the FCGI Specifaction
 */
// const int FCGI_BEGIN_REQUEST = 1; // unused
// const int FCGI_ABORT_REQUEST = 2; // unused
const int FCGI_END_REQUEST   = 3;
const int FCGI_PARAMS        = 4;

Server *Server::instance = 0;

bool Server::bindUDStoStdin(const std::string& socketPath, Wt::WServer& server)
{
  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s == -1) {
    LOG_ERROR_S(&server, "fatal: socket(): " << (const char *)strerror(errno));
    return false;
  }

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;

  strncpy (local.sun_path, socketPath.c_str(), sizeof (local.sun_path));
  local.sun_path[sizeof (local.sun_path) - 1] = '\0';
  unlink(local.sun_path);
  socklen_t len = offsetof (struct sockaddr_un, sun_path)
    + strlen(local.sun_path) + 1;

  if (bind(s, (struct sockaddr *)& local, len) == -1) {
    LOG_ERROR_S(&server, "fatal: bind(): " << (const char *)strerror(errno));
    close(s);
    return false;
  }

  if (listen(s, 5) == -1) {
    LOG_ERROR_S(&server, "fatal: listen(): " << (const char *)strerror(errno));
    close(s);
    return false;
  }

  if (dup2(s, STDIN_FILENO) == -1) {
    LOG_ERROR_S(&server, "fatal: dup2(): " << (const char *)strerror(errno));
    close(s);
    return false;
  }

  return true;
}

Server::Server(WServer& wt,
               const std::string &applicationName,
               const std::vector<std::string> &args)
  : wt_(wt),
    applicationName_(applicationName),
    args_(args),
    childrenDied_(0),
    handleSigChld_(0)
{
  instance = this;

  srand48(getpid());

  /*
   * Spawn the session processes for shared process policy
   */
  Configuration& conf = wt_.configuration();

  if (conf.sessionPolicy() == Configuration::SharedProcess)
    for (int i = 0; i < conf.numProcesses(); ++i)
      spawnSharedProcess();
}

void Server::execChild(bool debug, const std::string& extraArg)
{
  Configuration& conf = wt_.configuration();

  std::string prepend;
  if (debug && conf.debug())
    prepend = conf.valgrindPath();

  std::vector<std::string> prependArgv;
  if (!prepend.empty())
    boost::split(prependArgv, prepend, boost::is_any_of(" "));

  /* up to 3 arguments + 0: argv_[0] client [extraArg] */
  const char **argv = new const char *[prependArgv.size() + 4];

  unsigned i = 0;
  for (; i < prependArgv.size(); ++i)
    argv[i] = prependArgv[i].c_str();

  argv[i++] = applicationName_.c_str();
  argv[i++] = "client";
  if (!extraArg.empty())
    argv[i++] = extraArg.c_str();
  argv[i++] = 0;

  for (unsigned i = 0; ; ++i) {
    if (argv[i] == 0)
      break;
    LOG_DEBUG("argv[" << i << "]: " << argv[i]);
  }

#ifdef WT_THREADED
  // Unblocking all signals before exec
  sigset_t mask;
  sigfillset(&mask);
  pthread_sigmask(SIG_UNBLOCK, &mask, 0);
#endif // WT_THREADED

  execv(argv[0], const_cast<char *const *>(argv));

  delete[] argv;
}

void Server::spawnSharedProcess()
{
  pid_t pid = fork();
  if (pid == -1) {
    LOG_ERROR_S(&wt_, "fatal error: fork(): " << (const char *)strerror(errno));
    exit(1);
  } else if (pid == 0) {
    /* the child process */
    execChild(true, std::string());
    exit(1);
  } else {
    LOG_INFO_S(&wt_, "spawned session process: pid = " << pid);
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> sessionsLock(mutex_);
#endif
    sessionProcessPids_.push_back(pid);
  }
}

const std::string Server::socketPath(const std::string& sessionId)
{
  Configuration& conf = wt_.configuration();

  std::string sessionPath = conf.runDirectory() + "/" + sessionId;

  if (conf.sessionPolicy() == Configuration::SharedProcess) {
    std::ifstream f(sessionPath.c_str());

    if (f) {
      std::string pid;
      f >> pid;

      if (!pid.empty())
	return conf.runDirectory() + "/server-" + pid;
      else
	return std::string();
    } else
      return std::string();

  } else
    return sessionPath;
}

void handleSigChld(int)
{
  Server::instance->handleSigChld();
}

void handleServerSigTerm(int)
{
  Server::instance->handleSignal("SIGTERM");
}

void handleServerSigUsr1(int)
{
  Server::instance->handleSignal("SIGUSR1");
}

void handleServerSigHup(int)
{
  Server::instance->handleSignal("SIGHUP");
}

void Server::handleSignal(const char *signal)
{
  LOG_INFO_S(&wt_, "shutdown (caught " << signal << ")");

  /* We need to kill all children */
  for (unsigned i = 0; i < sessionProcessPids_.size(); ++i)
    kill(sessionProcessPids_[i], SIGTERM); 

  exit(0);
}

void Server::handleSigChld()
{
  handleSigChld_ = 1;
}

void Server::doHandleSigChld()
{
  pid_t cpid;
  int stat;

  while ((cpid = waitpid(0, &stat, WNOHANG)) > 0) {
    LOG_INFO_S(&wt_, "caught SIGCHLD: pid=" << cpid << ", stat=" << stat);

    Configuration& conf = wt_.configuration();

    if (conf.sessionPolicy() == Configuration::DedicatedProcess) {
#ifdef WT_THREADED
      std::unique_lock<std::recursive_mutex> sessionsLock(mutex_);
#endif
      for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end();
	   ++i) {
	if (i->second->childPId() == cpid) {
	  LOG_INFO_S(&wt_, "deleting session: " << i->second->sessionId());

	  unlink(socketPath(i->second->sessionId()).c_str());
	  delete i->second;
	  sessions_.erase(i);

	  break;
	}
      }
    } else {
#ifdef WT_THREADED
      std::unique_lock<std::recursive_mutex> sessionsLock(mutex_);
#endif
      for (unsigned i = 0; i < sessionProcessPids_.size(); ++i) {
	if (sessionProcessPids_[i] == cpid) {
	  sessionProcessPids_.erase(sessionProcessPids_.begin() + i);

	  /*
	   * TODO: cleanup all sessions that pointed to this pid
	   */

	  ++childrenDied_;

	  if (childrenDied_ < 5) {
	    spawnSharedProcess();
	  } else
	    LOG_ERROR_S(&wt_, "sessions process restart limit (5) reached");

	  break;
	}
      }
    }
  }
}

bool Server::getSessionFromQueryString(const std::string& queryString,
				       std::string& sessionId)
{
  Configuration& conf = wt_.configuration();

  static const std::regex
    session_e(".*wtd=([a-zA-Z0-9]{" + std::to_string(conf.fullSessionIdLength())
	      + "}).*");

  std::smatch what;
  if (std::regex_match(queryString, what, session_e)) {
    sessionId = what[1];
    return true;
  }

  return false;
}

int Server::connectToSession(const std::string& sessionId,
			     const std::string& socketPath, int maxTries)
{
  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s == -1) {
    LOG_ERROR_S(&wt_, "fatal: socket(): " << (const char *)strerror(errno));
    exit(1);
  }

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socketPath.c_str());
  socklen_t len = strlen(local.sun_path) + sizeof(local.sun_family) + 1;

  int tries = 0;
  for (tries = 0; tries < maxTries; ++tries) {
    int result = connect(s, (struct sockaddr *)&local, len);
    if (result == -1) {
      usleep(100000); // 0.1 second
    } else
      break;
  }

  if (tries == maxTries) {
    LOG_ERROR_S(&wt_, "connect(): " << (const char *)strerror(errno));
    LOG_INFO_S(&wt_, "giving up on session: " << sessionId
	     << " (" << socketPath << ")");
    close(s);
    unlink(socketPath.c_str());

    return -1;
  }

  return s;
}

void Server::checkConfig()
{
  /*
   * Create the run directory if it does not yet exist.
   */
  Configuration& conf = wt_.configuration();

  FILE *test = fopen((conf.runDirectory() + "/test").c_str(), "w+");

  if (test == NULL) {
    if (mkdir(conf.runDirectory().c_str(), 777) != 0) {
      LOG_ERROR_S(&wt_, "fatal: cannot create run directory '"
		<< conf.runDirectory() << "'");
      exit(1);
    }
  } else {
    unlink((conf.runDirectory() + "/test").c_str());
    fclose(test);
  }
}

int Server::run()
{
  checkConfig();

  /*
   * We partially parse the FCGI protocol. We need to delineate the
   * FCGI_BEGIN_REQUEST in the server stream,
   * and the end-of FCGI_PARAMS, for determining presence of the session ID.
   * and FCGI_END_REQUEST messages from the application stream.
   */
  struct sockaddr_un clientname;
  socklen_t socklen = sizeof(clientname);

  if (signal(SIGCHLD, Wt::handleSigChld) == SIG_ERR) 
    LOG_ERROR_S(&wt_, "cannot catch SIGCHLD: signal(): "
		<< (const char *)strerror(errno));
  if (signal(SIGTERM, Wt::handleServerSigTerm) == SIG_ERR)
    LOG_ERROR_S(&wt_, "cannot catch SIGTERM: signal(): "
		<< (const char *)strerror(errno));
  if (signal(SIGUSR1, Wt::handleServerSigUsr1) == SIG_ERR) 
    LOG_ERROR_S(&wt_, "cannot catch SIGUSR1: signal(): "
		<< (const char *)strerror(errno));
  if (signal(SIGHUP, Wt::handleServerSigHup) == SIG_ERR) 
    LOG_ERROR_S(&wt_, "cannot catch SIGHUP: signal(): "
		<< (const char *)strerror(errno));

  if (args_.size() == 1 && boost::starts_with(args_[0], "--socket=")) {
    std::string socketName = args_[0].substr(9);
    boost::trim(socketName);
    if (!bindUDStoStdin(socketName, wt_))
      return -1;
    LOG_INFO_S(&wt_,
	       "reading FastCGI stream from socket '" << socketName << '\'');
  } else
    LOG_INFO_S(&wt_, "reading FastCGI stream from stdin");

  wt_.ioService().start();

  for (;;) {
    int serverSocket = accept(STDIN_FILENO, (sockaddr *) &clientname,
			      &socklen);

    if (serverSocket < 0) {
      LOG_ERROR_S(&wt_, "fatal: accept(): " << (const char *)strerror(errno));
      exit (1);
    }

    checkAndQueueSigChld();

    handleRequestThreaded(serverSocket);
  }

  return 0;
}

void Server::checkAndQueueSigChld()
{
  if (handleSigChld_ == 1) {
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGCHLD);
    sigset_t old_mask;
#ifdef WT_THREADED
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#else
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
#endif // WT_THREADED
    if (handleSigChld_ == 1) {
      handleSigChld_ = 0;
#ifdef WT_THREADED
      wt_.ioService().post(std::bind(&Server::doHandleSigChld, this));
#else
      doHandleSigChld();
#endif // WT_THREADED
    }
#ifdef WT_THREADED
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);
#else
    sigprocmask(SIG_SETMASK, &old_mask, 0);
#endif // WT_THREADED
  }
}

void Server::handleRequestThreaded(int serverSocket)
{
#ifdef WT_THREADED
  wt_.ioService().post(std::bind(&Server::handleRequest, this, serverSocket));
#else
  handleRequest(serverSocket);
#endif // WT_THREADED
}

bool Server::writeToSocket(int socket, const unsigned char *buf, int bufsize)
{
  while (bufsize > 0) {
    int result = write(socket, buf, bufsize);
    if (result < 0)
      return false;
    else {
      bufsize -= result;
      buf += result;
    }
  }

  return true;
}

void Server::handleRequest(int serverSocket)
{
  int clientSocket = -1;
  try {
    /*
     * handle a new request
     */
    std::vector<FCGIRecord *> consumedRecords_;

    bool haveSessionId = false;
    std::string sessionId = "";

    std::string cookies;
    std::string scriptName;

    for (;;) {
      FCGIRecord *d = new FCGIRecord();
      d->read(serverSocket);

      LOG_DEBUG_S(&wt_, "server read");

      if (d->good()) {
	// LOG_DEBUG_S(&wt_, *d); BROKEN !
	consumedRecords_.push_back(d);

	if (d->type() == FCGI_PARAMS) {
	  if (d->contentLength() == 0)
	    break;
	  else {
	    std::string value;

	    if (d->getParam("QUERY_STRING", value))
	      haveSessionId = getSessionFromQueryString(value, sessionId);
	    if (d->getParam("HTTP_COOKIE", value))
	      cookies = value;
	    if (d->getParam("SCRIPT_NAME", value))
	      scriptName = value;
	  }
	}
      } else {
	delete d;
	break;
      }
    }

    /*
     * Session tracking:
     *   what should we give priority ? We should give priority to the
     *   cookie, because in that case the URL may still contain an invalid
     *   session id (when the user had for example bookmarked like that)
     *
     * But not when we want to get a new session when reloading, in that
     * case we ignore the set cookie.
     */
    Configuration& conf = wt_.configuration();

    if ((conf.sessionTracking() == Configuration::CookiesURL)
	&& !cookies.empty() && !scriptName.empty()
	&& !conf.reloadIsNewSession()) {
      std::string cookieSessionId
	= WebController::sessionFromCookie(cookies.c_str(), scriptName,
					   conf.fullSessionIdLength());
      if (!cookieSessionId.empty()) {
	sessionId = cookieSessionId;
	haveSessionId = true;
      }
    }

    /*
     * Forward the request to the session.
     */
    clientSocket = -1;

    /*
     * See if the session is alive.
     */
    if (haveSessionId) {
      struct stat finfo;

      // exists, try to connect (for 1 second)
      std::string path = socketPath(sessionId);
      if (stat(path.c_str(), &finfo) != -1)
	clientSocket = connectToSession(sessionId, path, 10);
    }

    while (clientSocket == -1) {
      /*
       * New session
       */
      if (conf.sessionPolicy() == Configuration::DedicatedProcess) {
	/*
	 * For dedicated process, create session at server, so that we
	 * can keep track of process id for the session
	 */
	do {
	  sessionId = conf.generateSessionId();
	  if (!conf.registerSessionId(std::string(), sessionId))
	    sessionId.clear();
	} while (sessionId.empty());

	std::string path = socketPath(sessionId);

	/*
	 * Create and fork a new session.
	 *
	 * But not if we have already too many sessions running...
	 */
	if ((int)sessions_.size() > conf.maxNumSessions()) {
	  LOG_ERROR_S(&wt_, "session limit reached (" << 
		      conf.maxNumSessions() << ')');
	  break;
	}

	pid_t pid = fork();
	if (pid == -1) {
	  LOG_ERROR_S(&wt_, "fatal: fork(): " << (const char *)strerror(errno));
	  exit(1);
	} else if (pid == 0) {
	  /* the child process */
	  execChild(true, sessionId);
	  exit(1);
	} else {
	  LOG_INFO_S(&wt_, "spawned dedicated process for " << sessionId
		   << ": pid=" << pid);
	  {
#ifdef WT_THREADED
	    std::unique_lock<std::recursive_mutex> sessionsLock(mutex_);
#endif
	    sessions_[sessionId] = new SessionInfo(sessionId, pid);
	  }

	  clientSocket = connectToSession(sessionId, path, 1000);
	}
      } else {
	/*
	 * For SharedProcess, connect to a random server.
	 */

	/*
	 * Patch from Andrii Arsirii: it could be that the
	 * sessionProcessPids_ vector is empty because concurrently a
	 * shared process died: we need to wait until it is respawned.
	 */
	for (;;) {
	  int processCount = sessionProcessPids_.size();
	  if (processCount == 0)
	    sleep(1);
	  else {
	    unsigned i = lrand48() % processCount;

	    if (i >= sessionProcessPids_.size())
	      sleep(1);
	    else {
	      // This is still not entirely okay: a race condition could
	      // still occur between checking the size and getting the
	      // element.
	      int pid = sessionProcessPids_[i];
	      std::string path = conf.runDirectory() + "/server-"
		+ std::to_string(pid);

	      clientSocket = connectToSession("", path, 100);

	      if (clientSocket == -1)
		LOG_ERROR_S(&wt_, "session process " << pid <<
			    " not responding ?");

	      break;
	    }
	  }
	}
      }
    }

    if (clientSocket == -1) {
      close(serverSocket);
      return;
    }

    /*
     * Forward all data that was consumed to the application.
     */
    for (unsigned i = 0; i < consumedRecords_.size(); ++i) {
      if (!writeToSocket(clientSocket, consumedRecords_[i]->plainText(),
			 consumedRecords_[i]->plainTextLength())) {
	LOG_ERROR_S(&wt_, "error writing to client");
	return;
      }

      delete consumedRecords_[i];
    }

    /*
     * Now, we must copy data from both the server to the application,
     * as well as from the application to the server, until the application
     * sends the FCGI_END_REQUEST message.
     */
    for (;;) {
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(serverSocket, &rfds);
      FD_SET(clientSocket, &rfds);

      LOG_DEBUG_S(&wt_, "select()");
      if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0) {
	if (errno != EINTR)
	  LOG_ERROR_S(&wt_, "fatal: select(): "
		      << (const char *)strerror(errno));

	break;
      }

      if (FD_ISSET(serverSocket, &rfds)) {
	FCGIRecord d;
	d.read(serverSocket);

	if (d.good()) {
	  // LOG_DEBUG_S(&wt_, "record from server: " << d); BROKEN !
	  if (!writeToSocket(clientSocket, d.plainText(),
			     d.plainTextLength())) {
	    LOG_ERROR_S(&wt_, "error writing to application");

	    break;
	  }
	} else {
	  LOG_ERROR_S(&wt_, "error reading from web server");

	  break;
	}
      }

      if (FD_ISSET(clientSocket, &rfds)) {
	FCGIRecord d;
	d.read(clientSocket);

	if (d.good()) {
	  // LOG_DEBUG_S(&wt_, "record from client: " << d); BROKEN !
	  if (!writeToSocket(serverSocket, d.plainText(),
			     d.plainTextLength())) {
	    LOG_ERROR_S(&wt_, "error writing to web server");

	    break;
	  }

	  if (d.type() == FCGI_END_REQUEST)
	    break;
	} else {
	  LOG_ERROR_S(&wt_, "error reading from application");

	  break;
	}
      }
    }

    LOG_DEBUG_S(&wt_, "request done.");

    shutdown(serverSocket, SHUT_RDWR);
    close(serverSocket);
    close(clientSocket);
  } catch (std::exception&) {
    close(serverSocket);
    if (clientSocket != -1)
      close(clientSocket);
  }
}

}
