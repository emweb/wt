///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: row.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, Row class implementation
//
///////////////////////////////////////////////////////////////////////////////
//
//	(C) Copyright 2000-2006 T.I.P. Group S.A. and the IBPP Team (www.ibpp.org)
//
//	The contents of this file are subject to the IBPP License (the "License");
//	you may not use this file except in compliance with the License.  You may
//	obtain a copy of the License at http://www.ibpp.org or in the 'license.txt'
//	file which must have been distributed along with this file.
//
//	This software, distributed under the License, is distributed on an "AS IS"
//	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
//	License for the specific language governing rights and limitations
//	under the License.
//
///////////////////////////////////////////////////////////////////////////////
//
//	COMMENTS
//	* Tabulations should be set every four characters when editing this file.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable: 4786 4996)
#ifndef _DEBUG
#pragma warning(disable: 4702)
#endif
#endif

#include "_ibpp.h"

#ifdef HAS_HDRSTOP
#pragma hdrstop
#endif

#include <math.h>
#include <time.h>
#include <string.h>

using namespace ibpp_internals;

//	(((((((( OBJECT INTERFACE IMPLEMENTATION ))))))))

void RowImpl::SetNull(int param)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::SetNull", _("The row is not initialized."));
	if (param < 1 || param > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::SetNull", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[param-1]);
	if (! (var->sqltype & 1))
		throw LogicExceptionImpl("Row::SetNull", _("This column can't be null."));

	*var->sqlind = -1;	// Set the column to SQL NULL
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, bool value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[bool]", _("The row is not initialized."));

	SetValue(param, ivBool, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const char* cstring)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[char*]", _("The row is not initialized."));
	if (cstring == 0)
		throw LogicExceptionImpl("Row::Set[char*]", _("null char* pointer detected."));

	SetValue(param, ivByte, cstring, (int)strlen(cstring));
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const void* bindata, int len)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[void*]", _("The row is not initialized."));
	if (bindata == 0)
		throw LogicExceptionImpl("Row::Set[void*]", _("null char* pointer detected."));
	if (len < 0)
		throw LogicExceptionImpl("Row::Set[void*]", _("Length must be >= 0"));
		
	SetValue(param, ivByte, bindata, len);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const std::string& s)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[string]", _("The row is not initialized."));

	SetValue(param, ivString, (void*)&s);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, int16_t value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[int16_t]", _("The row is not initialized."));
											
	SetValue(param, ivInt16, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, int32_t value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[int32_t]", _("The row is not initialized."));

	SetValue(param, ivInt32, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, int64_t value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[int64_t]", _("The row is not initialized."));

	SetValue(param, ivInt64, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, float value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[float]", _("The row is not initialized."));

	SetValue(param, ivFloat, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, double value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[double]", _("The row is not initialized."));

	SetValue(param, ivDouble, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::Timestamp& value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Timestamp]", _("The row is not initialized."));

	SetValue(param, ivTimestamp, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::Date& value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Date]", _("The row is not initialized."));

	if (mDialect == 1)
	{
		// In dialect 1, IBPP::Date is supposed to work with old 'DATE'
		// fields which are actually ISC_TIMESTAMP.
		IBPP::Timestamp timestamp(value);
		SetValue(param, ivTimestamp, &timestamp);
	}
	else
	{
		// Dialect 3
		SetValue(param, ivDate, (void*)&value);
	}

	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::Time& value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Time]", _("The row is not initialized."));
	if (mDialect == 1)
		throw LogicExceptionImpl("Row::Set[Time]", _("Requires use of a dialect 3 database."));

	SetValue(param, ivTime, &value);
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::Blob& blob)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Blob]", _("The row is not initialized."));
	if (mDatabase != 0 && blob->DatabasePtr() != mDatabase)
		throw LogicExceptionImpl("Row::Set[Blob]",
			_("IBlob and Row attached to different databases"));
	if (mTransaction != 0 && blob->TransactionPtr() != mTransaction)
		throw LogicExceptionImpl("Row::Set[Blob]",
			_("IBlob and Row attached to different transactions"));

	SetValue(param, ivBlob, blob.intf());
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::Array& array)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Array]", _("The row is not initialized."));
	if (mDatabase != 0 && array->DatabasePtr() != mDatabase)
		throw LogicExceptionImpl("Row::Set[Array]",
			_("IArray and Row attached to different databases"));
	if (mTransaction != 0 && array->TransactionPtr() != mTransaction)
		throw LogicExceptionImpl("Row::Set[Array]",
			_("IArray and Row attached to different transactions"));

	SetValue(param, ivArray, (void*)array.intf());
	mUpdated[param-1] = true;
}

void RowImpl::Set(int param, const IBPP::DBKey& key)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[DBKey]", _("The row is not initialized."));

	SetValue(param, ivDBKey, (void*)&key);
	mUpdated[param-1] = true;
}

/*
void RowImpl::Set(int param, const IBPP::Value& value)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Set[Value]", _("The row is not initialized."));

	//SetValue(param, ivDBKey, (void*)&key);
	//mUpdated[param-1] = true;
}
*/

