#ifndef WPLOTLY_H_
#define WPLOTLY_H_

#include <Wt/WColor.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WSignal.h>
#include <vector>
#include <string>

namespace Wt
{
  ///////////////////////////////////////////////////////////////////////////////////////
  //WPlotly
  ///////////////////////////////////////////////////////////////////////////////////////

  class WT_API WPlotly : public WCompositeWidget
  {
  public:
    WPlotly(const std::string &js);
  protected:
    virtual void render(WFlags<RenderFlag> flags);
    std::string m_javascrit;
  };

} //namespace Wt

#endif // WLEAFLET_H_
