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

#include "Wt/WResource"
#include "Wt/WServer"
#include "Wt/WLogger"

using std::exit;
using std::strcpy;
using std::strlen;
using std::memset;

namespace {

bool bindUDStoStdin(const std::string& socketPath, Wt::Configuration& conf)
{
  int s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s == -1) {
    conf.log("fatal") << "socket(): " << strerror(errno);
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
    conf.log("fatal") << "bind(): " << strerror(errno);
    return false;
  }

  if (listen(s, 5) == -1) {
    conf.log("fatal") << "listen(): " << strerror(errno);
    return false;
  }

  if (dup2(s, STDIN_FILENO) == -1) {
    conf.log("fatal") << "dup2(): " << strerror(errno);
    return false;
  }

  return true;
}

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
    conf_(argv[0], "", "", Configuration::FcgiServer,
	  "Wt: initializing FastCGI session process manager")
#ifdef WT_THREADED
  , threadPool_(conf_.numThreads())
#endif // WT_THREADED
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
  if (debug && conf_.debug())
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
  conf_.log("notice") << "Shutdown (caught " << signal << ")";

  /* We need to kill all children */
  for (unsigned i = 0; i < sessionProcessPids_.size(); ++i)
    kill(sessionProcessPids_[i], SIGTERM); 

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
   * Create the run directory if it does not yet exist.
   */
  FILE *test = fopen((conf_.runDirectory() + "/test").c_str(), "w+");

  if (test == NULL) {
    if (mkdir(conf_.runDirectory().c_str(), 777) != 0) {
      conf_.log("fatal") << "Cannot create run directory '"
			 << conf_.runDirectory() << "'";
      exit(1);
    }
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
  if (signal(SIGTERM, Wt::handleServerSigTerm) == SIG_ERR)
    conf_.log("error") << "Cannot catch SIGTERM: signal(): "
		       << strerror(errno);
  if (signal(SIGUSR1, Wt::handleServerSigUsr1) == SIG_ERR) 
    conf_.log("error") << "Cannot catch SIGUSR1: signal(): "
		       << strerror(errno);
  if (signal(SIGHUP, Wt::handleServerSigHup) == SIG_ERR) 
    conf_.log("error") << "Cannot catch SIGHUP: signal(): "
		       << strerror(errno);

  if (argc_ == 2 && boost::starts_with(argv_[1], "--socket=")) {
    std::string socketName = std::string(argv_[1]).substr(9);
    boost::trim(socketName);
    if (!bindUDStoStdin(socketName, conf_))
      return -1;
    conf_.log("notice") << "Reading FastCGI stream from socket '"
			<< socketName << '\'';
  } else
    conf_.log("notice") << "Reading FastCGI stream from stdin";

  for (;;) {
    //std::cerr << "accepting()" << std::endl;

    int serverSocket = accept(STDIN_FILENO, (sockaddr *) &clientname,
			      &socklen);

    //std::cerr << "accept()" << std::endl;

    if (serverSocket < 0) {
      conf_.log("fatal") << "accept(): " << strerror(errno);
      exit (1);
    }

    handleRequestThreaded(serverSocket);
  }

  return 0;
}

void Server::handleRequestThreaded(int serverSocket)
{
#ifdef WT_THREADED
  threadPool_.schedule(boost::bind(&Server::handleRequest, this, serverSocket));
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

      //std::cerr << "server read" << std::endl;

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
	  {
#ifdef WT_THREADED
	    boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
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
	      std::string path = conf_.runDirectory() + "/server-"
		+ boost::lexical_cast<std::string>(pid);

	      clientSocket = connectToSession("", path, 100);

	      if (clientSocket == -1)
		conf_.log("error") << "Session process " << pid
				   << " not responding ?";

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
	conf_.log("error") << "Error writing to client";
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

      //std::cerr << "select()" << std::endl;
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
	  if (!writeToSocket(clientSocket, d.plainText(),
			     d.plainTextLength())) {
	    conf_.log("error") << "Error writing to application";

	    break;
	  }
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
	  //std::cerr << "Got record from client: " << d << std::endl;
	  if (!writeToSocket(serverSocket, d.plainText(),
			     d.plainTextLength())) {
	    conf_.log("error") << "Error writing to web server";

	    break;
	  }

	  if (d.type() == FCGI_END_REQUEST)
	    break;
	} else {
	  conf_.log("error") << "Error reading from application";

	  break;
	}
      }
    }

    //std::cerr << "Request done." << std::endl;

    shutdown(serverSocket, SHUT_RDWR);
    close(serverSocket);
    close(clientSocket);
  } catch (std::exception&) {
    close(serverSocket);
    if (clientSocket != -1)
      close(clientSocket);
  }
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
    theController->configuration().log("notice") << "Caught " << signal;
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

