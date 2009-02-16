/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <csignal>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <signal.h>
#include <exception>
#include <vector>
#include <stdlib.h>

#include "fcgiapp.h"
#include "FCGIRecord.h"
#include "FCGIStream.h"
#include "Server.h"
#include "SessionInfo.h"
#include "WebController.h"

using std::exit;
using std::strcpy;
using std::strlen;
using std::memset;

namespace {

Wt::EntryPointList entryPoints;

}

namespace Wt {
 
/*
 * From the FCGI Specifaction
 */
const int FCGI_BEGIN_REQUEST = 1;
const int FCGI_ABORT_REQUEST = 2;
const int FCGI_END_REQUEST   = 3;
const int FCGI_PARAMS        = 4;

/*
 * New server implementation: 2 modes.
 *
 * 1) one-session-per-process (current implementation)
 *     session file = socket
 *
 * 2) spread sessions over X processes
 *     session file points to socket file
 *     session is allocated and session file is created within the session
 */

Server *Server::instance = 0;

Server::Server(int argc, char *argv[])
  : argc_(argc),
    argv_(argv),
    conf_(argv[0], "", Configuration::FcgiServer,
	  "Wt: initializing FastCGI session process manager")
{
  instance = this;

  srand48(getpid());

  /*
   * Spawn the session processes for shared process policy
   */
  if (conf_.sessionPolicy() == Configuration::SharedProcess) {
    for (int i = 0; i < conf_.numProcesses(); ++i) {
      spawnSharedProcess();
    }
  }
}

void Server::execChild(bool debug, const std::string& extraArg)
{
#ifdef _GNU_SOURCE
  /*
   * if you want to make sure that all delete actually releases
   * memory back to the OS:
   */
  //const char *const envp[]
  //  = { "GLIBCXX_FORCE_NEW=1",
  //      "GLIBCPP_FORCE_NEW=1",
  //	NULL };

  /*
   * It's more useful to pass on the environment:
   */
  char **envp = environ;
#else
  const char *const envp[] = { NULL };
#endif // _GNU_SOURCE

  std::string prepend;
  if (debug && conf_.allowDebug())
    prepend = conf_.valgrindPath();

  std::vector<std::string > prependArgv;
  if (!prepend.empty())
    boost::split(prependArgv, prepend, boost::is_any_of(" "));

  /* up to 3 arguments + 0: argv_[0] client [extraArg] */
  const char **argv = new const char *[prependArgv.size() + 4];

  unsigned i = 0;
  for (; i < prependArgv.size(); ++i)
    argv[i] = prependArgv[i].c_str();

  argv[i++] = argv_[0];
  argv[i++] = "client";
  if (!extraArg.empty())
    argv[i++] = extraArg.c_str();
  argv[i++] = 0;

  execve(argv[0], const_cast<char *const *>(argv),
	 const_cast<char *const *>(envp));

  delete[] argv;
}

void Server::spawnSharedProcess()
{
  pid_t pid = fork();
  if (pid == -1) {
    conf_.log("fatal") << "fork(): " << strerror(errno);
    exit(1);
  } else if (pid == 0) {
    /* the child process */
    execChild(true, std::string());
    exit(1);
  } else {
    conf_.log("notice") << "Spawned session process: pid = " << pid;
    sessionProcessPids_.push_back(pid);
  }
}

const std::string Server::socketPath(const std::string& sessionId)
{
  std::string sessionPath = conf_.runDirectory() + "/" + sessionId;

  if (conf_.sessionPolicy() == Configuration::SharedProcess) {
    std::ifstream f(sessionPath.c_str());

    if (f) {
      std::string pid;
      f >> pid;

      if (!pid.empty())
	return conf_.runDirectory() + "/server-" + pid;
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
  Server::instance->handleSigTerm();
}

void Server::handleSigTerm()
{
  conf_.log("notice") << "Shutdown (caught SIGTERM).";
  exit(0);
}

void Server::handleSigChld()
{
  pid_t cpid;
  int stat;

  while ((cpid = waitpid(0, &stat, WNOHANG)) > 0) {
    conf_.log("notice") << "Caught SIGCHLD: pid=" << cpid
			<< ", stat=" << stat;

    if (conf_.sessionPolicy() == Configuration::DedicatedProcess) {
      for (Server::SessionMap::iterator i = sessions_.begin();
	   i != sessions_.end(); ++i)
	if (i->second->childPId() == cpid) {
	  conf_.log("notice") << "Deleting session: " << i->second->sessionId();

	  unlink(socketPath(i->second->sessionId()).c_str());
	  delete i->second;
	  sessions_.erase(i);

	  break;
	}
    } else {
      for (unsigned i = 0; i < sessionProcessPids_.size(); ++i) {
	if (sessionProcessPids_[i] == cpid) {
	  sessionProcessPids_.erase(sessionProcessPids_.begin() + i);

	  /*
	   * TODO: cleanup all sessions that pointed to this pid
	   */

	  static int childrenDied = 0;

	  ++childrenDied;

	  if (childrenDied < 5)
	    spawnSharedProcess();
	  else
	    conf_.log("error") << "Sessions process restart limit (5) reached";

	  break;
	}
      }
    }
  }
}

bool Server::getSessionFromQueryString(const std::string& queryString,
				       std::string& sessionId)
{
  static const boost::regex
    session_e(".*wtd=([a-zA-Z0-9]{"
	      + boost::lexical_cast<std::string>(conf_.sessionIdLength())
	      + "}).*");

  boost::smatch what;
  if (boost::regex_match(queryString, what, session_e)) {
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
    conf_.log("fatal") << "socket(): " << strerror(errno);
    exit(1);
  }

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socketPath.c_str());
  socklen_t len = strlen(local.sun_path) + sizeof(local.sun_family);

  int tries = 0;
  for (tries = 0; tries < maxTries; ++tries) {
    int result = connect(s, (struct sockaddr *)&local, len);
    if (result == -1) {
      usleep(100000); // 0.1 second
    } else
      break;
  }

  if (tries == maxTries) {
    conf_.log("error") << "connect(): " << strerror(errno);
    conf_.log("notice") << "Giving up on session: " << sessionId
			<< " (" << socketPath << ")";
    close(s);
    unlink(socketPath.c_str());

    return -1;
  }

  return s;
}

void Server::checkConfig()
{
  /*
   * Check obvious configuration problem: the RUNDIR must exist and
   * we must have proper read/write permissions.
   */
  FILE *test = fopen((conf_.runDirectory() + "/test").c_str(), "w+");

  if (test == NULL) {
    conf_.log("fatal") << "Cannot create files in run-directory ("
		       << conf_.runDirectory()
		       << "), please create dir and set permissions..";
    exit(1);
  } else
    unlink((conf_.runDirectory() + "/test").c_str());
}

int Server::main()
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
    conf_.log("error") << "Cannot catch SIGCHLD: signal(): "
		       << strerror(errno);

  for (;;) {
    int serverSocket = accept(STDIN_FILENO, (sockaddr *) &clientname,
			      &socklen);

    if (serverSocket < 0) {
      conf_.log("fatal") << "accept(): " << strerror(errno);
      exit (1);
    }

    int clientSocket = -1;
    bool debug = false;

    try {
      /*
       * handle a new request
       */
      std::vector<FCGIRecord *> consumedRecords_;

      bool haveSessionId = false;
      std::string sessionId = "";

      std::string cookies;
      std::string scriptName;

      char version;
      short requestId;

      for (;;) {
	FCGIRecord *d = new FCGIRecord();
	d->read(serverSocket);
	version = d->version();
	requestId = d->requestId();

	if (d->good()) {
	  //std::cerr << *d << std::endl;
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
      if ((conf_.sessionTracking() == Configuration::CookiesURL)
	  && !cookies.empty() && !scriptName.empty()
	  && !conf_.reloadIsNewSession()) {
	std::string cookieSessionId
	  = WebController::sessionFromCookie(cookies, scriptName,
					     conf_.sessionIdLength());
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
	if (conf_.sessionPolicy() == Configuration::DedicatedProcess) {
	  /*
	   * For dedicated process, create session at server, so that we
	   * can keep track of process id for the session
	   */
	  sessionId = conf_.generateSessionId();
	  std::string path = socketPath(sessionId);

	  /*
	   * Create and fork a new session.
	   *
	   * But not if we have already too many sessions running...
	   */
	  if ((int)sessions_.size() > conf_.maxNumSessions()) {
	    conf_.log("error") << "Session limit reached ("
			       << conf_.maxNumSessions() << ')';
	    break;
	  }

	  pid_t pid = fork();
	  if (pid == -1) {
	    conf_.log("fatal") << "fork(): " << strerror(errno);
	    exit(1);
	  } else if (pid == 0) {
	    /* the child process */
	    execChild(debug, sessionId);
	    exit(1);
	  } else {
	    conf_.log("notice") << "Spawned dedicated process for "
				<< sessionId << ": pid=" << pid;
	    sessions_[sessionId] = new SessionInfo(sessionId, pid);

	    clientSocket = connectToSession(sessionId, path, 1000);
	  }
	} else {
	  /*
	   * For SharedProcess, connect to a random server.
	   */
	  int i = lrand48() % sessionProcessPids_.size();

	  std::string path = conf_.runDirectory() + "/server-"
	    + boost::lexical_cast<std::string>(sessionProcessPids_[i]);

	  clientSocket = connectToSession("", path, 100);

	  if (clientSocket == -1)
	    conf_.log("error") << "Session process " << sessionProcessPids_[i]
			       << " not responding ?";
	}
      }

      if (clientSocket == -1) {
	close(serverSocket);
	continue;
      }

      /*
       * Forward all data that was consumed to the application.
       */
      for (unsigned i = 0; i < consumedRecords_.size(); ++i) {
	write(clientSocket, consumedRecords_[i]->plainText(),
	      consumedRecords_[i]->plainTextLength());
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

	if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0) {
	  if (errno != EINTR)
	    conf_.log("fatal") << "select(): " << strerror(errno);

	  break;
	}

	bool got = false;
	if (FD_ISSET(serverSocket, &rfds)) {
	  got = true;
	  FCGIRecord d;
	  d.read(serverSocket);
    
	  if (d.good()) {
	    //std::cerr << "Got record from server: " << d << std::endl;
	    write(clientSocket, d.plainText(), d.plainTextLength());
	  } else {
	    conf_.log("error") << "Error reading from web server";

	    break;
	  }
	}

	if (FD_ISSET(clientSocket, &rfds)) {
	  got = true;
	  FCGIRecord d;
	  d.read(clientSocket);

	  if (d.good()) {
	    write(serverSocket, d.plainText(), d.plainTextLength());
	    if (d.type() == FCGI_END_REQUEST)
	      break;
	  } else {
	    conf_.log("error") << "Error reading from application";

	    break;
	  }
	}
      }

      close(serverSocket);
      close(clientSocket);
    } catch (std::exception&) {
      close(serverSocket);
      if (clientSocket != -1)
	close(clientSocket);
    }
  }

  return 0;
}

/*
 ******************
 * Client routines
 ******************
 */

static std::string socketPath;
static WebController *theController = 0;

static void doShutdown(const char *signal)
{
  unlink(socketPath.c_str());
  if (theController) {
    WebController::conf().log("notice") << "Caught " << signal;
    theController->forceShutdown();
  }

  exit(0);
}

static void handleSigTerm(int)
{
  doShutdown("SIGTERM");
}

static void handleSigUsr1(int)
{
  doShutdown("SIGUSR1");
}

void runSession(Configuration& conf, std::string sessionId)
{
  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s == -1) {
    conf.log("fatal") << "socket(): " << strerror(errno);
    exit(1);
  }

  socketPath = conf.runDirectory() + "/" + sessionId;

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socketPath.c_str());
  unlink(local.sun_path);
  socklen_t len = strlen(local.sun_path) + sizeof(local.sun_family);

