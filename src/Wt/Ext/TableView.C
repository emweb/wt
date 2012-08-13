/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WAbstractItemModel"
#include "Wt/WDate"
#include "Wt/WLogger"

#include "Wt/Ext/DataStore"
#include "Wt/Ext/TableView"
#include "Wt/Ext/FormField"
#include "Wt/Ext/PagingToolBar"

#include "WebUtils.h"

namespace {
  void parseNumberList(const std::string& s, std::vector<int>& numbers)
  {
    std::istringstream ss(s);

    while (ss) {
      int a;
      ss >> a;

      if (!ss)
	break;
      else {
	numbers.push_back(a);
      }
    }
  }
}

namespace Wt {

LOGGER("Ext.TableView");

  namespace Ext {

TableView::TableView(WContainerWidget *parent)
  : Panel(parent),
    cellClicked_(this),
    currentCellChanged_(this),
    itemSelectionChanged_(this),
    dataLocation_(ClientSide),
    model_(0),
    selectionMode_(NoSelection),
    selectionBehavior_(SelectRows),
    currentRow_(-1),
    currentColumn_(-1),
    pageSize_(0),
    autoExpandColumn_(-1),
    autoExpandColumnMinWidth_(50),
    autoExpandColumnMaxWidth_(1000),
    autoFill_(false),
    forceFit_(false),
    columnMove_(false),
    alternatingRowColors_(false),
    highlightMouseOver_(false),
    dataStore_(0),
    edited_(this, "edited"),
    selectionChanged_(this, "sc"),
    rawCellClicked_(this, "ck")
{
  rawCellClicked_.connect(this, &TableView::onCellClicked);
}

void TableView::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  /* connect slots to new model */
  modelConnections_.push_back
    (model_->columnsInserted().connect(this, &TableView::modelColumnsInserted));
  modelConnections_.push_back
    (model_->columnsRemoved().connect(this, &TableView::modelColumnsRemoved));
  modelConnections_.push_back
    (model_->rowsInserted().connect(this, &TableView::modelRowsInserted));
  modelConnections_.push_back
    (model_->rowsRemoved().connect(this, &TableView::modelRowsRemoved));
  modelConnections_.push_back
    (model_->dataChanged().connect(this, &TableView::modelDataChanged));
  modelConnections_.push_back
    (model_->layoutChanged().connect(this, &TableView::modelLayoutChanged));
  modelConnections_.push_back
    (model_->modelReset().connect(this, &TableView::modelLayoutChanged));

