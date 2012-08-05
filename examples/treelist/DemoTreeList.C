/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WPushButton>

#include "DemoTreeList.h"
#include "TreeNode.h"
#include "IconPair.h"

using namespace Wt;
using std::rand;

DemoTreeList::DemoTreeList(WContainerWidget *parent)
  : WContainerWidget(parent),
    testCount_(0)
{
  addWidget
    (new WText("<h2>Wt Tree List example</h2>"
	       "<p>This is a simple demo of a treelist, implemented using"
	       " <a href='http://witty.sourceforge.net/'>Wt</a>.</p>"
	       "<p>The leafs of the tree contain the source code of the "
	       "tree-list in the classes <b>TreeNode</b> and "
	       "<b>IconPair</b>, as well as the implementation of this "
	       "demo itself in the class <b>DemoTreeList</b>.</p>"));

  tree_ = makeTreeMap("Examples", 0);
  addWidget(tree_);

  TreeNode *treelist = makeTreeMap("Tree List", tree_);
  TreeNode *wstateicon = makeTreeMap("class IconPair", treelist);
  makeTreeFile("<a href=\"IconPair.h\">IconPair.h</a>", wstateicon);
  makeTreeFile("<a href=\"IconPair.C\">IconPair.C</a>", wstateicon);
  TreeNode *wtreenode = makeTreeMap("class TreeNode", treelist);
  makeTreeFile("<a href=\"TreeNode.h\">TreeNode.h</a>", wtreenode);
  makeTreeFile("<a href=\"TreeNode.C\">TreeNode.C</a>", wtreenode);
  TreeNode *demotreelist = makeTreeMap("class DemoTreeList", treelist);
  makeTreeFile("<a href=\"DemoTreeList.h\">DemoTreeList.h</a>", demotreelist);
  makeTreeFile("<a href=\"DemoTreeList.C\">DemoTreeList.C</a>", demotreelist);

  testMap_ = makeTreeMap("Test map", tree_);

  /*
   * Buttons to dynamically demonstrate changing the tree contents.
   */
  addWidget
    (new WText("<p>Use the following buttons to change the tree "
	       "contents:</p>"));

  addMapButton_
    = new WPushButton("Add map", this);
  addMapButton_->clicked().connect(this, &DemoTreeList::addMap);

  removeMapButton_
    = new WPushButton("Remove map", this);
  removeMapButton_->clicked().connect(this, &DemoTreeList::removeMap);
  removeMapButton_->disable();

  addWidget
    (new WText("<p>Remarks:"
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

void DemoTreeList::addMap()
{
  TreeNode *node
    = makeTreeMap("Map " + boost::lexical_cast<std::string>(++testCount_),
		  testMap_);
  makeTreeFile("File " + boost::lexical_cast<std::string>(testCount_),
	       node);

  removeMapButton_->enable();
}

void DemoTreeList::removeMap()
{
  int numMaps = testMap_->childNodes().size();

  if (numMaps > 0) {
    int c = rand() % numMaps;

    TreeNode *child = testMap_->childNodes()[c];
    testMap_->removeChildNode(child);
    delete child;

    if (numMaps == 1)
      removeMapButton_->disable();
  }
}

TreeNode *DemoTreeList::makeTreeMap(const std::string name, TreeNode *parent)
{
  IconPair *labelIcon
    = new IconPair("icons/yellow-folder-closed.png",
		   "icons/yellow-folder-open.png",
		   false);

  TreeNode *node = new TreeNode(name, PlainText, labelIcon, 0);
  if (parent)
    parent->addChildNode(node);

  return node;
}

TreeNode *DemoTreeList::makeTreeFile(const std::string name,
				      TreeNode *parent)
{
  IconPair *labelIcon
    = new IconPair("icons/document.png", "icons/yellow-folder-open.png",
		   false);

  TreeNode *node = new TreeNode(name, XHTMLText, labelIcon, 0);
  if (parent)
    parent->addChildNode(node);

  return node;
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  new DemoTreeList(app->root());

  /*
   * The look & feel of the tree node is configured using a CSS style sheet.
   * If you are not familiar with CSS, you can use the WCssDecorationStyle
   * class ...
   */
  WCssDecorationStyle treeNodeLabelStyle;
  treeNodeLabelStyle.font().setFamily(WFont::Serif, "Helvetica");
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

