/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/NotificationListener.h"
#include "Wt/Dbo/Logger.h"

namespace Wt {
  namespace Dbo{

LOGGER("Dbo.NotificationListener");

NotificationListener::NotificationListener()
  : connection_(),
    disconnecting_(false),
    thread_()
{ }

NotificationListener::NotificationListener(std::unique_ptr<Dbo::SqlConnection> connection)
  : NotificationListener()
{
  connect(std::move(connection));
}

NotificationListener::~NotificationListener()
{
  disconnect();
}

void NotificationListener::connect(std::unique_ptr<Dbo::SqlConnection> connection)
{
  if (connection_) {
    disconnect();
  }
  connection_ = std::move(connection);
  if (connection_) {
    connection_->setupNotify();
    for (auto it = callbackMap_.begin(); it != callbackMap_.end(); ++it) {
      connection_->subscribe(it->first);
    }
    thread_ = std::make_unique<std::thread>([=](){
      this->listen();
    });
  }

}

void NotificationListener::disconnect()
{
  if (connection_) {
    LOG_DEBUG("Disconnecting from database");
    disconnecting_ = true;
    connection_->stopListen();
    if (thread_) {
      thread_->join();
    }
    thread_.reset();
    connection_.reset();
    disconnecting_ = false;
  }
}

void NotificationListener::setCallback(const std::string& channel,
                                       const std::function<void(const std::string&, const std::string&)>& callback)
{
  std::unique_lock<std::mutex> l(mapLock_);
  auto result = callbackMap_.emplace(channel, callback);
  if (!result.second) {
    result.first->second = callback;
  }
  l.unlock();
  if (result.second && connection_) {
    LOG_DEBUG("Subscribing to channel " << channel);
    connection_->subscribe(channel);
  }
}

void NotificationListener::removeCallback(const std::string& channel)
{
  std::unique_lock<std::mutex> l(mapLock_);
  auto result = callbackMap_.erase(channel);
  l.unlock();
}

void NotificationListener::listen()
{
  while (!disconnecting_) {
    handleNextNotification();
  }
}

void NotificationListener::handleNextNotification()
{
  std::pair<std::string, std::string> notify = connection_->getNextNotify();
  std::string channel = notify.first;
  std::string message = notify.second;
  std::function<void(const std::string&, const std::string&)> callback = [](const std::string&, const std::string&){}; //does nothing

  std::unique_lock<std::mutex> l(mapLock_);
  auto entry = callbackMap_.find(channel);
  if (entry != callbackMap_.end()) {
    LOG_DEBUG("Received notification on channel " << notify.first << " with message " << notify.second);
    callback = entry->second;
  }
  l.unlock();

  callback(message, channel);
}

void NotificationListener::notify(const std::string& channel, const std::string& message)
{
  if (connection_) {
    connection_->notify(channel, message);
  }
}

  }
}