bool RowImpl::IsNull(int column)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::IsNull", _("The row is not initialized."));
	if (column < 1 || column > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::IsNull", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[column-1]);
	return ((var->sqltype & 1) && *(var->sqlind) != 0) ? true : false;
}

bool RowImpl::Get(int column, bool& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivBool);
	if (pvalue != 0)
		retvalue = (*(char*)pvalue == 0 ? false : true);
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, char* retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));
	if (retvalue == 0)
		throw LogicExceptionImpl("Row::Get", _("Null pointer detected"));

	int sqllen;
	void* pvalue = GetValue(column, ivByte, &sqllen);
	if (pvalue != 0)
	{
		memcpy(retvalue, pvalue, sqllen);
		retvalue[sqllen] = '\0';
	}
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, void* bindata, int& userlen)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));
	if (bindata == 0)
		throw LogicExceptionImpl("Row::Get", _("Null pointer detected"));
	if (userlen < 0)
		throw LogicExceptionImpl("Row::Get", _("Length must be >= 0"));

	int sqllen;
	void* pvalue = GetValue(column, ivByte, &sqllen);
	if (pvalue != 0)
	{
		// userlen says how much bytes the user can accept
		// let's shorten it, if there is less bytes available
		if (sqllen < userlen) userlen = sqllen;
		memcpy(bindata, pvalue, userlen);
	}
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, std::string& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivString, &retvalue);
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, int16_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivInt16);
	if (pvalue != 0)
		retvalue = *(int16_t*)pvalue;
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, int32_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivInt32);
	if (pvalue != 0)
		retvalue = *(int32_t*)pvalue;
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, int64_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivInt64);
	if (pvalue != 0)
		retvalue = *(int64_t*)pvalue;
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, float& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivFloat);
	if (pvalue != 0)
		retvalue = *(float*)pvalue;
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, double& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivDouble);
	if (pvalue != 0)
		retvalue = *(double*)pvalue;
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, IBPP::Timestamp& timestamp)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivTimestamp, (void*)&timestamp);
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, IBPP::Date& date)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	if (mDialect == 1)
	{
		// Dialect 1. IBPP::Date is supposed to work with old 'DATE'
		// fields which are actually ISC_TIMESTAMP.
		IBPP::Timestamp timestamp;
		void* pvalue = GetValue(column, ivTimestamp, (void*)&timestamp);
		if (pvalue != 0) date = timestamp;
		return pvalue == 0 ? true : false;
	}
	else
	{
		void* pvalue = GetValue(column, ivDate, (void*)&date);
		return pvalue == 0 ? true : false;
	}
}

bool RowImpl::Get(int column, IBPP::Time& time)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivTime, (void*)&time);
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, IBPP::Blob& retblob)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivBlob, (void*)retblob.intf());
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, IBPP::DBKey& retkey)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivDBKey, (void*)&retkey);
	return pvalue == 0 ? true : false;
}

bool RowImpl::Get(int column, IBPP::Array& retarray)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	void* pvalue = GetValue(column, ivArray, (void*)retarray.intf());
	return pvalue == 0 ? true : false;
}

/*
const IBPP::Value RowImpl::Get(int column)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	//void* value = GetValue(column, ivArray, (void*)retarray.intf());
	//return value == 0 ? true : false;
	return IBPP::Value();
}
*/

bool RowImpl::IsNull(const std::string& name)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::IsNull", _("The row is not initialized."));

	return IsNull(ColumnNum(name));
}

bool RowImpl::Get(const std::string& name, bool& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, char* retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get[char*]", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, void* retvalue, int& count)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get[void*,int]", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue, count);
}

bool RowImpl::Get(const std::string& name, std::string& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::GetString", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, int16_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, int32_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, int64_t& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, float& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, double& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, IBPP::Timestamp& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, IBPP::Date& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, IBPP::Time& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string&name, IBPP::Blob& retblob)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retblob);
}

bool RowImpl::Get(const std::string& name, IBPP::DBKey& retvalue)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retvalue);
}

bool RowImpl::Get(const std::string& name, IBPP::Array& retarray)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name), retarray);
}

/*
const IBPP::Value RowImpl::Get(const std::string& name)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Get", _("The row is not initialized."));

	return Get(ColumnNum(name));
}
*/

int RowImpl::Columns()
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::Columns", _("The row is not initialized."));

	return mDescrArea->sqld;
}

