/*
 * Copyright (C) 2006 Wim Dumon, Koen Deforche
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

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

using namespace Wt;

/**
 * \defgroup gitmodelexample Git model example
 */
/*@{*/

/*! \class SourceView
 *  \brief View class for source code.
 *
 * A view class is used so that no server-side memory is used while displaying
 * a potentially large file.
 */
class SourceView : public WViewWidget
{
public:
  /*! \brief Constructor.
   *
   * The <i>role</i> will be used to retrieve data from a model index to
   * be displayed.
   */
  SourceView(int role)
    : role_(role)
  { }

  /*! \brief Set model index.
   *
   * The view is rerendered if the index contains new data.
   */
  void setIndex(const WModelIndex& index) {
    if (index != index_
	&& (!index.isValid() || !index.data(role_).empty())) {
      index_ = index;
      update();
    }
  }

  /*! \brief Return the widget that renders the view.
   *
   * Returns he view contents: renders the file to a WText widget.
   * WViewWidget deletes this widget after every rendering step.
   */
  virtual WWidget *renderView() {
    WText *result = new WText();
    result->setInline(false);

    if (!index_.isValid())
      return result;

    boost::any d = index_.data(role_);
    const std::string& t = boost::any_cast<const std::string&>(d);

    result->setTextFormat(PlainText);
    result->setText(t);

    return result;
  }

private:
  /// The index that is currently displayed.
  WModelIndex index_;

  /// The role that is currently displayed.
  int         role_;
};

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

    repositoryEdit_->enterPressed
      .connect(SLOT(this, GitViewApplication::loadGitModel));
    revisionEdit_->enterPressed
      .connect(SLOT(this, GitViewApplication::loadGitModel));

    WPushButton *b = new WPushButton("Load");
    b->clicked.connect(SLOT(this, GitViewApplication::loadGitModel));
    grid->addWidget(b, 2, 0, AlignLeft);

    gitView_ = new WTreeView();
    gitView_->resize(300, WLength());
    gitView_->setSortingEnabled(false);
    gitView_->setModel(gitModel_ = new GitModel(this));
    gitView_->setSelectionMode(SingleSelection);
    gitView_->selectionChanged.connect
      (SLOT(this, GitViewApplication::showFile));

    sourceView_ = new SourceView(GitModel::ContentsRole);
    sourceView_->setStyleClass("source-view");

    if (environment().javaScript()) {
      /*
       * We have JavaScript: We can use layout managers so everything will
       * always fit nicely in the window.
       */
      WVBoxLayout *topLayout = new WVBoxLayout();
      topLayout->addLayout(grid, 0, AlignTop | AlignLeft);

      WHBoxLayout *gitLayout = new WHBoxLayout();
      gitLayout->setLayoutHint("table-layout", "fixed");
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
      top->setLayout(grid, AlignTop | AlignLeft);
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
