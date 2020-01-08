// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TOPIC_TEMPLATE_H_
#define TOPIC_TEMPLATE_H_

#include "Sample.h"

#include <Wt/WTemplate.h>

class TopicTemplate : public Wt::WTemplate
{
public:
  TopicTemplate(const char *trKey);

  virtual void resolveString(const std::string& varName,
			     const std::vector<Wt::WString>& args,
			     std::ostream& result);

private:
  std::string docUrl(const std::string& className);
  std::string getString(const std::string& varName);
  
  static std::string escape(const std::string& name);
};

#endif // TOPIC_TEMPLATE_H_
