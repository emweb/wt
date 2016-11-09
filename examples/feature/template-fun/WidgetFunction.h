/*
 * Copyright (C) 2014 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WIDGET_FUNCTION_H_
#define WIDGET_FUNCTION_H_

#include <Wt/WWidget>
#include <Wt/WString>
#include <map>

class WidgetFunction
{
public:
  typedef boost::function<Wt::WWidget *(const std::vector<Wt::WString>&)>
    InstantiateWidget;

  bool operator()(Wt::WTemplate *t, const std::vector<Wt::WString>& args,
		  std::ostream& result);

  void registerType(const std::string& name, InstantiateWidget instantiate);

private:
  typedef std::map<std::string, InstantiateWidget> RegistryMap;
  RegistryMap registeredTypes_;

  static std::string getArg(const std::string& name,
			    const std::vector<Wt::WString>& args);
};

#endif // WIDGET_FUNCTION_H_
