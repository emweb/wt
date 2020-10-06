/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>

#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDatePicker.h>
#include <Wt/WDateValidator.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WIntValidator.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WGridLayout.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSortFilterProxyModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>
#include <Wt/WTreeView.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

#include <Wt/Chart/WPieChart.h>

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
  FileModel()
    : WStandardItemModel() { }

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

WString FileModel::dateDisplayFormat(WString("MMM dd, yyyy"));
WString FileModel::dateEditFormat(WString("dd-MM-yyyy"));

/*! \class FileEditDialog
 *  \brief A dialog for editing a 'file'.
 */
class FileEditDialog : public WDialog
{
public:
  FileEditDialog(std::shared_ptr<WAbstractItemModel> model, const WModelIndex& item)
    : WDialog("Edit..."),
      model_(model),
      item_(item)
  {
    int modelRow = item_.row();

    resize(300, WLength::Auto);

    /*
     * Create the form widgets, and load them with data from the model.
     */

    // name
    auto nameEdit = std::make_unique<WLineEdit>(asString(model_->data(modelRow, 1)));
    nameEdit_ = nameEdit.get();

    // type
    auto typeEdit = std::make_unique<WComboBox>();
    typeEdit_ = typeEdit.get();
    typeEdit_->addItem("Document");
    typeEdit_->addItem("Spreadsheet");
    typeEdit_->addItem("Presentation");
    typeEdit_->setCurrentIndex
      (typeEdit_->findText(asString(model_->data(modelRow, 2))));

    // size
    auto sizeEdit = std::make_unique<WLineEdit>(asString(model_->data(modelRow, 3)));
    sizeEdit_ = sizeEdit.get();
    sizeEdit_->setValidator
      (std::make_shared<WIntValidator>(0, std::numeric_limits<int>::max()));


    // created
    auto createdPicker = std::make_unique<WDatePicker>();
    createdPicker_ = createdPicker.get();
    createdPicker_->lineEdit()->validator()->setMandatory(true);
    createdPicker_->setFormat(FileModel::dateEditFormat);
    createdPicker_->setDate(cpp17::any_cast<WDate>(model_->data(modelRow, 4)));

    // modified
    auto modifiedPicker = std::make_unique<WDatePicker>();
    modifiedPicker_ = modifiedPicker.get();
    modifiedPicker_->lineEdit()->validator()->setMandatory(true);
    modifiedPicker_->setFormat(FileModel::dateEditFormat);
    modifiedPicker_->setDate(cpp17::any_cast<WDate>(model_->data(modelRow, 5)));

    /*
     * Use a grid layout for the labels and fields
     */
    auto layout = std::make_unique<WGridLayout>();

    std::unique_ptr<WLabel> label;
    int row = 0;

    label = std::make_unique<WLabel>("Name:");
    label->setBuddy(nameEdit_);
    layout->addWidget(std::move(label), row, 0);
    layout->addWidget(std::move(nameEdit), row, 1);
    ++row;

    label = std::make_unique<WLabel>("Type:");
    label->setBuddy(typeEdit_);
    layout->addWidget(std::move(label), row, 0);
    layout->addWidget(std::move(typeEdit), row, 1);
    ++row;

    label = std::make_unique<WLabel>("Size");
    label->setBuddy(sizeEdit_);
    layout->addWidget(std::move(label), row, 0);
    layout->addWidget(std::move(sizeEdit), row, 1);
    ++row;

    label = std::make_unique<WLabel>("Created:");
    label->setBuddy(createdPicker_->lineEdit());
    layout->addWidget(std::move(label), row, 0);
    layout->addWidget(std::move(createdPicker), row, 2);
    ++row;

    label = std::make_unique<WLabel>("Modified:");
    label->setBuddy(modifiedPicker_->lineEdit());
    layout->addWidget(std::move(label), row, 0);
    layout->addWidget(std::move(modifiedPicker), row, 2);
    ++row;

    std::unique_ptr<WPushButton>button;
    auto buttons = std::make_unique<WContainerWidget>();

    button = std::make_unique<WPushButton>("Save");
    button->clicked().connect(this, &WDialog::accept);
    buttons->addWidget(std::move(button));

    button = std::make_unique<WPushButton>("Cancel");
    contents()->enterPressed().connect(this, &WDialog::accept);
    button->clicked().connect(this, &WDialog::reject);
    buttons->addWidget(std::move(button));

    /*
     * Focus the form widget that corresonds to the selected item.
     */
    switch (item.column()) {
    case 2:
      typeEdit_->setFocus(); break;
    case 3:
      sizeEdit_->setFocus(); break;
    case 4:
      createdPicker_->lineEdit()->setFocus();
      break;
    case 5:
      modifiedPicker_->lineEdit()->setFocus();
      break;
    default:
      nameEdit_->setFocus(); break;
    }

    layout->addWidget(std::move(buttons), row, 0, 0, 3, AlignmentFlag::Center);
    layout->setColumnStretch(1, 1);

    contents()->setLayout(std::move(layout));

    finished().connect(this, &FileEditDialog::handleFinish);

    show();
  }

private:
  std::shared_ptr<WAbstractItemModel> model_;
  WModelIndex                         item_;

