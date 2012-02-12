///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: user.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, User class implementation
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

void IBPP::User::copyfrom(const IBPP::User& r)
{
	username = r.username;
	password = r.password;
	firstname = r.firstname;
	middlename = r.middlename;
	lastname = r.lastname;
	userid = r.userid;
	groupid = r.groupid;
}

//	Public implementation

void IBPP::User::clear()
{
	username.erase(); password.erase();
	firstname.erase(); middlename.erase(); lastname.erase();
	userid = groupid = 0;
}

//
//	EOF
//
