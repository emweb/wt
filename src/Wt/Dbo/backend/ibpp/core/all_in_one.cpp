///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: all_in_one.cpp 57 2006-04-02 17:44:00Z epocman $
//	Subject : "All In One" source code file
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
//	* This file is just an "all in one" holder for all the core source files
//	  of IBPP. When you build a project made of each individual source code
//	  files, please DON'T include this one.
//	  Though if you prefer, maybe for better compiler optimizations, you can
//	  compile all of IBPP at once by just compiling this single .cpp file,
//	  which in turn, includes all the others. Presenting such a single
//	  compilation unit to the compiler may help it do better optimizations.
//	  This is just provided for convenience and is in no case required.
//
///////////////////////////////////////////////////////////////////////////////

#include "_ibpp.cpp"
#include "_dpb.cpp"
#include "_ibs.cpp"
#include "_rb.cpp"
#include "_spb.cpp"
#include "_tpb.cpp"

#include "array.cpp"
#include "blob.cpp"
#include "database.cpp"
#include "date.cpp"
#include "dbkey.cpp"
#include "events.cpp"
#include "exception.cpp"
#include "row.cpp"
#include "service.cpp"
#include "statement.cpp"
#include "time.cpp"
#include "transaction.cpp"
#include "user.cpp"

// Eof