  WLineEdit   *nameEdit_, *sizeEdit_;
  WComboBox   *typeEdit_;
  WDatePicker *createdPicker_, *modifiedPicker_;

  void handleFinish(DialogCode result)
  {
    if (result == DialogCode::Accepted) {
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
      std::shared_ptr<WAbstractItemModel> m = model_;
      int modelRow = item_.row();

      std::shared_ptr<WAbstractProxyModel> proxyModel =
          std::dynamic_pointer_cast<WAbstractProxyModel>(m);
      if (proxyModel) {
	m = proxyModel->sourceModel();
	modelRow = proxyModel->mapToSource(item_).row();
      }

      m->setData(modelRow, 1, cpp17::any(nameEdit_->text()));
      m->setData(modelRow, 2, cpp17::any(typeEdit_->currentText()));
      m->setData(modelRow, 3, cpp17::any(asNumber
					 (sizeEdit_->text().toUTF8())));
      m->setData(modelRow, 4, cpp17::any(createdPicker_->date()));
      m->setData(modelRow, 5, cpp17::any(modifiedPicker_->date()));
    }

    //delete this;
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
    : WApplication(env),
      popup_(nullptr),
      popupActionBox_(nullptr)
  {
    setCssTheme("polished");

    /*
     * Create the data models.
     */
    folderModel_ =
        std::make_shared<WStandardItemModel>(0, 1);
    populateFolders();


    fileModel_ =
        std::make_shared<FileModel>();
    populateFiles();

    /*
     * The header items are also endered using an ItemDelegate, and thus
     * support other data, e.g.:
     *
     * fileModel_->setHeaderFlags(0, Horizontal, HeaderIsUserCheckable);
     * fileModel_->setHeaderData(0, Horizontal,
     *                           std::string("icons/file.gif"),
     *                           Wt::DecorationRole);
     */
    fileFilterModel_ = std::make_shared<WSortFilterProxyModel>();
    fileFilterModel_->setSourceModel(fileModel_);
    fileFilterModel_->setDynamicSortFilter(true);
    fileFilterModel_->setFilterKeyColumn(0);
    fileFilterModel_->setFilterRole(ItemDataRole::User);

    /*
     * Setup the user interface.
     */
    createUI();

  }

  virtual ~TreeViewDragDrop()
  {
    dialog_.reset();
  }

private:
  /// The folder model (used by folderView_)
  std::shared_ptr<WStandardItemModel>     folderModel_;

  /// The file model (used by fileView_)
  std::shared_ptr<WStandardItemModel>     fileModel_;

  /// The sort filter proxy model that adapts fileModel_
  std::shared_ptr<WSortFilterProxyModel>  fileFilterModel_;

  /// Maps folder id's to folder descriptions.
  std::map<std::string, WString>          folderNameMap_;

  /// The folder view.
  WTreeView                              *folderView_;

  /// The file view.
  WTableView                             *fileView_;

  std::unique_ptr<FileEditDialog>         dialog_;

  /// Popup menu on the folder view
  std::unique_ptr<WPopupMenu>             popup_;

  /// Message box to confirm the poup menu action
  std::unique_ptr<WMessageBox>            popupActionBox_;

  /*! \brief Setup the user interface.
   */
  void createUI() {
    WContainerWidget *w = root();
    w->setStyleClass("maindiv");

    /*
     * The main layout is a 3x2 grid layout.
     */
    std::unique_ptr<WGridLayout> layout =
        std::make_unique<WGridLayout>();
    layout->addWidget(createTitle("Folders"), 0, 0);
    layout->addWidget(createTitle("Files"), 0, 1);
    layout->addWidget(folderView(), 1, 0);
    layout->setColumnResizable(0);

    // select the first folder
    folderView_->select(folderModel_->index(0, 0, folderModel_->index(0, 0)));

    std::unique_ptr<WVBoxLayout> vbox =
        std::make_unique<WVBoxLayout>();
    vbox->addWidget(fileView(), 1);
    vbox->addWidget(pieChart(), 1);
    vbox->setResizable(0);

    layout->addLayout(std::move(vbox), 1, 1);

    layout->addWidget(aboutDisplay(), 2, 0, 1, 2);

    /*
     * Let row 1 and column 1 take the excess space.
     */
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);

    w->setLayout(std::move(layout));
  }

