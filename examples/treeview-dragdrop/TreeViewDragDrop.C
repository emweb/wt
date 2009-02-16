/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>

#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WDatePicker>
#include <Wt/WDateValidator>
#include <Wt/WDialog>
#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WGridLayout>
#include <Wt/WPopupMenu>
#include <Wt/WSortFilterProxyModel>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WTreeView>
#include <Wt/WText>

#include "CsvUtil.h"
#include "FolderView.h"

using namespace Wt;

/**
 * \defgroup treeviewdragdrop Drag and drop in WTreeView example
 */
/*@{*/

/*! \class FileModel
 *  \brief A specialized standard item model which report a specific
 *         drag and drop mime type.
 *
 * A specific drag and drop mime type instead of the generic abstract
 * item model is returned by the model.
 */
class FileModel : public WStandardItemModel
{
public:
  /*! \brief Constructor.
   */
  FileModel(WObject *parent)
    : WStandardItemModel(parent) { }

  /*! \brief Return the mime type.
   */
  virtual std::string mimeType() const {
    return FolderView::FileSelectionMimeType;
  }

  /// Date display format.
  static WString dateDisplayFormat;

  /// Date edit format.
  static WString dateEditFormat;
};

WString FileModel::dateDisplayFormat("MMM dd, yyyy");
WString FileModel::dateEditFormat("dd-MM-yyyy");

/*! \class FileView
 *  \brief A specialized WTreeView that support editing using a WDialog box.
 *
 * \note Native inline editing support for WTreeView is planned early 2009.
 */
class FileView : public WTreeView
{
public:
  /*! \brief Create a new file view.
   */
  FileView(WContainerWidget *parent = 0)
    : WTreeView(parent)
  {
    doubleClicked.connect(SLOT(this, FileView::edit));
  }

private:
  /*! \brief Edit a particular row.
   */
  void edit(const WModelIndex& item) {
    int modelRow = item.row();

    WDialog d("Edit...");
    d.resize(300, WLength());

    /*
     * Create the form widgets, and load them with data from the model.
     */

    // name
    WLineEdit *nameEdit = new WLineEdit(asString(model()->data(modelRow, 1)));

    // type
    WComboBox *typeEdit = new WComboBox();
    typeEdit->addItem("Document");
    typeEdit->addItem("Spreadsheet");
    typeEdit->addItem("Presentation");
    typeEdit->setCurrentIndex
      (typeEdit->findText(asString(model()->data(modelRow, 2))));

    // size
    WLineEdit *sizeEdit
      = new WLineEdit(asString(model()->data(modelRow, 3)));
    sizeEdit->setValidator
      (new WIntValidator(0, std::numeric_limits<int>::max(), this));

    // created
    WLineEdit *createdEdit = new WLineEdit();
    createdEdit->setValidator
      (new WDateValidator(FileModel::dateEditFormat, this));
    createdEdit->validator()->setMandatory(true);

    WDatePicker *createdPicker
      = new WDatePicker(new WImage("icons/calendar_edit.png"), createdEdit);
    createdPicker->setFormat(FileModel::dateEditFormat);
    createdPicker->setDate(boost::any_cast<WDate>(model()->data(modelRow, 4)));

    // modified
    WLineEdit *modifiedEdit = new WLineEdit();
    modifiedEdit->setValidator
      (new WDateValidator(FileModel::dateEditFormat, this));
    modifiedEdit->validator()->setMandatory(true);

    WDatePicker *modifiedPicker
      = new WDatePicker(new WImage("icons/calendar_edit.png"), modifiedEdit);
    modifiedPicker->setFormat(FileModel::dateEditFormat);
    modifiedPicker->setDate(boost::any_cast<WDate>(model()->data(modelRow, 5)));

    /*
     * Use a grid layout for the labels and fields
     */
    WGridLayout *layout = new WGridLayout();

    WLabel *l;
    int row = 0;

    layout->addWidget(l = new WLabel("Name:"), row, 0);
    layout->addWidget(nameEdit, row, 1);
    l->setBuddy(nameEdit);
    ++row;

    layout->addWidget(l = new WLabel("Type:"), row, 0);
    layout->addWidget(typeEdit, row, 1, AlignTop);
    l->setBuddy(typeEdit);
    ++row;

    layout->addWidget(l = new WLabel("Size:"), row, 0);
    layout->addWidget(sizeEdit, row, 1);
    l->setBuddy(sizeEdit);
    ++row;

    layout->addWidget(l = new WLabel("Created:"), row, 0);
    layout->addWidget(createdEdit, row, 1);
    layout->addWidget(createdPicker, row, 2);
    l->setBuddy(createdEdit);
    ++row;

    layout->addWidget(l = new WLabel("Modified:"), row, 0);
    layout->addWidget(modifiedEdit, row, 1);
    layout->addWidget(modifiedPicker, row, 2);
    l->setBuddy(modifiedEdit);
    ++row;

    WPushButton *b;
    WContainerWidget *buttons = new WContainerWidget();
    buttons->addWidget(b = new WPushButton("Save"));
    b->clicked.connect(SLOT(&d, WDialog::accept));
    d.contents()->enterPressed.connect(SLOT(&d, WDialog::accept));
    buttons->addWidget(b = new WPushButton("Cancel"));
    b->clicked.connect(SLOT(&d, WDialog::reject));

    /*
     * Focus the form widget that corresonds to the selected item.
     */
    switch (item.column()) {
    case 2:
      typeEdit->setFocus(); break;
    case 3:
      sizeEdit->setFocus(); break;
    case 4:
      createdEdit->setFocus(); break;
    case 5:
      modifiedEdit->setFocus(); break;
    default:
      nameEdit->setFocus(); break;
    }

    layout->addWidget(buttons, row, 0, 0, 2, AlignCenter);
    layout->setColumnStretch(1, 1);

    d.contents()->setLayout(layout, AlignTop | AlignJustify);

    if (d.exec() == WDialog::Accepted) {
      /*
       * Update the model with data from the edit widgets.
       *
       * You will want to do some validation here...
       *
       * Note that we directly update the source model to avoid
       * problems caused by the dynamic sorting of the proxy model,
       * which reorders row numbers, and would cause us to switch to editing
       * the wrong data.
       */
      WAbstractItemModel *m = model();

      WAbstractProxyModel *proxyModel = dynamic_cast<WAbstractProxyModel *>(m);
      if (proxyModel) {
	m = proxyModel->sourceModel();
	modelRow = proxyModel->mapToSource(item).row();
      }

      m->setData(modelRow, 1, boost::any(nameEdit->text()));
      m->setData(modelRow, 2, boost::any(typeEdit->currentText()));
      m->setData(modelRow, 3, boost::any(boost::lexical_cast<int>
					 (sizeEdit->text().toUTF8())));
      m->setData(modelRow, 4, boost::any(createdPicker->date()));
      m->setData(modelRow, 5, boost::any(modifiedPicker->date()));
    }
  }
};

