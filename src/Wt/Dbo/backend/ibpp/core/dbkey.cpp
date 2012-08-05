///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: dbkey.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, DBKey class implementation
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

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace ibpp_internals;

//	Private implementation

//	Public implementation

void IBPP::DBKey::Clear()
{
	mDBKey.erase();
	mString.erase();
}

void IBPP::DBKey::SetKey(const void* key, int size)
{
	if (key == 0)
		throw LogicExceptionImpl("IBPP::DBKey::SetKey", _("Null DBKey reference detected."));
	if (size <= 0 || ((size >> 3) << 3) != size)
		throw LogicExceptionImpl("IBPP::DBKey::SetKey", _("Invalid DBKey size."));

	mDBKey.assign((const char*)key, (size_t)size);
	mString.erase();
}

void IBPP::DBKey::GetKey(void* key, int size) const
{
	if (mDBKey.empty())
		throw LogicExceptionImpl("IBPP::DBKey::GetKey", _("DBKey not assigned."));
	if (key == 0)
		throw LogicExceptionImpl("IBPP::DBKey::GetKey", _("Null DBKey reference detected."));
	if (size != (int)mDBKey.size())
		throw LogicExceptionImpl("IBPP::DBKey::GetKey", _("Incompatible DBKey size detected."));

	mDBKey.copy((char*)key, mDBKey.size());
}

const char* IBPP::DBKey::AsString() const
{
	if (mDBKey.empty())
		throw LogicExceptionImpl("IBPP::DBKey::GetString", _("DBKey not assigned."));

	if (mString.empty())
	{
		std::ostringstream hexkey;
		hexkey.setf(std::ios::hex, std::ios::basefield);
		hexkey.setf(std::ios::uppercase);

		const uint32_t* key = reinterpret_cast<const uint32_t*>(mDBKey.data());
		int n = (int)mDBKey.size() / 8;
		for (int i = 0; i < n; i++)
		{
			if (i != 0) hexkey<< "-";
			hexkey<< std::setw(4)<< key[i*2]<< ":";
			hexkey<< std::setw(8)<< key[i*2+1];
		}

		mString = hexkey.str();
	}

	return mString.c_str();
}

IBPP::DBKey::DBKey(const DBKey& copied)
{
	mDBKey = copied.mDBKey;
	mString = copied.mString;
}

IBPP::DBKey& IBPP::DBKey::operator=(const IBPP::DBKey& assigned)
{
	mDBKey = assigned.mDBKey;
	mString = assigned.mString;
	return *this;
}

//
//	EOF
//
