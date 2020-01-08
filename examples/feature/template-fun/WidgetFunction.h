/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WIDGET_FUNCTION_H_
#define WIDGET_FUNCTION_H_

#include <Wt/WWidget.h>
#include <Wt/WString.h>
#include <map>

using namespace Wt;

class WidgetFunction
{
public:
  typedef std::function<std::unique_ptr<WWidget> (const std::vector<WString>&)>
    InstantiateWidget;

  bool operator()(WTemplate *t, const std::vector<WString>& args,
		  std::ostream& result);

  void registerType(const std::string& name, InstantiateWidget instantiate);

private:
  typedef std::map<std::string, InstantiateWidget> RegistryMap;
  RegistryMap registeredTypes_;

  static std::string getArg(const std::string& name,
                            const std::vector<WString>& args);
};

#endif // WIDGET_FUNCTION_H_
