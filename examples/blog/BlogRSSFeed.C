/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/Http/Response>

#include "BlogRSSFeed.h"

#include "BlogSession.h"
#include "model/User.h"
#include "model/Post.h"
#include "model/Comment.h"
#include "model/Tag.h"

namespace dbo = Wt::Dbo;

BlogRSSFeed::BlogRSSFeed(const std::string& sqliteDb,
			 const std::string& title,
			 const std::string& url,
			 const std::string& description)
  : session_(new BlogSession(sqliteDb)),
    title_(title),
    url_(url),
    description_(description)
{ }

BlogRSSFeed::~BlogRSSFeed()
{
  delete session_;
}

void BlogRSSFeed::handleRequest(const Wt::Http::Request &request,
				Wt::Http::Response &response)
{
  response.setMimeType("text/xml");

  std::string url = url_;

  if (url.empty()) {
    url = request.urlScheme() + "://" + request.serverName();
    if (!request.serverPort().empty() && request.serverPort() != "80")
      url += ":" + request.serverPort();
    url += request.path();

    // remove '/feed/'
    url.erase(url.length() - 6);
  }

  response.out() <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<rss version=\"2.0\">\n"
    "  <channel>\n"
    "    <title>" << title_ << "</title>\n"
    "    <link>" << url << "</link>\n"
    "    <description>" << description_ << "</description>\n";

  dbo::Transaction t(*session_);

  Posts posts = session_->find<Post>
    ("where state = ? "
     "order by date desc "
     "limit 10").bind(Post::Published);

  for (Posts::const_iterator i = posts.begin(); i != posts.end(); ++i) {
    dbo::ptr<Post> post = *i;

    std::string permaLink = url + "/" + post->permaLink();

    response.out() <<
      "    <item>\n"
      "      <title>" << post->title.toUTF8() << "</title>\n"
      "      <pubDate>" << post->date.toString("ddd, d MMM yyyy hh:mm:ss UTC")
		   << "</pubDate>\n"
      "      <guid isPermaLink=\"true\">" << permaLink << "</guid>\n";

    std::string description = post->briefHtml.toUTF8();
    if (!post->bodySrc.empty())
      description +=
	"<p><a href=\"" + permaLink + "\">Read the rest...</a></p>";

    response.out() << 
      "      <description><![CDATA[" << description << "]]></description>\n"
      "    </item>\n";
  }

  response.out() <<
    "  </channel>\n"
    "</rss>\n";

  t.commit();
}
