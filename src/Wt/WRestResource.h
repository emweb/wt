// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2018 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRESTRESOURCE_H_
#define WRESTRESOURCE_H_

#include <Wt/WException.h>
#include <Wt/WResource.h>

#include <Wt/Http/Method.h>

#include <Wt/Signals/signals.hpp>

#include <array>
#include <functional>
#include <string>
#include <type_traits>

namespace Wt {

class WRestResource final : public WResource {
public:
  struct Exception final : Wt::WException {
    Exception(int status);

    int status() { return status_; }
  private:
    int status_;
  };

  struct RestTraits final {
    template<typename ReturnType>
    struct Result final {
      static const char *mimeType();
      static void output(std::ostream &os, ReturnType &&ret);
    };
    template<typename ParamType>
    struct Param final {
      static ParamType parse(const std::string &param);
    };
  };

  WRestResource();
  virtual ~WRestResource() override;

  WRestResource(const WRestResource &) = delete;
  WRestResource& operator=(const WRestResource &) = delete;
  WRestResource(WRestResource&&) = delete;
  WRestResource& operator=(WRestResource&&) = delete;

  virtual void handleRequest(const Http::Request &request,
                             Http::Response &response) override;

  void setHandler(Wt::Http::Method method, std::function<void(const Http::Request &, Http::Response &)> handler);

  template<typename F>
  void setHandler(Wt::Http::Method method, F handler);

private:
  std::array<std::function<void(const Http::Request &, Http::Response &)>, 5> handlers_;

  template<typename ReturnType, typename... Args>
  static std::function<void(const Http::Request &, Http::Response &)> makeFunction(std::function<ReturnType(const Http::Request &,
                                                                                                            Http::Response &,
                                                                                                            Args...)> f);

  template<typename ReturnType, typename... Args>
  struct CallHelper {
  public:
    using F = std::function<ReturnType(const Http::Request &, Http::Response &, Args...)>;
    CallHelper(const Http::Request &req,
               Http::Response &res,
               F &f);

    template<typename... Args2>
    typename std::enable_if<sizeof...(Args) == sizeof...(Args2) && std::is_same<ReturnType, void>::value>::type
    call(Args2&&... args2);

    template<typename... Args2>
    typename std::enable_if<sizeof...(Args) == sizeof...(Args2) && !std::is_same<ReturnType, void>::value>::type
    call(Args2&&... args2);

    template<typename... Args2>
    typename std::enable_if<sizeof...(Args) != sizeof...(Args2)>::type
    call(Args2&&... args2);

  private:
    const Http::Request &req_;
    Http::Response &res_;
    F &f_;
  };

  template<typename ReturnType>
  static void setMimeType(Http::Response &res);
  static void setMimeType(Http::Response &res, const std::string &mimeType);

  template<typename ReturnType>
  static void handleResult(Http::Response &res, ReturnType &&ret);
  template<typename ReturnType>
  static void handleResult(std::ostream &os, ReturnType &&ret);

  // Hiding Wt/Http/Request.h and Wt/Http/Response.h
  static std::ostream &ostream(Http::Response &res);
  static const std::string &param(const Http::Request &req, std::size_t n);

  template<std::size_t N, typename... Args>
  using NthType = typename std::tuple_element<N, std::tuple<Args...>>::type;
};

// Implementation

template<typename F>
void WRestResource::setHandler(Wt::Http::Method method, F handler)
{
  setHandler(method, makeFunction(Signals::Impl::toFunction(std::move(handler))));
}

template<typename ReturnType, typename... Args>
std::function<void(const Http::Request &, Http::Response &)>
WRestResource::makeFunction(std::function<ReturnType(const Http::Request &,
                                                     Http::Response &,
                                                     Args...)> f)
{
  return std::bind([](decltype(f)& f, const Http::Request &req, Http::Response &res) {
    CallHelper<ReturnType, Args...>(req, res, f).call();
  }, std::move(f), std::placeholders::_1, std::placeholders::_2);
}

template<typename ReturnType, typename... Args>
WRestResource::CallHelper<ReturnType, Args...>::CallHelper(const Http::Request &req,
                                                           Http::Response &res,
                                                           F &f)
  : req_(req), res_(res), f_(f)
{ }

template<typename ReturnType, typename... Args>
template<typename... Args2>
typename std::enable_if<sizeof...(Args) == sizeof...(Args2) && std::is_same<ReturnType, void>::value>::type
WRestResource::CallHelper<ReturnType, Args...>::call(Args2&&... args2)
{
  f_(req_, res_, std::forward<Args2>(args2)...);
}

template<typename ReturnType, typename... Args>
template<typename... Args2>
typename std::enable_if<sizeof...(Args) == sizeof...(Args2) && !std::is_same<ReturnType, void>::value>::type
WRestResource::CallHelper<ReturnType, Args...>::call(Args2&&... args2)
{
  setMimeType<ReturnType>(res_);
  handleResult(res_, f_(req_, res_, std::forward<Args2>(args2)...));
}

template<typename ReturnType, typename... Args>
template<typename... Args2>
typename std::enable_if<sizeof...(Args) != sizeof...(Args2)>::type
WRestResource::CallHelper<ReturnType, Args...>::call(Args2&&... args2)
{
  std::size_t n = sizeof...(Args2);
  using T = NthType<sizeof...(Args2), Args...>;
  call(std::forward<Args2>(args2)..., RestTraits::Param<T>::parse(param(req_, n)));
}

template<typename ReturnType>
void WRestResource::setMimeType(Http::Response &res)
{
  setMimeType(res, RestTraits::Result<ReturnType>::mimeType());
}

template<typename ReturnType>
void WRestResource::handleResult(Http::Response &res, ReturnType &&ret)
{
  handleResult(ostream(res), std::forward<ReturnType>(ret));
}

template<typename ReturnType>
void WRestResource::handleResult(std::ostream &os, ReturnType &&ret)
{
  RestTraits::Result<ReturnType>::output(os, std::forward<ReturnType>(ret));
}

// Declare specializations

template <> struct WRestResource::RestTraits::Result<std::string> {
  static const char *mimeType();
  static void output(std::ostream &os, std::string &&ret);
};
template <> struct WRestResource::RestTraits::Result<Wt::WString> {
  static const char *mimeType();
  static void output(std::ostream &os, Wt::WString &&ret);
};

template <>
struct WRestResource::RestTraits::Param<std::string> {
  static std::string parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<Wt::WString> {
  static Wt::WString parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<int> {
  static int parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<unsigned> {
  static unsigned parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<long> {
  static long parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<unsigned long> {
  static unsigned long parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<long long> {
  static long long parse(const std::string &);
};
template <>
struct WRestResource::RestTraits::Param<unsigned long long> {
  static unsigned long long parse(const std::string &);
};

}

#endif // WRESTRESOURCE_H_
