#include "WebSocketApplication.h"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTemplate.h>

WebSocketApplication::WebSocketApplication(const Wt::WEnvironment& env, const std::string& resourceURL)
  : Wt::WApplication(env),
    webSocketResourceURL_(resourceURL)
{
  setTheme(std::make_shared<Wt::WBootstrap5Theme>());

  createLayout();
}

void WebSocketApplication::createLayout()
{
  templ_ = root()->addNew<Wt::WTemplate>("<div class='container'>"
                                         "  <div class='row'>"
                                         "    <div class='col-xs-10'>"
                                         "      This application is simply informative. It does not offer any functionality."
                                         "      It simply states how to use a simple <a href='https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1WWebSocketResource.html'>WWebSocketResource</a>"
                                         "      that has been deployed statically."
                                         "      Statically in this context means that its is present on a fixed URL/endpoint on the server."
                                         "    </div>"
                                         "  </div>"
                                         "  <div class='row'>"
                                         "    <div class='col-xs-4 mt-4'>"
                                         "      <h3>Connecting:</h3>"
                                         "      <p>The resource is located on the relative path: <code>${path}</code>. "
                                         "      You are able to connect to this endpoint with any WebSocket client, or using your browser console.</p>"
                                         "      <p>For your browser the set-up will look like <code>let socket = new WebSocket(URL)</code> "
                                         "      Depending on whether you want to connect over TLS or not, the request will start with <code>wss://</code>, or <code>ws://</code> "
                                         "      respectively."
                                         "      A full request will be formatted like <code>PROTOCOL://HOST:PORT/PATH</code>. "
                                         "      Here PROTOCOL can thus be either <code>wss</code>, or <code>ws</code>. "
                                         "      HOST simply stands for the hostname you are conencting to, and PORT to the open port to which the server is listening for connections. "
                                         "      The PATH section will contain the URL of the resource, but also optionally the deployement path, if this isn't empty.</p>"
                                         "    </div>"
                                         "  </div>"
                                         "</div>");
  templ_->bindString("path", webSocketResourceURL_);
}
