/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WVideo>
#include <Wt/WImage>
#include <Wt/WFlashObject>

using namespace Wt;

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  WApplication* app = new WApplication(env);

  app->messageResourceBundle().use(WApplication::appRoot() + "text");

  std::string ogvVideo =
    "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";
  std::string mp4Video =
    "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
  std::string poster = "sintel_trailer.jpg";
  
  new WText(WString::tr("intro"), app->root());

  new WText(WString::tr("html5"), app->root());
  WVideo *v1 = new WVideo(app->root());
  v1->addSource(mp4Video);
  v1->addSource(ogvVideo);
  v1->setPoster(poster);
  v1->setAlternativeContent(new WImage(poster));
  v1->resize(640, 360);
  
  new WText(WString::tr("flash-fallback"), app->root());
  WFlashObject *flash2 =
    new WFlashObject("http://www.webtoolkit.eu/videos/player_flv_maxi.swf");
  flash2->setFlashVariable("startimage", "sintel_trailer.jpg");
  flash2->setFlashParameter("allowFullScreen", "true");
  flash2->setFlashVariable("flv", mp4Video);
  flash2->setFlashVariable("showvolume", "1");
  flash2->setFlashVariable("showfullscreen", "1");
  flash2->setAlternativeContent(new WImage(poster));
  flash2->resize(640, 360);
  WVideo *v2 = new WVideo(app->root());
  v2->addSource(mp4Video);
  v2->addSource(ogvVideo);
  v2->setAlternativeContent(flash2);
  v2->setPoster(poster);
  v2->resize(640, 360);

  return app;
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}

