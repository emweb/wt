// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLINK_H_
#define WLINK_H_

#include <Wt/WGlobal.h>

#include <string>

namespace Wt {

/*! \brief An enumeration for a link type.
 *
 * \sa type()
 */
enum class LinkType {
  Url,          //!< A static URL.
  Resource,     //!< A dynamic resource.
  InternalPath  //!< An internal path.
};

/*! \brief A value class that defines a hyperlink target.
 *
 * This class abstracts a link target. Depending on the context, it
 * may reference a URL, a dynamic \link WResource resource\endlink, or
 * (for certain usages) an internal path.
 *
 * \sa WAnchor, WImage, WMediaPlayer, WPopupMenuItem, WPushButton
 */
class WT_API WLink
{
public:
  /*! \brief Typedef for enum Wt::LinkType */
  typedef LinkType Type;
  /*! \brief Typedef for enum Wt::LinkTarget */
  typedef LinkTarget Target;

  /*! \brief Default constructor.
   *
   * This constructs a null link.
   */
  WLink();

  /*! \brief Creates a link to a (static) URL.
   *
   * \sa setUrl()
   */
  WLink(const char *url);

  /*! \brief Creates a link to a (static) URL.
   *
   * \sa setUrl()
   */
  WLink(const std::string& url);

  /*! \brief Creates a link to a (static) URL or an internal path.
   *
   * Using this constructor, you can create a link to a static URL (\p
   * type == LinkType::Url) or an internal path (\p type ==
   * LinkType::InternalPath). For an internal path, the \p value will
   * be interpreted as a CharEncoding::UTF8 encoded string.
   *
   * \sa setUrl(), setInternalPath()
   */
  WLink(LinkType type, const std::string& value);

  /*! \brief Creates a link to a resource.
   *
   * \sa setResource()
   */
  WLink(const std::shared_ptr<WResource>& resource);

  /*! \brief Returns the link type.
   *
   * The type is implicitly set depending on the constructor or after
   * calling setUrl(), setResource() or setInternalPath().
   *
   * The default type for a null link is LinkType::Url.
   */
  LinkType type() const { return type_; }

  /*! \brief Returns whether the link is unspecified.
   *
   * A null link is a link created using the default constructor and
   * points to nowhere.
   *
   * \sa WLink()
   */
  bool isNull() const;

  /*! \brief Sets the link URL.
   *
   * This sets the type to LinkType::Url.
   */
  void setUrl(const std::string& url);

  /*! \brief Returns the link URL.
   *
   * The return value is the URL set by setUrl(), the resource URL of
   * the resource set using setResource(), or the canonical URL of
   * an internal path within the current application context.
   */
  std::string url() const;

  /*! \brief Sets the link resource.
   *
   * This sets the type to LinkType::Resource.
   */
  void setResource(const std::shared_ptr<WResource>& resource);

  /*! \brief Returns the link resource.
   *
   * This returns the resource previously set using setResource(), or \c 0.
   *
   * \sa setResource()
   */
  std::shared_ptr<WResource> resource() const;

  /*! \brief Sets the link internal path.
   *
   * This points the link to the given internal path.
   */
  void setInternalPath(const WT_USTRING& internalPath);

  /*! \brief Returns the internal path.
   *
   * This returns the internal path perviously set using setInternalPath(),
   * or an empty string otherwise.
   *
   * \sa setInternalPath().
   */
  WT_USTRING internalPath() const;
  
  /*! \brief Sets the location where the linked content should be
   *         displayed.
   *
   * By default, the linked content is displayed in the application
   * (Wt::LinkTarget::Self). When the destination is an HTML document, the
   * application is replaced with the new document. When the link is
   * to a document that cannot be displayed in the browser, it is
   * offered for download or opened using an external program,
   * depending on browser settings.
   *
   * By setting \p target to Wt::LinkTarget::NewWindow, the destination
   * is displayed in a new browser window or tab.
   *
   * \sa target()
   */
  void setTarget(LinkTarget target);

  /*! \brief Returns the location where the linked content should be
   *         displayed.
   *
   * \sa setTarget()
   */
  LinkTarget target() const { return target_; }

  /*! \brief Comparison operator.
   */
  bool operator== (const WLink& other) const;

  /*! \brief Comparison operator.
   */
  bool operator!= (const WLink& other) const;

  std::string resolveUrl(WApplication *app) const;

private:
  LinkType type_;
  std::string stringValue_;
  std::shared_ptr<WResource> resource_;

  LinkTarget  target_;

  JSlot *manageInternalPathChange(WApplication *app, WInteractWidget *widget,
				  JSlot *slot) const;

  friend class WAnchor;
  friend class WAbstractArea;
  friend class WPushButton;
};

}

#endif // WLINK_H_
