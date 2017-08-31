IF(INSTALL_EXTENSIONLESS_HEADERS)
  INSTALL(FILES
    Client
    Message
    Method
    Request
    Response
    ResponseContinuation
    WtClient
    DESTINATION include/Wt/Http
  )
ENDIF(INSTALL_EXTENSIONLESS_HEADERS)

INSTALL(FILES
  Client.h
  Message.h
  Method.h
  Request.h
  Response.h
  ResponseContinuation.h
  WtClient.h
  DESTINATION include/Wt/Http
)