/*! \class TreeViewDragDrop
 *  \brief Main application class.
 */
class TreeViewDragDrop : public WApplication
{
public:
  /*! \brief Constructor.
   */
  TreeViewDragDrop(const WEnvironment &env)
    : WApplication(env) {

    /*
     * Create the data models.
     */
    folderModel_ = new WStandardItemModel(0, 1, this);
    populateFolders();

    fileModel_ = new FileModel(this);
    populateFiles();

    fileFilterModel_ = new WSortFilterProxyModel(this);
    fileFilterModel_->setSourceModel(fileModel_);
    fileFilterModel_->setDynamicSortFilter(true);
    fileFilterModel_->setFilterKeyColumn(0);
    fileFilterModel_->setFilterRole(UserRole);

    /*
     * Setup the user interface.
     */
    createUI();

    /*
     * Select the first folder
     */
    folderView_->select
      (folderModel_->index(0, 0, folderModel_->index(0, 0)));
  }

private:
  /// The folder model (used by folderView_)
  WStandardItemModel    *folderModel_;

  /// The file model (used by fileView_)
  WStandardItemModel    *fileModel_;

  /// The sort filter proxy model that adapts fileModel_
  WSortFilterProxyModel *fileFilterModel_;

  /// Maps folder id's to folder descriptions.
  std::map<std::string, WString> folderNameMap_;

  /// The folder view.
  WTreeView *folderView_;

  /// The file view.
  WTreeView *fileView_;