  if (bind(s, (struct sockaddr *)& local, len) == -1) {
    conf.log("fatal") << "bind(): " << strerror(errno);
    exit(1);
  }

  if (listen(s, 5) == -1) {
    conf.log("fatal") << "listen(): " << strerror(errno);
    exit(1);
  }

  if (dup2(s, STDIN_FILENO) == -1) {
    conf.log("fatal") << "dup2(): " << strerror(errno);
    exit(1);
  }

  try {
    FCGIStream fcgiStream;
    WebController controller(conf, fcgiStream, sessionId);
    theController = &controller;

    controller.run();

    sleep(1);

    theController = 0;

    unlink(socketPath.c_str());
  } catch (std::exception& e) {
    conf.log("fatal") << "Dedicated session process for " << sessionId 
		      << ": caught unhandled exception: " << e.what();

    unlink(socketPath.c_str());

    throw;
  } catch (...) {
    conf.log("fatal") << "Dedicated session process for " << sessionId 
		      << ": caught unkown, unhandled exception.";

    unlink(socketPath.c_str());

    throw;
  }
}

void startSharedProcess(Configuration& conf)
{
  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s == -1) {
    conf.log("fatal") << "socket(): " << strerror(errno);
    exit(1);
  }

  socketPath = conf.runDirectory() + "/server-"
    + boost::lexical_cast<std::string>(getpid());

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socketPath.c_str());
  unlink(local.sun_path);
  socklen_t len = strlen(local.sun_path) + sizeof(local.sun_family);

  if (bind(s, (struct sockaddr *)& local, len) == -1) {
    conf.log("fatal") << "socket(): " << strerror(errno);
    exit(1);
  }

  if (listen(s, 5) == -1) {
    perror("listen");
    exit(1);
  }

  if (dup2(s, STDIN_FILENO) == -1) {
    perror("dup2");
    exit(1);
  }

  try {
    FCGIStream fcgiStream;
    WebController controller(conf, fcgiStream);
    theController = &controller;

    controller.run();

    theController = 0;

    unlink(socketPath.c_str());
  } catch (std::exception& e) {
    conf.log("fatal") << "Shared session server: caught unhandled exception: "
		      << e.what();

    unlink(socketPath.c_str());

    throw;
  } catch (...) {
    conf.log("fatal") << "Shared session server: caught unknown, unhandled "
      "exception.";

    unlink(socketPath.c_str());

    throw;
  }
}

