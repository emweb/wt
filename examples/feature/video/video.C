/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WText.h>
#include <Wt/WVideo.h>
#include <Wt/WImage.h>
#include <Wt/WFlashObject.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<WApplication>(env);

  app->messageResourceBundle().use(WApplication::appRoot() + "text");

  std::string ogvVideo =
    "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";
  std::string mp4Video =
    "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
  std::string poster = "sintel_trailer.jpg";

  app->root()->addWidget(std::make_unique<WText>(WString::tr("intro")));

  app->root()->addWidget(std::make_unique<WText>(WString::tr("html5")));
  WVideo *v1 = app->root()->addWidget(std::make_unique<WVideo>());
  v1->addSource(mp4Video);
  v1->addSource(ogvVideo);
  v1->setPoster(poster);
  v1->setAlternativeContent(std::make_unique<WImage>(poster));
  v1->resize(640, 360);

  app->root()->addWidget(std::make_unique<WText>(WString::tr("flash-fallback")));
  std::unique_ptr<WFlashObject> flash2
      = std::make_unique<WFlashObject>("http://www.webtoolkit.eu/videos/player_flv_maxi.swf");
  flash2->setFlashVariable("startimage", "sintel_trailer.jpg");
  flash2->setFlashParameter("allowFullScreen", "true");
  flash2->setFlashVariable("flv", mp4Video);
  flash2->setFlashVariable("showvolume", "1");
  flash2->setFlashVariable("showfullscreen", "1");
  flash2->setAlternativeContent(std::make_unique<WImage>(poster));
  flash2->resize(640, 360);
  WVideo *v2 =
      app->root()->addWidget(std::make_unique<WVideo>());
  v2->addSource(mp4Video);
  v2->addSource(ogvVideo);
  v2->setAlternativeContent(std::move(flash2));
  v2->setPoster(poster);
  v2->resize(640, 360);

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

