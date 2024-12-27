// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBADGE_H_
#define WBADGE_H_

#include "Wt/WText.h"

namespace Wt {

/*! \class WBadge Wt/WBadge.h Wt/WBadge.h
 *  \brief A widget that represent a badge.
 *
 * A badge is a short text, often shown against a contrasting
 * background, which is used to convey important information at a
 * glance.
 *
 * For instance, in a shopping application, a badge with the
 * word "new" can be added to an article to show that this article
 * was recently added.
 *
 * Another example is to add a badge next to the channel in a chat
 * application to show the number of unread messages in that channel.
 *
 * A WBadge is an inline WText with a specific look. This look is
 * defined in any of %Wt's themes. Either they are "Wt-badge" for the
 * polished and default theme, or "badge" for any of the bootstrap
 * themes. For more information on the latter visit [Bootstrap's
 * documentation](https://getbootstrap.com/docs/5.3/components/badge)
 *
 * \sa WPushButton::setBadge(), WMenuItem::setBadge()
 *
 * \note With default %Wt themes, empty WBadges will not be shown.
 */
class WT_API WBadge: public WText
{
public:
  /*! \brief Creates an empty badge.
   *
   * \sa WText::WText()
   */
  WBadge();

  /*! \brief Creates a badge with the given text.
   *
   * \sa WText::WText(const WString&)
   */
  WBadge(const WString& text);

  /*! \brief Creates a badge with the given text and format.
   *
   * \sa WText::WText(const WString&,TextFormat)
   */
  WBadge(const WString& text, TextFormat textFormat);

  bool isInline() const override { return true; }

protected:
  void updateDom(DomElement& element, bool all) override;
};

}

#endif //WBADGE_H_