int RowImpl::ColumnNum(const std::string& name)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnNum", _("The row is not initialized."));
	if (name.empty())
		throw LogicExceptionImpl("Row::ColumnNum", _("Column name <empty> not found."));

	XSQLVAR* var;
	char Uname[sizeof(var->sqlname)+1];		// Max size of sqlname + '\0'

	// Local upper case copy of the column name
	size_t len = name.length();
	if (len > sizeof(var->sqlname)) len = sizeof(var->sqlname);
	strncpy(Uname, name.c_str(), len);
	Uname[len] = '\0';
	char* p = Uname;
	while (*p != '\0') { *p = char(toupper(*p)); ++p; }

	// Loop through the columns of the descriptor
	for (int i = 0; i < mDescrArea->sqld; i++)
	{
		var = &(mDescrArea->sqlvar[i]);
		if (var->sqlname_length != (int16_t)len) continue;
		if (strncmp(Uname, var->sqlname, len) == 0) return i+1;
	}

	// Failed finding the column name, let's retry using the aliases
	char Ualias[sizeof(var->aliasname)+1];		// Max size of aliasname + '\0'

	// Local upper case copy of the column name
	len = name.length();
	if (len > sizeof(var->aliasname)) len = sizeof(var->aliasname);
	strncpy(Ualias, name.c_str(), len);
	Ualias[len] = '\0';
	p = Ualias;
	while (*p != '\0') { *p = char(toupper(*p)); ++p; }

	// Loop through the columns of the descriptor
	for (int i = 0; i < mDescrArea->sqld; i++)
	{
		var = &(mDescrArea->sqlvar[i]);
		if (var->aliasname_length != (int16_t)len) continue;
		if (strncmp(Ualias, var->aliasname, len) == 0) return i+1;
	}

	throw LogicExceptionImpl("Row::ColumnNum", _("Could not find matching column."));
#ifdef __DMC__
	return 0;	// DMC errronously warns here about a missing return
#endif
}

/*
ColumnName, ColumnAlias, ColumnTable : all these 3 have a mistake.
Ideally, the strings should be stored elsewhere (like _Numerics and so on) to
take into account the final '\0' which needs to be added. For now, we insert
the '\0' in the original data, which will cut the 32th character. Not terribly
bad, but should be cleanly rewritten.
*/

const char* RowImpl::ColumnName(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnName", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumName", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
	if (var->sqlname_length >= 31) var->sqlname_length = 31;
	var->sqlname[var->sqlname_length] = '\0';
	return var->sqlname;
}

const char* RowImpl::ColumnAlias(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnAlias", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnAlias", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
	if (var->aliasname_length >= 31) var->aliasname_length = 31;
	var->aliasname[var->aliasname_length] = '\0';
	return var->aliasname;
}

const char* RowImpl::ColumnTable(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnTable", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnTable", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
	if (var->relname_length >= 31) var->relname_length = 31;
	var->relname[var->relname_length] = '\0';
	return var->relname;
}

IBPP::SDT RowImpl::ColumnType(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnType", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnType", _("Variable index out of range."));

	IBPP::SDT value;
	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);

	switch (var->sqltype & ~1)
	{
		case SQL_TEXT :      value = IBPP::sdString;    break;
		case SQL_VARYING :   value = IBPP::sdString;    break;
		case SQL_SHORT :     value = IBPP::sdSmallint;  break;
		case SQL_LONG :      value = IBPP::sdInteger;   break;
		case SQL_INT64 :     value = IBPP::sdLargeint;  break;
		case SQL_FLOAT :     value = IBPP::sdFloat;     break;
		case SQL_DOUBLE :    value = IBPP::sdDouble;    break;
		case SQL_TIMESTAMP : value = IBPP::sdTimestamp; break;
		case SQL_TYPE_DATE : value = IBPP::sdDate;      break;
		case SQL_TYPE_TIME : value = IBPP::sdTime;      break;
		case SQL_BLOB :      value = IBPP::sdBlob;      break;
		case SQL_ARRAY :     value = IBPP::sdArray;     break;
		default : throw LogicExceptionImpl("Row::ColumnType",
						_("Found an unknown sqltype !"));
	}

	return value;
}

int RowImpl::ColumnSubtype(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnSubtype", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnSubtype", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
	return (int)var->sqlsubtype;
}

int RowImpl::ColumnSize(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnSize", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnSize", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
    return var->sqllen;
}

int RowImpl::ColumnScale(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnScale", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnScale", _("Variable index out of range."));

	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
    return -var->sqlscale;
}

bool RowImpl::ColumnUpdated(int varnum)
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnUpdated", _("The row is not initialized."));
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("Row::ColumnUpdated", _("Variable index out of range."));

	return mUpdated[varnum-1];
}

bool RowImpl::Updated()
{
	if (mDescrArea == 0)
		throw LogicExceptionImpl("Row::ColumnUpdated", _("The row is not initialized."));

	for (int i = 0; i < mDescrArea->sqld; i++)
		if (mUpdated[i]) return true;
	return false;
}

IBPP::Database RowImpl::DatabasePtr() const
{
	return mDatabase;
}

IBPP::Transaction RowImpl::TransactionPtr() const
{
	return mTransaction;
}

IBPP::IRow* RowImpl::Clone()
{
	// By definition the clone of an IBPP Row is a new row (so refcount=0).

	RowImpl* clone = new RowImpl(*this);
	return clone;
}

IBPP::IRow* RowImpl::AddRef()
{
	ASSERTION(mRefCount >= 0);
	++mRefCount;
	return this;
}

void RowImpl::Release()
{
	// Release cannot throw, except in DEBUG builds on assertion
	ASSERTION(mRefCount >= 0);
	--mRefCount;
	try { if (mRefCount <= 0) delete this; }
		catch (...) { }
}

//	(((((((( OBJECT INTERNAL METHODS ))))))))

