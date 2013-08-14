/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLineEdit"
#include "Wt/WMenu"
#include "Wt/WNavigationBar"
#include "Wt/WPushButton"

namespace Wt {

LOGGER("WNavigationBar");

class NavContainer : public WContainerWidget
{
public:
  NavContainer(WContainerWidget *parent = 0)
    : WContainerWidget(parent) 
  { }

  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation())
  {
    if (animation.empty()) {
      /* Comply with bootstrap responsive CSS assumptions */
      /* When animations are used, this is actually done in wtAnimatedHidden */
      if (hidden)
	setHeight(0);
      else
	setHeight(WLength::Auto);
    }

    WContainerWidget::setHidden(hidden, animation);
  }
};

WNavigationBar::WNavigationBar(WContainerWidget *parent)
  : WTemplate(tr("Wt.WNavigationBar.template"), parent)
{
  setStyleClass("navbar");

  bindEmpty("collapse-button");
  bindEmpty("expand-button");
  bindEmpty("title-link");
  bindWidget("contents", new NavContainer());

  implementStateless(&WNavigationBar::collapseContents,
		     &WNavigationBar::undoExpandContents);

  implementStateless(&WNavigationBar::expandContents,
		     &WNavigationBar::undoExpandContents);
}

void WNavigationBar::setTitle(const WString& title, const WLink& link)
{
  WAnchor *titleLink = resolve<WAnchor *>("title-link");

  if (!titleLink) {
    bindWidget("title-link", titleLink = new WAnchor());
    titleLink->addStyleClass("brand");
  }
  
  titleLink->setText(title);
  titleLink->setLink(link);
}

void WNavigationBar::setResponsive(bool responsive)
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");

  if (responsive) {
    WInteractWidget *collapseButton
      = resolve<WInteractWidget *>("collapse-button");
    WInteractWidget *expandButton
      = resolve<WInteractWidget *>("expand-button");

    if (!collapseButton) {
      bindWidget("collapse-button", collapseButton = createCollapseButton());
      collapseButton->clicked().connect(this,
					&WNavigationBar::collapseContents);

      collapseButton->hide();

      bindWidget("expand-button", expandButton = createExpandButton());
      expandButton->clicked().connect(this,
				      &WNavigationBar::expandContents);
    }

    contents->addStyleClass("nav-collapse");
    contents->hide();

    /* Comply with bootstrap responsive CSS assumptions */
    contents->setJavaScriptMember
      ("wtAnimatedHidden",
       "function(hidden) {"
       """if (hidden) "
       ""  "this.style.height=''; this.style.display='';"
       "}");
  } else {
    bindEmpty("collapse-button");
    contents->removeStyleClass("nav-collapse");
  }
}

void WNavigationBar::addMenu(WMenu *menu, AlignmentFlag alignment)
{
  addWidget((WWidget *)menu, alignment);
}

void WNavigationBar::addFormField(WWidget *widget, AlignmentFlag alignment)
{
  addWidget(widget, alignment);
}

void WNavigationBar::addWidget(WWidget *widget, AlignmentFlag alignment)
{
  if (dynamic_cast<WMenu *>(widget)) {
    align(widget, alignment);

    WContainerWidget *contents = resolve<WContainerWidget *>("contents");
    contents->addWidget(widget);
    contents->setLoadLaterWhenInvisible(false);
  } else
    addWrapped(widget, alignment, "navbar-form");
}

void WNavigationBar::addWrapped(WWidget *widget, AlignmentFlag alignment,
				const char *wrapClass)
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");

  WContainerWidget *wrap = new WContainerWidget(contents);
  wrap->setStyleClass(wrapClass);
  align(wrap, alignment);
  wrap->addWidget(widget);
}

void WNavigationBar::addSearch(WLineEdit *field, AlignmentFlag alignment)
{
  field->addStyleClass("search-query");

  addWrapped(field, alignment, "navbar-search");
}

void WNavigationBar::align(WWidget *widget, AlignmentFlag alignment)
{
  switch (alignment) {
  case AlignLeft:
    widget->addStyleClass("pull-left");
    break;
  case AlignRight:
    widget->addStyleClass("pull-right");
    break;
  default:
    LOG_ERROR("addWidget(...): unsupported alignment " << alignment);
  }
}

void WNavigationBar::collapseContents()
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");
  WInteractWidget *collapseButton
    = resolve<WInteractWidget *>("collapse-button");
  WInteractWidget *expandButton
    = resolve<WInteractWidget *>("expand-button");

  collapseButton->hide();
  expandButton->show();

  if (!animatedResponsive())
    contents->hide();
  else {
    if (canOptimizeUpdates())
      contents->show(); /* We are collapsed only in appearance */
    else
      contents->animateHide(WAnimation(WAnimation::SlideInFromTop,
				       WAnimation::Ease));
  }
}

void WNavigationBar::expandContents()
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");
  WInteractWidget *collapseButton
    = resolve<WInteractWidget *>("collapse-button");
  WInteractWidget *expandButton
    = resolve<WInteractWidget *>("expand-button");

  collapseButton->show();
  expandButton->hide();

  if (!animatedResponsive())
    contents->show();
  else {
    if (canOptimizeUpdates())
      contents->show();
    else
      contents->animateShow(WAnimation(WAnimation::SlideInFromTop,
				       WAnimation::Ease));
  }
}

void WNavigationBar::undoExpandContents()
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");
  WInteractWidget *collapseButton
    = resolve<WInteractWidget *>("collapse-button");
  WInteractWidget *expandButton
    = resolve<WInteractWidget *>("expand-button");

  collapseButton->hide();
  expandButton->show();

  if (!animatedResponsive())
    contents->hide();
  else
    contents->show();  /* We are collapsed only in appearance */
}

WInteractWidget *WNavigationBar::createExpandButton()
{
  WPushButton *result = new WPushButton(tr("Wt.WNavigationBar.expand-button"));
  result->setTextFormat(XHTMLText);
  result->setStyleClass("btn-navbar");
  return result;
}

WInteractWidget *WNavigationBar::createCollapseButton()
{
  return createExpandButton();
}

bool WNavigationBar::animatedResponsive() const
{
  return WApplication::instance()->environment().supportsCss3Animations() &&
    WApplication::instance()->environment().ajax();
}

}
