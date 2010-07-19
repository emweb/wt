#include "WebController.h"
#include "Server.h"
#include "IsapiRequest.h"

#include <windows.h>
#include <httpext.h>

#include <sstream>

using namespace Wt;


namespace {
  isapi::IsapiServer *theServer;
}

BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO* pVer)
{
#ifdef _DEBUG
  char buffer[2048];
  std::string workingdir;
  DWORD retval = GetCurrentDirectory(sizeof(buffer), buffer);
  if (retval > 0 && retval < sizeof(buffer)) {
    workingdir = buffer;
  }
  std::stringstream ss;
  ss << "Please attach a debugger to the process " << GetCurrentProcessId() << " and click OK" << std::endl;
  ss << "Current working dir: " << workingdir;
  MessageBox(NULL, ss.str().c_str(), "Wt/ISAPI Debug Time!", MB_OK|MB_SERVICE_NOTIFICATION);
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
#if 1
  // IsapiRequest will schedule itself to be processed in the
  // server when it is completely received.
  new isapi::IsapiRequest(lpECB, theServer, true);
  return HSE_STATUS_PENDING;
#else
  std::string status = "200 OK";
  std::string body = "<h1>Hello World!</h1>";
  std::stringstream header;
  header << "Content-Type: text/html\r\n"
    << "Content-Length: " << body.size() << "\r\n"
    << "\r\n";
  std::string headerString = header.str();
  HSE_SEND_HEADER_EX_INFO hei = { 0 };
  hei.pszStatus = status.c_str();
  hei.cchStatus = status.size();
  hei.pszHeader = headerString.c_str();
  hei.cchHeader = headerString.size();
  hei.fKeepConn = true;
  lpECB->ServerSupportFunction(lpECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &hei, 0, 0);
  DWORD bytes = body.size();
  int success = lpECB->WriteClient(lpECB->ConnID, (LPVOID)body.c_str(), &bytes, HSE_IO_SYNC);
  if (!success) {
    int err = GetLastError();
    body += err;
  }
  return HSE_STATUS_SUCCESS_AND_KEEP_CONN;
  return HSE_STATUS_ERROR;
#endif
}

#if 0
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID lpvReserved)
{
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      MessageBeep(MB_ICONASTERISK);
      break;
  }
  return TRUE;
}
#endif
