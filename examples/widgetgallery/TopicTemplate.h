// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TOPIC_TEMPLATE_H_
#define TOPIC_TEMPLATE_H_

#include "Sample.h"
#include "BaseTemplate.h"

class TopicTemplate : public BaseTemplate {
public:
  explicit TopicTemplate(const char *trKey);

  virtual void resolveString(const std::string& varName,
                             const std::vector<Wt::WString>& args,
                             std::ostream& result);

private:
  std::string docUrl(const std::string& type,
                     const std::string& className);
  std::string getString(const std::string& varName);

  static std::string escape(const std::string& name);

  std::map<std::string, std::string> namespaceToPackage;
};

#endif // TOPIC_TEMPLATE_H_
