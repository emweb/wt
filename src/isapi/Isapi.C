#include "WebController.h"
#include "Server.h"
#include "IsapiRequest.h"

#include <windows.h>
#include <WtsApi32.h>
#include <httpext.h>

#include <fstream>
#include <process.h>
#include <sstream>

using namespace Wt;


namespace {
  isapi::IsapiServer *theServer;
}

BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO* pVer)
{
  //DebugBreak();
#if 0
//#ifdef _DEBUG
  char buffer[2048];
  std::string workingdir;
  DWORD retval = GetCurrentDirectory(sizeof(buffer), buffer);
  if (retval > 0 && retval < sizeof(buffer)) {
    workingdir = buffer;
  }
  std::stringstream ss;
  ss << "Please attach a debugger to the process " << GetCurrentProcessId() << " and click OK" << std::endl;
  ss << "Current working dir: " << workingdir;
  char title[] = "Wt/ISAPI Debug Time!";
  DWORD response;
  WTSSendMessage(WTS_CURRENT_SERVER_HANDLE, WTSGetActiveConsoleSessionId(), title, sizeof(title), (LPSTR)ss.str().c_str(), ss.str().length() + 1, MB_OK|MB_ICONINFORMATION, 0, &response, TRUE);
  MessageBox(NULL, ss.str().c_str(), "Wt/ISAPI Debug Time!", MB_OK|MB_SERVICE_NOTIFICATION);
//#endif
#endif
  pVer->dwExtensionVersion = HSE_VERSION;
  strncpy_s(pVer->lpszExtensionDesc, "Wt ISAPI Connector", HSE_MAX_EXT_DLL_NAME_LEN);

  // Instantiate the server
  theServer = isapi::IsapiServer::instance();
  return TRUE;
}

BOOL WINAPI TerminateExtension(DWORD dwFlags)
{
  isapi::IsapiServer::instance()->shutdown();
  delete theServer;
  return TRUE;
}

DWORD WINAPI HttpExtensionProc(LPEXTENSION_CONTROL_BLOCK lpECB)
{
  // IsapiRequest will schedule itself to be processed in the
  // server when it is completely received.
  new isapi::IsapiRequest(lpECB, theServer, false);
  return HSE_STATUS_PENDING;
}

