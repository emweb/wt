/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Comment.h"
#include "Post.h"
#include "User.h"
#include "Tag.h"
#include "Token.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/WWebWidget.h>

DBO_INSTANTIATE_TEMPLATES(Comment)

namespace {
  std::string& replace(std::string& s, const std::string& k,
		       const std::string& r)
  {
    std::string::size_type p = 0;

    while ((p = s.find(k, p)) != std::string::npos) {
      s.replace(p, k.length(), r);
      p += r.length();
    }

    return s;
  }
}

void Comment::setText(const Wt::WString& src)
{
  textSrc_ = src;

  std::string html = Wt::WWebWidget::escapeText(src, true).toUTF8();

  std::string::size_type b = 0;

  // Replace &lt;code&gt;...&lt/code&gt; with <pre>...</pre>
  // This is kind of very ad-hoc!

  while ((b = html.find("&lt;code&gt;", b)) != std::string::npos) {
    std::string::size_type e = html.find("&lt;/code&gt;", b);
    if (e == std::string::npos)
      break;
    else {
      if (b > 6 && html.substr(b - 6, 6) == "<br />") {
	html.erase(b - 6, 6);
	b -= 6;
	e -= 6;
      }

      html.replace(b, 12, "<pre>");
      e -= 7;

      if (html.substr(b + 5, 6) == "<br />") {
	html.erase(b + 5, 6);
	e -= 6;
      }

      if (html.substr(e - 6, 6) == "<br />") {
	html.erase(e - 6, 6);
	e -= 6;
      }

      html.replace(e, 13, "</pre>");
      e += 6;

      if (e + 6 <= html.length() && html.substr(e, 6) == "<br />") {
	html.erase(e, 6);
	e -= 6;
      }

      b = e;
    }
  }

  // We would also want to replace <br /><br /> (empty line) with
  // <div class="vspace"></div>
  replace(html, "<br /><br />", "<div class=\"vspace\"></div>");

  textHtml_ = Wt::WString(html);
}

void Comment::setDeleted()
{
  textHtml_ = Wt::WString::tr("comment-deleted");
}
