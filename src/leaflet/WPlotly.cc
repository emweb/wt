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

#include "leaflet/WPlotly.hh"

namespace Wt
{
  LOGGER("WPlotly");

  ///////////////////////////////////////////////////////////////////////////////////////
  //WPlotly::WPlotly
  ///////////////////////////////////////////////////////////////////////////////////////

  WPlotly::WPlotly(const std::string &js)
  {
    setImplementation(std::unique_ptr<WWidget>(new WContainerWidget()));
    this->addCssRule("html", "height: 100%");
    this->addCssRule("body", "height: 100%");
    this->addCssRule("#" + id(), "position:relative; top:0; bottom:0; height: 100%");
    WApplication *app = WApplication::instance();
    const std::string library = "https://cdn.plot.ly/plotly-latest.min.js";
    app->require(library, "plotly");
    m_javascrit = js;
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WPlotly::render
  ///////////////////////////////////////////////////////////////////////////////////////

  void WPlotly::render(WFlags<RenderFlag> flags)
  {
    if (flags.test(RenderFlag::Full))
    {
      Wt::WApplication * app = Wt::WApplication::instance();
      Wt::WString initFunction = app->javaScriptClass() + ".init_leaflet_" + id();
      Wt::WStringStream strm;

      strm
        << "{ " << initFunction.toUTF8() << " = function() {\n"
        << "  var self = " << jsRef() << ";\n"
        << "  if (!self) {\n"
        << "    setTimeout(" << initFunction.toUTF8() << ", 0);\n"
        << "  }\n";

      //chart code
      strm << m_javascrit;

      //pass 'DIV', data object MUST named 'data', layout object MUST named 'layout'
      strm
        << "  Plotly.newPlot(" << jsRef()
        << "  , data, layout)\n";

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
