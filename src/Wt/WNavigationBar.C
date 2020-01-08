/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WMenu.h"
#include "Wt/WNavigationBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WTheme.h"

#include <exception>

namespace Wt {

LOGGER("WNavigationBar");

class NavContainer final : public WContainerWidget
{
public:
  NavContainer()
  { }

  bool isBootstrap2Responsive() const {
    return styleClass().toUTF8().find("nav-collapse") != std::string::npos;
  }

  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override
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

WNavigationBar::WNavigationBar()
  : WTemplate(tr("Wt.WNavigationBar.template"))
{
  bindEmpty("collapse-button");
  bindEmpty("expand-button");
  bindEmpty("title-link");
  bindWidget("contents", std::unique_ptr<WWidget>(new NavContainer()));

  implementStateless(&WNavigationBar::collapseContents,
                     &WNavigationBar::undoExpandContents);

  implementStateless(&WNavigationBar::expandContents,
                     &WNavigationBar::undoExpandContents);
}

void WNavigationBar::setTitle(const WString& title, const WLink& link)
{
  WAnchor *titleLink = resolve<WAnchor *>("title-link");

  if (!titleLink) {
    titleLink = bindWidget("title-link", cpp14::make_unique<WAnchor>());
    wApp->theme()->apply(this, titleLink, NavBrand);
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
      auto b = createCollapseButton();
      collapseButton = b.get();
      bindWidget("collapse-button", std::move(b));
      collapseButton->clicked().connect(this,
                                        &WNavigationBar::collapseContents);

      collapseButton->hide();

      b = createExpandButton();
      expandButton = b.get();
      bindWidget("expand-button", std::move(b));
      expandButton->clicked().connect(this,
                                      &WNavigationBar::expandContents);
    }

    wApp->theme()->apply(this, contents, NavCollapse);

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

WMenu *WNavigationBar::addMenu(std::unique_ptr<WMenu> menu,
                             AlignmentFlag alignment)
{
  WMenu *m = menu.get();
  addWidget(std::move(menu), alignment);
  wApp->theme()->apply(this, m, NavbarMenu);
  return m;
}

void WNavigationBar::addFormField(std::unique_ptr<WWidget> widget,
                                  AlignmentFlag alignment)
{
  addWidget(std::move(widget), alignment);
}

void WNavigationBar::addWidget(std::unique_ptr<WWidget> widget,
                               AlignmentFlag alignment)
{
  if (dynamic_cast<WMenu *>(widget.get())) {
    align(widget.get(), alignment);
    WContainerWidget *contents = resolve<WContainerWidget *>("contents");
    contents->addWidget(std::move(widget));
    contents->setLoadLaterWhenInvisible(false);
  } else
    addWrapped(std::move(widget), alignment, "navbar-form");
}

void WNavigationBar::addWrapped(std::unique_ptr<WWidget> widget,
				AlignmentFlag alignment,
				const char *wrapClass)
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");

  WContainerWidget *wrap
    = contents->addWidget(cpp14::make_unique<WContainerWidget>());
  wrap->setStyleClass(wrapClass);
  align(wrap, alignment);
  wrap->addWidget(std::move(widget));
}

void WNavigationBar::addWrapped(std::unique_ptr<WWidget> widget,
				WWidget* parent, int role,
				AlignmentFlag alignment)
{
  WContainerWidget *contents = resolve<WContainerWidget *>("contents");

  WContainerWidget *wrap
    = contents->addWidget(cpp14::make_unique<WContainerWidget>());
  wApp->theme()->apply(widget.get(), parent, role);
  align(wrap, alignment);
  wrap->addWidget(std::move(widget));
}

void WNavigationBar::addSearch(std::unique_ptr<WLineEdit> field,
                               AlignmentFlag alignment)
{
  wApp->theme()->apply(this, field.get(), NavbarSearch);
  addWrapped(std::move(field), alignment, "navbar-form");
}

void WNavigationBar::align(WWidget *widget, AlignmentFlag alignment)
{
  switch (alignment) {
  case AlignmentFlag::Left:
    wApp->theme()->apply(this, widget, NavbarAlignLeft);
    break;
  case AlignmentFlag::Right:
    wApp->theme()->apply(this, widget, NavbarAlignRight);
    break;
  default:
    LOG_ERROR("addWidget(...): unsupported alignment "
              << static_cast<unsigned int>(alignment));
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
#ifndef WT_TARGET_JAVA
    if (canOptimizeUpdates())
      contents->show(); /* We are collapsed only in appearance */
    else
#endif // WT_TARGET_JAVA
      contents->animateHide
        (WAnimation(AnimationEffect::SlideInFromTop,
                    TimingFunction::Ease));
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
#ifndef WT_TARGET_JAVA
    if (canOptimizeUpdates())
      contents->show();
    else
#endif // WT_TARGET_JAVA
      contents->animateShow
        (WAnimation(AnimationEffect::SlideInFromTop,
                    TimingFunction::Ease));
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

std::unique_ptr<WInteractWidget> WNavigationBar::createExpandButton()
{
  std::unique_ptr<WPushButton> result
    (new WPushButton(tr("Wt.WNavigationBar.expand-button")));
  result->setTextFormat(TextFormat::XHTML);
  wApp->theme()->apply(this, result.get(), NavbarBtn);
  return std::move(result);
}

std::unique_ptr<WInteractWidget> WNavigationBar::createCollapseButton()
{
  return createExpandButton();
}

bool WNavigationBar::animatedResponsive() const
{
  return WApplication::instance()->environment().supportsCss3Animations() &&
    WApplication::instance()->environment().ajax();
}

}