  modelLayoutChanged();
}

void TableView::modelLayoutChanged()
{
  if (dataStore_) {
    dataStore_->setModel(model_);
    repaint();
  }
}

void TableView::setPageSize(int pageSize)
{
  pageSize_ = pageSize;
}

ToolBar *TableView::createPagingToolBar()
{
  return new PagingToolBar(elVar() + "ds", this);
}

void TableView::setDataLocation(DataLocation dataLocation)
{
  dataLocation_ = dataLocation;
}

void TableView::resizeColumnsToContents(bool whenTableResizes)
{
  autoFill_ = true;
  if (whenTableResizes)
    forceFit_ = true;
}

void TableView::setColumnsMovable(bool movable)
{
  columnMove_ = movable;
}

void TableView::setAlternatingRowColors(bool enable)
{
  alternatingRowColors_ = enable;
}

void TableView::setHighlightMouseOver(bool highlight)
{
  highlightMouseOver_ = highlight;
}

void TableView::clearSelection()
{
  setCurrentCell(-1, 0);
  selectedRows_.clear();
}

void TableView::setCurrentCell(int row, int column)
{
  currentRow_ = std::max(-1, std::min(model_->rowCount(), row));
  currentColumn_ = std::max(-1, std::min(model_->columnCount(), column));

  if (isRendered()) {
    if (currentRow_ == -1) {
      addUpdateJS(elVar() + ".getSelectionModel().clearSelections();");      
    } else {
      if (selectionBehavior_ == SelectRows)
	addUpdateJS(elVar() + ".getSelectionModel().selectRow("
		    + boost::lexical_cast<std::string>(row) + ");");
      else
	addUpdateJS(elVar() + ".getSelectionModel().select("
		    + boost::lexical_cast<std::string>(row) + ","
		    + boost::lexical_cast<std::string>(column) + ");");
    }
  }
}

void TableView::onSelectionChange(const std::string selection)
{
  /* format: a b c with row ids or a pair of row column */
  std::vector<int> numbers;

  parseNumberList(selection, numbers);

  if (selectionBehavior_ == SelectRows) {
    for (unsigned i = 0; i < numbers.size(); ++i)
      numbers[i] = dataStore_->rowFromId(numbers[i]);

    selectedRows_ = numbers;
  } else {
    int pr = currentRow_;
    int pc = currentColumn_;

    currentRow_ = dataStore_->rowFromId(numbers[0]);
    currentColumn_ = numbers[1];

    currentCellChanged_.emit(currentRow_, currentColumn_, pr, pc);
  }

  itemSelectionChanged_.emit();
}

void TableView::setEditor(int column, FormField *editor)
{
  columnInfo_[column].editor_ = editor;
  editor->useAsTableViewEditor();
  addOrphan(editor);

  selectionBehavior_ = SelectItems;
}

void TableView::setRenderer(int column, const std::string& rendererJS)
{
  columnInfo_[column].rendererJS_ = rendererJS;
}

std::string TableView::dateRenderer(const WString& format)
{
  return "Ext.util.Format.dateRenderer('" + WDate::extFormat(format) + "')";
}

bool TableView::isColumnHidden(int column) const
{
  ColumnMap::const_iterator k = columnInfo_.find(column);
  if (k != columnInfo_.end())
    return k->second.hidden_;
  else
    return false;
}

void TableView::setColumnHidden(int column, bool hide)
{
  columnInfo_[column].hidden_ = hide;
  addUpdateJS(elVar() + ".getColumnModel().setHidden("
	      + boost::lexical_cast<std::string>(column) + ","
	      + (hide ? "true" : "false") + ");");
}

int TableView::columnWidth(int column) const
{
  ColumnMap::const_iterator k = columnInfo_.find(column);
  if (k != columnInfo_.end())
    return k->second.width_;
  else
    return 100;
}

void TableView::setColumnWidth(int column, int pixels)
{
  columnInfo_[column].width_ = pixels;
  addUpdateJS(elVar() + ".getColumnModel().setColumnWidth("
	      + boost::lexical_cast<std::string>(column) + ","
	      + boost::lexical_cast<std::string>(pixels) + ");");
}

void TableView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  if (alignment & AlignVerticalMask) {
    LOG_ERROR("setColumnAlignment(): alignment (" << alignment
	      << ") is vertical, expected horizontal (column "
	      << column << ")");
    alignment = AlignmentFlag(alignment & AlignHorizontalMask);
  }

  columnInfo_[column].alignment_ = alignment;
}

AlignmentFlag TableView::columnAlignment(int column) const
{
  ColumnMap::const_iterator k = columnInfo_.find(column);
  if (k != columnInfo_.end())
    return k->second.alignment_;
  else
    return AlignLeft;
}

bool TableView::isColumnSortable(int column) const
{
  ColumnMap::const_iterator k = columnInfo_.find(column);
  if (k != columnInfo_.end())
    return k->second.sortable_;
  else
    return false;
}

void TableView::setColumnSortable(int column, bool sortable)
{
  columnInfo_[column].sortable_ = sortable;
}

bool TableView::isColumnHidingEnabled(int column) const
{
  ColumnMap::const_iterator k = columnInfo_.find(column);
  if (k != columnInfo_.end())
    return k->second.hideable_;
  else
    return false;
}

void TableView::enableColumnHiding(int column, bool enable)
{
  columnInfo_[column].hideable_ = enable;  
}

void TableView::setAutoExpandColumn(int column, int minWidth, int maxWidth)
{
  autoExpandColumn_ = column;
  autoExpandColumnMinWidth_ = minWidth;
  autoExpandColumnMaxWidth_ = maxWidth;
}

int TableView::autoExpandColumn() const
{
  if (autoExpandColumn_ == -1)
    return model_->columnCount() - 1;
  else
    return autoExpandColumn_;
}

void TableView::hideColumn(int column)
{
  setColumnHidden(column, true);
}

void TableView::showColumn(int column)
{
  setColumnHidden(column, false);
}

void TableView::modelColumnsInserted(const WModelIndex& parent,
				     int start, int end)
{
  if (dataStore_)
    LOG_ERROR("cannot deal with column inserts");
}