  /*! \brief Setup the user interface.
   */
  void createUI() {
    WContainerWidget *w = root();
    w->setStyleClass("maindiv");

    /*
     * The main layout is a 3x2 grid layout.
     */
    WGridLayout *layout = new WGridLayout();
    layout->addWidget(createTitle("Folders"), 0, 0);
    layout->addWidget(createTitle("Files"), 0, 1);
    layout->addWidget(folderView(), 1, 0);
    layout->addWidget(fileView(), 1, 1);

    layout->addWidget(aboutDisplay(), 2, 0, 1, 2, AlignTop);

    /*
     * Let row 1 and column 1 take the excess space.
     */
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);

    w->setLayout(layout);
  }

  /*! \brief Creates a title widget.
   */
  WText *createTitle(const WString& title) {
    WText *result = new WText(title);
    result->setInline(false);
    result->setStyleClass("title");

    return result;
  }

  /*! \brief Creates the folder WTreeView
   */
  WTreeView *folderView() {
    WTreeView *treeView = new FolderView();

    /*
     * To support right-click, we need to disable the built-in browser
     * context menu.
     *
     * Note that disabling the context menu and catching the
     * right-click does not work reliably on all browsers.
     */
    treeView->setAttributeValue
      ("oncontextmenu",
       "event.cancelBubble = true; event.returnValue = false; return false;");
    treeView->setModel(folderModel_);
    treeView->resize(200, WLength::Auto);
    treeView->setSelectionMode(SingleSelection);
    treeView->expandToDepth(1);
    treeView->selectionChanged.connect(SLOT(this,
					    TreeViewDragDrop::folderChanged));

    treeView->mouseWentDown.connect(SLOT(this, TreeViewDragDrop::showPopup));

    folderView_ = treeView;

    return treeView;
  }

  /*! \brief Creates the file table view (also a WTreeView)
   */
  WTreeView *fileView() {
    WTreeView *treeView = new FileView();

    // Hide the tree-like decoration on the first column, to make it
    // resemble a plain table
    treeView->setRootIsDecorated(false);

    treeView->setModel(fileFilterModel_);
    treeView->setSelectionMode(ExtendedSelection);
    treeView->setDragEnabled(true);

    treeView->setColumnWidth(0, 100);
    treeView->setColumnWidth(1, 150);
    treeView->setColumnWidth(2, 100);
    treeView->setColumnWidth(3, 80);
    treeView->setColumnWidth(4, 120);
    treeView->setColumnWidth(5, 120);

    treeView->setColumnFormat(4, FileModel::dateDisplayFormat);
    treeView->setColumnFormat(5, FileModel::dateDisplayFormat);

    treeView->setColumnAlignment(3, AlignRight);
    treeView->setColumnAlignment(4, AlignRight);
    treeView->setColumnAlignment(5, AlignRight);

    treeView->sortByColumn(1, AscendingOrder);

    fileView_ = treeView;

    return treeView;
  }

  /*! \brief Creates the hints text.
   */
  WWidget *aboutDisplay() {
    WText *result = new WText(WString::tr("about-text"));
    result->setStyleClass("about");
    return result;
  }

  /*! \brief Change the filter on the file view when the selected folder
   *         changes.
   */
  void folderChanged() {
    if (folderView_->selectedIndexes().empty())
      return;

    WModelIndex selected = *folderView_->selectedIndexes().begin();
    boost::any d = selected.data(UserRole);
    if (!d.empty()) {
      std::string folder = boost::any_cast<std::string>(d);

      // For simplicity, we assume here that the folder-id does not
      // contain special regexp characters, otherwise these need to be
      // escaped -- or use the \Q \E qutoing escape regular expression
      // syntax (and escape \E)
      fileFilterModel_->setFilterRegExp(folder);
    }
  }

  /*! \brief Show a popup for a folder item.
   */
  void showPopup(const WModelIndex& item, const WMouseEvent& event) {
    if (event.button() == WMouseEvent::RightButton) {

      // Select the item, it was not yet selected.
      folderView_->select(item);

      WPopupMenu popup;
      popup.addItem("icons/folder_new.gif", "Create a New Folder");
      popup.addItem("Rename this Folder")->setCheckable(true);
      popup.addItem("Delete this Folder");
      popup.addSeparator();
      popup.addItem("Folder Details");
      popup.addSeparator();
      popup.addItem("Application Inventory");
      popup.addItem("Hardware Inventory");
      popup.addSeparator();

      WPopupMenu *subMenu = new WPopupMenu();
      subMenu->addItem("Sub Item 1");
      subMenu->addItem("Sub Item 2");
      popup.addMenu("File Deployments", subMenu);

      /*
       * This is one method of executing a popup, i.e. by using a reentrant
       * event loop (blocking the current thread).
       *
       * Alternatively you could call WPopupMenu::popup(), listen for
       * to the WPopupMenu::aboutToHide signal, and check the
       * WPopupMenu::result()
       */
      WPopupMenuItem *item = popup.exec(event);

      if (item) {
	/*
	 * You may bind extra data to an item using setData() and
	 * check here for the action asked.
	 */
	WMessageBox::show("Sorry.",
			  "Action '" + item->text() + "' is not implemented.",
			  Ok);
      }
    }
  }

  /*! \brief Populate the files model.
   *
   * Data (and headers) is read from the CSV file data/files.csv. We
   * add icons to the first column, resolve the folder id to the
   * actual folder name, and configure item flags, and parse date
   * values.
   */
  void populateFiles() {
    fileModel_->invisibleRootItem()->setRowCount(0);

    std::ifstream f("data/files.csv");
    readFromCsv(f, fileModel_);

    for (int i = 0; i < fileModel_->rowCount(); ++i) {
      WStandardItem *item = fileModel_->item(i, 0);
      item->setFlags(item->flags() | ItemIsDragEnabled);
      item->setIcon("icons/file.gif");

      std::string folderId = item->text().toUTF8();

      item->setData(boost::any(folderId), UserRole);
      item->setText(folderNameMap_[folderId]);

      convertToDate(fileModel_->item(i, 4));
      convertToDate(fileModel_->item(i, 5));
    }
  }

  /*! \brief Convert a string to a date.
   */
  void convertToDate(WStandardItem *item) {
    WDate d = WDate::fromString(item->text(), FileModel::dateEditFormat);
    item->setData(boost::any(d), DisplayRole);
  }

  /*! \brief Populate the folders model.
   */
  void populateFolders() {
    WStandardItem *level1, *level2;

    folderModel_->appendRow(level1 = createFolderItem("San Fransisco"));
    level1->appendRow(level2 = createFolderItem("Investors", "sf-investors"));
    level1->appendRow(level2 = createFolderItem("Fellows", "sf-fellows"));

    folderModel_->appendRow(level1 = createFolderItem("Sophia Antipolis"));
    level1->appendRow(level2 = createFolderItem("R&D", "sa-r_d"));
    level1->appendRow(level2 = createFolderItem("Services", "sa-services"));
    level1->appendRow(level2 = createFolderItem("Support", "sa-support"));
    level1->appendRow(level2 = createFolderItem("Billing", "sa-billing"));

    folderModel_->appendRow(level1 = createFolderItem("New York"));
    level1->appendRow(level2 = createFolderItem("Marketing", "ny-marketing"));
    level1->appendRow(level2 = createFolderItem("Sales", "ny-sales"));
    level1->appendRow(level2 = createFolderItem("Advisors", "ny-advisors"));

    folderModel_->appendRow(level1 = createFolderItem
			     (WString::fromUTF8("Frankfürt")));
    level1->appendRow(level2 = createFolderItem("Sales", "frank-sales"));

    folderModel_->setHeaderData(0, Horizontal,
				 boost::any(std::string("SandBox")));
  }

  /*! \brief Create a folder item.
   *
   * Configures flags for drag and drop support.
   */
  WStandardItem *createFolderItem(const WString& location,
				  const std::string& folderId = std::string())
  {
    WStandardItem *result = new WStandardItem(location);

    if (!folderId.empty()) {
      result->setData(boost::any(folderId));
      result->setFlags(result->flags() | ItemIsDropEnabled);
      folderNameMap_[folderId] = location;
    } else
      result->setFlags(result->flags() & ~ItemIsSelectable);

    result->setIcon("icons/folder.gif");

    return result;
  }

};

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new TreeViewDragDrop(env);
  app->setTitle("WTreeView Drag & Drop");
  app->useStyleSheet("styles.css");
  app->messageResourceBundle().use("about");
  app->refresh();
  
  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

/*@}*/
