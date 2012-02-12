///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: _ibs.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, internal Status class implementation
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

#include <string.h>

using namespace ibpp_internals;

int IBS::SqlCode() const
{
	return (int)(*gds.Call()->m_sqlcode)(&mVector[0]);
}

const char* IBS::ErrorMessage() const
{
	char msg[1024];
	ISC_LONG sqlcode;

	if (! mMessage.empty()) return mMessage.c_str();	// If message compiled, returns it

	// Compiles the message (SQL part)
	std::ostringstream message;
	sqlcode = (*gds.Call()->m_sqlcode)(mVector);
	if (sqlcode != -999)
	{
		(*gds.Call()->m_sql_interprete)((short)sqlcode, msg, sizeof(msg));
		message<< _("SQL Message : ")<< sqlcode<< "\n"<< msg<< "\n\n";
	}

	message<< _("Engine Code    : ")<< EngineCode()<< "\n";

	// Compiles the message (Engine part)
	ISC_STATUS* error = &mVector[0];
	try { (*gds.Call()->m_interprete)(msg, &error); }
	catch(...) { msg[0] = '\0'; }
	message<< _("Engine Message :")<< "\n"<< msg;
	try
	{
		while ((*gds.Call()->m_interprete)(msg, &error))
			message<< "\n"<< msg;
	}
	catch (...) { }

	message<< "\n";
	mMessage = message.str();
	return mMessage.c_str();
}

void IBS::Reset()
{
	for (int i = 0; i < 20; i++) mVector[i] = 0;
	mMessage.erase();
}

IBS::IBS()
{
	Reset();
}

IBS::~IBS()
{
}

/** Copy Constructor
*/

IBS::IBS(IBS& copied)
{
	memcpy(mVector, copied.mVector, sizeof(mVector));
}

//
//	EOF
//
