#include <Wt/WLogger.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include "web/WebUtils.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <utility>
#include <iostream>
#include <cmath>

#include "leaflet/WCelsium.hh"

namespace Wt
{
  LOGGER("WCelsium");

  ///////////////////////////////////////////////////////////////////////////////////////
  //WCelsium::WCelsium
  ///////////////////////////////////////////////////////////////////////////////////////

  WCelsium::WCelsium(const std::string &js)
  {
    setImplementation(std::unique_ptr<WWidget>(new WContainerWidget()));
    this->addCssRule("html", "width: 100%; height: 100%; margin: 0; padding: 0; overflow: hidden;");
    this->addCssRule("body", "width: 100%; height: 100%; margin: 0; padding: 0; overflow: hidden;");
    this->addCssRule("#" + id(), "width: 100%; height: 100%; margin: 0; padding: 0; overflow: hidden;");
    WApplication *app = WApplication::instance();
    app->useStyleSheet("http://cesiumjs.org/releases/1.48/Build/Cesium/Widgets/widgets.css");
    const std::string library = "http://cesiumjs.org/releases/1.48/Build/Cesium/Cesium.js";
    app->require(library, "celsium");
    m_javascrit = js;
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WCelsium::render
  ///////////////////////////////////////////////////////////////////////////////////////

  void WCelsium::render(WFlags<RenderFlag> flags)
  {
    if (flags.test(RenderFlag::Full))
    {
      Wt::WApplication * app = Wt::WApplication::instance();
      Wt::WString initFunction = app->javaScriptClass() + ".init_celsium_" + id();
      Wt::WStringStream strm;

      strm
        << "{ " << initFunction.toUTF8() << " = function() {\n"
        << "  var self = " << jsRef() << ";\n"
        << "  if (!self) {\n"
        << "    setTimeout(" << initFunction.toUTF8() << ", 0);\n"
        << "  }\n";

      //pass 'DIV'
      strm
        << "  var viewer = new Cesium.Viewer(" << jsRef() << ");\n";

      //rendering code (assume 'viewer' variable)
      strm << m_javascrit;

      strm
        << "  setTimeout(function(){ delete " << initFunction.toUTF8() << ";}, 0)};\n"
        << "}\n"
        << initFunction.toUTF8() << "();\n";

      if (1) LOG_INFO(strm.str());
      app->doJavaScript(strm.str(), true);
    }
    Wt::WCompositeWidget::render(flags);
  }
}