void RowImpl::SetValue(int varnum, IITYPE ivType, const void* value, int userlen)
{
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("RowImpl::SetValue", _("Variable index out of range."));
	if (value == 0)
		throw LogicExceptionImpl("RowImpl::SetValue", _("Unexpected null pointer detected."));

	int16_t len;
	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);
	switch (var->sqltype & ~1)
	{
		case SQL_TEXT :
			if (ivType == ivString)
			{
				std::string* svalue = (std::string*)value;
				len = (int16_t)svalue->length();
				if (len > var->sqllen) len = var->sqllen;
				strncpy(var->sqldata, svalue->c_str(), len);
				while (len < var->sqllen) var->sqldata[len++] = ' ';
			}
			else if (ivType == ivByte)
			{
				if (userlen > var->sqllen) userlen = var->sqllen;
				memcpy(var->sqldata, value, userlen);
				while (userlen < var->sqllen) var->sqldata[userlen++] = ' ';
			}
			else if (ivType == ivDBKey)
			{
				IBPP::DBKey* key = (IBPP::DBKey*)value;
				key->GetKey(var->sqldata, var->sqllen);
			}
			else if (ivType == ivBool)
			{
				var->sqldata[0] = *(bool*)value ? 'T' : 'F';
				len = 1;
				while (len < var->sqllen) var->sqldata[len++] = ' ';
			}
			else throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_VARYING :
			if (ivType == ivString)
			{
				std::string* svalue = (std::string*)value;
				len = (int16_t)svalue->length();
				if (len > var->sqllen) len = var->sqllen;
				*(int16_t*)var->sqldata = (int16_t)len;
				strncpy(var->sqldata+2, svalue->c_str(), len);
			}
			else if (ivType == ivByte)
			{
				if (userlen > var->sqllen) userlen = var->sqllen;
				*(int16_t*)var->sqldata = (int16_t)userlen;
				memcpy(var->sqldata+2, value, userlen);
			}
			else if (ivType == ivBool)
			{
				*(int16_t*)var->sqldata = (int16_t)1;
				var->sqldata[2] = *(bool*)value ? 'T' : 'F';
			}
			else throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_SHORT :
			if (ivType == ivBool)
			{
				*(int16_t*)var->sqldata = int16_t(*(bool*)value ? 1 : 0);
			}
			else if (ivType == ivInt16)
			{
				*(int16_t*)var->sqldata = *(int16_t*)value;
			}
			else if (ivType == ivInt32)
			{
				if (*(int32_t*)value < consts::min16 || *(int32_t*)value > consts::max16)
					throw LogicExceptionImpl("RowImpl::SetValue",
						_("Out of range numeric conversion !"));
				*(int16_t*)var->sqldata = (int16_t)*(int32_t*)value;
			}
			else if (ivType == ivInt64)
			{
				if (*(int64_t*)value < consts::min16 || *(int64_t*)value > consts::max16)
					throw LogicExceptionImpl("RowImpl::SetValue",
						_("Out of range numeric conversion !"));
				*(int16_t*)var->sqldata = (int16_t)*(int64_t*)value;
			}
			else if (ivType == ivFloat)
			{
				// This SQL_SHORT is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(int16_t*)var->sqldata =
					(int16_t)floor(*(float*)value * multiplier + 0.5);
			}
			else if (ivType == ivDouble)
			{
				// This SQL_SHORT is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(int16_t*)var->sqldata =
					(int16_t)floor(*(double*)value * multiplier + 0.5);
			}
			else throw WrongTypeImpl("RowImpl::SetValue",  var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_LONG :
			if (ivType == ivBool)
			{
				*(ISC_LONG*)var->sqldata = *(bool*)value ? 1 : 0;
			}
			else if (ivType == ivInt16)
			{
				*(ISC_LONG*)var->sqldata = *(int16_t*)value;
			}
			else if (ivType == ivInt32)
			{
				*(ISC_LONG*)var->sqldata = *(ISC_LONG*)value;
			}
			else if (ivType == ivInt64)
			{
				if (*(int64_t*)value < consts::min32 || *(int64_t*)value > consts::max32)
					throw LogicExceptionImpl("RowImpl::SetValue",
						_("Out of range numeric conversion !"));
				*(ISC_LONG*)var->sqldata = (ISC_LONG)*(int64_t*)value;
			}
			else if (ivType == ivFloat)
			{
				// This SQL_LONG is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(ISC_LONG*)var->sqldata =
					(ISC_LONG)floor(*(float*)value * multiplier + 0.5);
			}
			else if (ivType == ivDouble)
			{
				// This SQL_LONG is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(ISC_LONG*)var->sqldata =
					(ISC_LONG)floor(*(double*)value * multiplier + 0.5);
			}
			else throw WrongTypeImpl("RowImpl::SetValue",  var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_INT64 :
			if (ivType == ivBool)
			{
				*(int64_t*)var->sqldata = *(bool*)value ? 1 : 0;
			}
			else if (ivType == ivInt16)
			{
				*(int64_t*)var->sqldata = *(int16_t*)value;
			}
			else if (ivType == ivInt32)
			{
				*(int64_t*)var->sqldata = *(int32_t*)value;
			}
			else if (ivType == ivInt64)
			{
				*(int64_t*)var->sqldata = *(int64_t*)value;
			}
			else if (ivType == ivFloat)
			{
				// This SQL_INT64 is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(int64_t*)var->sqldata =
					(int64_t)floor(*(float*)value * multiplier + 0.5);
			}
			else if (ivType == ivDouble)
			{
				// This SQL_INT64 is a NUMERIC(x,y), scale it !
				double multiplier = consts::dscales[-var->sqlscale];
				*(int64_t*)var->sqldata =
					(int64_t)floor(*(double*)value * multiplier + 0.5);
			}
			else throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_FLOAT :
			if (ivType != ivFloat || var->sqlscale != 0)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
									_("Incompatible types."));
			*(float*)var->sqldata = *(float*)value;
			break;

		case SQL_DOUBLE :
			if (ivType != ivDouble)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			if (var->sqlscale != 0)
			{
				// Round to scale of NUMERIC(x,y)
				double multiplier = consts::dscales[-var->sqlscale];
				*(double*)var->sqldata =
					floor(*(double*)value * multiplier + 0.5) / multiplier;
			}
			else *(double*)var->sqldata = *(double*)value;
			break;

		case SQL_TIMESTAMP :
			if (ivType != ivTimestamp)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			encodeTimestamp(*(ISC_TIMESTAMP*)var->sqldata, *(IBPP::Timestamp*)value);
			break;

		case SQL_TYPE_DATE :
			if (ivType != ivDate)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			encodeDate(*(ISC_DATE*)var->sqldata, *(IBPP::Date*)value);
			break;

		case SQL_TYPE_TIME :
			if (ivType != ivTime)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
											_("Incompatible types."));
			encodeTime(*(ISC_TIME*)var->sqldata, *(IBPP::Time*)value);
			break;

		case SQL_BLOB :
			if (ivType == ivBlob)
			{
				BlobImpl* blob = (BlobImpl*)value;
				blob->GetId((ISC_QUAD*)var->sqldata);
			}
			else if (ivType == ivString)
			{
				BlobImpl blob(mDatabase, mTransaction);
				blob.Save(*(std::string*)value);
				blob.GetId((ISC_QUAD*)var->sqldata);
			}
			else throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_ARRAY :
			if (ivType != ivArray)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			{
				ArrayImpl* array = (ArrayImpl*)value;
				array->GetId((ISC_QUAD*)var->sqldata);
				// When an array has been affected to a column, we want to reset
				// its ID. This way, the next WriteFrom() on the same Array object
				// will allocate a new ID. This protects against storing the same
				// array ID in multiple columns or rows.
				array->ResetId();
			}
			break;

		default : throw LogicExceptionImpl("RowImpl::SetValue",
						_("The field uses an unsupported SQL type !"));
	}

	if (var->sqltype & 1) *var->sqlind = 0;		// Remove the 0 flag
}

