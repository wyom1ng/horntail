#ifndef HORNTAIL_DATABASE_PLUGIN_H
#define HORNTAIL_DATABASE_PLUGIN_H

#include <drogon/plugins/Plugin.h>

class Database : public drogon::Plugin<Database> {
 public:
  void initAndStart(const Json::Value &config) override;

  void shutdown() override;

 private:
  static void deletion_routine(const std::stop_token& stoken, std::chrono::seconds interval);

  std::jthread deletion_thread;
};

#endif