  /*! \brief Creates a title widget.
   */
  std::unique_ptr<WText> createTitle(const WString& title) {
    auto result = std::make_unique<WText>(title);
    result->setInline(false);
    result->setStyleClass("title");

    return result;
  }

  /*! \brief Creates the folder WTreeView
   */
  std::unique_ptr<WTreeView> folderView() {
    auto treeView = std::make_unique<FolderView>();

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
    treeView->setSelectionMode(SelectionMode::Single);
    treeView->expandToDepth(1);
    treeView->selectionChanged()
      .connect(this, &TreeViewDragDrop::folderChanged);

    treeView->mouseWentUp().connect(this, &TreeViewDragDrop::showPopup);

    folderView_ = treeView.get();

    return std::move(treeView);
  }

  /*! \brief Creates the file table view (a WTableView)
   */
  std::unique_ptr<WTableView> fileView() {
    auto tableView
        = std::make_unique<WTableView>();

    tableView->setAlternatingRowColors(true);

    tableView->setModel(fileFilterModel_);
    tableView->setSelectionMode(SelectionMode::Extended);
    tableView->setDragEnabled(true);

    tableView->setColumnWidth(0, 100);
    tableView->setColumnWidth(1, 150);
    tableView->setColumnWidth(2, 100);
    tableView->setColumnWidth(3, 60);
    tableView->setColumnWidth(4, 100);
    tableView->setColumnWidth(5, 100);

    auto delegate = std::make_shared<WItemDelegate>();
    delegate->setTextFormat(FileModel::dateDisplayFormat);
    tableView->setItemDelegateForColumn(4, delegate);
    tableView->setItemDelegateForColumn(5, delegate);

    tableView->setColumnAlignment(3, AlignmentFlag::Right);
    tableView->setColumnAlignment(4, AlignmentFlag::Right);
    tableView->setColumnAlignment(5, AlignmentFlag::Right);

    tableView->sortByColumn(1, SortOrder::Ascending);

    tableView->doubleClicked().connect(this, std::bind(&TreeViewDragDrop::editFile,
                                                       this, std::placeholders::_1));

    fileView_ = tableView.get();

    return tableView;
  }

  /*! \brief Edit a particular row.
   */
  void editFile(const WModelIndex& item) {
    dialog_ = std::make_unique<FileEditDialog>(fileView_->model(), item);
  }

