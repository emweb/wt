#include "GoogleService.h"

#include "Wt/WLogger.h"

#define ERROR_MSG(e) WString::tr("Wt.Auth.GoogleService." e)

namespace {
  const char *RedirectEndpointProperty = "google-oauth2-redirect-endpoint";
  const char *RedirectEndpointPathProperty = "google-oauth2-redirect-endpoint-"
    "path";
  const char *ClientIdProperty = "google-oauth2-client-id";
  const char *ClientSecretProperty = "google-oauth2-client-secret";
}


namespace Wt {

LOGGER("Auth.GoogleService");

  namespace Auth {

GoogleService::GoogleService(const AuthService& baseAuth)
  : OidcService(baseAuth)
{
  setRedirectEndpoint(configurationProperty(RedirectEndpointProperty));
  setClientId(configurationProperty(ClientIdProperty));
  setClientSecret(configurationProperty(ClientSecretProperty));

  setAuthEndpoint("https://accounts.google.com/o/oauth2/v2/auth");
  setTokenEndpoint("https://www.googleapis.com/oauth2/v4/token");
  setUserInfoEndpoint("https://www.googleapis.com/oauth2/v3/userinfo");

  setAuthenticationScope("openid email profile");

  setName("google");
  setDescription("Google Account");
  setPopupWidth(550);
}

bool GoogleService::configured()
{
  try {
    configurationProperty(RedirectEndpointProperty);
    configurationProperty(ClientIdProperty);
    configurationProperty(ClientSecretProperty);

    return true;
  } catch (const std::exception& e) {
    LOG_INFO("not configured: " << e.what());

    return false;
  }
}

std::string GoogleService::redirectEndpointPath() const
{
  try {
    return configurationProperty(RedirectEndpointPathProperty);
  } catch (const std::exception& e) {
    return OAuthService::redirectEndpointPath();
  }
}

  }
}