void* RowImpl::GetValue(int varnum, IITYPE ivType, void* retvalue)
{
	if (varnum < 1 || varnum > mDescrArea->sqld)
		throw LogicExceptionImpl("RowImpl::GetValue", _("Variable index out of range."));

	void* value;
	int len;
	XSQLVAR* var = &(mDescrArea->sqlvar[varnum-1]);

	// When there is no value (SQL NULL)
	if ((var->sqltype & 1) && *(var->sqlind) != 0) return 0;

	switch (var->sqltype & ~1)
	{
		case SQL_TEXT :
			if (ivType == ivString)
			{
				// In case of ivString, 'void* retvalue' points to a std::string where we
				// will directly store the data.
				std::string* str = (std::string*)retvalue;
				str->erase();
				str->append(var->sqldata, var->sqllen);
				value = retvalue;	// value != 0 means 'not null'
			}
			else if (ivType == ivByte)
			{
				// In case of ivByte, void* retvalue points to an int where we
				// will store the len of the available data
				if (retvalue != 0) *(int*)retvalue = var->sqllen;
				value = var->sqldata;
			}
			else if (ivType == ivDBKey)
			{
				IBPP::DBKey* key = (IBPP::DBKey*)retvalue;
				key->SetKey(var->sqldata, var->sqllen);
				value = retvalue;
			}
			else if (ivType == ivBool)
			{
				mBools[varnum-1] = 0;
				if (var->sqllen >= 1)
				{
					char c = var->sqldata[0];
					if (c == 't' || c == 'T' || c == 'y' || c == 'Y' ||	c == '1')
						mBools[varnum-1] = 1;
				}
				value = &mBools[varnum-1];
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_VARYING :
			if (ivType == ivString)
			{
				// In case of ivString, 'void* retvalue' points to a std::string where we
				// will directly store the data.
				std::string* str = (std::string*)retvalue;
				str->erase();
				str->append(var->sqldata+2, (int32_t)*(int16_t*)var->sqldata);
				value = retvalue;
			}
			else if (ivType == ivByte)
			{
				// In case of ivByte, void* retvalue points to an int where we
				// will store the len of the available data
				if (retvalue != 0) *(int*)retvalue = (int)*(int16_t*)var->sqldata;
				value = var->sqldata+2;
			}
			else if (ivType == ivBool)
			{
				mBools[varnum-1] = 0;
				len = *(int16_t*)var->sqldata;
				if (len >= 1)
				{
					char c = var->sqldata[2];
					if (c == 't' || c == 'T' || c == 'y' || c == 'Y' ||	c == '1')
						mBools[varnum-1] = 1;
				}
				value = &mBools[varnum-1];
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_SHORT :
			if (ivType == ivInt16)
			{
				value = var->sqldata;
			}
			else if (ivType == ivBool)
			{
				if (*(int16_t*)var->sqldata == 0) mBools[varnum-1] = 0;
				else mBools[varnum-1] = 1;
				value = &mBools[varnum-1];
			}
			else if (ivType == ivInt32)
			{
				mInt32s[varnum-1] = *(int16_t*)var->sqldata;
				value = &mInt32s[varnum-1];
			}
			else if (ivType == ivInt64)
			{
				mInt64s[varnum-1] = *(int16_t*)var->sqldata;
				value = &mInt64s[varnum-1];
			}
			else if (ivType == ivFloat)
			{
				// This SQL_SHORT is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mFloats[varnum-1] = (float)(*(int16_t*)var->sqldata / divisor);

				value = &mFloats[varnum-1];
			}
			else if (ivType == ivDouble)
			{
				// This SQL_SHORT is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mNumerics[varnum-1] = *(int16_t*)var->sqldata / divisor;
				value = &mNumerics[varnum-1];
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_LONG :
			if (ivType == ivInt32)
			{
				value = var->sqldata;
			}
			else if (ivType == ivBool)
			{
				if (*(int32_t*)var->sqldata == 0) mBools[varnum-1] = 0;
				else mBools[varnum-1] = 1;
				value = &mBools[varnum-1];
			}
			else if (ivType == ivInt16)
			{
				int32_t tmp = *(int32_t*)var->sqldata;
				if (tmp < consts::min16 || tmp > consts::max16)
					throw LogicExceptionImpl("RowImpl::GetValue",
						_("Out of range numeric conversion !"));
				mInt16s[varnum-1] = (int16_t)tmp;
				value = &mInt16s[varnum-1];
			}
			else if (ivType == ivInt64)
			{
				mInt64s[varnum-1] = *(int32_t*)var->sqldata;
				value = &mInt64s[varnum-1];
			}
			else if (ivType == ivFloat)
			{
				// This SQL_LONG is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mFloats[varnum-1] = (float)(*(int32_t*)var->sqldata / divisor);
				value = &mFloats[varnum-1];
			}
			else if (ivType == ivDouble)
			{
				// This SQL_LONG is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mNumerics[varnum-1] = *(int32_t*)var->sqldata / divisor;
				value = &mNumerics[varnum-1];
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_INT64 :
			if (ivType == ivInt64)
			{
				value = var->sqldata;
			}
			else if (ivType == ivBool)
			{
				if (*(int64_t*)var->sqldata == 0) mBools[varnum-1] = 0;
				else mBools[varnum-1] = 1;
				value = &mBools[varnum-1];
			}
			else if (ivType == ivInt16)
			{
				int64_t tmp = *(int64_t*)var->sqldata;
				if (tmp < consts::min16 || tmp > consts::max16)
					throw LogicExceptionImpl("RowImpl::GetValue",
						_("Out of range numeric conversion !"));
				mInt16s[varnum-1] = (int16_t)tmp;
				value = &mInt16s[varnum-1];
			}
			else if (ivType == ivInt32)
			{
				int64_t tmp = *(int64_t*)var->sqldata;
				if (tmp < consts::min32 || tmp > consts::max32)
					throw LogicExceptionImpl("RowImpl::GetValue",
						_("Out of range numeric conversion !"));
				mInt32s[varnum-1] = (int32_t)tmp;
				value = &mInt32s[varnum-1];
			}
			else if (ivType == ivFloat)
			{
				// This SQL_INT64 is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mFloats[varnum-1] = (float)(*(int64_t*)var->sqldata / divisor);
				value = &mFloats[varnum-1];
			}
			else if (ivType == ivDouble)
			{
				// This SQL_INT64 is a NUMERIC(x,y), scale it !
				double divisor = consts::dscales[-var->sqlscale];
				mNumerics[varnum-1] = *(int64_t*)var->sqldata / divisor;
				value = &mNumerics[varnum-1];
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;

		case SQL_FLOAT :
			if (ivType != ivFloat)
				throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			value = var->sqldata;
			break;

		case SQL_DOUBLE :
			if (ivType != ivDouble)
				throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			if (var->sqlscale != 0)
			{
				// Round to scale y of NUMERIC(x,y)
				double multiplier = consts::dscales[-var->sqlscale];
				mNumerics[varnum-1] =
					floor(*(double*)var->sqldata * multiplier + 0.5) / multiplier;
				value = &mNumerics[varnum-1];
			}
			else value = var->sqldata;
			break;

		case SQL_TIMESTAMP :
			if (ivType != ivTimestamp)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			decodeTimestamp(*(IBPP::Timestamp*)retvalue, *(ISC_TIMESTAMP*)var->sqldata);
			value = retvalue;
			break;

		case SQL_TYPE_DATE :
			if (ivType != ivDate)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			decodeDate(*(IBPP::Date*)retvalue, *(ISC_DATE*)var->sqldata);
			value = retvalue;
			break;

		case SQL_TYPE_TIME :
			if (ivType != ivTime)
				throw WrongTypeImpl("RowImpl::SetValue", var->sqltype, ivType,
										_("Incompatible types."));
			decodeTime(*(IBPP::Time*)retvalue, *(ISC_TIME*)var->sqldata);
			value = retvalue;
			break;

		case SQL_BLOB :
			if (ivType == ivBlob)
			{
				BlobImpl* blob = (BlobImpl*)retvalue;
				blob->SetId((ISC_QUAD*)var->sqldata);
				value = retvalue;
			}
			else if (ivType == ivString)
			{
				BlobImpl blob(mDatabase, mTransaction);
				blob.SetId((ISC_QUAD*)var->sqldata);
				std::string* str = (std::string*)retvalue;
				blob.Load(*str);
				value = retvalue;
			}
			else throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
										_("Incompatible types."));
			break;
			
		case SQL_ARRAY :
			if (ivType != ivArray)
				throw WrongTypeImpl("RowImpl::GetValue", var->sqltype, ivType,
											_("Incompatible types."));
			{
				ArrayImpl* array = (ArrayImpl*)retvalue;
				array->SetId((ISC_QUAD*)var->sqldata);
				value = retvalue;
			}
			break;

		default : throw LogicExceptionImpl("RowImpl::GetValue",
						_("Found an unknown sqltype !"));
	}

	return value;
}

void RowImpl::Free()
{
	if (mDescrArea != 0)
	{
		for (int i = 0; i < mDescrArea->sqln; i++)
		{
			XSQLVAR* var = &(mDescrArea->sqlvar[i]);
			if (var->sqldata != 0)
			{
				switch (var->sqltype & ~1)
				{
					case SQL_ARRAY :
					case SQL_BLOB :		delete (ISC_QUAD*) var->sqldata; break;
					case SQL_TIMESTAMP :delete (ISC_TIMESTAMP*) var->sqldata; break;
					case SQL_TYPE_TIME :delete (ISC_TIME*) var->sqldata; break;
					case SQL_TYPE_DATE :delete (ISC_DATE*) var->sqldata; break;
					case SQL_TEXT :
					case SQL_VARYING :	delete [] var->sqldata; break;
					case SQL_SHORT :	delete (int16_t*) var->sqldata; break;
					case SQL_LONG :		delete (int32_t*) var->sqldata; break;
					case SQL_INT64 :	delete (int64_t*) var->sqldata; break;
					case SQL_FLOAT : 	delete (float*) var->sqldata; break;
					case SQL_DOUBLE :	delete (double*) var->sqldata; break;
					default : throw LogicExceptionImpl("RowImpl::Free",
								_("Found an unknown sqltype !"));
				}
			}
			if (var->sqlind != 0) delete var->sqlind;
		}
		delete [] (char*)mDescrArea;
		mDescrArea = 0;
	}

	mNumerics.clear();
	mFloats.clear();
	mInt64s.clear();
	mInt32s.clear();
	mInt16s.clear();
	mBools.clear();
	mStrings.clear();
	mUpdated.clear();

	mDialect = 0;
	mDatabase = 0;
	mTransaction = 0;
}

void RowImpl::Resize(int n)
{
	const int size = XSQLDA_LENGTH(n);

	Free();
    mDescrArea = (XSQLDA*) new char[size];

	memset(mDescrArea, 0, size);
	mNumerics.resize(n);
	mFloats.resize(n);
	mInt64s.resize(n);
	mInt32s.resize(n);
	mInt16s.resize(n);
	mBools.resize(n);
	mStrings.resize(n);
	mUpdated.resize(n);
	for (int i = 0; i < n; i++)
	{
		mNumerics[i] = 0.0;
		mFloats[i] = 0.0;
		mInt64s[i] = 0;
		mInt32s[i] = 0;
		mInt16s[i] = 0;
		mBools[i] = 0;
		mStrings[i].erase();
		mUpdated[i] = false;
	}

	mDescrArea->version = SQLDA_VERSION1;
	mDescrArea->sqln = (int16_t)n;
}

void RowImpl::AllocVariables()
{
	int i;
	for (i = 0; i < mDescrArea->sqld; i++)
	{
		XSQLVAR* var = &(mDescrArea->sqlvar[i]);
		switch (var->sqltype & ~1)
		{
			case SQL_ARRAY :
			case SQL_BLOB :		var->sqldata = (char*) new ISC_QUAD;
								memset(var->sqldata, 0, sizeof(ISC_QUAD));
								break;
			case SQL_TIMESTAMP :var->sqldata = (char*) new ISC_TIMESTAMP;
								memset(var->sqldata, 0, sizeof(ISC_TIMESTAMP));
								break;
			case SQL_TYPE_TIME :var->sqldata = (char*) new ISC_TIME;
								memset(var->sqldata, 0, sizeof(ISC_TIME));
								break;
			case SQL_TYPE_DATE :var->sqldata = (char*) new ISC_DATE;
								memset(var->sqldata, 0, sizeof(ISC_DATE));
								break;
			case SQL_TEXT :		var->sqldata = new char[var->sqllen+1];
								memset(var->sqldata, ' ', var->sqllen);
								var->sqldata[var->sqllen] = '\0';
								break;
			case SQL_VARYING :	var->sqldata = new char[var->sqllen+3];
								memset(var->sqldata, 0, 2);
								memset(var->sqldata+2, ' ', var->sqllen);
								var->sqldata[var->sqllen+2] = '\0';
								break;
			case SQL_SHORT :	var->sqldata = (char*) new int16_t(0); break;
			case SQL_LONG :		var->sqldata = (char*) new int32_t(0); break;
			case SQL_INT64 :	var->sqldata = (char*) new int64_t(0); break;
			case SQL_FLOAT : 	var->sqldata = (char*) new float(0.0); break;
			case SQL_DOUBLE :	var->sqldata = (char*) new double(0.0); break;
			default : throw LogicExceptionImpl("RowImpl::AllocVariables",
						_("Found an unknown sqltype !"));
		}
		if (var->sqltype & 1) var->sqlind = new short(-1);	// 0 indicator
	}
}

bool RowImpl::MissingValues()
{
	for (int i = 0; i < mDescrArea->sqld; i++)
		if (! mUpdated[i]) return true;
	return false;
}

RowImpl& RowImpl::operator=(const RowImpl& copied)
{
	Free();

	const int n = copied.mDescrArea->sqln;
	const int size = XSQLDA_LENGTH(n);

	// Initial brute copy
    mDescrArea = (XSQLDA*) new char[size];
	memcpy(mDescrArea, copied.mDescrArea, size);

	// Copy of the columns data
	for (int i = 0; i < mDescrArea->sqld; i++)
	{
		XSQLVAR* var = &(mDescrArea->sqlvar[i]);
		XSQLVAR* org = &(copied.mDescrArea->sqlvar[i]);
		switch (var->sqltype & ~1)
		{
			case SQL_ARRAY :
			case SQL_BLOB :		var->sqldata = (char*) new ISC_QUAD;
								memcpy(var->sqldata, org->sqldata, sizeof(ISC_QUAD));
								break;
			case SQL_TIMESTAMP :var->sqldata = (char*) new ISC_TIMESTAMP;
								memcpy(var->sqldata, org->sqldata, sizeof(ISC_TIMESTAMP));
								break;
			case SQL_TYPE_TIME :var->sqldata = (char*) new ISC_TIME;
								memcpy(var->sqldata, org->sqldata, sizeof(ISC_TIME));
								break;
			case SQL_TYPE_DATE :var->sqldata = (char*) new ISC_DATE;
								memcpy(var->sqldata, org->sqldata, sizeof(ISC_DATE));
								break;
			case SQL_TEXT :		var->sqldata = new char[var->sqllen+1];
								memcpy(var->sqldata, org->sqldata, var->sqllen+1);
								break;
			case SQL_VARYING :	var->sqldata = new char[var->sqllen+3];
								memcpy(var->sqldata, org->sqldata, var->sqllen+3);
								break;
			case SQL_SHORT :	var->sqldata = (char*) new int16_t(*(int16_t*)org->sqldata); break;
			case SQL_LONG :		var->sqldata = (char*) new int32_t(*(int32_t*)org->sqldata); break;
			case SQL_INT64 :	var->sqldata = (char*) new int64_t(*(int64_t*)org->sqldata); break;
			case SQL_FLOAT : 	var->sqldata = (char*) new float(*(float*)org->sqldata); break;
			case SQL_DOUBLE :	var->sqldata = (char*) new double(*(double*)org->sqldata); break;
			default : throw LogicExceptionImpl("RowImpl::Ctor",
						_("Found an unknown sqltype !"));
		}
		if (var->sqltype & 1) var->sqlind = new short(*org->sqlind);	// 0 indicator
	}

	// Pointers init, real data copy
	mNumerics = copied.mNumerics;
	mFloats = copied.mFloats;
	mInt64s = copied.mInt64s;
	mInt32s = copied.mInt32s;
	mInt16s = copied.mInt16s;
	mBools = copied.mBools;
	mStrings = copied.mStrings;

	mDialect = copied.mDialect;
	mDatabase = copied.mDatabase;
	mTransaction = copied.mTransaction;
	
	return *this;
}

RowImpl::RowImpl(const RowImpl& copied)
	: IBPP::IRow(), mRefCount(0), mDescrArea(0)
{
	// mRefCount and mDescrArea are set to 0 before using the assignment operator
	*this = copied;		// The assignment operator does the real copy
}

RowImpl::RowImpl(int dialect, int n, DatabaseImpl* db, TransactionImpl* tr)
	: mRefCount(0), mDescrArea(0)
{
	Resize(n);
	mDialect = dialect;
	mDatabase = db;
	mTransaction = tr;
}

RowImpl::~RowImpl()
{
	try { Free(); }
		catch (...) { }
}

//
//	EOF
//