  /*! \brief Creates the chart.
   */
  std::unique_ptr<WWidget> pieChart() {
    using namespace Chart;

    auto chart = std::make_unique<WPieChart>();
    // chart->setPreferredMethod(WPaintedWidget::PngImage);
    chart->setModel(fileFilterModel_);
    chart->setTitle("File sizes");

    chart->setLabelsColumn(1); // Name
    chart->setDataColumn(3);   // Size

    chart->setPerspectiveEnabled(true, 0.2);
    chart->setDisplayLabels(LabelOption::Outside | LabelOption::TextLabel);

    if (!WApplication::instance()->environment().ajax()) {
      chart->resize(500, 200);
      chart->setMargin(WLength::Auto, Side::Left | Side::Right);

      auto w = std::make_unique<WContainerWidget>();
      w->addWidget(std::move(chart));
      w->setStyleClass("about");
      return std::move(w);
    } else {
      chart->setStyleClass("about");
      return std::move(chart);
    }
  }

  /*! \brief Creates the hints text.
   */
  std::unique_ptr<WWidget> aboutDisplay() {
    std::unique_ptr<WText> result
        = std::make_unique<WText>(WString::tr("about-text"));
    result->setStyleClass("about");
    return std::move(result);
  }

  /*! \brief Change the filter on the file view when the selected folder
   *         changes.
   */
  void folderChanged() {
    if (folderView_->selectedIndexes().empty())
      return;

    WModelIndex selected = *folderView_->selectedIndexes().begin();
    cpp17::any d = selected.data(ItemDataRole::User);
    if (cpp17::any_has_value(d)) {
        std::string folder = cpp17::any_cast<std::string>(d);

      // For simplicity, we assume here that the folder-id does not
      // contain special regexp characters, otherwise these need to be
      // escaped -- or use the \Q \E qutoing escape regular expression
      // syntax (and escape \E)
     fileFilterModel_->setFilterRegExp(std::unique_ptr<std::regex>(new std::regex(folder)));
    }
  }

  /*! \brief Show a popup for a folder item.
   */
  void showPopup(const WModelIndex& item, const WMouseEvent& event) {
    if (event.button() == MouseButton::Right) {
      // Select the item, it was not yet selected.
      if (!folderView_->isSelected(item))
	folderView_->select(item);

      if (!popup_) {
        popup_ = std::make_unique<WPopupMenu>();
	popup_->addItem("icons/folder_new.gif", "Create a New Folder");
	popup_->addItem("Rename this Folder")->setCheckable(true);
	popup_->addItem("Delete this Folder");
	popup_->addSeparator();
	popup_->addItem("Folder Details");
	popup_->addSeparator();
	popup_->addItem("Application Inventory");
	popup_->addItem("Hardware Inventory");
	popup_->addSeparator();

	std::unique_ptr<WPopupMenu> subMenu = std::make_unique<WPopupMenu>();
	subMenu->addItem("Sub Item 1");
	subMenu->addItem("Sub Item 2");
	popup_->addMenu("File Deployments", std::move(subMenu));

	/*
	 * This is one method of executing a popup, which does not block a
	 * thread for a reentrant event loop, and thus scales.
	 *
	 * Alternatively you could call WPopupMenu::exec(), which returns
	 * the result, but while waiting for it, blocks the thread.
	 */      
	popup_->aboutToHide().connect(this, &TreeViewDragDrop::popupAction);
      }

      if (popup_->isHidden())
      	popup_->popup(event);
      else
	popup_->hide();
    }
  }

  /** \brief Process the result of the popup menu
   */
  void popupAction() {
    if (popup_->result()) {
      /*
       * You could also bind extra data to an item using setData() and
       * check here for the action asked. For now, we just use the text.
       */
      WString text = popup_->result()->text();
      popup_->hide();

      popupActionBox_ = std::make_unique<WMessageBox>("Sorry.","Action '"
                                        + text + "' is not implemented.",
                                        Icon::None,
                                        StandardButton::Ok);
      popupActionBox_->buttonClicked()
	.connect(this, &TreeViewDragDrop::dialogDone);
      popupActionBox_->show();
    } else {
      popup_->hide();
    }
  }