void TableView::modelColumnsRemoved(const WModelIndex& parent,
				    int start, int end)
{
  if (dataStore_)
    LOG_ERROR("cannot deal with column inserts");
}

void TableView::modelRowsInserted(const WModelIndex& parent,
				  int start, int end)
{
  if (dataStore_) {
    dataStore_->modelRowsInserted(start, end);
    repaint();
  }

  shiftSelectedRows(start, end - start + 1);
}

void TableView::modelRowsRemoved(const WModelIndex& parent,
				 int start, int end)
{
  if (dataStore_) {
    dataStore_->modelRowsRemoved(start, end);
    repaint();
  }

  shiftSelectedRows(start, - (end - start + 1));
}

void TableView::shiftSelectedRows(int start, int count)
{
  if (count < 0) {
    for (int i = start; i < start - count; ++i)
      Utils::erase(selectedRows_, i);

    if (currentRow_ >= start && currentRow_ < (start - count)) {
      currentRow_ = -1;
      currentColumn_ = -1;
    }
  }

  for (unsigned i = 0; i < selectedRows_.size(); ++i)
    if (selectedRows_[i] >= start)
      selectedRows_[i] += count;

  if (currentRow_ >= start)
    currentRow_ += start;
}

void TableView::modelDataChanged(const WModelIndex& topLeft,
				 const WModelIndex& bottomRight)
{
  if (dataStore_) {
    dataStore_->modelDataChanged(topLeft, bottomRight);
    repaint();
  }
}

void TableView::onCellClicked(std::string field, int rowId)
{
  int row = dataStore_->rowFromId(rowId);
  int col = boost::lexical_cast<int>(field.substr(1));

  cellClicked_.emit(row, col);
}

void TableView::onEdit(std::string field, int rowId, std::string value)
{
  int row = dataStore_->rowFromId(rowId);

  try {
    int col = boost::lexical_cast<int>(field.substr(1));
    model_->setData(row, col,
		    Wt::Impl::updateFromJS(model_->data(row, col), value));
  } catch (boost::bad_lexical_cast& e) {
    LOG_ERROR("internal error reading field name '" << field << "'");
  }
}

void TableView::refresh()
{
  modelDataChanged
    (model_->index(0, 0),
     model_->index(model_->columnCount() - 1, model_->rowCount() - 1));

  Panel::refresh();
}

void TableView::updateExt()
{
  if (dataStore_)
    addUpdateJS(dataStore_->jsGetUpdates(elVar() + ".getStore()"));

  Panel::updateExt();
}

std::string TableView::extClassName() const
{
  for (ColumnMap::const_iterator k = columnInfo_.begin();
       k != columnInfo_.end(); ++k)
    if (k->second.editor_)
      return "Ext.grid.EditorGridPanel";

  return "Ext.grid.GridPanel";
}

std::string TableView::createJS(DomElement *inContainer)
{
  assert(inContainer);

  if (!edited_.isConnected()) {
    edited_.connect(this, &TableView::onEdit);
    selectionChanged_.connect(this, &TableView::onSelectionChange);
  }

  std::stringstream result;
  bool haveEditor = false;

  for (ColumnMap::const_iterator k = columnInfo_.begin();
       k != columnInfo_.end(); ++k) {
    const ColumnModel& ci = k->second;

    if (ci.editor_) {
      haveEditor = true;
      ci.editor_->createExtElement(result, 0);
    }
  }

  if (!dataStore_) {
    dataStore_ = new DataStore(model_, dataLocation_, this);

    for (int i = 0; i < model_->columnCount(); ++i) {
      dataStore_
	->addColumn(i, "c" + boost::lexical_cast<std::string>(i));
    }
  }

  result << "var " << elVar() << "ds=" << dataStore_->jsCreateStore() << ";";

  result << Panel::createJS(inContainer);

  result << dataStore_->jsCreateRecordDef(elVar() + ".getStore()");

  if (selectionBehavior_ == SelectRows) {
    result << elVar()
	   << ".getSelectionModel().on('selectionchange',function(s){"
      "var ss=\"\";var m=s.getSelections();"
      "for (var i = 0; i < m.length; ++i) {"
      "ss += \" \" + m[i].id;" 
      "}"
	   << selectionChanged_.createCall("ss");
  } else {
    /*
     *FIXME: convert column to field to be insensitive to reordered columns ?
     */
    result << elVar() 
	   << ".getSelectionModel().on('selectionchange',function(s,sel){"
	   << selectionChanged_.createCall
      ("sel?''+sel.record.id+' '+sel.cell[1]:'-1 -1'");
  }
  result << "}," << elVar() << ",{buffer:10});";

  if (haveEditor)
    result << elVar() << ".on('afteredit',function(ge){"
	   << edited_.createCall("ge.field",
				 "ge.record.id",
				 "ge.value") << "});";

  
  bindEventHandler("cellclick", "ccH", result);

  result << dataStore_->load(elVar() + "ds", pageSize_);

  return result.str();
}

