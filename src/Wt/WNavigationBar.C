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
#include "Wt/WTheme"

#include <exception>

namespace Wt {

LOGGER("WNavigationBar");

class NavContainer : public WContainerWidget
{
public:
  NavContainer(WContainerWidget *parent = 0)
    : WContainerWidget(parent) 
  { }

  bool isBootstrap2Responsive() const {
    return styleClass().toUTF8().find("nav-collapse") != std::string::npos;
  }

  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation())
  {
    if (isBootstrap2Responsive()) {
      if (animation.empty()) {
	/* Comply with bootstrap responsive CSS assumptions */
	/* When animations are used, this is actually done in wtAnimatedHidden */
	if (hidden)
	  setHeight(0);
	else
	  setHeight(WLength::Auto);
      }
    }

    WContainerWidget::setHidden(hidden, animation);
  }
};

WNavigationBar::WNavigationBar(WContainerWidget *parent)
  : WTemplate(tr("Wt.WNavigationBar.template"), parent)
{
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
    wApp->theme()->apply(this, titleLink, NavBrandRole);
  }
  
  titleLink->setText(title);
  titleLink->setLink(link);
}

void WNavigationBar::setResponsive(bool responsive)
{
  NavContainer *contents = resolve<NavContainer *>("contents");

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

    wApp->theme()->apply(this, contents, NavCollapseRole);

    contents->hide();

    if (contents->isBootstrap2Responsive()) {
      /* Comply with bootstrap responsive CSS assumptions */
      contents->setJavaScriptMember
	("wtAnimatedHidden",
	 "function(hidden) {"
	 """if (hidden) "
	 ""  "this.style.height=''; this.style.display='';"
	 "}");
    }
  } else {
    bindEmpty("collapse-button");
  }
}

void WNavigationBar::addMenu(WMenu *menu, AlignmentFlag alignment)
{
  addWidget((WWidget *)menu, alignment);  
  wApp->theme()->apply(this, menu, NavbarMenuRole);
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

void WNavigationBar::addWrapped(WWidget *widget, WWidget* parent, int role,
                AlignmentFlag alignment)
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");

  WContainerWidget *wrap = new WContainerWidget(contents);
  wApp->theme()->apply(widget, parent, role);
  align(wrap, alignment);
  wrap->addWidget(widget);
}

void WNavigationBar::addSearch(WLineEdit *field, AlignmentFlag alignment)
{
  wApp->theme()->apply(this, field, NavbarSearchRole);
  addWrapped(field, alignment, "navbar-form");
}

void WNavigationBar::align(WWidget *widget, AlignmentFlag alignment)
{
  switch (alignment) {
  case AlignLeft:
    wApp->theme()->apply(this, widget, NavbarAlignLeftRole);
    break;
  case AlignRight:
    wApp->theme()->apply(this, widget, NavbarAlignRightRole);
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
  wApp->theme()->apply(this, result, NavbarBtn);
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
