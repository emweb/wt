#include <Wt/WContainerWidget.h>
#include <Wt/WMenuItem.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(Tab)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WTabWidget *tabW = container->addNew<Wt::WTabWidget>();
tabW->addTab(std::make_unique<Wt::WTextArea>("This is the contents of the first tab."),
             "First", Wt::ContentLoading::Eager);
tabW->addTab(std::make_unique<Wt::WTextArea>("The contents of the tabs are pre-loaded in"
			       " the browser to ensure swift switching."),
             "Preload", Wt::ContentLoading::Eager);
tabW->addTab(std::make_unique<Wt::WTextArea>("You could change any other style attribute of the"
			       " tab widget by modifying the style class."
			       " The style class 'trhead' is applied to this tab."),
             "Style", Wt::ContentLoading::Eager)->setStyleClass("trhead");

Wt::WMenuItem *tab
    = tabW->addTab(std::make_unique<Wt::WTextArea>("You can close this tab"
				     " by clicking on the close icon."),
		   "Close");
tab->setCloseable(true);

tabW->setStyleClass("tabwidget");

SAMPLE_END(return std::move(container))
