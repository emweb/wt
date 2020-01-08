// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_IMPL_H_
#define WT_DBO_IMPL_H_

#include <Wt/Dbo/Types.h>

#include <Wt/Dbo/collection_impl.h>
#include <Wt/Dbo/ptr_impl.h>
#include <Wt/Dbo/weak_ptr_impl.h>
#include <Wt/Dbo/Call_impl.h>
#include <Wt/Dbo/DbAction_impl.h>
#include <Wt/Dbo/Query_impl.h>
#include <Wt/Dbo/Field_impl.h>
#include <Wt/Dbo/SqlTraits_impl.h>
#include <Wt/Dbo/Session_impl.h>

#if !defined(_MSC_VER) && !defined(__SUNPRO_C)
#define DBO_INSTANTIATE_TEMPLATES(C)					\
  template class Wt::Dbo::ptr<C>;					\
  template class Wt::Dbo::Dbo<C>;					\
  template class Wt::Dbo::MetaDbo<C>;					\
  template class Wt::Dbo::collection< Wt::Dbo::ptr<C> >;		\
  template class Wt::Dbo::Query< Wt::Dbo::ptr<C>,			\
				 Wt::Dbo::DynamicBinding >;		\
  template class Wt::Dbo::Query< Wt::Dbo::ptr<C>,			\
				 Wt::Dbo::DirectBinding >;		\
  template Wt::Dbo::ptr<C> Wt::Dbo::Session::add<C>(ptr<C>&);		\
  template Wt::Dbo::ptr<C> Wt::Dbo::Session::add<C>(std::unique_ptr<C>);\
  template Wt::Dbo::ptr<C> Wt::Dbo::Session::load<C>			\
	(const dbo_traits<C>::IdType&, bool forceReread);		\
  template void Wt::Dbo::Session::mapClass<C>(const char *);		\
  template struct Wt::Dbo::Session::Mapping<C>;				\
  template Wt::Dbo::Query< Wt::Dbo::ptr<C>, Wt::Dbo::DynamicBinding >	\
	Wt::Dbo::Session::find<C, Wt::Dbo::DynamicBinding>		\
	(const std::string&);						\
  template Wt::Dbo::Query< Wt::Dbo::ptr<C>, Wt::Dbo::DirectBinding >	\
	Wt::Dbo::Session::find<C, Wt::Dbo::DirectBinding>		\
	(const std::string&);
#else
// Functionality is broken on MSVC 2005 and 2008
// Functionality is broken on Sun Studio Express
#define DBO_INSTANTIATE_TEMPLATES(C)
#endif

#endif // WT_DBO_IMPL_H_