void TableView::createConfig(std::ostream& config)
{
  config << ",ds:" << elVar() << "ds,cm:new Ext.grid.ColumnModel([";

  for (int i = 0; i < model_->columnCount(); ++i) {    
    if (i != 0)
      config << ',';
    config << "{id:'c" << i << "',header:"
	   << Wt::Impl::asJSLiteral(model_->headerData(i), PlainText);

    ColumnMap::const_iterator k = columnInfo_.find(i);
    if (k != columnInfo_.end()) {
      const ColumnModel& ci = k->second;

      config << ",sortable:" << (ci.sortable_ ? "true" : "false")
	     << ",width:" << ci.width_;
      if (!ci.resizable_)
	config << ",resizable:false";
      if (ci.hidden_)
	config << ",hidden:true";
      if (!ci.hideable_)
	config << ",hideable:false";
      if (ci.editor_)
	config << ",editor:new Ext.grid.GridEditor("
	       << ci.editor_->elVar() << ')';
      switch (ci.alignment_) {
      case AlignLeft:
	break;
      case AlignRight:
	config << ",align:'right'"; break;
      case AlignCenter:
	config << ",align:'center'"; break;
      case AlignJustify:
	config << ",align:'justify'"; break;
      default:
	break;
      }
      if (!ci.rendererJS_.empty())
	config << ",renderer:" << ci.rendererJS_;
    } else {
      config << ",hideable:false";
    }
    config << ",dataIndex:'c" << i << '\'' << '}';
  }

  config << "])";

  if (autoExpandColumn_ != -1) {
    config << ",autoExpandColumn:'c" << autoExpandColumn() << '\'';

    if (autoExpandColumnMaxWidth_ != 1000)
      config << ",autoExpandMax:" << autoExpandColumnMaxWidth_;
    if (autoExpandColumnMinWidth_ != 50)
      config << ",autoExpandMin:" << autoExpandColumnMinWidth_;
  }

  if (!columnMove_)
    config << ",enableColumnMove:false";
  if (alternatingRowColors_)
    config << ",stripeRows:true";
  if (!highlightMouseOver_)
    config << ",trackMouseOver:false";

  config << ",viewConfig:{a:0";

  if (autoFill_)
    config << ",autoFill:true";
  if (forceFit_)
    config << ",forceFit:true";

  config << '}';

  if (selectionMode_ == NoSelection)
    config << ",disableSelection:true";
  else {
    if (selectionBehavior_ == SelectRows) {
      config << ",sm:new Ext.grid.RowSelectionModel(";
      if (selectionMode_ == SingleSelection)
	config << "{singleSelect:true}";
      config << ')';
    } else {
      config << ",sm:new Ext.grid.CellSelectionModel()";
    }
  }

  addWtSignalConfig("ccH", &rawCellClicked_, rawCellClicked_.name(),
		    "g,ri,ci", "g.getColumnModel().getDataIndex(ci),"
		    "g.getStore().getAt(ri).id", config);

  Panel::createConfig(config);
}

void TableView::setSelectionMode(SelectionMode mode)
{
  selectionMode_ = mode;
}

void TableView::setSelectionBehavior(SelectionBehavior behavior)
{
  selectionBehavior_ = behavior;
}

TableView::ColumnModel::ColumnModel()
  : sortable_(false),
    hideable_(false),
    hidden_(false),
    resizable_(true),
    width_(100),
    editor_(0),
    alignment_(AlignLeft)
{ }

TableView::ColumnModel::~ColumnModel()
{
  delete editor_;
}

  }
}
