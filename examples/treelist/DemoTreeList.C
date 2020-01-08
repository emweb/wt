/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WImage.h>
#include <Wt/WPushButton.h>

#include "DemoTreeList.h"
#include "TreeNode.h"
#include "IconPair.h"

using namespace Wt;
using std::rand;

DemoTreeList::DemoTreeList()
  : WContainerWidget(),
    testCount_(0)
{
  addWidget
    (cpp14::make_unique<WText>("<h2>Wt Tree List example</h2>"
	       "<p>This is a simple demo of a treelist, implemented using"
	       " <a href='http://witty.sourceforge.net/'>Wt</a>.</p>"
	       "<p>The leafs of the tree contain the source code of the "
	       "tree-list in the classes <b>TreeNode</b> and "
	       "<b>IconPair</b>, as well as the implementation of this "
	       "demo itself in the class <b>DemoTreeList</b>.</p>"));

  auto tree = makeTreeFolder("Examples");
  tree_ = addWidget(std::move(tree));

  TreeNode *treelist = makeTreeFolder("Tree List", tree_);
  TreeNode *wstateicon = makeTreeFolder("class IconPair", treelist);
  makeTreeFile("<a href=\"IconPair.h\">IconPair.h</a>", wstateicon);
  makeTreeFile("<a href=\"IconPair.C\">IconPair.C</a>", wstateicon);
  TreeNode *wtreenode = makeTreeFolder("class TreeNode", treelist);
  makeTreeFile("<a href=\"TreeNode.h\">TreeNode.h</a>", wtreenode);
  makeTreeFile("<a href=\"TreeNode.C\">TreeNode.C</a>", wtreenode);
  TreeNode *demotreelist = makeTreeFolder("class DemoTreeList", treelist);
  makeTreeFile("<a href=\"DemoTreeList.h\">DemoTreeList.h</a>", demotreelist);
  makeTreeFile("<a href=\"DemoTreeList.C\">DemoTreeList.C</a>", demotreelist);

  testFolder_ = makeTreeFolder("Test folder", tree_);

  /*
   * Buttons to dynamically demonstrate changing the tree contents.
   */
  addWidget
    (cpp14::make_unique<WText>("<p>Use the following buttons to change the tree "
	       "contents:</p>"));

  addFolderButton_
      = this->addWidget(cpp14::make_unique<WPushButton>("Add folder"));
  addFolderButton_->clicked().connect(this, &DemoTreeList::addFolder);

  removeFolderButton_
    = this->addWidget(cpp14::make_unique<WPushButton>("Remove folder"));
  removeFolderButton_->clicked().connect(this, &DemoTreeList::removeFolder);
  removeFolderButton_->disable();

  addWidget
    (cpp14::make_unique<WText>("<p>Remarks:"
	       "<ul>"
	       "<li><p>This is not the instantiation of a pre-defined "
	       "tree list component, but the full implementation of such "
	       "a component, in about 350 lines of C++ code !</p> "
	       "<p>In comparison, the <a href='http://myfaces.apache.org'> "
	       "Apache MyFaces</a> JSF implementation of tree2, with similar "
	       "functionality, uses about 2400 lines of Java, and 140 lines "
	       "of JavaScript code.</p></li>"
	       "<li><p>Once loaded, the tree list does not require any "
	       "interaction with the server for handling the click events on "
	       "the <img src='icons/nav-plus-line-middle.gif' /> and "
	       "<img src='icons/nav-minus-line-middle.gif' /> icons, "
	       "because these events have been connected to slots using "
	       "STATIC connections. Such connections are converted to the "
	       "appropriate JavaScript code that is inserted into the page. "
	       "Still, the events are signaled to the server to update the "
	       "application state.</p></li>"
	       "<li><p>In contrast, the buttons for manipulating the tree "
	       "contents use DYNAMIC connections, and thus the update "
	       "is computed at server-side, and communicated back to the "
	       "browser (by default using AJAX).</p></li>"
	       "<li><p>When loading a page, only visible widgets (that are not "
	       "<b>setHidden(true)</b>) are transmitted. "
	       "The remaining widgets are loaded in the background after "
	       "rendering the page. "
	       "As a result the application is loaded as fast as possible.</p>"
	       "</li>"
	       "<li><p>The browser reload button is supported and behaves as "
	       "expected: the page is reloaded from the server. Again, "
	       "only visible widgets are transmitted immediately.</p> "
	       "<p>(For the curious, this is the way to see the actual "
	       "HTML/JavaScript code !)</p></li>"
	       "</ul></p>"));
}

void DemoTreeList::addFolder()
{
  TreeNode *node
    = makeTreeFolder("Folder " + std::to_string(++testCount_), testFolder_);
  makeTreeFile("File " + std::to_string(testCount_), node);

  removeFolderButton_->enable();
}

void DemoTreeList::removeFolder()
{
  int numFolders = testFolder_->childNodes().size();

  if (numFolders > 0) {
    int c = rand() % numFolders;

    TreeNode *child = testFolder_->childNodes()[c];
    testFolder_->removeChildNode(child, c);

    if (numFolders == 1)
      removeFolderButton_->disable();
  }
}

TreeNode *DemoTreeList::makeTreeFolder(const std::string name, TreeNode *parent)
{
  auto labelIcon = cpp14::make_unique<IconPair>(
                   "icons/yellow-folder-closed.png",
		   "icons/yellow-folder-open.png",
		   false);

  auto node =
      cpp14::make_unique<TreeNode>(name, TextFormat::Plain, std::move(labelIcon));
  auto node_ = node.get();
  parent->addChildNode(std::move(node));

  return node_;
}

std::unique_ptr<TreeNode> DemoTreeList::makeTreeFolder(const std::string name)
{
  auto labelIcon = cpp14::make_unique<IconPair>(
                   "icons/yellow-folder-closed.png",
                   "icons/yellow-folder-open.png",
                   false);
  auto node =
      cpp14::make_unique<TreeNode>(name, TextFormat::Plain, std::move(labelIcon));

  return node;
}

TreeNode *DemoTreeList::makeTreeFile(const std::string name,
				      TreeNode *parent)
{
  auto labelIcon
    = cpp14::make_unique<IconPair>("icons/document.png", "icons/yellow-folder-open.png",
		   false);

  auto node = cpp14::make_unique<TreeNode>(name, TextFormat::XHTML, std::move(labelIcon));
  auto node_ = node.get();
  if (parent)
    parent->addChildNode(std::move(node));

  return node_;
}

std::unique_ptr<TreeNode> DemoTreeList::makeTreeFile(const std::string name)
{
  auto labelIcon
    = cpp14::make_unique<IconPair>("icons/document.png", "icons/yellow-folder-open.png",
                   false);
  auto node =
      cpp14::make_unique<TreeNode>(name, TextFormat::XHTML, std::move(labelIcon));

  return node;
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app
      = cpp14::make_unique<WApplication>(env);
  app->root()->addWidget(cpp14::make_unique<DemoTreeList>());

  /*
   * The look & feel of the tree node is configured using a CSS style sheet.
   * If you are not familiar with CSS, you can use the WCssDecorationStyle
   * class ...
   */
  WCssDecorationStyle treeNodeLabelStyle;
  treeNodeLabelStyle.font().setFamily(FontFamily::Serif, "Helvetica");
  app->styleSheet().addRule(".treenodelabel", treeNodeLabelStyle);

  /*
   * ... or if you speak CSS fluently, you can add verbatim rules.
   */
  app->styleSheet().addRule(".treenodechildcount",
			    "color:blue; font-family:Helvetica,serif;");

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

