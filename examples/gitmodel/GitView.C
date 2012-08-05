/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <stdlib.h>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WLineEdit>
#include <Wt/WGridLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WText>
#include <Wt/WTreeView>
#include <Wt/WVBoxLayout>
#include <Wt/WViewWidget>

#include "GitModel.h"
#include "../wt-homepage/SourceView.h"

using namespace Wt;

/**
 * \defgroup gitmodelexample Git model example
 */
/*@{*/

/*! \class GitViewApplication
 *  \brief A simple application to navigate a git repository.
 *
 * This examples demonstrates how to use the custom model use GitModel
 * with a WTreeView.
 */
class GitViewApplication : public WApplication
{
public:
  /*! \brief Constructor.
   */
  GitViewApplication(const WEnvironment& env) 
    : WApplication(env)
  {
    useStyleSheet("gitview.css");
    setTitle("Git model example");

    const char *gitRepo = getenv("GITVIEW_REPOSITORY_PATH");

    WGridLayout *grid = new WGridLayout();
    grid->addWidget(new WText("Git repository path:"), 0, 0);
    grid->addWidget(repositoryEdit_ = new WLineEdit(gitRepo ? gitRepo : "")
		    , 0, 1, AlignLeft);
    grid->addWidget(repositoryError_ = new WText(), 0, 2);
    grid->addWidget(new WText("Revision:"), 1, 0);
    grid->addWidget(revisionEdit_ = new WLineEdit("master"), 1, 1, AlignLeft);
    grid->addWidget(revisionError_ = new WText(), 1, 2);

    repositoryEdit_->setTextSize(30);
    revisionEdit_->setTextSize(20);
    repositoryError_->setStyleClass("error-msg");
    revisionError_->setStyleClass("error-msg");

    repositoryEdit_->enterPressed()
      .connect(this, &GitViewApplication::loadGitModel);
    revisionEdit_->enterPressed()
      .connect(this, &GitViewApplication::loadGitModel);

    WPushButton *b = new WPushButton("Load");
    b->clicked().connect(this, &GitViewApplication::loadGitModel);
    grid->addWidget(b, 2, 0, AlignLeft);

    gitView_ = new WTreeView();
    gitView_->resize(300, WLength::Auto);
    gitView_->setSortingEnabled(false);
    gitView_->setModel(gitModel_ = new GitModel(this));
    gitView_->setSelectionMode(SingleSelection);
    gitView_->selectionChanged().connect(this, &GitViewApplication::showFile);

    sourceView_ = new SourceView(DisplayRole, 
				 GitModel::ContentsRole, 
				 GitModel::FilePathRole);
    sourceView_->setStyleClass("source-view");

    if (environment().javaScript()) {
      /*
       * We have JavaScript: We can use layout managers so everything will
       * always fit nicely in the window.
       */
      WVBoxLayout *topLayout = new WVBoxLayout();
      topLayout->addLayout(grid, 0);

      WHBoxLayout *gitLayout = new WHBoxLayout();
      gitLayout->addWidget(gitView_, 0);
      gitLayout->addWidget(sourceView_, 1);
      topLayout->addLayout(gitLayout, 1);

      root()->setLayout(topLayout);
      root()->setStyleClass("maindiv");
    } else {
      /*
       * No JavaScript: let's make the best of the situation using regular
       * CSS-based layout
       */
      root()->setStyleClass("maindiv");
      WContainerWidget *top = new WContainerWidget();
      top->setLayout(grid);
      root()->addWidget(top);
      root()->addWidget(gitView_);
      gitView_->setFloatSide(Left);
      gitView_->setMargin(6);
      root()->addWidget(sourceView_);
      sourceView_->setMargin(6);
    }
  }

private:
  WLineEdit  *repositoryEdit_, *revisionEdit_;
  WText      *repositoryError_, *revisionError_;
  GitModel   *gitModel_;
  WTreeView  *gitView_;
  SourceView *sourceView_;

  /*! \brief Change repository and/or revision
   */
  void loadGitModel() {
    sourceView_->setIndex(WModelIndex());
    repositoryError_->setText("");
    revisionError_->setText("");
    try {
      gitModel_->setRepositoryPath(repositoryEdit_->text().toUTF8());
      try {
	gitModel_->loadRevision(revisionEdit_->text().toUTF8());
      } catch (const Git::Exception& e) {
	revisionError_->setText(e.what());
      }
    } catch (const Git::Exception& e) {
      repositoryError_->setText(e.what());
    }
  }

  /*! \brief Displayed the currently selected file.
   */
  void showFile() {
    if (gitView_->selectedIndexes().empty())
      return;

    WModelIndex selected = *gitView_->selectedIndexes().begin();
    sourceView_->setIndex(selected);
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  return new GitViewApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

/*@}*/
