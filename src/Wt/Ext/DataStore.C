/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Ext/DataStore"

#include "Wt/WAbstractItemModel"
#include "Wt/WLogger"
#include "Wt/WWebWidget"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

namespace Wt {

LOGGER("Ext.DataStore");

  namespace Ext {

DataStore::DataStore(WAbstractItemModel *model, DataLocation dataLocation,
		     WObject *parent)
  : WResource(parent),
    model_(model),
    dataLocation_(dataLocation),
    nextId_(0),
    modelFilterColumn_(-1),
    rowsInserted_(false),
    needRefresh_(false)
{ 
  for (int i = 0; i < model->rowCount(); ++i) {
    recordIds_.push_back(-1);
    rowsInserted_ = true;
  }
}

DataStore::~DataStore()
{
  beingDeleted();
}

void DataStore::addColumn(int columnIndex, const std::string& fieldName)
{
  columns_.push_back(Column(columnIndex, fieldName));
}

void DataStore::setFilterColumn(int columnIndex)
{
  modelFilterColumn_ = columnIndex;
}

int DataStore::rowFromId(int id) const
{
  if (id == -1)
    return -1;

  if (id < (int)recordIds_.size() && recordIds_[id] == id)
    return id;

  for (unsigned i = 0; i < recordIds_.size(); ++i)
    if (recordIds_[i] == id)
      return i;

  return -1;
}

int DataStore::getRecordId(int row)
{
  if (recordIds_[row] == -1)
    recordIds_[row] = nextId_++;

  return recordIds_[row];
}

std::string DataStore::load(const std::string& storeVar, int pageSize)
{
  if (dataLocation_ == ServerSide) {
    return storeVar
      + ".load({params:{start:0,limit:"
      + boost::lexical_cast<std::string>(pageSize) + "}});"; 
  } else
    return std::string();
}

std::string DataStore::jsCreateStore()
{
  std::string store;

  switch (dataLocation_) {
  case ClientSide:
    store += "new Ext.data.SimpleStore({fields:[";

    for (unsigned i = 0; i < columns_.size(); ++i) {
      if (i != 0)
	store += ",";
      store += "{name:'c" + boost::lexical_cast<std::string>(i) + "',"
	"mapping:" + boost::lexical_cast<std::string>(i+1) + "}";
    }

    store += "],data:[";

    for (int i = 0; i < model_->rowCount(); ++i) {
      if (i != 0)
	store += ",";
      store += "[" + boost::lexical_cast<std::string>(getRecordId(i));
      for (unsigned j = 0; j < columns_.size(); ++j)
	store += "," + dataAsJSLiteral(i, columns_[j].modelColumn);
      store += "]";
    }

    store += "],id:0})";

    break;
  case ServerSide:
    store = "new Ext.data.Store({"
      "proxy: new Ext.data.HttpProxy({"
        "url:'" + generateUrl() + "'"
      "}),"
      "reader: new Ext.data.JsonReader({"
        "totalProperty:'count',"
        "root:'data',"
        "id:'id'"
      "},[";

    for (unsigned i = 0; i < columns_.size(); ++i) {
      if (i != 0)
	store += ",";
      store += "{name:'c" + boost::lexical_cast<std::string>(i) + "'}";
    }

    store +="])})";
  }

  rowsInserted_ = false;
  rowsDeleted_.clear();
  jsChanges_.clear();

  return store;
}

std::string DataStore::jsCreateRecordDef(const std::string& storeVar) const
{
  std::string recordDef = "Ext.data.Record.create([";

  for (unsigned i = 0; i < columns_.size(); ++i) {
    if (i != 0)
      recordDef += ",";
    recordDef += "{name:'c" + boost::lexical_cast<std::string>(i) + "',"
      "mapping:" + boost::lexical_cast<std::string>(i+1) + "}";
  }

  recordDef += "])";

  std::string result;

  result = storeVar + ".recordDef=" + recordDef + ";";

  return result;
}

void DataStore::setModel(WAbstractItemModel *model)
{
  model_ = model;

  jsChanges_ = "store.removeAll();";
  rowsDeleted_.clear();

  recordIds_.clear();

  for (int i = 0; i < model_->rowCount(); ++i) {
    recordIds_.push_back(-1);
  }
  rowsInserted_ = true;

  if (dataLocation_ == ServerSide)
    needRefresh_ = true;
}

std::string DataStore::dataAsJSLiteral(int row, int col) const
{
  WModelIndex index = model_->index(row, col);

  TextFormat tf = 
    model_->flags(index) & ItemIsXHTMLText ? XHTMLText : PlainText;
  return Wt::Impl::asJSLiteral(model_->data(index), tf);
}

std::string DataStore::jsGetUpdates(const std::string& storeVar)
{
  if (dataLocation_ == ClientSide) {
    if (jsChanges_.empty() && rowsDeleted_.empty() && !rowsInserted_)
      return std::string();

    std::stringstream result;

    result << "{var store=" << storeVar
	   << ";store.clearFilter();var RD=store.recordDef;";
  
    result << jsChanges_;
    jsChanges_.clear();

    for (int i = rowsDeleted_.size() - 1; i >= 0; --i) {
      result << "store.remove(store.getById(" << rowsDeleted_[i] << "));";
    }
    rowsDeleted_.clear();

    if (rowsInserted_) {
      for (int i = 0; i < model_->rowCount(); ++i) {
	if (recordIds_[i] == -1) {
	  result << "store.insert(" << i << ",[new RD({";

	  for (unsigned j = 0; j < columns_.size(); ++j) {
	    if (j != 0)
	      result << ',';
	    result << "'c" << j << "':"
		   << dataAsJSLiteral(i, columns_[j].modelColumn);
	  }

	  result << "}," << getRecordId(i) << ")]);";
	}
      }

      rowsInserted_ = false;
    }

    result << "};";

    return result.str();
  } else {
    if (needRefresh_) {
      needRefresh_ = false;
      return storeVar + ".reload();";
    } else
      return std::string();
  }
}

void DataStore::modelRowsInserted(int start, int end)
{
  for (int i = start; i <= end; ++i)
    recordIds_.insert(recordIds_.begin() + i, -1);

  rowsInserted_ = true;
  needRefresh_ = true;
}

void DataStore::modelRowsRemoved(int start, int end)
{
  if (dataLocation_ == ClientSide) {
    for (int i = start; i <= end; ++i)
      if (recordIds_[i] != -1)
	rowsDeleted_.push_back(recordIds_[i]);
  } else
    needRefresh_ = true;

  for (int i = start; i <= end; ++i)
    recordIds_.erase(recordIds_.begin() + start);
}
 
void DataStore::modelDataChanged(const WModelIndex& topLeft,
				 const WModelIndex& bottomRight)
{
  if (dataLocation_ == ClientSide) {
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
      if (recordIds_[i] == -1)
	continue;

      for (unsigned j = 0; j < columns_.size(); ++j) {
	const Column& c = columns_[j];

	if (c.modelColumn >= topLeft.column()
	    && c.modelColumn <= bottomRight.column()) {

	  jsChanges_ += "store.getById("
	    + boost::lexical_cast<std::string>(recordIds_[i]) + ").set('"
	    + c.fieldName + "',"
	    + dataAsJSLiteral(i, c.modelColumn) + ");";
	}
      }
    }
  } else
    needRefresh_ = true;
}

void DataStore::handleRequest(const Http::Request& request,
			      Http::Response& response)
{
  response.setMimeType("text/x-json");

  WModelIndexList matches;

  if (modelFilterColumn_ != -1) {
    WString query;

    const std::string *queryE = request.getParameter("query");
    if (queryE)
      query = WString::fromUTF8(*queryE);

    matches = model_->match(model_->index(0, modelFilterColumn_),
			    DisplayRole, boost::any(query));
  }

  int start = 0;
  int rowCount = (modelFilterColumn_ == -1 ?
		  model_->rowCount() : matches.size());
  int limit = rowCount;

  const std::string *s;

  s = request.getParameter("start");
  if (s)
    try {
      start = std::max(0, std::min(limit, boost::lexical_cast<int>(*s)));
    } catch (boost::bad_lexical_cast& e) {
      LOG_ERROR("start '" << *s << "' is not-a-number.");
    }

  s = request.getParameter("limit");
  if (s)
    try {
      limit = std::max(0, std::min(limit - start,
				   boost::lexical_cast<int>(*s)));
    } catch (boost::bad_lexical_cast& e) {
      LOG_ERROR("limit '" << *s << "' is not-a-number.");
    }

  std::ostream& o = response.out();

  o << "{"
    << "'count':" << rowCount << ","
    << "'data':[";

  for (int row = start; row < start + limit; ++row) {
    if (row != start)
      o << ",";
    o << "{";

    int modelRow = modelFilterColumn_ == -1 ? row : matches[row].row();

    o << "'id':" << getRecordId(modelRow);

    for (unsigned j = 0; j < columns_.size(); ++j)
      o << ",'" << columns_[j].fieldName << "':"
	<< dataAsJSLiteral(modelRow, columns_[j].modelColumn);

    o << "}";
  }

  o << "]}";
}

DataStore::Column::Column(int aModelColumn, const std::string& aFieldName)
  : modelColumn(aModelColumn),
    fieldName(aFieldName)
{ }

  }
}
