///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: _rb.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, internal RB class implementation
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
//	* RB == Result Block/Buffer, see Interbase 6.0 C-API
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

#include <string.h>

using namespace ibpp_internals;

char* RB::FindToken(char token)
{
	char* p = mBuffer;

	while (*p != isc_info_end)
	{
		int len;

		if (*p == token) return p;
		len = (*gds.Call()->m_vax_integer)(p+1, 2);
		p += (len + 3);
	}

	return 0;
}

char* RB::FindToken(char token, char subtoken)
{
	char* p = mBuffer;

	while (*p != isc_info_end)
	{
		int len;

		if (*p == token)
		{
			// Found token, now find subtoken
			int inlen = (*gds.Call()->m_vax_integer)(p+1, 2);
			p += 3;
			while (inlen > 0)
			{
				if (*p == subtoken) return p;
				len = (*gds.Call()->m_vax_integer)(p+1, 2);
				p += (len + 3);
				inlen -= (len + 3);
			}
			return 0;
		}
		len = (*gds.Call()->m_vax_integer)(p+1, 2);
		p += (len + 3);
	}

	return 0;
}

int RB::GetValue(char token)
{
	int value;
	int len;
	char* p = FindToken(token);

	if (p == 0)
		throw LogicExceptionImpl("RB::GetValue", _("Token not found."));

	len = (*gds.Call()->m_vax_integer)(p+1, 2);
	if (len == 0) value = 0;
	else value = (*gds.Call()->m_vax_integer)(p+3, (short)len);

	return value;
}

int RB::GetCountValue(char token)
{
	// Specifically used on tokens like isc_info_insert_count and the like
	// which return detailed counts per relation. We sum up the values.
	int value;
	int len;
	char* p = FindToken(token);

	if (p == 0)
		throw LogicExceptionImpl("RB::GetCountValue", _("Token not found."));

	// len is the number of bytes in the following array
	len = (*gds.Call()->m_vax_integer)(p+1, 2);
	p += 3;
	value = 0;
	while (len > 0)
	{
		// Each array item is 6 bytes : 2 bytes for the relation_id which
		// we skip, and 4 bytes for the count value which we sum up accross
		// all tables.
		value += (*gds.Call()->m_vax_integer)(p+2, 4);
		p += 6;
		len -= 6;
	}

	return value;
}

int RB::GetValue(char token, char subtoken)
{
	int value;
	int len;
	char* p = FindToken(token, subtoken);

	if (p == 0)
		throw LogicExceptionImpl("RB::GetValue", _("Token/Subtoken not found."));

	len = (*gds.Call()->m_vax_integer)(p+1, 2);
	if (len == 0) value = 0;
	else value = (*gds.Call()->m_vax_integer)(p+3, (short)len);

	return value;
}

bool RB::GetBool(char token)
{
	int value;
	char* p = FindToken(token);

	if (p == 0)
		throw LogicExceptionImpl("RB::GetBool", _("Token not found."));

	value = (*gds.Call()->m_vax_integer)(p+1, 4);

	return value == 0 ? false : true;
}

int RB::GetString(char token, std::string& data)
{
	int len;
	char* p = FindToken(token);

	if (p == 0)
		throw LogicExceptionImpl("RB::GetString", _("Token not found."));

	len = (*gds.Call()->m_vax_integer)(p+1, 2);
	data = std::string(p+3, len);
	return len;
}

void RB::Reset()
{
	delete [] mBuffer;
	mBuffer = new char [mSize];
	memset(mBuffer, 255, mSize);
}

RB::RB()
{
	mSize = 1024;
	mBuffer = new char [1024];
	memset(mBuffer, 255, mSize);
}

RB::RB(int Size)
{
	mSize = Size;
	mBuffer = new char [Size];
	memset(mBuffer, 255, mSize);
}

RB::~RB()
{
	try { delete [] mBuffer; }
		catch (...) { }
}

//
//	EOF
//