void AddApplicationEntryPoint(ApplicationCreator createApplication,
			      const std::string& path)
{
  entryPoints.push_back
    (EntryPoint(WebSession::Application, createApplication, path));
}

void AddWidgetSetEntryPoint(ApplicationCreator createApplication,
			    const std::string& path)
{
  entryPoints.push_back
    (EntryPoint(WebSession::WidgetSet, createApplication, path));  
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  if (argc == 1) {
    Server webServer(argc, argv);
    return webServer.main();
  } else {
    Configuration conf(argv[0], "", Configuration::FcgiServer,
		       "Wt: initializing session process");

    if (signal(SIGTERM, Wt::handleSigTerm) == SIG_ERR)
      conf.log("error") << "Cannot catch SIGTERM: signal(): "
			<< strerror(errno);
    if (signal(SIGUSR1, Wt::handleSigUsr1) == SIG_ERR) 
      conf.log("error") << "Cannot catch SIGUSR1: signal(): "
			<< strerror(errno);

    if (createApplication)
      entryPoints.push_back(EntryPoint(WebSession::Application,
				       createApplication, std::string()));
    for (unsigned i = 0; i < entryPoints.size(); ++i)
      conf.addEntryPoint(entryPoints[i]);

    if (argc == 2) {
      startSharedProcess(conf);
    } else {
      runSession(conf, argv[2]);
    }
  }

  return 0;
}

}
