#ifndef WCelsium_H_
#define WCelsium_H_

#include <Wt/WColor.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WSignal.h>
#include <vector>
#include <string>

namespace Wt
{
  ///////////////////////////////////////////////////////////////////////////////////////
  //WCelsium
  ///////////////////////////////////////////////////////////////////////////////////////

  class WT_API WCelsium : public WCompositeWidget
  {
  public:
    WCelsium(const std::string &js);
  protected:
    virtual void render(WFlags<RenderFlag> flags);
    std::string m_javascrit;
  };

} //namespace Wt

#endif 
