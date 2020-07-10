// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_TYPES_H_
#define WT_DBO_TYPES_H_

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/weak_ptr.h>
#include <Wt/Dbo/collection.h>
#include <Wt/Dbo/Call.h>
#include <Wt/Dbo/DbAction.h>
#include <Wt/Dbo/Exception.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/Query.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/StdSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>

#define DBO_EXTERN_TEMPLATES(C)						\
  extern template class Wt::Dbo::ptr<C>;				\
  extern template class Wt::Dbo::Dbo<C>;				\
  extern template class Wt::Dbo::MetaDbo<C>;				\
  extern template class Wt::Dbo::collection< Wt::Dbo::ptr<C> >;		\
  extern template class Wt::Dbo::Query< Wt::Dbo::ptr<C>,		\
					Wt::Dbo::DynamicBinding >;	\
  extern template class Wt::Dbo::Query< Wt::Dbo::ptr<C>,		\
					Wt::Dbo::DirectBinding >;	\
  extern template class Wt::Dbo::query_result_traits< Wt::Dbo::ptr<C> >;\
  extern template Wt::Dbo::ptr<C> Wt::Dbo::Session::add<C>(ptr<C>&);	\
  extern template Wt::Dbo::ptr<C> Wt::Dbo::Session::add<C>(std::unique_ptr<C>);	\
  extern template Wt::Dbo::ptr<C> Wt::Dbo::Session::load<C>		\
	(const dbo_traits<C>::IdType&, bool forceReread);		\
  extern template void Wt::Dbo::Session::mapClass<C>(const char *);	\
  extern template struct Wt::Dbo::Session::Mapping<C>;			\
  extern template Wt::Dbo::Query< Wt::Dbo::ptr<C>,			\
				  Wt::Dbo::DynamicBinding>		\
	Wt::Dbo::Session::find<C, Wt::Dbo::DynamicBinding>		\
	(const std::string&);						\
  extern template Wt::Dbo::Query< Wt::Dbo::ptr<C>,			\
				  Wt::Dbo::DirectBinding>		\
	Wt::Dbo::Session::find<C, Wt::Dbo::DirectBinding>		\
	(const std::string&);

#endif // WT_DBO_TYPES_H_
