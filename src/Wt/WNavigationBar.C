/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WBootstrap5Theme.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WMenu.h"
#include "Wt/WNavigationBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WTheme.h"

#include <exception>
#include <memory>

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

  auto app = WApplication::instance();
  auto bs5Theme = std::dynamic_pointer_cast<WBootstrap5Theme>(app->theme());
  if (bs5Theme) {
    setResponsive(true);
  }
}

void WNavigationBar::setTitle(const WString& title, const WLink& link)
{
  WAnchor *titleLink = resolve<WAnchor *>("title-link");

  if (!titleLink) {
    titleLink = bindWidget("title-link", std::make_unique<WAnchor>());
    auto app = WApplication::instance();
    app->theme()->apply(this, titleLink, NavBrand);
  }

  titleLink->setText(title);
  titleLink->setLink(link);
}

void WNavigationBar::setResponsive(bool responsive)
{
  NavContainer *contents = resolve<NavContainer *>("contents");

  auto app = WApplication::instance();
  auto bs5Theme = std::dynamic_pointer_cast<WBootstrap5Theme>(app->theme());
  if (bs5Theme) {
    // We ignore responsive set to false, since we don't support that with Bootstrap 5,
    // and we only want to do the changes below once
    if (!responsive || resolve<WInteractWidget*>("collapse-button")) {
      return;
    }
  }

  if (bs5Theme) {
    auto collapseButtonPtr = createCollapseButton();
    auto collapseButton = collapseButtonPtr.get();
    bindWidget("collapse-button", std::move(collapseButtonPtr));

    collapseButton->clicked().connect(
      "function(o){"
      "" "let navbarCollapse = o.parentElement.querySelector('.navbar-collapse');"
      "" "if (typeof navbarCollapse === 'null') return;"
      "" "new bootstrap.Collapse(navbarCollapse);"
      "}");

    if (!app->environment().ajax()) {
      collapseButton->clicked().connect(this, &WNavigationBar::toggleContents);
    }

    wApp->theme()->apply(this, contents, NavCollapse);
  } else if (responsive) {
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
  auto app = WApplication::instance();
  app->theme()->apply(this, m, NavbarMenu);
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
    auto contents = resolve<WContainerWidget *>("contents");
    contents->addWidget(std::move(widget));
    contents->setLoadLaterWhenInvisible(false);
  } else {
    addWrapped(std::move(widget), alignment, NavbarForm);
  }
}

void WNavigationBar::addWrapped(std::unique_ptr<WWidget> widget,
                                AlignmentFlag alignment,
                                int role)
{
  auto contents = resolve<WContainerWidget *>("contents");
  auto wrap = contents->addNew<WContainerWidget>();

  auto app = WApplication::instance();
  app->theme()->apply(this, wrap, role);
  align(wrap, alignment);
  wrap->addWidget(std::move(widget));
}

void WNavigationBar::addSearch(std::unique_ptr<WLineEdit> field,
                               AlignmentFlag alignment)
{
  auto app = WApplication::instance();
  app->theme()->apply(this, field.get(), NavbarSearchInput);
  addWrapped(std::move(field), alignment, NavbarSearchForm);
}

void WNavigationBar::align(WWidget *widget, AlignmentFlag alignment)
{
  auto app = WApplication::instance();
  switch (alignment) {
  case AlignmentFlag::Left:
    app->theme()->apply(this, widget, NavbarAlignLeft);
    break;
  case AlignmentFlag::Right:
    app->theme()->apply(this, widget, NavbarAlignRight);
    break;
  default:
    LOG_ERROR("addWidget(...): unsupported alignment "
              << static_cast<unsigned int>(alignment));
  }
}

void WNavigationBar::toggleContents()
{
  auto app = Wt::WApplication::instance();
  if (app->environment().ajax()) {
    // We don't need to do these updates server side if we have JS support
    return;
  }

  auto contents = resolve<WContainerWidget *>("contents");
  auto collapseButton = resolve<WInteractWidget*>("collapse-button");

  if (contents->hasStyleClass("show")) {
    contents->removeStyleClass("show");
    collapseButton->addStyleClass("collapsed");
  } else {
    contents->addStyleClass("show");
    collapseButton->removeStyleClass("collapsed");
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
   std::unique_ptr<WPushButton> result(new WPushButton(tr("Wt.WNavigationBar.expand-button")));
   result->setTextFormat(TextFormat::XHTML);
   auto app = Wt::WApplication::instance();
   app->theme()->apply(this, result.get(), NavbarBtn);
   return result;
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
