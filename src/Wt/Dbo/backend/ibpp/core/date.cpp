///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: date.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, Date class implementation
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

#include <time.h>		// Can't use <ctime> thanks to MSVC6 buggy library

using namespace ibpp_internals;

void IBPP::Date::Today()
{
	time_t systime = time(0);
	tm* loctime = localtime(&systime);

	if (! IBPP::itod(&mDate, loctime->tm_year + 1900,
		loctime->tm_mon + 1, loctime->tm_mday))
			throw LogicExceptionImpl("Date::Today", _("Out of range"));
}

void IBPP::Date::SetDate(int dt)
{
	if (! IBPP::dtoi(dt, 0, 0, 0))
		throw LogicExceptionImpl("Date::SetDate", _("Out of range"));
	mDate = dt;
}

void IBPP::Date::SetDate(int year, int month, int day)
{
	if (! IBPP::itod(&mDate, year, month, day))
		throw LogicExceptionImpl("Date::SetDate", _("Out of range"));
}

void IBPP::Date::GetDate(int& year, int& month, int& day) const
{
	if (! IBPP::dtoi(mDate, &year, &month, &day))
		throw LogicExceptionImpl("Date::GetDate", _("Out of range"));
}

int IBPP::Date::Year() const
{
	int year;
	if (! IBPP::dtoi(mDate, &year, 0, 0))
		throw LogicExceptionImpl("Date::Year", _("Out of range"));
	return year;
}

int IBPP::Date::Month() const
{
	int month;
	if (! IBPP::dtoi(mDate, 0, &month, 0))
		throw LogicExceptionImpl("Date::Month", _("Out of range"));
	return month;
}

int IBPP::Date::Day() const
{
	int day;
	if (! IBPP::dtoi(mDate, 0, 0, &day))
		throw LogicExceptionImpl("Date::Day", _("Out of range"));
	return day;
}

void IBPP::Date::Add(int days)
{
	int newdate = mDate + days;		// days can be signed
	if (! IBPP::dtoi(newdate, 0, 0, 0))
		throw LogicExceptionImpl("Date::Add()", _("Out of range"));
	mDate = newdate;
}

void IBPP::Date::StartOfMonth()
{
	int year, month;
	if (! IBPP::dtoi(mDate, &year, &month, 0))
		throw LogicExceptionImpl("Date::StartOfMonth()", _("Out of range"));
	if (! IBPP::itod(&mDate, year, month, 1))		// First of same month
		throw LogicExceptionImpl("Date::StartOfMonth()", _("Out of range"));
}

void IBPP::Date::EndOfMonth()
{
	int year, month;
	if (! IBPP::dtoi(mDate, &year, &month, 0))
		throw LogicExceptionImpl("Date::EndOfMonth()", _("Out of range"));
	if (++month > 12) { month = 1; year++; }
	if (! IBPP::itod(&mDate, year, month, 1))	// First of next month
		throw LogicExceptionImpl("Date::EndOfMonth()", _("Out of range"));
	mDate--;	// Last day of original month, all weird cases accounted for
}

IBPP::Date::Date(int year, int month, int day)
{
	SetDate(year, month, day);
}

IBPP::Date::Date(const IBPP::Date& copied)
{
	mDate = copied.mDate;
}

IBPP::Date& IBPP::Date::operator=(const IBPP::Timestamp& assigned)
{
	mDate = assigned.GetDate();
	return *this;
}

IBPP::Date& IBPP::Date::operator=(const IBPP::Date& assigned)
{
	mDate = assigned.mDate;
	return *this;
}

// The following date calculations were inspired by web pages found on
// Peter Baum web homepage at 'http://www.capecod.net/~pbaum/'.
// His contact info is at : 'http://home.capecod.net/~pbaum/contact.htm'.
// Please, understand that Peter Baum is not related to this IBPP project.
// So __please__, do not contact him regarding IBPP matters.

//	Take a date, in its integer format as used in IBPP internals and splits
//	it in year (4 digits), month (1-12), day (1-31)

bool IBPP::dtoi (int date, int *y, int *m, int *d)
{
    int RataDie, Z, H, A, B, C;
    int year, month, day;

	// Validity control.
	if (date < IBPP::MinDate || date > IBPP::MaxDate)
		return false;

	// The "Rata Die" is the date specified as the number of days elapsed since
	// 31 Dec of year 0. So 1 Jan 0001 is 1.

	RataDie = date + ibpp_internals::consts::Dec31_1899;	// Because IBPP sets the '0' on 31 Dec 1899.

    Z = RataDie + 306;
    H = 100*Z - 25;
    A = H/3652425;
    B = A - A/4;
    year = (100*B + H) / 36525;
    C = B + Z - 365*year - year / 4;
    month = (5*C + 456) / 153;
    day = C - (153*month - 457) / 5;
    if (month > 12) { year += 1; month -= 12; }

	if (y != 0) *y = (int)year;
	if (m != 0) *m = (int)month;
	if (d != 0) *d = (int)day;

	return true;
}

//	Take a date from its components year, month, day and convert it to the
//	integer representation used internally in IBPP.

bool IBPP::itod (int *pdate, int year, int month, int day)
{
    int RataDie, result;
	int y, m, d;

	d = day;	m = month;		y = year;
    if (m < 3) { m += 12; y -= 1; }
    RataDie = d + (153*m - 457) / 5 + 365*y + y/4 - y/100 + y/400 - 306;

	result = RataDie - ibpp_internals::consts::Dec31_1899;   // Because IBPP sets the '0' on 31 Dec 1899

	// Validity control
	if (result < IBPP::MinDate || result > IBPP::MaxDate)
		return false;

	*pdate = result;
	return true;
}

//	Eof
