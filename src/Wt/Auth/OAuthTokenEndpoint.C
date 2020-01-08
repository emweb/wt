/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "OAuthTokenEndpoint.h"

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "Wt/WResource.h"
#include "Wt/WRandom.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/IssuedToken.h"
#include "Wt/Auth/OAuthClient.h"
#include "Wt/Json/Serializer.h"
#include "Wt/Utils.h"
#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Value.h"
#include "WebUtils.h"
#include "Wt/WLogger.h"
#include "Wt/WException.h"

#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
#include <openssl/sha.h>
#include <openssl/ssl.h> // NID_sha256
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA

namespace {
const std::string GRANT_TYPE = "authorization_code";
const std::string AUTH_TYPE = "Basic";
}

namespace Wt {

LOGGER("OAuthTokenEndpoint");

namespace Auth {

OAuthTokenEndpoint::OAuthTokenEndpoint(AbstractUserDatabase& db,
                                       std::string issuer)
  : db_(&db),
    accessExpSecs_(3600),
    idExpSecs_(3600),
    iss_(issuer)
#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
   ,privateKey(NULL)
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
{
}

OAuthTokenEndpoint::~OAuthTokenEndpoint()
{
  beingDeleted();
#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
  if (privateKey)
    RSA_free(privateKey);
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
}

void OAuthTokenEndpoint::handleRequest(const Http::Request &request, Http::Response &response)
{
#ifdef WT_TARGET_JAVA
  try {
#endif // WT_TARGET_JAVA
  response.setMimeType("application/json");
  response.addHeader("Cache-Control", "no-store");
  response.addHeader("Pragma", "no-cache");

  const std::string *grantType = request.getParameter("grant_type");
  const std::string *redirectUri = request.getParameter("redirect_uri");
  const std::string *code = request.getParameter("code");
  std::string clientId;
  std::string clientSecret;
  ClientSecretMethod authMethod = HttpAuthorizationBasic;

  // Preferred method: get authorization information from
  // Http Basic authentication
  std::string headerSecret;
  std::string authHeader = request.headerValue("Authorization");
  if (authHeader.length() > AUTH_TYPE.length() + 1) {
#ifndef WT_TARGET_JAVA
    headerSecret = Utils::base64Decode(authHeader.substr(AUTH_TYPE.length() + 1));
#else
    headerSecret = Utils::base64DecodeS(authHeader.substr(AUTH_TYPE.length() + 1));
#endif // WT_TARGET_JAVA
    std::vector<std::string> tokens;
    boost::split(tokens, headerSecret, boost::is_any_of(":"));
    if (tokens.size() == 2) {
      clientId = Utils::urlDecode(tokens[0]);
      clientSecret = Utils::urlDecode(tokens[1]);
      authMethod = HttpAuthorizationBasic;
    }
  }

  // Alternative method: pass authorization information as parameters
  // (only allowed for post methods)
  if (clientId.empty() && clientSecret.empty()) {
    const std::string *clientIdParam = request.getParameter("client_id");
    const std::string *clientSecretParam = request.getParameter("client_secret");
    if (clientIdParam && clientSecretParam) {
      clientId = *clientIdParam;
      clientSecret = *clientSecretParam;
      authMethod = RequestBodyParameter;
    }
  }

  if (!code || clientId.empty() || clientSecret.empty() || !grantType || !redirectUri) {
    response.setStatus(400);
    response.out() << "{\"error\": \"invalid_request\"}" << std::endl;
    LOG_INFO("{\"error\": \"invalid_request\"}:"
      << " code:" << (code ? *code : "NULL")
      << " clientId: " << clientId
      << " clientSecret: " << (clientSecret.empty() ? "MISSING" : "NOT MISSING")
      << " grantType: " << (grantType ? *grantType : "NULL")
      << " redirectUri: " << (redirectUri ? *redirectUri : "NULL"));
    return;
  }
  OAuthClient client = db_->idpClientFindWithId(clientId);
  if (!client.checkValid() || !client.verifySecret(clientSecret)
      || client.authMethod() != authMethod) {
    response.setStatus(401);
    if (!authHeader.empty()) {
      if (client.authMethod() == HttpAuthorizationBasic)
        response.addHeader("WWW-Authenticate", AUTH_TYPE);
      else
        response.addHeader("WWW-Authenticate",
            methodToString(client.authMethod()));
    }
    response.out() << "{\n\"error\": \"invalid_client\"\n}" << std::endl;
    LOG_INFO("{\"error\": \"invalid_client\"}: "
      << " id: " << clientId
      << " client: " << (client.checkValid() ? "valid" : "not valid")
      << " secret: " << (client.verifySecret(clientSecret) ? "correct" : "incorrect")
	     << " method: " << (client.authMethod() != authMethod ? "no match" : "match")
    );
    return;
  }
  if (*grantType != GRANT_TYPE) {
    response.setStatus(400);
    response.out() << "{\n\"error\": \"unsupported_grant_type\"\n}" << std::endl;
    LOG_INFO("{\"error\": \"unsupported_grant_type\"}: "
      << " id: " << clientId
      << " grantType: " << grantType
    );
    return;
  }
  IssuedToken authCode = db_->idpTokenFindWithValue(GRANT_TYPE, *code);
  if (!authCode.checkValid() || authCode.redirectUri() != *redirectUri
      || WDateTime::currentDateTime() > authCode.expirationTime()) {
    response.setStatus(400);
    response.out() << "{\n\"error\": \"invalid_grant\"\n}" << std::endl;
    LOG_INFO("{\"error\": \"invalid_grant\"}:"
      << " id: " << clientId
      << " code: " << *code
      << " authCode: " << (authCode.checkValid() ? "valid" : "not valid")
      << " redirectUri: " << *redirectUri << (authCode.redirectUri() != *redirectUri ? " - invalid" : " - valid")
      << " timestamp: " << authCode.expirationTime().toString() << (WDateTime::currentDateTime() > authCode.expirationTime() ? ", expired" : ", not expired")
    );
    return;
  }
  std::string accessTokenValue = WRandom::generateId();
  WDateTime expirationTime = WDateTime::currentDateTime().addSecs(accessExpSecs_);
  const User &user = authCode.user();
  const OAuthClient &authClient = authCode.authClient();
  const std::string scope = authCode.scope();
  db_->idpTokenAdd(accessTokenValue, expirationTime, "access_token", scope,
                   authCode.redirectUri(), user, authClient);
  db_->idpTokenRemove(authCode);
  response.setStatus(200);
  Json::Object root;
  root["access_token"] = Json::Value(accessTokenValue);
  root["token_type"] = Json::Value("Bearer");
  root["expires_in"] = Json::Value(accessExpSecs_);
  if (authCode.scope().find("openid") != std::string::npos) {
    std::string header;
    std::string signature;
    std::string payload = Utils::base64Encode(idTokenPayload(authClient.clientId(), scope, user), false);
#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
    if (privateKey) {
      header    = Utils::base64Encode("{\n\"typ\": \"JWT\",\n\"alg\": \"RS256\"\n}", false);
      signature = Utils::base64Encode(rs256(header + "." + payload), false);
    } else {
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
      header    = Utils::base64Encode("{\n\"typ\": \"JWT\",\n\"alg\": \"none\"\n}", false);
      signature = Utils::base64Encode("", false);
#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
    }
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
    root["id_token"] = Json::Value(header + "." + payload + "." + signature);
  }
  response.out() << Json::serialize(root);

  LOG_INFO("success: " << clientId << ", " << user.id() << ", " << db_->email(user));

#ifdef WT_TARGET_JAVA
  } catch (std::io_exception ioe) {
    LOG_ERROR(ioe.message());
  }
#endif
}

const std::string OAuthTokenEndpoint::idTokenPayload(const std::string &clientId,
                                                     const std::string &scope,
                                                     const User &user)
{
  Json::Object root;
  root["iss"] = Json::Value(iss_);
  root["sub"] = Json::Value(user.id());
  root["aud"] = Json::Value(clientId);
  WDateTime curTime = WDateTime::currentDateTime();
  root["exp"] = Json::Value(static_cast<long long>(curTime.addSecs(idExpSecs_).toTime_t()));
  root["iat"] = Json::Value(static_cast<long long>(curTime.toTime_t()));
  root["auth_time"] =
    Json::Value(boost::lexical_cast<std::string>(
          user.lastLoginAttempt().toTime_t()));

  return Json::serialize(root);
}

void OAuthTokenEndpoint::setAccessExpSecs(int seconds)
{
  accessExpSecs_ = seconds;
}

void OAuthTokenEndpoint::setIdExpSecs(int seconds)
{
  idExpSecs_ = seconds;
}

std::string OAuthTokenEndpoint::methodToString(ClientSecretMethod method)
{
  switch(method)
  {
    case HttpAuthorizationBasic: return "client_secret_basic";
    case RequestBodyParameter:   return "client_secret_post";
    default: return "";
  }
}


#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
std::string OAuthTokenEndpoint::rs256(const std::string &token )
{
  // sha256
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, token.c_str(), token.size());
  SHA256_Final(hash, &sha256);
  unsigned int len;
  unsigned char buff[512]; // size 256 should be ok

  // rsa sign
  int status = RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, buff, &len, privateKey);
  return std::string((const char *) buff, len);
}

void OAuthTokenEndpoint::setRSAKey(const std::string &path)
{
  if (privateKey)
    RSA_free(privateKey);
  RSA* rsa = RSA_new();
  privateKey = PEM_read_RSAPrivateKey(fopen(path.c_str(), "rb"), &rsa, NULL, NULL);
  if (!privateKey) {
    throw WException("OAuthTokenEndpoint: invalid RSA key \"" + path + "\"");
  }
}
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA

}
}