  /** \brief Process the result of the message box.
   */
  void dialogDone() {
    popupActionBox_.reset();
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

    std::ifstream f((appRoot() + "data/files.csv").c_str());

    if (!f)
      throw std::runtime_error("Could not read: data/files.csv");

    readFromCsv(f, fileModel_);

    for (int i = 0; i < fileModel_->rowCount(); ++i) {
      WStandardItem *item = fileModel_->item(i, 0);
      item->setFlags(item->flags() | ItemFlag::DragEnabled);
      item->setIcon("icons/file.gif");

      std::string folderId = item->text().toUTF8();

      item->setData(cpp17::any(folderId), ItemDataRole::User);
      item->setText(folderNameMap_[folderId]);

      convertToNumber(fileModel_->item(i, 3));
      convertToDate(fileModel_->item(i, 4));
      convertToDate(fileModel_->item(i, 5));
    }
  }

  /*! \brief Convert a string to a date.
   */
  void convertToDate(WStandardItem *item) {
    WDate d = WDate::fromString(item->text(), FileModel::dateEditFormat);
    item->setData(cpp17::any(d), ItemDataRole::Display);
  }

  /*! \brief Convert a string to a number.
   */
  void convertToNumber(WStandardItem *item) {
    int i = asNumber(item->text());
    item->setData(cpp17::any(i), ItemDataRole::Edit);
  }

  /*! \brief Populate the folders model.
   */
  void populateFolders() {
    std::unique_ptr<WStandardItem> level1;

    level1 = createFolderItem("San Fransisco");
    level1->appendRow(createFolderItem("Investors", "sf-investors"));
    level1->appendRow(createFolderItem("Fellows", "sf-fellows"));
    folderModel_->appendRow(std::move(level1));

    level1 = createFolderItem("Sophia Antipolis");
    level1->appendRow(createFolderItem("R&D", "sa-r_d"));
    level1->appendRow(createFolderItem("Services", "sa-services"));
    level1->appendRow(createFolderItem("Support", "sa-support"));
    level1->appendRow(createFolderItem("Billing", "sa-billing"));
    folderModel_->appendRow(std::move(level1));

    level1 = createFolderItem("New York");
    level1->appendRow(createFolderItem("Marketing", "ny-marketing"));
    level1->appendRow(createFolderItem("Sales", "ny-sales"));
    level1->appendRow(createFolderItem("Advisors", "ny-advisors"));
    folderModel_->appendRow(std::move(level1));

    level1 = createFolderItem(WString(u8"Frankf\u00DCrt"));
    level1->appendRow(createFolderItem("Sales", "frank-sales"));
    folderModel_->appendRow(std::move(level1));

    folderModel_->setHeaderData(0, Orientation::Horizontal,
                                 cpp17::any(std::string("SandBox")));
  }

  /*! \brief Create a folder item.
   *
   * Configures flags for drag and drop support.
   */
  std::unique_ptr<WStandardItem> createFolderItem(const WString& location,
				  const std::string& folderId = std::string())
  {
    auto result
        = std::make_unique<WStandardItem>(location);

    if (!folderId.empty()) {
      result->setData(cpp17::any(folderId));
      result->setFlags(result->flags() | ItemFlag::DropEnabled);
      folderNameMap_[folderId] = location;
    } else
      result->setFlags(result->flags().clear(ItemFlag::Selectable));

    result->setIcon("icons/folder.gif");

    return result;
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<TreeViewDragDrop>(env);
  app->setTwoPhaseRenderingThreshold(0);
  app->setTitle("WTreeView Drag & Drop");
  app->useStyleSheet("styles.css");
  app->messageResourceBundle().use(WApplication::appRoot() + "about");
  app->refresh();
  
  return std::move(app);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

/*@}*/