static void handleSigHup(int)
{
  doShutdown("SIGHUP");
}

void runSession(Configuration& conf, WServer *server, std::string sessionId)
{
  if (!bindUDStoStdin(conf.runDirectory() + "/" + sessionId, conf))
    exit(1);

  try {
    FCGIStream fcgiStream;
    WebController controller(conf, server, &fcgiStream, sessionId);
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

void startSharedProcess(Configuration& conf, WServer *server)
{
  if (!bindUDStoStdin(conf.runDirectory() + "/server-"
		      + boost::lexical_cast<std::string>(getpid()),
		      conf))
    exit(1);

  try {
    FCGIStream fcgiStream;
    WebController controller(conf, server, &fcgiStream);
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

struct WServerImpl {
  WServerImpl(const std::string& applicationPath,
	      const std::string& configurationFile)
    : applicationPath_(applicationPath),
      configurationFile_(configurationFile),
      configuration_(0),
      relayServer_(0),
      running_(false)
  { }

  std::string applicationPath_;
  std::string configurationFile_;

  Configuration *configuration_;
  Server        *relayServer_;
  bool          running_;
  std::string   sessionId_;
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& configurationFile)
  : impl_(new WServerImpl(applicationPath, configurationFile))
{ }

WServer::~WServer()
{
  delete impl_;
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{
  bool isServer = argc < 2 || strcmp(argv[1], "client") != 0; 

  if (isServer) {
    Server webServer(argc, argv);
    exit(webServer.main());
  } else {
    /*
     * FastCGI configures the working directory to the location of the
     * binary, which is a convenient default, but not really ideal.
     */
    std::string appRoot;
    impl_->configuration_
      = new Configuration(impl_->applicationPath_,
			  appRoot,
			  impl_->configurationFile_,
			  Configuration::FcgiServer,
			  "Wt: initializing session process");

    if (argc >= 3)
      impl_->sessionId_ = argv[2];
  }
}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path, const std::string& favicon)
{
  if (!impl_->configuration_)
    throw Exception("WServer::addEntryPoint(): "
		    "call setServerConfiguration() first");

  impl_->configuration_
    ->addEntryPoint(EntryPoint(type, callback, path, favicon));
}

bool WServer::start()
{
  if (!impl_->configuration_)
    throw Exception("WServer::start(): call setServerConfiguration() first");

  if (isRunning()) {
    std::cerr << "WServer::start() error: server already started!" << std::endl;
    return false;
  }

  if (signal(SIGTERM, Wt::handleSigTerm) == SIG_ERR)
    impl_->configuration_->log("error") << "Cannot catch SIGTERM: signal(): "
					<< strerror(errno);
  if (signal(SIGUSR1, Wt::handleSigUsr1) == SIG_ERR) 
    impl_->configuration_->log("error") << "Cannot catch SIGUSR1: signal(): "
					<< strerror(errno);
  if (signal(SIGHUP, Wt::handleSigHup) == SIG_ERR) 
    impl_->configuration_->log("error") << "Cannot catch SIGHUP: signal(): "
					<< strerror(errno);

  impl_->running_ = true;

  if (impl_->sessionId_.empty())
    startSharedProcess(*impl_->configuration_, this);
  else
    runSession(*impl_->configuration_, this, impl_->sessionId_);

  return false;
}

bool WServer::isRunning() const
{
  return impl_ && impl_->running_;
}

int WServer::httpPort() const
{
  return -1;
}

void WServer::stop()
{
  if (!isRunning()) {
    std::cerr << "WServer::stop() error: server not yet started!" << std::endl;
    return;
  }
}

int WServer::waitForShutdown(const char *restartWatchFile)
{
  for (;;)
    sleep(10000);

  return 0;
}

void WServer::addResource(WResource *resource, const std::string& path)
{
  if (!boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addResource() error: "
                             "static resource path should start with \'/\'");

  resource->setInternalPath(path);
  impl_->configuration_->addEntryPoint(EntryPoint(resource, path));

}

void WServer::handleRequest(WebRequest *request)
{
  theController->handleRequest(request);
}

std::string WServer::appRoot() const
{
  return impl_->configuration_->appRoot();
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  try {
    WServer server(argv[0], "");

    try {
      server.setServerConfiguration(argc, argv);
      server.addEntryPoint(Application, createApplication);
      server.start();

      return 0;
    } catch (std::exception& e) {
      server.impl()->configuration_->log("fatal") << e.what();
      return 1;
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    return 1;
  }
}

}
