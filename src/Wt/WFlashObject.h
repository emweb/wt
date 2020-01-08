// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFLASHOBJECT_H_
#define WFLASHOBJECT_H_

#include <Wt/WWebWidget.h>

namespace Wt {

/*! \class WFlashObject Wt/WFlashObject.h Wt/WFlashObject.h
 *  \brief A widget that renders a Flash object (also known as Flash movie).
 *
 * This class loads a .swf Flash file in the browser.
 *
 * \if cpp
 * Usage example:
 * \code
 * WFlashObject *player = parent->addWidget(std::make_unique<WFlashObject>("dummy.swf"));
 * player->resize(300, 600);
 * player->setFlashParameter("allowScriptAccess", "always");
 * player->setFlashParameter("quality", "high");
 * player->setFlashParameter("bgcolor", "#aaaaaa");
 * player->setFlashVariable("someVar", "foo");
 * \endcode
 * \endif
 *
 * Flash objects must have their size set, so do not forget to call
 * resize() after instantiation or your content will be invisible. %Wt
 * will modify width and height attributes of the Flash object if resize()
 * is called after the object is instantiated; it is however not clear
 * if this is permitted by the Flash plugin.
 *
 * Any WWidget can be set with setAlternativeContent(), and this widget
 * will be shown only when the browser has no Flash support. By default,
 * a 'Download Flash' button will be displayed that links to a website
 * where the Flash player can be downloaded. You may modify this to be
 * any widget, such as a WImage, or a native %Wt implementation of the
 * Flash movie.
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 */
class WT_API WFlashObject : public WWebWidget
{
public:
  /*! \brief Constructs a Flash widget.
   */
  WFlashObject(const std::string &url);

  /*! \brief Destructor
   *
   * The Flash object is removed.
   */
  ~WFlashObject();

  virtual void resize(const WLength &width, const WLength &height) override;

  /*! \brief Sets a Flash parameter.
   *
   * The Flash parameters are items such as quality, scale, menu, ...
   * They are passed as PARAM objects to the Flash movie. See the
   * adobe website for more information about these parameters:
   * http://www.adobe.com/cfusion/knowledgebase/index.cfm?id=tn_12701
   *
   * Setting the same Flash parameter a second time will overwrite the
   * previous value. Flash parameters can only be set before the widget
   * is rendered for the first time, so it is recommended to call this
   * method shortly after construction before returning to the idle loop.
   */
  void setFlashParameter(const std::string &name, const WString &value);

  /*! \brief Sets a Flash variable.
   *
   * This method is a helper function to set variable values in the
   * flashvars parameter.
   *
   * The flash variables will be properly encoded (URL encoding) before
   * being passed to the flashvars parameter.
   *
   * Setting the same Flash variable a second time will overwrite the
   * previous value. Flash variables can only be set before the widget
   * is rendered for the first time, so it is recommended to call this
   * method shortly after construction before returning to the idle loop.
   */
  void setFlashVariable(const std::string &name, const WString &value);

  /*! \brief A JavaScript expression that returns the DOM node of the Flash
   *         object.
   *
   * The Flash object is not stored in jsRef(), but in jsFlashRef(). Use this
   * method in conjuction with WApplication::doJavaScript() or JSlot in custom
   * JavaScript code to refer to the Flash content.
   *
   * The expression returned by jsFlashRef() may be null,
   * for example on IE when flash is not installed.
   */
  std::string jsFlashRef() const;

  /*! \brief Sets content to be displayed if Flash is not available.
   *
   * Any widget can be a placeholder when Flash is not installed in the
   * users browser. By default, this will show a 'Download Flash' button
   * and link to the Flash download site.
   *
   * Call this method with a NULL pointer to remove the alternative content.
   */
  void setAlternativeContent(std::unique_ptr<WWidget> alternative);

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual DomElementType domElementType() const override;

private:
  std::string url_;
  bool sizeChanged_;
  std::map<std::string, WString> parameters_;
  std::map<std::string, WString> variables_;
  std::unique_ptr<WWidget> alternative_;
  JSignal<> ieRendersAlternative_;
  bool replaceDummyIeContent_;
  void renderIeAltnerative();
};

}

#endif // WFLASHOBJECT_H_

